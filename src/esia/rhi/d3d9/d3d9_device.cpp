// Esia - Direct3D 9 backend (see esia/rhi/d3d9.hpp), shader model 3 (docs/backends/README.md, "Direct3D 9"):
//   * shaders: the shared HLSL with the SM3 prelude (esia_sm3_prelude.hlsli) compiled at runtime; constants and
//     samplers are bound by name from each shader's constant table (CTAB), the FX shader per feature mask;
//   * instancing: SetStreamSourceFreq over a static stream of corner ids (0..3, indexed strip) and one of instance
//     ids (0..n-1); the full-screen triangle from a static stream of 3 ids; FX instances in a dynamic A32B32G32R32F
//     texture read with vertex texture fetch;
//   * textures: render targets and copy destinations in D3DPOOL_DEFAULT (StretchRect copies and resolves only write
//     render targets; their uploads go through UpdateSurface), everything else dynamic (uploaded with LockRect,
//     read back the same way); RGBA8 is stored as A8R8G8B8
//     (swizzled on upload and readback), R8 as L8;
//   * half-pixel offset: gEsiaHalfPixel per pass target; sRGB targets with D3DRS_SRGBWRITEENABLE;
//   * timestamps: D3DQUERYTYPE_TIMESTAMP / TIMESTAMPDISJOINT / TIMESTAMPFREQ, read a few frames later.
#include "esia/rhi/d3d9.hpp"
#include "d3d_shader.hpp"
#include "esia/core/fx.hpp"
#include "esia/rhi/backend_registry.hpp"
#include <d3d9.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace esia::rhi::d3d9
{
    namespace blobs
    {
        extern const unsigned char esia_sm3_prelude_hlsli[];   // embedded by CMakeLists.txt
    }

    using d3d::ComPtr;
    using d3d::LogLevel;

    namespace
    {
        constexpr int kProfileSlots = 4;
        constexpr int kMaxStamps = 256;
        constexpr UINT kInitialInstanceIds = 16384;
        constexpr std::uint64_t kStagingFrames = 3;          // frames before a kept staging texture is written again
        constexpr std::size_t kMaxStaging = 8;               // staging textures kept
        constexpr std::size_t kMaxStagingBytes = 4u << 20;   // larger uploads (whole images) get a temporary one

        // A defect of the shared FX shader: one feature test uses integer bit operations instead of FX_HAS, which
        // SM3 cannot compile (STATUS.md, core change requests). Replaced before compilation; a no-op once fixed.
        D3DFORMAT D3DFormatOf(Format f)
        {
            switch (f)
            {
            case Format::RGBA8_UNORM:
            case Format::RGBA8_SRGB:
            case Format::BGRA8_UNORM:
            case Format::BGRA8_SRGB: return D3DFMT_A8R8G8B8;   // RGBA8 swizzled: A8B8G8R8 is rarely supported
            case Format::RGB10A2_UNORM: return D3DFMT_A2B10G10R10;
            case Format::RGBA16_FLOAT: return D3DFMT_A16B16G16R16F;
            case Format::RGBA32_FLOAT: return D3DFMT_A32B32G32R32F;
            case Format::R8_UNORM: return D3DFMT_L8;   // reads (l, l, l, 1): the shaders use .r
            case Format::Unknown: break;
            }
            return D3DFMT_UNKNOWN;
        }

        // How a D3D9 format's bytes are laid out, as the rhi::Format with the same layout (for ConvertToRgba8).
        Format LayoutOf(D3DFORMAT f)
        {
            switch (f)
            {
            case D3DFMT_A8R8G8B8:
            case D3DFMT_X8R8G8B8: return Format::BGRA8_UNORM;
            case D3DFMT_A2B10G10R10: return Format::RGB10A2_UNORM;
            case D3DFMT_A16B16G16R16F: return Format::RGBA16_FLOAT;
            case D3DFMT_A32B32G32R32F: return Format::RGBA32_FLOAT;
            case D3DFMT_L8: return Format::R8_UNORM;
            default: return Format::Unknown;
            }
        }

        bool SwizzledRgba(Format f) { return f == Format::RGBA8_UNORM || f == Format::RGBA8_SRGB; }

        // Rows of `data` (the rhi format's bytes) into a locked D3D9 rect.
        void CopyIn(Format f, const void* data, int rowPitch, int w, int h, const D3DLOCKED_RECT& lr)
        {
            const std::size_t row = (std::size_t)w * (std::size_t)BytesPerPixel(f);
            const std::size_t pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            for (int y = 0; y < h; ++y)
            {
                const auto* s = static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y;
                auto* d = static_cast<std::uint8_t*>(lr.pBits) + (std::size_t)lr.Pitch * (std::size_t)y;
                if (!SwizzledRgba(f))
                {
                    std::memcpy(d, s, row);
                    continue;
                }
                for (int x = 0; x < w; ++x)
                {
                    d[x * 4 + 0] = s[x * 4 + 2];
                    d[x * 4 + 1] = s[x * 4 + 1];
                    d[x * 4 + 2] = s[x * 4 + 0];
                    d[x * 4 + 3] = s[x * 4 + 3];
                }
            }
        }

        // ------------------------------------------------------------------ constant tables
        enum Source : std::uint8_t { SrcFrame, SrcPass, SrcDraw, SrcHalfPixel, SrcFxSize, SrcCount };
        constexpr int kSourceRows[SrcCount] = {12, 2, 2, 1, 1};

        struct ConstBind
        {
            Source source;
            int row;
            UINT reg, count;
        };

        struct SamplerBind
        {
            int slot;   // rhi texture slot
            UINT reg;   // sampler register of the stage
        };

        struct ShaderInfo
        {
            std::vector<ConstBind> consts;
            std::vector<SamplerBind> samplers;
        };

        // Maps the constants and samplers of an SM3 shader's CTAB comment to the RHI's binding model by name.
        bool ParseConstantTable(const std::vector<std::uint8_t>& code, ShaderInfo& out, std::string& error)
        {
            static const struct
            {
                const char* name;
                Source source;
                int row;
            } kConstants[] = {
                {"gXform", SrcFrame, 0}, {"gTarget", SrcFrame, 1}, {"gDisplay", SrcFrame, 2}, {"gTime", SrcFrame, 3}, {"gLevel", SrcFrame, 4},
                {"gText", SrcFrame, 10}, {"gConv", SrcFrame, 11}, {"gPass0", SrcPass, 0}, {"gPass1", SrcPass, 1}, {"gFade", SrcDraw, 0},
                {"gDrawInfo", SrcDraw, 1}, {"gEsiaHalfPixel", SrcHalfPixel, 0}, {"gEsiaFxSize", SrcFxSize, 0},
            };
            static const char* kSamplers[kTextureSlots] = {"gTex", "gBackdrop0", "gBackdrop1", "gBackdrop2", "gBackdrop3", "gBackdrop4", "gBackdrop5", "gFxData"};

            const std::size_t words = code.size() / 4;
            auto word = [&](std::size_t i) {
                std::uint32_t w;
                std::memcpy(&w, code.data() + i * 4, 4);
                return w;
            };
            // comment blocks (opcode 0xFFFE, length in DWORDs in bits 16..30) follow the version token
            for (std::size_t i = 1; i < words && (word(i) & 0xFFFFu) == 0xFFFEu;)
            {
                const std::size_t len = (word(i) >> 16) & 0x7FFFu;
                if (len >= 8 && i + 1 + len <= words && word(i + 1) == 0x42415443u)   // 'CTAB'
                {
                    const std::uint8_t* base = code.data() + (i + 2) * 4;
                    const std::size_t size = (len - 1) * 4;
                    auto u32 = [&](std::size_t at) {
                        std::uint32_t v = 0;
                        if (at + 4 <= size)
                            std::memcpy(&v, base + at, 4);
                        return v;
                    };
                    auto u16 = [&](std::size_t at) {
                        std::uint16_t v = 0;
                        if (at + 2 <= size)
                            std::memcpy(&v, base + at, 2);
                        return v;
                    };
                    const std::uint32_t count = u32(12), info = u32(16);
                    for (std::uint32_t c = 0; c < count; ++c)
                    {
                        const std::size_t e = info + (std::size_t)c * 20;
                        const std::uint32_t nameAt = u32(e);
                        if (e + 20 > size || nameAt >= size)
                        {
                            error = "malformed constant table";
                            return false;
                        }
                        const char* name = reinterpret_cast<const char*>(base + nameAt);
                        if (!std::memchr(name, 0, size - nameAt))
                        {
                            error = "malformed constant table";
                            return false;
                        }
                        const std::uint16_t set = u16(e + 4), reg = u16(e + 6), regs = u16(e + 8);
                        bool known = false;
                        if (set == 3)   // D3DXRS_SAMPLER
                        {
                            for (int s = 0; s < kTextureSlots; ++s)
                                if (std::strcmp(name, kSamplers[s]) == 0)
                                {
                                    out.samplers.push_back({s, reg});
                                    known = true;
                                }
                        }
                        else if (set == 2)   // D3DXRS_FLOAT4
                        {
                            for (const auto& k : kConstants)
                                if (std::strcmp(name, k.name) == 0)
                                {
                                    out.consts.push_back({k.source, k.row, reg, std::min<UINT>(regs, (UINT)(kSourceRows[k.source] - k.row))});
                                    known = true;
                                }
                        }
                        if (!known)
                        {
                            error = std::string("unexpected shader constant '") + name + "'";
                            return false;
                        }
                    }
                    return true;
                }
                i += 1 + len;
            }
            error = "no constant table in the bytecode";
            return false;
        }

        // ------------------------------------------------------------------ resources
        struct Tex
        {
            ComPtr<IDirect3DTexture9> tex;       // null for multisampled and texture-less host surfaces
            ComPtr<IDirect3DSurface9> surface;   // level 0 / the render target surface
            TextureDesc desc;
            D3DFORMAT format = D3DFMT_UNKNOWN;
            bool renderTarget = false;           // D3DUSAGE_RENDERTARGET (else D3DUSAGE_DYNAMIC)
            IDirect3DSurface9* hostSurface = nullptr;   // wrapped host target: the key of wrapped_
        };

        struct Buf
        {
            ComPtr<IDirect3DVertexBuffer9> vb;
            ComPtr<IDirect3DIndexBuffer9> ib;
            BufferDesc desc;
            std::size_t valid = 0;
        };

        struct Shader
        {
            ComPtr<IDirect3DVertexShader9> vs;
            ComPtr<IDirect3DPixelShader9> ps;
            ShaderInfo info;
        };

        struct Pipe
        {
            const Shader* vs = nullptr;
            const Shader* ps = nullptr;
            IDirect3DVertexDeclaration9* decl = nullptr;
            PipelineDesc desc;
            // a background FX variant (Caps::asyncPipelines): both stages compile on workers; GetPipelineStatus makes
            // the shaders when they are done (vs / ps stay null until then, or for good when `failed`)
            bool pending = false, failed = false;
            d3d::ShaderRequest vr, pr;
        };

        struct ProfileSlot
        {
            ComPtr<IDirect3DQuery9> disjoint, frequency;
            ComPtr<IDirect3DQuery9> stamps[kMaxStamps];
            int used = 0;
            bool pending = false;
            d3d::ProfileFrame frame;
        };
    }

    class D3D9Device final : public Device
    {
    public:
        explicit D3D9Device(const Desc& d) : dev_(d.device), restore_(d.restoreHostState), log_(d.debug) {}

        bool Init(std::string& error)
        {
            if (!dev_)
            {
                error = "no IDirect3DDevice9";
                return false;
            }
            if (!d3d::ShaderCompilerAvailable(error))
                return false;
            if (FAILED(dev_.As(ex_)))
                log_.Log(LogLevel::Warning, "not an IDirect3DDevice9Ex: D3DPOOL_DEFAULT resources are lost if the device is reset");
            D3DCAPS9 dc = {};
            dev_->GetDeviceCaps(&dc);
            D3DDEVICE_CREATION_PARAMETERS cp = {};
            dev_->GetCreationParameters(&cp);
            dev_->GetDirect3D(&d3d_);
            adapter_ = cp.AdapterOrdinal;
            deviceType_ = cp.DeviceType;
            D3DDISPLAYMODE mode = {};
            d3d_->GetAdapterDisplayMode(adapter_, &mode);
            adapterFormat_ = mode.Format != D3DFMT_UNKNOWN ? mode.Format : D3DFMT_X8R8G8B8;

            if (dc.VertexShaderVersion < D3DVS_VERSION(3, 0) || dc.PixelShaderVersion < D3DPS_VERSION(3, 0))
                error = "the d3d9 backend needs shader model 3 (vs_3_0 / ps_3_0)";
            else if (!(dc.DeclTypes & D3DDTCAPS_UBYTE4N))
                error = "the d3d9 backend needs D3DDECLTYPE_UBYTE4N vertex colors";
            else if (dc.MaxVertexIndex <= 0xFFFF)
                error = "the d3d9 backend needs 32-bit indices";
            else if (!(dc.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND))
                error = "the d3d9 backend needs separate alpha blending";
            else if (dc.MaxStreams < 2)
                error = "the d3d9 backend needs two vertex streams (instancing)";
            else if (!Supports(D3DUSAGE_DYNAMIC | D3DUSAGE_QUERY_VERTEXTEXTURE, D3DFMT_A32B32G32R32F))
                error = "the d3d9 backend needs vertex texture fetch of A32B32G32R32F (the FX instances)";
            if (!error.empty())
                return false;
            // the glass variants of the FX shader take about 2k instruction slots, the full shader 3.8k (512 are guaranteed)
            static bool warned = false;   // once per process: every headless device of the conformance suite asks
            if (dc.MaxPixelShader30InstructionSlots < 4096 && !std::exchange(warned, true))
                log_.Printf(LogLevel::Info, "%lu pixel shader instruction slots: liquid-glass pipelines may fail to build",
                            (unsigned long)dc.MaxPixelShader30InstructionSlots);

            renderTargets_ = std::max<DWORD>(1, dc.NumSimultaneousRTs);
            psSlots_ = dc.MaxPixelShader30InstructionSlots;
            caps_.fxStorage = FxStorage::Texture;
            caps_.shaderFormat = (std::uint8_t)shaders::Format::DxbcSm3;
            caps_.halfPixelOffset = true;
            caps_.floatRenderTargets = Supports(D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING | D3DUSAGE_QUERY_FILTER, D3DFMT_A16B16G16R16F);
            caps_.sampleRenderTarget = true;
            caps_.readback = true;
            caps_.runtimeEffects = true;
            // FX variants: a batch's variant is a fraction of the full shader (fill ~450 instruction slots, all ~3.8k),
            // compiled in the background: the first frames draw with the full shader, started below. Only where the
            // full shader does not fit (the D3D9 test keeps it under 4096): each variant is a shader the driver
            // compiles again at its first draw, every run (~20 in the showcase, 0.85 s of stalled frames on NVIDIA),
            // and a device that takes the full shader runs it as fast
            caps_.fxFeatureVariants = dc.MaxPixelShader30InstructionSlots < 4096;
            caps_.asyncPipelines = true;
            caps_.maxTextureSize = (int)std::min(dc.MaxTextureWidth, dc.MaxTextureHeight);
            // FxFetch computes `instance % perRow` and `instance / perRow` in floats here (SM3 has no integers), and
            // fxc's float modulo is inexact for most divisors (6 % 682 comes out as 5.9999, truncated to the previous
            // instance's texels). With 24 * 2^k texels per row, perRow = 2^k and both are exact.
            int perRow = 1;
            while (perRow * 2 * (int)fx::kInstanceVec4Count <= (int)dc.MaxTextureWidth)
                perRow *= 2;
            caps_.maxFxDataWidth = perRow * (int)fx::kInstanceVec4Count;

            // esia::Vertex; the id streams of the instanced and full-screen draws (TEXCOORD6 / 7, the prelude's)
            const D3DVERTEXELEMENT9 ui[] = {{0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                                            {0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                                            {0, 16, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
                                            D3DDECL_END()};
            const D3DVERTEXELEMENT9 ids[] = {{0, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6},
                                             {1, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7},
                                             D3DDECL_END()};
            const D3DVERTEXELEMENT9 fullscreen[] = {{0, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6}, D3DDECL_END()};
            bool ok = log_.Check(dev_->CreateVertexDeclaration(ui, &uiDecl_), "CreateVertexDeclaration");
            ok = ok && log_.Check(dev_->CreateVertexDeclaration(ids, &fxDecl_), "CreateVertexDeclaration");
            ok = ok && log_.Check(dev_->CreateVertexDeclaration(fullscreen, &fullscreenDecl_), "CreateVertexDeclaration");
            const float corners[4] = {0, 1, 2, 3};
            ok = ok && StaticVertices(corners, sizeof(corners), cornerIds_);
            ok = ok && StaticVertices(corners, 3 * sizeof(float), fullscreenIds_);
            ok = ok && EnsureInstanceIds(kInitialInstanceIds);
            if (ok)
            {
                const std::uint16_t quad[4] = {0, 1, 2, 3};
                void* p = nullptr;
                ok = log_.Check(dev_->CreateIndexBuffer(sizeof(quad), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &quadIndices_, nullptr), "CreateIndexBuffer") &&
                     SUCCEEDED(quadIndices_->Lock(0, 0, &p, 0));
                if (ok)
                {
                    std::memcpy(p, quad, sizeof(quad));
                    quadIndices_->Unlock();
                }
            }
            InitTimestamps();
            // the full FX shader is the fallback of every variant still compiling: start it now, so that the first
            // frame's CreatePipeline finds it done or in flight (the compile cache waits for it, never compiles twice)
            d3d::ShaderRequest full;
            full.program = ShaderProgram::Fx;
            full.model = d3d::ShaderModel::Sm3;
            full.prelude = reinterpret_cast<const char*>(blobs::esia_sm3_prelude_hlsli);
            d3d::Bytecode started;
            for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
            {
                full.stage = s;
                d3d::CompileShaderAsync(full, log_, started);
            }
            if (!ok)
                error = "creating the D3D9 device objects failed";
            return ok;
        }

        const char* Name() const override { return "d3d9"; }
        const Caps& GetCaps() const override { return caps_; }

        // ------------------------------------------------------------------ resources
        Texture CreateTexture(const TextureDesc& desc, const void* data, int rowPitch) override
        {
            const D3DFORMAT fmt = D3DFormatOf(desc.format);
            if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize ||
                fmt == D3DFMT_UNKNOWN || desc.samples < 1)
            {
                log_.Printf(LogLevel::Error, "CreateTexture: unsupported %dx%d %s", desc.width, desc.height, FormatName(desc.format));
                return {};
            }
            Tex t;
            t.desc = desc;
            t.desc.debugName = nullptr;
            t.format = fmt;
            // copy destinations too: StretchRect only writes render targets, and a texture made one at its first copy
            // would lose what was uploaded into it (the renderer creates its backdrop copy zero-filled)
            if (desc.usage & (TextureUsage_RenderTarget | TextureUsage_CopyDst))
            {
                if (desc.samples > 1)
                {
                    // multisampled: a render-target surface (D3D9 textures cannot be multisampled), resolved by copies
                    t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;
                    const D3DMULTISAMPLE_TYPE ms = (D3DMULTISAMPLE_TYPE)desc.samples;
                    if (FAILED(d3d_->CheckDeviceMultiSampleType(adapter_, deviceType_, fmt, TRUE, ms, nullptr)) ||
                        !log_.Check(dev_->CreateRenderTarget((UINT)desc.width, (UINT)desc.height, fmt, ms, 0, FALSE, &t.surface, nullptr), "CreateRenderTarget"))
                    {
                        log_.Printf(LogLevel::Error, "CreateTexture: %d samples of %s are not supported", desc.samples, FormatName(desc.format));
                        return {};
                    }
                    t.renderTarget = true;
                    dev_->ColorFill(t.surface.Get(), nullptr, 0);
                }
                else if (!MakeRenderTarget(t))
                    return {};
            }
            else
            {
                // dynamic: uploads lock it directly, ReadPixels too
                if (!Supports(D3DUSAGE_DYNAMIC, fmt) ||
                    !log_.Check(dev_->CreateTexture((UINT)desc.width, (UINT)desc.height, 1, D3DUSAGE_DYNAMIC, fmt, D3DPOOL_DEFAULT, &t.tex, nullptr),
                                "CreateTexture"))
                    return {};
                t.tex->GetSurfaceLevel(0, &t.surface);
            }
            const std::uint32_t id = textures_.Add(std::move(t));
            if (data)
                UpdateTexture(Texture{id}, IRect{0, 0, desc.width, desc.height}, data, rowPitch);
            return Texture{id};
        }

        Texture Wrap(IDirect3DSurface9* surface, IDirect3DTexture9* texture, bool srgb)
        {
            if (!surface)
                return {};
            auto it = wrapped_.find(surface);
            if (it != wrapped_.end())
                return Texture{it->second};
            D3DSURFACE_DESC sd;
            surface->GetDesc(&sd);
            Tex t;
            switch (sd.Format)
            {
            case D3DFMT_A8R8G8B8:
            case D3DFMT_X8R8G8B8: t.desc.format = srgb ? Format::BGRA8_SRGB : Format::BGRA8_UNORM; break;
            case D3DFMT_A2B10G10R10: t.desc.format = Format::RGB10A2_UNORM; break;
            case D3DFMT_A16B16G16R16F: t.desc.format = Format::RGBA16_FLOAT; break;
            default:
                log_.Printf(LogLevel::Error, "WrapRenderTarget: D3DFORMAT %d is not a format the renderer can target", (int)sd.Format);
                return {};
            }
            t.surface = surface;
            t.tex = texture;
            t.format = sd.Format;
            t.renderTarget = true;
            t.hostSurface = surface;
            t.desc.width = (int)sd.Width;
            t.desc.height = (int)sd.Height;
            // D3DMULTISAMPLE_NONMASKABLE is 1: still multisampled (resolved by copies, never sampled)
            t.desc.samples = sd.MultiSampleType == D3DMULTISAMPLE_NONE ? 1 : std::max(2, (int)sd.MultiSampleType);
            t.desc.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (texture && t.desc.samples == 1 ? TextureUsage_Sampled : 0u);
            const std::uint32_t id = textures_.Add(std::move(t));
            wrapped_[surface] = id;
            return Texture{id};
        }

        void UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch) override
        {
            Tex* t = textures_.Find(tex.id);
            if (!t || !data || r.Empty() || !t->tex)
                return;
            RECT rc = {r.x0, r.y0, r.x1, r.y1};
            D3DLOCKED_RECT lr;
            if (!t->renderTarget)
            {
                const bool whole = r.x0 == 0 && r.y0 == 0 && r.x1 == t->desc.width && r.y1 == t->desc.height;
                if (log_.Check(t->tex->LockRect(0, &lr, whole ? nullptr : &rc, whole ? D3DLOCK_DISCARD : 0u), "LockRect (upload)"))
                {
                    CopyIn(t->desc.format, data, rowPitch, r.Width(), r.Height(), lr);
                    t->tex->UnlockRect(0);
                }
                return;
            }
            // render targets cannot be locked: through system memory
            const ComPtr<IDirect3DSurface9> sys = UploadStaging((UINT)r.Width(), (UINT)r.Height(), t->format, BytesPerPixel(t->desc.format));
            const RECT area = {0, 0, r.Width(), r.Height()};
            if (!sys || FAILED(sys->LockRect(&lr, &area, 0)))
                return;
            CopyIn(t->desc.format, data, rowPitch, r.Width(), r.Height(), lr);
            sys->UnlockRect();
            const POINT at = {r.x0, r.y0};
            log_.Check(dev_->UpdateSurface(sys.Get(), &area, t->surface.Get(), &at), "UpdateSurface");
        }

        void DestroyTexture(Texture tex) override
        {
            if (Tex* t = textures_.Find(tex.id))
                if (t->hostSurface)
                    wrapped_.erase(t->hostSurface);
            textures_.Remove(tex.id);
        }

        TextureDesc GetTextureDesc(Texture tex) const override
        {
            const Tex* t = textures_.Find(tex.id);
            return t ? t->desc : TextureDesc{};
        }

        Buffer CreateBuffer(const BufferDesc& desc) override
        {
            if (desc.size == 0 || desc.size > 0x7FFFFFF0u || desc.kind == BufferKind::FxInstances)
                return {};   // FxStorage::Texture: the FX instances are an A32B32G32R32F texture
            Buf b;
            b.desc = desc;
            const UINT size = (UINT)desc.size;
            const DWORD usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
            const bool ok = desc.kind == BufferKind::Vertex
                                ? log_.Check(dev_->CreateVertexBuffer(size, usage, 0, D3DPOOL_DEFAULT, &b.vb, nullptr), "CreateVertexBuffer")
                                : log_.Check(dev_->CreateIndexBuffer(size, usage, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &b.ib, nullptr), "CreateIndexBuffer");
            return ok ? Buffer{buffers_.Add(std::move(b))} : Buffer{};
        }

        void UpdateBuffer(Buffer buf, const void* data, std::size_t size) override
        {
            Buf* b = buffers_.Find(buf.id);
            if (!b || !data || size == 0 || size > b->desc.size)
                return;
            void* p = nullptr;
            // DISCARD: a new version of the buffer (earlier draws keep theirs); only the bytes written are read
            const HRESULT hr = b->vb ? b->vb->Lock(0, (UINT)size, &p, D3DLOCK_DISCARD) : b->ib->Lock(0, (UINT)size, &p, D3DLOCK_DISCARD);
            if (!log_.Check(hr, "Lock (buffer)"))
                return;
            std::memcpy(p, data, size);
            b->vb ? b->vb->Unlock() : b->ib->Unlock();
            b->valid = size;
        }

        void DestroyBuffer(Buffer buf) override { buffers_.Remove(buf.id); }

        Pipeline CreatePipeline(const PipelineDesc& desc) override
        {
            if (desc.program >= ShaderProgram::Count)
                return {};
            d3d::ShaderRequest vr;
            vr.program = desc.program;
            vr.stage = shaders::Stage::Vertex;
            vr.model = d3d::ShaderModel::Sm3;
            vr.fxFeatures = desc.program == ShaderProgram::Fx ? desc.fxFeatures : 0u;
            vr.prelude = reinterpret_cast<const char*>(blobs::esia_sm3_prelude_hlsli);
            d3d::ShaderRequest pr = vr;
            pr.stage = shaders::Stage::Pixel;
            if (desc.background && desc.program == ShaderProgram::Fx && desc.effect == 0 && desc.fxFeatures != 0)
            {
                // a variant takes up to seconds with D3DCompile on a real driver: the frame draws with a ready one
                Pipe p;
                p.desc = desc;
                p.desc.effectSource = nullptr;
                p.decl = fxDecl_.Get();
                p.pending = true;
                p.vr = vr;
                p.pr = pr;
                d3d::Bytecode started;
                d3d::CompileShaderAsync(vr, log_, started);
                d3d::CompileShaderAsync(pr, log_, started);
                return Pipeline{pipelines_.Add(std::move(p))};
            }
            d3d::Bytecode ps;
            if (desc.effect != 0)
            {
                if (!desc.effectSource)
                    return {};
                pr.effectSource = desc.effectSource;
                if (d3d::CompileShaderAsync(pr, log_, ps) != d3d::CompileState::Ready)
                    return {};   // compiling on a worker (asked again next frame), or rejected (logged)
            }
            else
                ps = d3d::CompileShader(pr, log_);
            const d3d::Bytecode vs = d3d::CompileShader(vr, log_);
            if (!vs || !ps)
                return {};
            Pipe p;
            p.desc = desc;
            p.desc.effectSource = nullptr;
            p.vs = ShaderOf(vs, true);
            p.ps = ShaderOf(ps, false);
            if (!p.vs || !p.ps)
                return {};
            p.decl = desc.layout == VertexLayout::UiVertex ? uiDecl_.Get() : desc.program == ShaderProgram::Fx ? fxDecl_.Get() : fullscreenDecl_.Get();
            return Pipeline{pipelines_.Add(std::move(p))};
        }

        void DestroyPipeline(Pipeline p) override { pipelines_.Remove(p.id); }   // a pending compile ends in the cache

        // The render thread asks: a background pipeline whose stages compiled gets its shader objects here.
        PipelineStatus GetPipelineStatus(Pipeline p) const override
        {
            Pipe* pipe = pipelines_.Find(p.id);
            if (!pipe || pipe->failed)
                return PipelineStatus::Failed;
            if (!pipe->pending)
                return PipelineStatus::Ready;
            d3d::Bytecode vs, ps;
            const d3d::CompileState v = d3d::CompileShaderAsync(pipe->vr, log_, vs);
            const d3d::CompileState f = d3d::CompileShaderAsync(pipe->pr, log_, ps);
            if (v != d3d::CompileState::Failed && f != d3d::CompileState::Failed && (v == d3d::CompileState::Pending || f == d3d::CompileState::Pending))
                return PipelineStatus::Pending;
            pipe->pending = false;
            if (vs && ps)
            {
                pipe->vs = ShaderOf(vs, true);
                pipe->ps = ShaderOf(ps, false);
            }
            pipe->failed = !pipe->vs || !pipe->ps;
            return pipe->failed ? PipelineStatus::Failed : PipelineStatus::Ready;
        }

        // ------------------------------------------------------------------ frame
        bool BeginFrame(const FrameDesc&) override
        {
            ++frame_;
            if (restore_)
            {
                log_.Check(dev_->CreateStateBlock(D3DSBT_ALL, &hostState_), "CreateStateBlock");
                dev_->GetRenderTarget(0, &hostTarget_);
                if (FAILED(dev_->GetDepthStencilSurface(&hostDepth_)))
                    hostDepth_ = nullptr;
            }
            if (caps_.timestampQueries)
            {
                ProfileSlot& s = profile_[frame_ % kProfileSlots];
                if (s.pending)
                    ReadSlot(s);   // kProfileSlots frames behind: the old numbers are dropped if not ready
                s.used = 0;
                s.pending = false;
                s.frame.Reset(frame_);
                s.disjoint->Issue(D3DISSUE_BEGIN);
                s.frame.frameStart = Stamp();
            }
            return true;
        }

        void EndFrame() override
        {
            if (caps_.timestampQueries)
            {
                ProfileSlot& s = profile_[frame_ % kProfileSlots];
                s.frame.frameEnd = Stamp();
                s.disjoint->Issue(D3DISSUE_END);
                s.frequency->Issue(D3DISSUE_END);
                s.pending = s.frame.frameEnd > 0;
            }
            for (DWORD i = 0; i < 16; ++i)
                dev_->SetTexture(i, nullptr);
            for (DWORD i = 0; i < 4; ++i)
                dev_->SetTexture(D3DVERTEXTEXTURESAMPLER0 + i, nullptr);
            if (restore_)
            {
                // the render target first: setting it resets the viewport, which the state block then restores
                if (hostTarget_)
                    dev_->SetRenderTarget(0, hostTarget_.Get());
                dev_->SetDepthStencilSurface(hostDepth_.Get());
                if (hostState_)
                    hostState_->Apply();
                hostState_ = nullptr;
                hostTarget_ = nullptr;
                hostDepth_ = nullptr;
            }
        }

        // ------------------------------------------------------------------ commands
        void BeginPass(const PassDesc& desc) override
        {
            Tex* t = textures_.Find(desc.target.id);
            if (!t || !t->renderTarget)
                return;
            // no texture of the last pass stays bound (it may be this pass's target)
            for (DWORD i = 0; i < kTextureSlots; ++i)
                dev_->SetTexture(i, nullptr);
            dev_->SetTexture(D3DVERTEXTEXTURESAMPLER0, nullptr);
            dev_->SetRenderTarget(0, t->surface.Get());   // also sets the viewport and scissor to the whole target
            for (DWORD i = 1; i < renderTargets_; ++i)
                dev_->SetRenderTarget(i, nullptr);
            dev_->SetDepthStencilSurface(nullptr);
            pass_ = t;
            ApplyPassState();
            dev_->BeginScene();
            inScene_ = true;
            if (desc.load == LoadOp::Clear)
            {
                // stored values (rhi.hpp): no sRGB encoding and no scissor while clearing
                auto c8 = [&](int i) { return (DWORD)std::lround(std::clamp(desc.clearColor[i], 0.0f, 1.0f) * 255.0f); };
                dev_->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
                dev_->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
                dev_->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(c8(3), c8(0), c8(1), c8(2)), 1.0f, 0);
                dev_->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
                dev_->SetRenderState(D3DRS_SRGBWRITEENABLE, IsSrgb(t->desc.format) ? TRUE : FALSE);
            }
        }

        void EndPass() override
        {
            if (inScene_)
                dev_->EndScene();
            inScene_ = false;
            pass_ = nullptr;
        }

        void SetPipeline(Pipeline p) override
        {
            const Pipe* pipe = pipelines_.Find(p.id);
            if (!pipe || !pipe->vs || !pipe->ps)
            {
                log_.Log(LogLevel::Error, "SetPipeline: a pipeline that is not Ready (GetPipelineStatus)");
                return;
            }
            if (hostTouched_)
                ApplyPassState();
            pipe_ = pipe;
            dev_->SetVertexShader(pipe->vs->vs.Get());
            dev_->SetPixelShader(pipe->ps->ps.Get());
            dev_->SetVertexDeclaration(pipe->decl);
            switch (pipe->desc.blend)
            {
            case BlendMode::Opaque: dev_->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); break;
            case BlendMode::Straight:
            case BlendMode::Premultiplied:
                dev_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                dev_->SetRenderState(D3DRS_SRCBLEND, pipe->desc.blend == BlendMode::Straight ? D3DBLEND_SRCALPHA : D3DBLEND_ONE);
                dev_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                dev_->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
                dev_->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
                break;
            }
            for (bool& d : dirty_)
                d = true;
            samplersDirty_ = true;
        }

        void SetScissor(const IRect& r) override
        {
            const RECT s = {r.x0, r.y0, r.x1, r.y1};
            dev_->SetScissorRect(&s);
        }

        void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) override
        {
            if (!data)
                return;
            float* dst = slot == ConstantSlot::Frame ? frameConstants_ : slot == ConstantSlot::Pass ? passConstants_ : drawConstants_;
            std::memcpy(dst, data, std::min<std::uint32_t>(size, (std::uint32_t)kSourceRows[(int)slot] * 16u));
            dirty_[(int)slot] = true;
        }

        void SetTexture(int slot, Texture tex) override
        {
            if (slot < 0 || slot >= kTextureSlots)
                return;
            bound_[slot] = tex.id;
            samplersDirty_ = true;
            if (slot == kSlotFxData)
                if (const Tex* t = textures_.Find(tex.id))
                {
                    const float w = (float)t->desc.width, h = (float)t->desc.height;
                    const float size[4] = {w, h, 1.0f / w, 1.0f / h};
                    std::memcpy(fxSize_, size, sizeof(size));
                    dirty_[SrcFxSize] = true;
                }
        }

        void SetFxBuffer(Buffer) override {}   // FxStorage::Texture: SetTexture(kSlotFxData, ...)

        void SetVertexBuffer(Buffer buf) override { vertexBuffer_ = buf.id; }
        void SetIndexBuffer(Buffer buf) override { indexBuffer_ = buf.id; }

        void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override
        {
            if (!PrepareDraw())
                return;
            dev_->SetStreamSource(0, fullscreenIds_.Get(), 0, sizeof(float));
            dev_->DrawPrimitive(D3DPT_TRIANGLELIST, firstVertex, vertexCount / 3);
        }

        void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) override
        {
            const Buf* vb = buffers_.Find(vertexBuffer_);
            const Buf* ib = buffers_.Find(indexBuffer_);
            if (!vb || !ib || !vb->vb || !ib->ib || !PrepareDraw())
                return;
            dev_->SetStreamSource(0, vb->vb.Get(), 0, 20);   // sizeof(esia::Vertex)
            dev_->SetIndices(ib->ib.Get());
            dev_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, (UINT)(vb->valid / 20), firstIndex, indexCount / 3);
        }

        void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) override
        {
            if (vertexCount != 4 || instanceCount == 0 || !EnsureInstanceIds(instanceCount) || !PrepareDraw())
                return;
            // hardware instancing: the 4 corners (indexed strip) repeated for every instance of stream 1
            dev_->SetStreamSource(0, cornerIds_.Get(), 0, sizeof(float));
            dev_->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | instanceCount);
            dev_->SetStreamSource(1, instanceIds_.Get(), 0, sizeof(float));
            dev_->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1u);
            dev_->SetIndices(quadIndices_.Get());
            dev_->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, 4, 0, 2);
            dev_->SetStreamSourceFreq(0, 1);
            dev_->SetStreamSourceFreq(1, 1);
        }

        void CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r) override
        {
            Tex* d = textures_.Find(dst.id);
            Tex* s = textures_.Find(src.id);
            if (!d || !s || r.Empty())
                return;
            if (!d->renderTarget)
            {
                log_.Log(LogLevel::Error, "CopyTexture: the destination was not created with TextureUsage_CopyDst");
                return;
            }
            const RECT sr = {r.x0, r.y0, r.x1, r.y1};
            const RECT dr = {dstX, dstY, dstX + r.Width(), dstY + r.Height()};
            log_.Check(dev_->StretchRect(s->surface.Get(), &sr, d->surface.Get(), &dr, D3DTEXF_NONE), "StretchRect");
        }

        void* NativeRenderState() override
        {
            hostTouched_ = true;   // the host may change anything: the pass state is applied again at the next pipeline
            return dev_.Get();
        }

        // ------------------------------------------------------------------ profiling
        void BeginProfile(ProfileCategory category) override
        {
            if (!caps_.timestampQueries)
                return;
            d3d::ProfileFrame& f = profile_[frame_ % kProfileSlots].frame;
            f.openStart = Stamp();
            f.openCategory = category;
        }

        void EndProfile() override
        {
            if (!caps_.timestampQueries)
                return;
            d3d::ProfileFrame& f = profile_[frame_ % kProfileSlots].frame;
            const int end = Stamp();
            if (f.openStart >= 0 && end >= 0)
                f.intervals.push_back({f.openStart, end, f.openCategory});
            f.openStart = -1;
        }

        bool ReadProfile(GpuProfile& out) override
        {
            for (int i = 1; i <= kProfileSlots; ++i)
            {
                ProfileSlot& s = profile_[(frame_ + (std::uint64_t)i) % kProfileSlots];
                if (s.pending)
                    ReadSlot(s);
            }
            out = latest_;
            return latest_.valid;
        }

        // ------------------------------------------------------------------ readback
        std::uint32_t ValidationErrors() const override { return log_.Problems(); }

        bool ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& rgba8) override
        {
            rgba8.clear();
            Tex* t = textures_.Find(tex.id);
            if (!t || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height)
                return false;
            const Format layout = LayoutOf(t->format);
            RECT rc = {r.x0, r.y0, r.x1, r.y1};
            D3DLOCKED_RECT lr;
            if (!t->renderTarget)
            {
                if (!log_.Check(t->tex->LockRect(0, &lr, &rc, D3DLOCK_READONLY), "LockRect (readback)"))
                    return false;
                d3d::ConvertToRgba8(layout, lr.pBits, (std::size_t)lr.Pitch, r.Width(), r.Height(), rgba8);
                t->tex->UnlockRect(0);
                return true;
            }
            ComPtr<IDirect3DSurface9> src = t->surface;
            if (t->desc.samples > 1)
            {
                // resolve into a plain render target first (GetRenderTargetData wants one sample)
                ComPtr<IDirect3DSurface9> resolved;
                if (!log_.Check(dev_->CreateRenderTarget((UINT)t->desc.width, (UINT)t->desc.height, t->format, D3DMULTISAMPLE_NONE, 0, FALSE, &resolved, nullptr),
                                "CreateRenderTarget (resolve)") ||
                    !log_.Check(dev_->StretchRect(src.Get(), nullptr, resolved.Get(), nullptr, D3DTEXF_NONE), "StretchRect (resolve)"))
                    return false;
                src = resolved;
            }
            ComPtr<IDirect3DTexture9> staging;
            ComPtr<IDirect3DSurface9> sys;
            if (!SystemMemorySurface((UINT)t->desc.width, (UINT)t->desc.height, t->format, "CreateTexture (readback)", staging, sys) ||
                !log_.Check(dev_->GetRenderTargetData(src.Get(), sys.Get()), "GetRenderTargetData") ||
                !log_.Check(sys->LockRect(&lr, &rc, D3DLOCK_READONLY), "LockRect (readback)"))
                return false;
            d3d::ConvertToRgba8(layout, lr.pBits, (std::size_t)lr.Pitch, r.Width(), r.Height(), rgba8);
            sys->UnlockRect();
            if (t->format == D3DFMT_X8R8G8B8)   // the X byte is undefined: opaque
                for (std::size_t i = 3; i < rgba8.size(); i += 4)
                    rgba8[i] = 255;
            return true;
        }

    private:
        bool Supports(DWORD usage, D3DFORMAT fmt) const
        {
            return SUCCEEDED(d3d_->CheckDeviceFormat(adapter_, deviceType_, adapterFormat_, usage, D3DRTYPE_TEXTURE, fmt));
        }

        // A system-memory image of a render target's format to go through (uploads, readback): a level of a
        // system-memory texture, which every texture format has. Offscreen plain surfaces do not: NVIDIA's driver has
        // no L8 one, the glyph atlas's format, and accepts L8 render targets.
        bool SystemMemorySurface(UINT width, UINT height, D3DFORMAT fmt, const char* what, ComPtr<IDirect3DTexture9>& tex, ComPtr<IDirect3DSurface9>& surface)
        {
            return log_.Check(dev_->CreateTexture(width, height, 1, 0, fmt, D3DPOOL_SYSTEMMEM, &tex, nullptr), what) &&
                   log_.Check(tex->GetSurfaceLevel(0, &surface), what);
        }

        // A system-memory surface for an upload of width x height texels to a render target: a texture kept from an
        // earlier upload whose copy is kStagingFrames frames old (writing it then does not wait for that copy), else a
        // new one with power-of-two sides, kept in place of the least recently used of kMaxStaging. The FX instance
        // texture is uploaded every frame: a new texture each time (its pages written for the first time) took 25 us
        // an upload on the RTX 4080, a kept one 3 us.
        ComPtr<IDirect3DSurface9> UploadStaging(UINT width, UINT height, D3DFORMAT fmt, int bytesPerTexel)
        {
            Staging* pick = nullptr;
            for (Staging& s : staging_)
                if (s.format == fmt && s.width >= width && s.height >= height && s.used + kStagingFrames <= frame_ &&
                    (!pick || s.width * s.height < pick->width * pick->height))
                    pick = &s;
            if (!pick)
            {
                Staging s;
                s.format = fmt;
                s.width = s.height = 1;
                while (s.width < width)
                    s.width *= 2;
                while (s.height < height)
                    s.height *= 2;
                const bool keep = (std::size_t)s.width * s.height * (std::size_t)bytesPerTexel <= kMaxStagingBytes;
                if (!keep)
                {
                    s.width = width;
                    s.height = height;
                }
                if (!SystemMemorySurface(s.width, s.height, fmt, "CreateTexture (upload)", s.tex, s.surface))
                    return {};
                if (!keep)
                    return s.surface;
                if (staging_.size() >= kMaxStaging)
                {
                    auto oldest = std::min_element(staging_.begin(), staging_.end(), [](const Staging& a, const Staging& b) { return a.used < b.used; });
                    *oldest = std::move(s);
                    pick = &*oldest;
                }
                else
                    pick = &staging_.emplace_back(std::move(s));
            }
            pick->used = frame_;
            return pick->surface;
        }

        // Creates `t` as a render-target texture cleared to transparent black.
        bool MakeRenderTarget(Tex& t)
        {
            ComPtr<IDirect3DTexture9> tex;
            ComPtr<IDirect3DSurface9> surface;
            if (!Supports(D3DUSAGE_RENDERTARGET, t.format) ||
                !log_.Check(dev_->CreateTexture((UINT)t.desc.width, (UINT)t.desc.height, 1, D3DUSAGE_RENDERTARGET, t.format, D3DPOOL_DEFAULT, &tex, nullptr),
                            "CreateTexture (render target)"))
            {
                log_.Printf(LogLevel::Error, "%s cannot be a render target here", FormatName(t.desc.format));
                return false;
            }
            tex->GetSurfaceLevel(0, &surface);
            dev_->ColorFill(surface.Get(), nullptr, 0);   // a Load pass over fresh contents is deterministic
            t.tex = tex;
            t.surface = surface;
            t.renderTarget = true;
            return true;
        }

        bool StaticVertices(const void* data, UINT size, ComPtr<IDirect3DVertexBuffer9>& out)
        {
            void* p = nullptr;
            if (!log_.Check(dev_->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &out, nullptr), "CreateVertexBuffer") ||
                FAILED(out->Lock(0, 0, &p, 0)))
                return false;
            std::memcpy(p, data, size);
            out->Unlock();
            return true;
        }

        // The instance-id stream (0, 1, 2 ... as floats) holds at least `count` ids.
        bool EnsureInstanceIds(UINT count)
        {
            if (instanceIdCount_ >= count)
                return true;
            UINT n = std::max(instanceIdCount_ * 2, kInitialInstanceIds);
            while (n < count)
                n *= 2;
            std::vector<float> ids(n);
            for (UINT i = 0; i < n; ++i)
                ids[i] = (float)i;
            ComPtr<IDirect3DVertexBuffer9> vb;
            if (!StaticVertices(ids.data(), n * (UINT)sizeof(float), vb))
                return false;
            instanceIds_ = vb;
            instanceIdCount_ = n;
            return true;
        }

        const Shader* ShaderOf(const d3d::Bytecode& code, bool vertex) const
        {
            auto it = shaders_.find(code.get());
            if (it != shaders_.end())
                return &it->second;
            Shader s;
            std::string error;
            const auto* words = reinterpret_cast<const DWORD*>(code->data());
            bool created = false;
            if (vertex)
                created = log_.Check(dev_->CreateVertexShader(words, &s.vs), "CreateVertexShader");
            else
            {
                const HRESULT hr = dev_->CreatePixelShader(words, &s.ps);
                created = SUCCEEDED(hr);
                // more instruction slots than the device has is a limit of the device, not an error (NVIDIA's driver
                // has 4096; the full FX shader takes about 3.8k, a large user effect may take more). The pipeline is
                // not built.
                const unsigned slots = created ? 0u : d3d::InstructionSlots(code);
                if (!created && slots > psSlots_)
                    log_.Printf(LogLevel::Info, "a pixel shader of about %u instruction slots exceeds the device's %lu: its pipeline is not built",
                                slots, (unsigned long)psSlots_);
                else if (!created)
                    log_.Check(hr, "CreatePixelShader");
            }
            if (!created)
                return nullptr;
            if (!ParseConstantTable(*code, s.info, error))
            {
                log_.Printf(LogLevel::Error, "SM3 bytecode: %s", error.c_str());
                return nullptr;
            }
            return &shaders_.emplace(code.get(), std::move(s)).first->second;
        }

        // Everything a pass relies on (and host callbacks may change).
        void ApplyPassState()
        {
            const bool touched = std::exchange(hostTouched_, false);
            pipe_ = nullptr;
            if (!pass_)
                return;
            if (touched)
            {
                dev_->SetRenderTarget(0, pass_->surface.Get());
                dev_->SetDepthStencilSurface(nullptr);
            }
            dev_->SetStreamSourceFreq(0, 1);
            dev_->SetStreamSourceFreq(1, 1);
            const D3DVIEWPORT9 vp = {0, 0, (DWORD)pass_->desc.width, (DWORD)pass_->desc.height, 0.0f, 1.0f};
            dev_->SetViewport(&vp);
            const RECT all = {0, 0, pass_->desc.width, pass_->desc.height};
            dev_->SetScissorRect(&all);
            static const struct
            {
                D3DRENDERSTATETYPE state;
                DWORD value;
            } kStates[] = {
                {D3DRS_ZENABLE, D3DZB_FALSE}, {D3DRS_ZWRITEENABLE, FALSE}, {D3DRS_STENCILENABLE, FALSE}, {D3DRS_ALPHATESTENABLE, FALSE},
                {D3DRS_CULLMODE, D3DCULL_NONE}, {D3DRS_FILLMODE, D3DFILL_SOLID}, {D3DRS_FOGENABLE, FALSE}, {D3DRS_LIGHTING, FALSE},
                {D3DRS_CLIPPING, TRUE}, {D3DRS_CLIPPLANEENABLE, 0}, {D3DRS_SCISSORTESTENABLE, TRUE}, {D3DRS_COLORWRITEENABLE, 0xF},
                {D3DRS_MULTISAMPLEANTIALIAS, TRUE}, {D3DRS_MULTISAMPLEMASK, 0xFFFFFFFFu}, {D3DRS_BLENDOP, D3DBLENDOP_ADD},
                {D3DRS_SEPARATEALPHABLENDENABLE, TRUE}, {D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD}, {D3DRS_ALPHABLENDENABLE, FALSE},
                {D3DRS_SRGBWRITEENABLE, FALSE},
            };
            for (const auto& s : kStates)
                dev_->SetRenderState(s.state, s.value);
            dev_->SetRenderState(D3DRS_SRGBWRITEENABLE, IsSrgb(pass_->desc.format) ? TRUE : FALSE);
            // pixel centers on integer coordinates: every vertex moves half a pixel of this target (the prelude)
            const float half[4] = {-1.0f / (float)pass_->desc.width, 1.0f / (float)pass_->desc.height, 0.0f, 0.0f};
            std::memcpy(halfPixel_, half, sizeof(half));
            for (bool& d : dirty_)
                d = true;
            for (std::uint32_t& b : bound_)
                b = 0;
            samplersDirty_ = true;
        }

        // Constants and samplers of the current pipeline, uploaded where they changed.
        bool PrepareDraw()
        {
            if (!pipe_)
                return false;
            const float* sources[SrcCount] = {frameConstants_, passConstants_, drawConstants_, halfPixel_, fxSize_};
            for (const ConstBind& c : pipe_->vs->info.consts)
                if (dirty_[c.source])
                    dev_->SetVertexShaderConstantF(c.reg, sources[c.source] + c.row * 4, c.count);
            for (const ConstBind& c : pipe_->ps->info.consts)
                if (dirty_[c.source])
                    dev_->SetPixelShaderConstantF(c.reg, sources[c.source] + c.row * 4, c.count);
            for (bool& d : dirty_)
                d = false;
            if (samplersDirty_)
            {
                samplersDirty_ = false;
                for (const SamplerBind& s : pipe_->ps->info.samplers)
                    BindSampler(s.reg, s.slot);
                for (const SamplerBind& s : pipe_->vs->info.samplers)
                    BindSampler(D3DVERTEXTEXTURESAMPLER0 + s.reg, s.slot);
            }
            return true;
        }

        void BindSampler(DWORD stage, int slot)
        {
            const Tex* t = textures_.Find(bound_[slot]);
            dev_->SetTexture(stage, t ? t->tex.Get() : nullptr);
            // s1 (point) where the shaders read gPoint: the FX instances and LayerComposite's sharp layer; else s0
            const bool point = slot == kSlotFxData || (slot == kSlotTexture && pipe_->desc.program == ShaderProgram::LayerComposite);
            const DWORD filter = point ? D3DTEXF_POINT : D3DTEXF_LINEAR;
            dev_->SetSamplerState(stage, D3DSAMP_MINFILTER, filter);
            dev_->SetSamplerState(stage, D3DSAMP_MAGFILTER, filter);
            dev_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
            dev_->SetSamplerState(stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            dev_->SetSamplerState(stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            dev_->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, FALSE);   // sampling never decodes (rhi.hpp)
            dev_->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, 0);
        }

        // ------------------------------------------------------------------ timestamps
        void InitTimestamps()
        {
            caps_.timestampQueries = SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMP, nullptr)) &&
                                     SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, nullptr)) &&
                                     SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, nullptr));
            for (ProfileSlot& s : profile_)
            {
                bool ok = caps_.timestampQueries && SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &s.disjoint)) &&
                          SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &s.frequency));
                for (ComPtr<IDirect3DQuery9>& q : s.stamps)
                    ok = ok && SUCCEEDED(dev_->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &q));
                caps_.timestampQueries = ok;
            }
        }

        int Stamp()
        {
            ProfileSlot& s = profile_[frame_ % kProfileSlots];
            if (s.used >= kMaxStamps)
                return -1;
            s.stamps[s.used]->Issue(D3DISSUE_END);
            return s.used++;
        }

        void ReadSlot(ProfileSlot& s)
        {
            // D3DGETDATA_FLUSH submits what is queued, so the results arrive even when nothing presents
            const DWORD f = D3DGETDATA_FLUSH;
            BOOL disjoint = TRUE;
            UINT64 frequency = 0;
            std::uint64_t ticks[kMaxStamps];
            if (s.disjoint->GetData(&disjoint, sizeof(disjoint), f) != S_OK || s.frequency->GetData(&frequency, sizeof(frequency), f) != S_OK)
                return;
            for (int i = 0; i < s.used; ++i)
                if (s.stamps[i]->GetData(&ticks[i], sizeof(UINT64), f) != S_OK)
                    return;
            s.pending = false;
            if (disjoint || frequency == 0)
                return;
            const GpuProfile p = s.frame.Resolve(ticks, s.used, (double)frequency);
            if (p.valid && p.frame > latest_.frame)
                latest_ = p;
        }

        ComPtr<IDirect3DDevice9> dev_;
        ComPtr<IDirect3DDevice9Ex> ex_;
        ComPtr<IDirect3D9> d3d_;
        UINT adapter_ = 0;
        D3DDEVTYPE deviceType_ = D3DDEVTYPE_HAL;
        D3DFORMAT adapterFormat_ = D3DFMT_X8R8G8B8;
        DWORD renderTargets_ = 1;   // D3DCAPS9::NumSimultaneousRTs
        DWORD psSlots_ = 512;       // D3DCAPS9::MaxPixelShader30InstructionSlots
        bool restore_ = true;
        d3d::Logger log_;
        Caps caps_;

        d3d::HandleTable<Tex> textures_;
        d3d::HandleTable<Buf> buffers_;
        mutable d3d::HandleTable<Pipe> pipelines_;   // mutable: GetPipelineStatus completes background builds
        std::unordered_map<IDirect3DSurface9*, std::uint32_t> wrapped_;
        // shader objects and their constant tables per bytecode (the process-wide cache keeps the keys alive)
        mutable std::unordered_map<const std::vector<std::uint8_t>*, Shader> shaders_;
        ComPtr<IDirect3DVertexDeclaration9> uiDecl_, fxDecl_, fullscreenDecl_;
        ComPtr<IDirect3DVertexBuffer9> cornerIds_, fullscreenIds_, instanceIds_;
        UINT instanceIdCount_ = 0;
        ComPtr<IDirect3DIndexBuffer9> quadIndices_;

        struct Staging
        {
            D3DFORMAT format = D3DFMT_UNKNOWN;
            UINT width = 0, height = 0;
            ComPtr<IDirect3DTexture9> tex;
            ComPtr<IDirect3DSurface9> surface;
            std::uint64_t used = 0;   // frame of its last upload
        };
        std::vector<Staging> staging_;   // UploadStaging

        ComPtr<IDirect3DStateBlock9> hostState_;
        ComPtr<IDirect3DSurface9> hostTarget_, hostDepth_;

        // this frame
        std::uint64_t frame_ = 0;
        Tex* pass_ = nullptr;
        const Pipe* pipe_ = nullptr;
        bool inScene_ = false, hostTouched_ = false, samplersDirty_ = true;
        float frameConstants_[48] = {}, passConstants_[8] = {}, drawConstants_[8] = {}, halfPixel_[4] = {}, fxSize_[4] = {1, 1, 1, 1};
        bool dirty_[SrcCount] = {};
        std::uint32_t bound_[kTextureSlots] = {};
        std::uint32_t vertexBuffer_ = 0, indexBuffer_ = 0;
        ProfileSlot profile_[kProfileSlots];
        GpuProfile latest_;
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        auto dev = std::make_unique<D3D9Device>(desc);
        std::string e;
        if (!dev->Init(e))
        {
            if (error)
                *error = e;
            return nullptr;
        }
        return dev;
    }

    Texture WrapRenderTarget(Device& device, IDirect3DSurface9* surface, IDirect3DTexture9* texture, bool srgb)
    {
        auto* d = dynamic_cast<D3D9Device*>(&device);
        return d ? d->Wrap(surface, texture, srgb) : Texture{};
    }

    // ------------------------------------------------------------------ headless (conformance suite)
    namespace
    {
        // Direct3D 9 needs a window to create a device, even one that never presents: one hidden window per process.
        HWND HiddenWindow()
        {
            static HWND hwnd = [] {
                WNDCLASSW wc = {};
                wc.lpfnWndProc = DefWindowProcW;
                wc.hInstance = GetModuleHandleW(nullptr);
                wc.lpszClassName = L"EsiaD3D9Headless";
                RegisterClassW(&wc);
                return CreateWindowW(L"EsiaD3D9Headless", L"esia d3d9", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
            }();
            return hwnd;
        }

        HeadlessDevice CreateHeadless(const HeadlessDesc& hd, std::string& error)
        {
            const HWND hwnd = HiddenWindow();
            if (!hwnd)
            {
                error = "no window for the D3D9 device";
                return {};
            }
            D3DPRESENT_PARAMETERS pp = {};
            pp.BackBufferWidth = pp.BackBufferHeight = 1;
            pp.BackBufferFormat = D3DFMT_UNKNOWN;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.hDeviceWindow = hwnd;
            pp.Windowed = TRUE;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            const DWORD flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_NOWINDOWCHANGES;
            ComPtr<IDirect3DDevice9> device;
            ComPtr<IDirect3D9Ex> d3dEx;
            HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3dEx);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDirect3DDevice9Ex> ex;
                hr = d3dEx->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, flags, &pp, nullptr, &ex);
                device = ex.Get();
            }
            if (FAILED(hr))
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "CreateDeviceEx failed: 0x%08lX (%s)", (unsigned long)hr, d3d::HResultName(hr));
                error = buf;
                return {};
            }
            Desc d;
            d.device = device.Get();
            d.restoreHostState = false;
            d.debug.debugLayer = d3d::DebugLevelFromEnvironment() > 0;
            HeadlessDevice h;
            h.device = CreateDevice(d, &error);
            if (!h.device)
                return {};
            // D3D9 only lists adapters with a display: the default one is the GPU of the main display
            D3DADAPTER_IDENTIFIER9 id = {};
            if (SUCCEEDED(d3dEx->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id)))
                h.adapter = id.Description;
            TextureDesc td;
            td.width = hd.width;
            td.height = hd.height;
            td.format = hd.format;
            td.samples = hd.samples;
            td.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (hd.sampleable && hd.samples == 1 ? TextureUsage_Sampled : 0u);
            td.debugName = "headless-target";
            h.target = h.device->CreateTexture(td, nullptr, 0);
            if (!h.target)
            {
                error = std::string("the device cannot render into ") + FormatName(hd.format) + (hd.samples > 1 ? " with MSAA" : "");
                return {};
            }
            return h;
        }
    }
}

void EsiaRegisterBackend_d3d9()
{
    esia::rhi::BackendInfo info;
    info.name = "d3d9";
    info.createHeadless = &esia::rhi::d3d9::CreateHeadless;
    esia::rhi::RegisterBackend(info);
}
