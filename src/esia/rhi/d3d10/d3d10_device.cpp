// Esia - Direct3D 10 backend (see esia/rhi/d3d10.hpp): the D3D11 backend's techniques on Direct3D 10 / 10.1
// (one device object, no context) with shader model 4: dynamic buffers written with MAP_WRITE_DISCARD, typeless
// storage so the backdrop copy and raw views of sRGB targets share bits, ResolveSubresource for multisampled
// targets, the target's own SRV for the direct read, timestamp queries read a few frames later, the host's state
// saved and restored around the frame. FX instances come from an RGBA32F texture at t7, read by both stages (SM4
// has no structured buffers: esia_fx.hlsl with ESIA_FX_STORAGE_TEXTURE).
#include "esia/rhi/d3d10.hpp"
#include "d3d_shader.hpp"
#include "esia/rhi/backend_registry.hpp"
#include <d3d10_1.h>
#include <d3d10sdklayers.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esia::rhi::d3d10
{
    using d3d::ComPtr;
    using d3d::LogLevel;

    namespace
    {
        constexpr int kProfileSlots = 4;     // frames in flight of timestamp queries
        constexpr int kMaxStamps = 256;      // per frame (2 per profile scope + 2)

        // The host's pipeline state, saved at BeginFrame and put back at EndFrame (restoreHostState).
        struct HostState
        {
            UINT scissorCount = 0, viewportCount = 0;
            D3D10_RECT scissors[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
            D3D10_VIEWPORT viewports[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
            ComPtr<ID3D10RasterizerState> rs;
            ComPtr<ID3D10BlendState> blend;
            FLOAT blendFactor[4] = {};
            UINT sampleMask = 0, stencilRef = 0;
            ComPtr<ID3D10DepthStencilState> depth;
            ID3D10ShaderResourceView* psSrv[kTextureSlots] = {};
            ID3D10ShaderResourceView* vsSrv[kTextureSlots] = {};
            ID3D10SamplerState* psSamplers[2] = {};
            ComPtr<ID3D10PixelShader> ps;
            ComPtr<ID3D10VertexShader> vs;
            ComPtr<ID3D10GeometryShader> gs;
            ID3D10Buffer* vsCb[3] = {};
            ID3D10Buffer* psCb[3] = {};
            D3D10_PRIMITIVE_TOPOLOGY topology = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
            ComPtr<ID3D10Buffer> ib;
            DXGI_FORMAT ibFormat = DXGI_FORMAT_UNKNOWN;
            UINT ibOffset = 0;
            ComPtr<ID3D10Buffer> vb;
            UINT vbStride = 0, vbOffset = 0;
            ComPtr<ID3D10InputLayout> layout;
            ID3D10RenderTargetView* rtvs[D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
            ComPtr<ID3D10DepthStencilView> dsv;
            ComPtr<ID3D10Predicate> predicate;
            BOOL predicateValue = FALSE;

            void Capture(ID3D10Device* c)
            {
                scissorCount = viewportCount = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
                c->RSGetScissorRects(&scissorCount, scissors);
                c->RSGetViewports(&viewportCount, viewports);
                c->RSGetState(&rs);
                c->OMGetBlendState(&blend, blendFactor, &sampleMask);
                c->OMGetDepthStencilState(&depth, &stencilRef);
                c->PSGetShaderResources(0, kTextureSlots, psSrv);
                c->VSGetShaderResources(0, kTextureSlots, vsSrv);
                c->PSGetSamplers(0, 2, psSamplers);
                c->PSGetShader(&ps);
                c->VSGetShader(&vs);
                c->GSGetShader(&gs);
                c->VSGetConstantBuffers(0, 3, vsCb);
                c->PSGetConstantBuffers(0, 3, psCb);
                c->IAGetPrimitiveTopology(&topology);
                c->IAGetIndexBuffer(&ib, &ibFormat, &ibOffset);
                c->IAGetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
                c->IAGetInputLayout(&layout);
                c->OMGetRenderTargets(D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, &dsv);
                // the frame runs unpredicated: a host predicate would silently skip its draws and copies
                c->GetPredication(&predicate, &predicateValue);
                c->SetPredication(nullptr, FALSE);
            }

            template <class T, std::size_t N>
            static void ReleaseAll(T* (&a)[N])
            {
                for (T*& p : a)
                    if (p)
                        std::exchange(p, nullptr)->Release();
            }

            void Restore(ID3D10Device* c)
            {
                c->OMSetRenderTargets(D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, dsv.Get());
                c->SetPredication(predicate.Get(), predicateValue);
                c->RSSetScissorRects(scissorCount, scissors);
                c->RSSetViewports(viewportCount, viewports);
                c->RSSetState(rs.Get());
                c->OMSetBlendState(blend.Get(), blendFactor, sampleMask);
                c->OMSetDepthStencilState(depth.Get(), stencilRef);
                c->PSSetShaderResources(0, kTextureSlots, psSrv);
                c->VSSetShaderResources(0, kTextureSlots, vsSrv);
                c->PSSetSamplers(0, 2, psSamplers);
                c->PSSetShader(ps.Get());
                c->VSSetShader(vs.Get());
                c->GSSetShader(gs.Get());
                c->VSSetConstantBuffers(0, 3, vsCb);
                c->PSSetConstantBuffers(0, 3, psCb);
                c->IASetPrimitiveTopology(topology);
                c->IASetIndexBuffer(ib.Get(), ibFormat, ibOffset);
                ID3D10Buffer* vbp = vb.Get();
                c->IASetVertexBuffers(0, 1, &vbp, &vbStride, &vbOffset);
                c->IASetInputLayout(layout.Get());
                ReleaseAll(psSrv);
                ReleaseAll(vsSrv);
                ReleaseAll(psSamplers);
                ReleaseAll(vsCb);
                ReleaseAll(psCb);
                ReleaseAll(rtvs);
                *this = HostState();
            }
        };

        struct Tex
        {
            ComPtr<ID3D10Texture2D> tex;
            ComPtr<ID3D10ShaderResourceView> srv;
            ComPtr<ID3D10RenderTargetView> rtv;
            TextureDesc desc;
            DXGI_FORMAT resolveFormat = DXGI_FORMAT_UNKNOWN;   // typed format for ResolveSubresource / staging copies
            ID3D10RenderTargetView* hostView = nullptr;       // wrapped host target: the key of wrapped_
        };

        struct Buf
        {
            ComPtr<ID3D10Buffer> buf;
            BufferDesc desc;
        };

        struct Pipe
        {
            ComPtr<ID3D10VertexShader> vs;
            ComPtr<ID3D10PixelShader> ps;
            ComPtr<ID3D10InputLayout> layout;
            ID3D10BlendState* blend = nullptr;
            D3D10_PRIMITIVE_TOPOLOGY topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            PipelineDesc desc;
        };

        struct ProfileSlot
        {
            ComPtr<ID3D10Query> disjoint;
            ComPtr<ID3D10Query> stamps[kMaxStamps];
            int used = 0;
            bool pending = false;
            d3d::ProfileFrame frame;
        };

        Caps MakeCaps()
        {
            Caps c;
            c.fxStorage = FxStorage::Texture;
            c.shaderFormat = (std::uint8_t)shaders::Format::DxbcSm4;
            c.framebufferOriginBottomLeft = false;
            c.clipSpaceYDown = false;
            c.halfPixelOffset = false;
            c.dualSourceBlend = true;
            c.floatRenderTargets = true;
            c.sampleRenderTarget = true;
            c.timestampQueries = true;
            c.readback = true;
            c.runtimeEffects = true;
            // the whole FX shader fits SM4: one pipeline instead of one per feature mask
            c.fxFeatureVariants = false;
            c.maxTextureSize = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            c.maxFxDataWidth = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            return c;
        }
    }

    class D3D10Device final : public Device
    {
    public:
        D3D10Device(const Desc& d) : dev_(d.device), restore_(d.restoreHostState), log_(d.debug), caps_(MakeCaps()) {}

        bool Init(std::string& error)
        {
            if (!dev_)
            {
                error = "no ID3D10Device";
                return false;
            }
            if (!d3d::ShaderCompilerAvailable(error))
                return false;
            if (log_.DebugLayer() && FAILED(dev_.As(info_)))
                log_.Log(LogLevel::Warning, "debug messages requested, but the device has no ID3D10InfoQueue (created without D3D10_CREATE_DEVICE_DEBUG?)");

            auto blend = [&](bool enable, D3D10_BLEND src, D3D10_BLEND dst, D3D10_BLEND srcA, D3D10_BLEND dstA, ComPtr<ID3D10BlendState>& out) {
                D3D10_BLEND_DESC b = {};
                b.BlendEnable[0] = enable;
                b.SrcBlend = src;
                b.DestBlend = dst;
                b.BlendOp = D3D10_BLEND_OP_ADD;
                b.SrcBlendAlpha = srcA;
                b.DestBlendAlpha = dstA;
                b.BlendOpAlpha = D3D10_BLEND_OP_ADD;
                b.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
                return log_.Check(dev_->CreateBlendState(&b, &out), "CreateBlendState");
            };
            bool ok = blend(false, D3D10_BLEND_ONE, D3D10_BLEND_ZERO, D3D10_BLEND_ONE, D3D10_BLEND_ZERO, blends_[(int)BlendMode::Opaque]);
            ok = ok && blend(true, D3D10_BLEND_SRC_ALPHA, D3D10_BLEND_INV_SRC_ALPHA, D3D10_BLEND_ONE, D3D10_BLEND_INV_SRC_ALPHA, blends_[(int)BlendMode::Straight]);
            ok = ok && blend(true, D3D10_BLEND_ONE, D3D10_BLEND_INV_SRC_ALPHA, D3D10_BLEND_ONE, D3D10_BLEND_INV_SRC_ALPHA, blends_[(int)BlendMode::Premultiplied]);
            ok = ok && blend(true, D3D10_BLEND_SRC1_COLOR, D3D10_BLEND_INV_SRC1_COLOR, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_INV_SRC1_ALPHA,
                             blends_[(int)BlendMode::DualSourceLcd]);

            D3D10_RASTERIZER_DESC rd = {};
            rd.FillMode = D3D10_FILL_SOLID;
            rd.CullMode = D3D10_CULL_NONE;
            rd.ScissorEnable = TRUE;
            rd.DepthClipEnable = TRUE;
            ok = ok && log_.Check(dev_->CreateRasterizerState(&rd, &raster_), "CreateRasterizerState");

            D3D10_DEPTH_STENCIL_DESC dsd = {};
            dsd.DepthEnable = FALSE;
            dsd.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
            dsd.DepthFunc = D3D10_COMPARISON_ALWAYS;
            dsd.FrontFace.StencilFailOp = dsd.FrontFace.StencilDepthFailOp = dsd.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
            dsd.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
            dsd.BackFace = dsd.FrontFace;
            ok = ok && log_.Check(dev_->CreateDepthStencilState(&dsd, &depth_), "CreateDepthStencilState");

            // s0 linear clamp, s1 point clamp; one mip level everywhere
            D3D10_SAMPLER_DESC sd = {};
            sd.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
            sd.AddressU = sd.AddressV = sd.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
            sd.ComparisonFunc = D3D10_COMPARISON_NEVER;
            sd.MaxLOD = 0.0f;
            ok = ok && log_.Check(dev_->CreateSamplerState(&sd, &samplers_[0]), "CreateSamplerState");
            sd.Filter = D3D10_FILTER_MIN_MAG_MIP_POINT;
            ok = ok && log_.Check(dev_->CreateSamplerState(&sd, &samplers_[1]), "CreateSamplerState");

            // Frame (192), Pass (32), Draw (32) bytes: dynamic, rewritten with WRITE_DISCARD at every SetConstants
            static const UINT kSizes[3] = {192, 32, 32};
            for (int i = 0; ok && i < 3; ++i)
            {
                D3D10_BUFFER_DESC b = {};
                b.ByteWidth = kSizes[i];
                b.Usage = D3D10_USAGE_DYNAMIC;
                b.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
                b.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
                ok = log_.Check(dev_->CreateBuffer(&b, nullptr, &cbs_[i]), "CreateBuffer (constants)");
            }

            for (ProfileSlot& s : profile_)
            {
                D3D10_QUERY_DESC q = {D3D10_QUERY_TIMESTAMP_DISJOINT, 0};
                if (FAILED(dev_->CreateQuery(&q, &s.disjoint)))
                    break;
                q.Query = D3D10_QUERY_TIMESTAMP;
                for (ComPtr<ID3D10Query>& t : s.stamps)
                    if (FAILED(dev_->CreateQuery(&q, &t)))
                    {
                        s.disjoint = nullptr;
                        break;
                    }
            }
            if (!profile_[kProfileSlots - 1].disjoint)
                caps_.timestampQueries = false;
            DrainMessages();
            if (!ok)
                error = "creating the D3D10 device objects failed";
            return ok;
        }

        const char* Name() const override { return "d3d10"; }
        const Caps& GetCaps() const override { return caps_; }

        // ------------------------------------------------------------------ resources
        Texture CreateTexture(const TextureDesc& desc, const void* data, int rowPitch) override
        {
            const d3d::DxgiFormats f = d3d::DxgiFormatsOf(desc.format);
            if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize ||
                f.storage == DXGI_FORMAT_UNKNOWN || desc.samples < 1)
            {
                log_.Printf(LogLevel::Error, "CreateTexture: unsupported %dx%d %s", desc.width, desc.height, FormatName(desc.format));
                return {};
            }
            const bool rt = desc.usage & TextureUsage_RenderTarget;
            const bool msaa = desc.samples > 1;
            if (msaa)
            {
                UINT quality = 0;
                if (!rt || FAILED(dev_->CheckMultisampleQualityLevels(f.rtv, (UINT)desc.samples, &quality)) || quality == 0)
                {
                    log_.Printf(LogLevel::Error, "CreateTexture: %d samples of %s are not supported", desc.samples, FormatName(desc.format));
                    return {};
                }
            }
            Tex t;
            t.desc = desc;
            t.desc.debugName = nullptr;
            // a multisampled texture is never sampled (Texture2DMS is not in the binding model): no SRV, not Sampled
            if (msaa)
                t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;
            t.resolveFormat = f.rtv;
            D3D10_TEXTURE2D_DESC td = {};
            td.Width = (UINT)desc.width;
            td.Height = (UINT)desc.height;
            td.MipLevels = td.ArraySize = 1;
            td.Format = f.storage;
            td.SampleDesc.Count = (UINT)desc.samples;
            td.Usage = D3D10_USAGE_DEFAULT;
            td.BindFlags = (t.desc.usage & TextureUsage_Sampled ? D3D10_BIND_SHADER_RESOURCE : 0u) | (rt ? D3D10_BIND_RENDER_TARGET : 0u);
            D3D10_SUBRESOURCE_DATA init = {data, (UINT)(rowPitch > 0 ? rowPitch : desc.width * BytesPerPixel(desc.format)), 0};
            if (!log_.Check(dev_->CreateTexture2D(&td, data ? &init : nullptr, &t.tex), "CreateTexture2D") || !CreateViews(t, f.srv, f.rtv))
            {
                DrainMessages();
                return {};
            }
            if (rt && !data)
            {
                // render targets start transparent black: a Load pass over fresh contents is then deterministic
                const float zero[4] = {};
                dev_->ClearRenderTargetView(t.rtv.Get(), zero);
            }
            SetDebugName(t.tex.Get(), desc.debugName);
            return Texture{textures_.Add(std::move(t))};
        }

        bool CreateViews(Tex& t, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvFormat)
        {
            if (t.desc.usage & TextureUsage_Sampled)
            {
                D3D10_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = srvFormat;
                sv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
                sv.Texture2D.MipLevels = 1;
                if (!log_.Check(dev_->CreateShaderResourceView(t.tex.Get(), &sv, &t.srv), "CreateShaderResourceView"))
                    return false;
            }
            if ((t.desc.usage & TextureUsage_RenderTarget) && !t.rtv)
            {
                D3D10_RENDER_TARGET_VIEW_DESC rv = {};
                rv.Format = rtvFormat;
                rv.ViewDimension = t.desc.samples > 1 ? D3D10_RTV_DIMENSION_TEXTURE2DMS : D3D10_RTV_DIMENSION_TEXTURE2D;
                if (!log_.Check(dev_->CreateRenderTargetView(t.tex.Get(), &rv, &t.rtv), "CreateRenderTargetView"))
                    return false;
            }
            return true;
        }

        Texture Wrap(ID3D10RenderTargetView* rtv)
        {
            if (!rtv)
                return {};
            auto it = wrapped_.find(rtv);
            if (it != wrapped_.end())
                return Texture{it->second};
            D3D10_RENDER_TARGET_VIEW_DESC rv;
            rtv->GetDesc(&rv);
            ComPtr<ID3D10Resource> res;
            rtv->GetResource(&res);
            Tex t;
            if (FAILED(res.As(t.tex)) || (rv.ViewDimension != D3D10_RTV_DIMENSION_TEXTURE2D && rv.ViewDimension != D3D10_RTV_DIMENSION_TEXTURE2DMS) ||
                (rv.ViewDimension == D3D10_RTV_DIMENSION_TEXTURE2D && rv.Texture2D.MipSlice != 0))
            {
                log_.Log(LogLevel::Error, "WrapRenderTarget: not a view of mip 0 of a 2D texture");
                return {};
            }
            D3D10_TEXTURE2D_DESC td;
            t.tex->GetDesc(&td);
            t.desc.width = (int)td.Width;
            t.desc.height = (int)td.Height;
            t.desc.format = d3d::FormatFromDxgi(rv.Format);
            t.desc.samples = (int)td.SampleDesc.Count;
            if (t.desc.format == Format::Unknown)
            {
                log_.Printf(LogLevel::Error, "WrapRenderTarget: DXGI format %d is not a format the renderer can target", (int)rv.Format);
                return {};
            }
            // Sampled only through a raw (non-sRGB) view: a typeless resource, or UNORM storage under an sRGB view
            const bool rawView = !d3d::DxgiIsSrgb(td.Format);
            t.desc.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc |
                           ((td.BindFlags & D3D10_BIND_SHADER_RESOURCE) && td.SampleDesc.Count == 1 && rawView ? TextureUsage_Sampled : 0u);
            t.rtv = rtv;
            t.hostView = rtv;
            // resolves and staging copies need a typed format: the view's for typeless storage
            const bool typeless = d3d::DxgiRaw(td.Format) != td.Format && !d3d::DxgiIsSrgb(td.Format);
            t.resolveFormat = typeless ? rv.Format : td.Format;
            if (!CreateViews(t, d3d::DxgiRaw(rv.Format), rv.Format))
                return {};
            const std::uint32_t id = textures_.Add(std::move(t));
            wrapped_[rtv] = id;
            return Texture{id};
        }

        void UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch) override
        {
            Tex* t = textures_.Find(tex.id);
            if (!t || !data || r.Empty())
                return;
            const D3D10_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            dev_->UpdateSubresource(t->tex.Get(), 0, &box, data, (UINT)(rowPitch > 0 ? rowPitch : r.Width() * BytesPerPixel(t->desc.format)), 0);
        }

        void DestroyTexture(Texture tex) override
        {
            if (Tex* t = textures_.Find(tex.id))
                if (t->hostView)
                    wrapped_.erase(t->hostView);
            textures_.Remove(tex.id);
        }

        TextureDesc GetTextureDesc(Texture tex) const override
        {
            const Tex* t = textures_.Find(tex.id);
            return t ? t->desc : TextureDesc{};
        }

        Buffer CreateBuffer(const BufferDesc& desc) override
        {
            if (desc.size == 0 || desc.size > 0xFFFFFFF0u)
                return {};
            Buf b;
            b.desc = desc;
            D3D10_BUFFER_DESC bd = {};
            bd.ByteWidth = (UINT)((desc.size + 15) & ~std::size_t(15));
            bd.Usage = D3D10_USAGE_DYNAMIC;
            bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
            switch (desc.kind)
            {
            case BufferKind::Vertex: bd.BindFlags = D3D10_BIND_VERTEX_BUFFER; break;
            case BufferKind::Index: bd.BindFlags = D3D10_BIND_INDEX_BUFFER; break;
            case BufferKind::FxInstances: return {};   // FxStorage::Texture: the instances are an RGBA32F texture
            }
            if (!log_.Check(dev_->CreateBuffer(&bd, nullptr, &b.buf), "CreateBuffer"))
            {
                DrainMessages();
                return {};
            }
            SetDebugName(b.buf.Get(), desc.debugName);
            return Buffer{buffers_.Add(std::move(b))};
        }

        void UpdateBuffer(Buffer buf, const void* data, std::size_t size) override
        {
            Buf* b = buffers_.Find(buf.id);
            if (!b || !data || size == 0 || size > b->desc.size)
                return;
            // WRITE_DISCARD: a new version of the buffer (earlier draws keep theirs); the renderer reads only the
            // bytes it wrote this frame, so the rest of the buffer need not survive
            void* m = nullptr;
            if (log_.Check(b->buf->Map(D3D10_MAP_WRITE_DISCARD, 0, &m), "Map (buffer)"))
            {
                std::memcpy(m, data, size);
                b->buf->Unmap();
            }
        }

        void DestroyBuffer(Buffer buf) override { buffers_.Remove(buf.id); }

        Pipeline CreatePipeline(const PipelineDesc& desc) override
        {
            if (desc.program >= ShaderProgram::Count || (desc.blend == BlendMode::DualSourceLcd) != (desc.program == ShaderProgram::TextLcd))
                return {};
            d3d::ShaderRequest vr;
            vr.program = desc.program;
            vr.stage = shaders::Stage::Vertex;
            vr.model = d3d::ShaderModel::Sm4;
            d3d::ShaderRequest pr = vr;
            pr.stage = shaders::Stage::Pixel;
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
            p.blend = blends_[(int)desc.blend].Get();
            p.topology = desc.topology == Topology::TriangleStrip ? D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            p.vs = VertexShader(vs);
            p.ps = PixelShader(ps);
            if (!p.vs || !p.ps)
                return {};
            if (desc.layout == VertexLayout::UiVertex)
            {
                if (!uiLayout_)
                {
                    // esia::Vertex: float2 pos, float2 uv, RGBA8 color
                    const D3D10_INPUT_ELEMENT_DESC e[] = {
                        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D10_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0},
                    };
                    if (!log_.Check(dev_->CreateInputLayout(e, 3, vs->data(), vs->size(), &uiLayout_), "CreateInputLayout"))
                        return {};
                }
                p.layout = uiLayout_;
            }
            return Pipeline{pipelines_.Add(std::move(p))};
        }

        void DestroyPipeline(Pipeline p) override { pipelines_.Remove(p.id); }

        // ------------------------------------------------------------------ frame
        bool BeginFrame(const FrameDesc&) override
        {
            ++frame_;
            // with restoreHostState, what the frame changes is captured and put back (ClearState would also drop the
            // host's compute, stream-output and UAV bindings, which are not); without it, start from a clean state
            if (restore_)
                host_.Capture(dev_.Get());
            else
                dev_->ClearState();
            if (caps_.timestampQueries)
            {
                ProfileSlot& s = profile_[frame_ % kProfileSlots];
                if (s.pending)
                    ReadSlot(s);   // the GPU is kProfileSlots frames behind: the old numbers are dropped if not ready
                s.used = 0;
                s.pending = false;
                s.frame.Reset(frame_);
                s.disjoint->Begin();
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
                s.disjoint->End();
                s.pending = s.frame.frameEnd > 0;
                // submitted now, so the queries complete even when nothing presents (headless, several targets)
                dev_->Flush();
            }
            if (restore_)
                host_.Restore(dev_.Get());
            else
                dev_->ClearState();
            DrainMessages();
        }

        // ------------------------------------------------------------------ commands
        void BeginPass(const PassDesc& desc) override
        {
            Tex* t = textures_.Find(desc.target.id);
            if (!t || !t->rtv)
                return;
            pass_ = t;
            // nothing bound at the start of a pass - in particular no view of this target as a shader resource
            ID3D10ShaderResourceView* none[kTextureSlots] = {};
            dev_->PSSetShaderResources(0, kTextureSlots, none);
            dev_->VSSetShaderResources(0, kTextureSlots, none);
            ApplyPassState();
            if (desc.load == LoadOp::Clear)
            {
                float c[4];
                d3d::ClearValueFor(t->desc.format, desc.clearColor, c);
                dev_->ClearRenderTargetView(t->rtv.Get(), c);
            }
            // LoadOp::DontCare needs nothing on an immediate-mode desktop API
        }

        void EndPass() override
        {
            dev_->OMSetRenderTargets(0, nullptr, nullptr);
            pass_ = nullptr;
        }

        void SetPipeline(Pipeline p) override
        {
            const Pipe* pipe = pipelines_.Find(p.id);
            if (!pipe)
                return;
            if (hostTouched_)
                ApplyPassState();
            dev_->IASetInputLayout(pipe->layout.Get());
            dev_->IASetPrimitiveTopology(pipe->topology);
            dev_->VSSetShader(pipe->vs.Get());
            dev_->PSSetShader(pipe->ps.Get());
            const float factor[4] = {};
            dev_->OMSetBlendState(pipe->blend, factor, 0xFFFFFFFFu);
        }

        void SetScissor(const IRect& r) override
        {
            const D3D10_RECT s = {r.x0, r.y0, r.x1, r.y1};
            dev_->RSSetScissorRects(1, &s);
        }

        void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) override
        {
            ID3D10Buffer* cb = cbs_[(int)slot].Get();
            void* m = nullptr;
            if (!data || size == 0 || !log_.Check(cb->Map(D3D10_MAP_WRITE_DISCARD, 0, &m), "Map (constants)"))
                return;
            std::memcpy(m, data, std::min<std::uint32_t>(size, slot == ConstantSlot::Frame ? 192u : 32u));
            cb->Unmap();
        }

        void SetTexture(int slot, Texture tex) override
        {
            if (slot < 0 || slot >= kTextureSlots)
                return;
            const Tex* t = textures_.Find(tex.id);
            ID3D10ShaderResourceView* srv = t ? t->srv.Get() : nullptr;
            dev_->PSSetShaderResources((UINT)slot, 1, &srv);
            // t7, the FX instance texture, is read by both stages (FxVS for the bounds, FxPS for the material)
            if (slot == kSlotFxData)
                dev_->VSSetShaderResources((UINT)slot, 1, &srv);
        }

        void SetFxBuffer(Buffer) override {}   // FxStorage::Texture: SetTexture(kSlotFxData, ...)

        void SetVertexBuffer(Buffer buf) override
        {
            const Buf* b = buffers_.Find(buf.id);
            ID3D10Buffer* vb = b ? b->buf.Get() : nullptr;
            const UINT stride = 20, offset = 0;   // sizeof(esia::Vertex)
            dev_->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        }

        void SetIndexBuffer(Buffer buf) override
        {
            const Buf* b = buffers_.Find(buf.id);
            dev_->IASetIndexBuffer(b ? b->buf.Get() : nullptr, DXGI_FORMAT_R32_UINT, 0);
        }

        void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override { dev_->Draw(vertexCount, firstVertex); }
        void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) override { dev_->DrawIndexed(indexCount, firstIndex, 0); }
        void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) override { dev_->DrawInstanced(vertexCount, instanceCount, 0, 0); }

        void CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r) override
        {
            Tex* d = textures_.Find(dst.id);
            Tex* s = textures_.Find(src.id);
            if (!d || !s || r.Empty())
                return;
            if (s->desc.samples > 1)
            {
                // ResolveSubresource has no region: resolve the whole target straight into a destination of the same
                // size at the same place (the backdrop copy - texels outside the region get the same, newer source
                // texels, which nothing reads), else through a temporary
                if (d->desc.width == s->desc.width && d->desc.height == s->desc.height && dstX == r.x0 && dstY == r.y0)
                {
                    dev_->ResolveSubresource(d->tex.Get(), 0, s->tex.Get(), 0, s->resolveFormat);
                    return;
                }
                ComPtr<ID3D10Texture2D> tmp = Resolved(*s);
                if (!tmp)
                    return;
                const D3D10_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
                dev_->CopySubresourceRegion(d->tex.Get(), 0, (UINT)dstX, (UINT)dstY, 0, tmp.Get(), 0, &box);
                return;
            }
            const D3D10_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            dev_->CopySubresourceRegion(d->tex.Get(), 0, (UINT)dstX, (UINT)dstY, 0, s->tex.Get(), 0, &box);
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
            // oldest first, so `latest_` ends as the newest complete frame
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
        bool ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& rgba8) override
        {
            rgba8.clear();
            Tex* t = textures_.Find(tex.id);
            if (!t || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height)
                return false;
            ComPtr<ID3D10Texture2D> src = t->tex;
            if (t->desc.samples > 1 && !(src = Resolved(*t)))
                return false;
            D3D10_TEXTURE2D_DESC sd = {};
            sd.Width = (UINT)r.Width();
            sd.Height = (UINT)r.Height();
            sd.MipLevels = sd.ArraySize = 1;
            sd.Format = d3d::DxgiRaw(t->resolveFormat);
            sd.SampleDesc.Count = 1;
            sd.Usage = D3D10_USAGE_STAGING;
            sd.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
            ComPtr<ID3D10Texture2D> staging;
            if (!log_.Check(dev_->CreateTexture2D(&sd, nullptr, &staging), "CreateTexture2D (readback)"))
                return false;
            const D3D10_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            dev_->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, src.Get(), 0, &box);
            D3D10_MAPPED_TEXTURE2D m;
            if (!log_.Check(staging->Map(0, D3D10_MAP_READ, 0, &m), "Map (readback)"))
                return false;
            d3d::ConvertToRgba8(t->desc.format, m.pData, m.RowPitch, r.Width(), r.Height(), rgba8);
            staging->Unmap(0);
            DrainMessages();
            return true;
        }

    private:
        // Everything a pass relies on that host callbacks may have changed (and ClearState reset).
        void ApplyPassState()
        {
            hostTouched_ = false;
            if (!pass_)
                return;
            ID3D10RenderTargetView* rtv = pass_->rtv.Get();
            dev_->OMSetRenderTargets(1, &rtv, nullptr);
            const D3D10_VIEWPORT vp = {0, 0, (UINT)pass_->desc.width, (UINT)pass_->desc.height, 0.0f, 1.0f};
            dev_->RSSetViewports(1, &vp);
            const D3D10_RECT all = {0, 0, pass_->desc.width, pass_->desc.height};
            dev_->RSSetScissorRects(1, &all);
            dev_->RSSetState(raster_.Get());
            dev_->OMSetDepthStencilState(depth_.Get(), 0);
            ID3D10SamplerState* s[2] = {samplers_[0].Get(), samplers_[1].Get()};
            dev_->PSSetSamplers(0, 2, s);
            ID3D10Buffer* cbs[3] = {cbs_[0].Get(), cbs_[1].Get(), cbs_[2].Get()};
            dev_->VSSetConstantBuffers(0, 3, cbs);
            dev_->PSSetConstantBuffers(0, 3, cbs);
            dev_->GSSetShader(nullptr);
        }

        // A single-sampled copy of a multisampled texture.
        ComPtr<ID3D10Texture2D> Resolved(const Tex& t)
        {
            D3D10_TEXTURE2D_DESC td = {};
            td.Width = (UINT)t.desc.width;
            td.Height = (UINT)t.desc.height;
            td.MipLevels = td.ArraySize = 1;
            td.Format = d3d::DxgiTypeless(t.resolveFormat);
            td.SampleDesc.Count = 1;
            td.Usage = D3D10_USAGE_DEFAULT;
            ComPtr<ID3D10Texture2D> tmp;
            if (!log_.Check(dev_->CreateTexture2D(&td, nullptr, &tmp), "CreateTexture2D (resolve)"))
                return nullptr;
            dev_->ResolveSubresource(tmp.Get(), 0, t.tex.Get(), 0, t.resolveFormat);
            return tmp;
        }

        ComPtr<ID3D10VertexShader> VertexShader(const d3d::Bytecode& code)
        {
            ComPtr<ID3D10VertexShader>& vs = vertexShaders_[code.get()];
            if (!vs)
                log_.Check(dev_->CreateVertexShader(code->data(), code->size(), &vs), "CreateVertexShader");
            return vs;
        }

        ComPtr<ID3D10PixelShader> PixelShader(const d3d::Bytecode& code)
        {
            ComPtr<ID3D10PixelShader>& ps = pixelShaders_[code.get()];
            if (!ps)
                log_.Check(dev_->CreatePixelShader(code->data(), code->size(), &ps), "CreatePixelShader");
            return ps;
        }

        int Stamp()
        {
            ProfileSlot& s = profile_[frame_ % kProfileSlots];
            if (s.used >= kMaxStamps)
                return -1;
            s.stamps[s.used]->End();
            return s.used++;
        }

        void ReadSlot(ProfileSlot& s)
        {
            D3D10_QUERY_DATA_TIMESTAMP_DISJOINT dj = {};
            std::uint64_t ticks[kMaxStamps];
            if (s.disjoint->GetData(&dj, sizeof(dj), D3D10_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
                return;   // not yet: try again later
            for (int i = 0; i < s.used; ++i)
                if (s.stamps[i]->GetData(&ticks[i], sizeof(std::uint64_t), D3D10_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
                    return;
            s.pending = false;
            if (dj.Disjoint || dj.Frequency == 0)
                return;   // the clock changed during the frame: its numbers mean nothing
            const GpuProfile p = s.frame.Resolve(ticks, s.used, (double)dj.Frequency);
            if (p.valid && p.frame > latest_.frame)
                latest_ = p;
        }

        template <class T>
        static void SetDebugName(T* object, const char* name)
        {
            if (object && name)
                object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)std::strlen(name), name);
        }

        void DrainMessages()
        {
            if (!info_)
                return;
            const UINT64 n = info_->GetNumStoredMessages();
            for (UINT64 i = 0; i < n; ++i)
            {
                SIZE_T size = 0;
                info_->GetMessage(i, nullptr, &size);
                std::vector<char> buf(size);
                auto* m = reinterpret_cast<D3D10_MESSAGE*>(buf.data());
                if (size == 0 || FAILED(info_->GetMessage(i, m, &size)))
                    continue;
                const LogLevel level = m->Severity <= D3D10_MESSAGE_SEVERITY_ERROR ? LogLevel::Error
                                       : m->Severity == D3D10_MESSAGE_SEVERITY_WARNING ? LogLevel::Warning
                                                                                         : LogLevel::Info;
                log_.Printf(level, "D3D10 debug layer: %.*s", (int)m->DescriptionByteLength, m->pDescription);
            }
            info_->ClearStoredMessages();
        }

        ComPtr<ID3D10Device> dev_;
        bool restore_ = true;
        d3d::Logger log_;
        ComPtr<ID3D10InfoQueue> info_;
        Caps caps_;

        d3d::HandleTable<Tex> textures_;
        d3d::HandleTable<Buf> buffers_;
        d3d::HandleTable<Pipe> pipelines_;
        std::unordered_map<ID3D10RenderTargetView*, std::uint32_t> wrapped_;
        // shader objects per bytecode (the process-wide cache keeps the bytecode, and so these keys, alive)
        std::unordered_map<const std::vector<std::uint8_t>*, ComPtr<ID3D10VertexShader>> vertexShaders_;
        std::unordered_map<const std::vector<std::uint8_t>*, ComPtr<ID3D10PixelShader>> pixelShaders_;
        ComPtr<ID3D10InputLayout> uiLayout_;
        ComPtr<ID3D10BlendState> blends_[4];
        ComPtr<ID3D10RasterizerState> raster_;
        ComPtr<ID3D10DepthStencilState> depth_;
        ComPtr<ID3D10SamplerState> samplers_[2];
        ComPtr<ID3D10Buffer> cbs_[3];

        HostState host_;
        bool hostTouched_ = false;
        Tex* pass_ = nullptr;
        std::uint64_t frame_ = 0;
        ProfileSlot profile_[kProfileSlots];
        GpuProfile latest_;
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        auto dev = std::make_unique<D3D10Device>(desc);
        std::string e;
        if (!dev->Init(e))
        {
            if (error)
                *error = e;
            return nullptr;
        }
        return dev;
    }

    Texture WrapRenderTarget(Device& device, ID3D10RenderTargetView* rtv)
    {
        auto* d = dynamic_cast<D3D10Device*>(&device);
        return d ? d->Wrap(rtv) : Texture{};
    }

    // ------------------------------------------------------------------ headless (conformance suite)
    namespace
    {
        HeadlessDevice CreateHeadless(const HeadlessDesc& hd, std::string& error)
        {
            const int debug = d3d::DebugLevelFromEnvironment();
            const D3D10_DRIVER_TYPE driver = d3d::UseWarpFromEnvironment() ? D3D10_DRIVER_TYPE_WARP : D3D10_DRIVER_TYPE_HARDWARE;
            // D3D10CreateDevice1 from the DLL: mingw-w64 has no d3d10_1 import library
            HMODULE dll = LoadLibraryW(L"d3d10_1.dll");
            auto create = dll ? reinterpret_cast<decltype(&D3D10CreateDevice1)>(reinterpret_cast<void*>(GetProcAddress(dll, "D3D10CreateDevice1"))) : nullptr;
            if (!create)
            {
                error = "d3d10_1.dll / D3D10CreateDevice1 not found";
                return {};
            }
            ComPtr<ID3D10Device1> device;
            HRESULT hr = E_FAIL;
            for (int attempt = debug > 0 ? 0 : 1; attempt < 2 && FAILED(hr); ++attempt)
            {
                // the debug layer needs the Graphics Tools optional feature: without it, run without
                const UINT flags = attempt == 0 ? D3D10_CREATE_DEVICE_DEBUG : 0u;
                for (D3D10_FEATURE_LEVEL1 level : {D3D10_FEATURE_LEVEL_10_1, D3D10_FEATURE_LEVEL_10_0})
                    if (FAILED(hr))
                        hr = create(nullptr, driver, nullptr, flags, level, D3D10_1_SDK_VERSION, &device);
            }
            if (FAILED(hr))
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "D3D10CreateDevice1 failed: 0x%08lX (%s)", (unsigned long)hr, d3d::HResultName(hr));
                error = buf;
                return {};
            }
            Desc d;
            d.device = device.Get();
            d.restoreHostState = false;
            d.debug.debugLayer = debug > 0;
            HeadlessDevice h;
            h.device = CreateDevice(d, &error);
            if (!h.device)
                return {};
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

void EsiaRegisterBackend_d3d10()
{
    esia::rhi::BackendInfo info;
    info.name = "d3d10";
    info.createHeadless = &esia::rhi::d3d10::CreateHeadless;
    esia::rhi::RegisterBackend(info);
}
