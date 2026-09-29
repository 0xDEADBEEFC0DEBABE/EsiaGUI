// Esia - Direct3D 11 backend (see esia/rhi/d3d11.hpp). The techniques are those of WGT's
// src/backends/d3d11_backend.cpp: dynamic buffers written with MAP_WRITE_DISCARD, FX instances in a structured
// buffer, typeless storage so the backdrop copy and raw views of sRGB targets share bits, ResolveSubresource for
// multisampled targets, the target's own SRV for the direct read, timestamp queries read a few frames later with
// D3D11_ASYNC_GETDATA_DONOTFLUSH, and the host's pipeline state saved and restored around the frame.
#include "esia/rhi/d3d11.hpp"
#include "d3d_shader.hpp"
#include "esia/rhi/backend_registry.hpp"
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esia::rhi::d3d11
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
            D3D11_RECT scissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
            D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
            ComPtr<ID3D11RasterizerState> rs;
            ComPtr<ID3D11BlendState> blend;
            FLOAT blendFactor[4] = {};
            UINT sampleMask = 0, stencilRef = 0;
            ComPtr<ID3D11DepthStencilState> depth;
            ID3D11ShaderResourceView* psSrv[kTextureSlots] = {};
            ID3D11ShaderResourceView* vsSrv[kTextureSlots] = {};
            ID3D11SamplerState* psSamplers[2] = {};
            ComPtr<ID3D11PixelShader> ps;
            ComPtr<ID3D11VertexShader> vs;
            ComPtr<ID3D11GeometryShader> gs;
            ComPtr<ID3D11HullShader> hs;
            ComPtr<ID3D11DomainShader> ds;
            ID3D11Buffer* vsCb[3] = {};
            ID3D11Buffer* psCb[3] = {};
            D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            ComPtr<ID3D11Buffer> ib;
            DXGI_FORMAT ibFormat = DXGI_FORMAT_UNKNOWN;
            UINT ibOffset = 0;
            ComPtr<ID3D11Buffer> vb;
            UINT vbStride = 0, vbOffset = 0;
            ComPtr<ID3D11InputLayout> layout;
            ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
            ComPtr<ID3D11DepthStencilView> dsv;
            ComPtr<ID3D11Predicate> predicate;
            BOOL predicateValue = FALSE;
            ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = {};

            void Capture(ID3D11DeviceContext* c)
            {
                scissorCount = viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
                c->RSGetScissorRects(&scissorCount, scissors);
                c->RSGetViewports(&viewportCount, viewports);
                c->RSGetState(&rs);
                c->OMGetBlendState(&blend, blendFactor, &sampleMask);
                c->OMGetDepthStencilState(&depth, &stencilRef);
                c->PSGetShaderResources(0, kTextureSlots, psSrv);
                c->VSGetShaderResources(0, kTextureSlots, vsSrv);
                c->PSGetSamplers(0, 2, psSamplers);
                c->PSGetShader(&ps, nullptr, nullptr);
                c->VSGetShader(&vs, nullptr, nullptr);
                c->GSGetShader(&gs, nullptr, nullptr);
                c->HSGetShader(&hs, nullptr, nullptr);
                c->DSGetShader(&ds, nullptr, nullptr);
                c->VSGetConstantBuffers(0, 3, vsCb);
                c->PSGetConstantBuffers(0, 3, psCb);
                c->IAGetPrimitiveTopology(&topology);
                c->IAGetIndexBuffer(&ib, &ibFormat, &ibOffset);
                c->IAGetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
                c->IAGetInputLayout(&layout);
                // UAVs share the output slots with the render targets: both are put back together
                c->OMGetRenderTargetsAndUnorderedAccessViews(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, &dsv, 0, D3D11_PS_CS_UAV_REGISTER_COUNT, uavs);
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

            void Restore(ID3D11DeviceContext* c)
            {
                UINT targets = 0;
                for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                    if (rtvs[i])
                        targets = i + 1;
                const UINT keepCounts[D3D11_PS_CS_UAV_REGISTER_COUNT] = {~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u};
                c->OMSetRenderTargetsAndUnorderedAccessViews(targets, rtvs, dsv.Get(), targets, D3D11_PS_CS_UAV_REGISTER_COUNT - targets, uavs + targets,
                                                             keepCounts);
                c->SetPredication(predicate.Get(), predicateValue);
                c->RSSetScissorRects(scissorCount, scissors);
                c->RSSetViewports(viewportCount, viewports);
                c->RSSetState(rs.Get());
                c->OMSetBlendState(blend.Get(), blendFactor, sampleMask);
                c->OMSetDepthStencilState(depth.Get(), stencilRef);
                c->PSSetShaderResources(0, kTextureSlots, psSrv);
                c->VSSetShaderResources(0, kTextureSlots, vsSrv);
                c->PSSetSamplers(0, 2, psSamplers);
                c->PSSetShader(ps.Get(), nullptr, 0);
                c->VSSetShader(vs.Get(), nullptr, 0);
                c->GSSetShader(gs.Get(), nullptr, 0);
                c->HSSetShader(hs.Get(), nullptr, 0);
                c->DSSetShader(ds.Get(), nullptr, 0);
                c->VSSetConstantBuffers(0, 3, vsCb);
                c->PSSetConstantBuffers(0, 3, psCb);
                c->IASetPrimitiveTopology(topology);
                c->IASetIndexBuffer(ib.Get(), ibFormat, ibOffset);
                ID3D11Buffer* vbp = vb.Get();
                c->IASetVertexBuffers(0, 1, &vbp, &vbStride, &vbOffset);
                c->IASetInputLayout(layout.Get());
                ReleaseAll(psSrv);
                ReleaseAll(vsSrv);
                ReleaseAll(psSamplers);
                ReleaseAll(vsCb);
                ReleaseAll(psCb);
                ReleaseAll(rtvs);
                ReleaseAll(uavs);
                *this = HostState();
            }
        };

        struct Tex
        {
            ComPtr<ID3D11Texture2D> tex;
            ComPtr<ID3D11ShaderResourceView> srv;
            ComPtr<ID3D11RenderTargetView> rtv;
            TextureDesc desc;
            DXGI_FORMAT resolveFormat = DXGI_FORMAT_UNKNOWN;   // typed format for ResolveSubresource / staging copies
            ID3D11RenderTargetView* hostView = nullptr;       // wrapped host target: the key of wrapped_
        };

        struct Buf
        {
            ComPtr<ID3D11Buffer> buf;
            ComPtr<ID3D11ShaderResourceView> srv;   // FX instances (structured buffer)
            BufferDesc desc;
        };

        struct Pipe
        {
            ComPtr<ID3D11VertexShader> vs;
            ComPtr<ID3D11PixelShader> ps;
            ComPtr<ID3D11InputLayout> layout;
            ID3D11BlendState* blend = nullptr;
            D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            PipelineDesc desc;
        };

        struct ProfileSlot
        {
            ComPtr<ID3D11Query> disjoint;
            ComPtr<ID3D11Query> stamps[kMaxStamps];
            int used = 0;
            bool pending = false;
            d3d::ProfileFrame frame;
        };

        Caps MakeCaps()
        {
            Caps c;
            c.fxStorage = FxStorage::Buffer;
            c.shaderFormat = (std::uint8_t)shaders::Format::DxbcSm5;
            c.framebufferOriginBottomLeft = false;
            c.clipSpaceYDown = false;
            c.halfPixelOffset = false;
            c.dualSourceBlend = true;
            c.floatRenderTargets = true;
            c.sampleRenderTarget = true;
            c.timestampQueries = true;
            c.readback = true;
            c.runtimeEffects = true;
            // the whole FX shader fits SM5: one pipeline instead of one per feature mask
            c.fxFeatureVariants = false;
            c.maxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            c.maxFxDataWidth = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            return c;
        }
    }

    class D3D11Device final : public Device
    {
    public:
        D3D11Device(const Desc& d) : dev_(d.device), ctx_(d.context), restore_(d.restoreHostState), log_(d.debug), caps_(MakeCaps()) {}

        bool Init(std::string& error)
        {
            if (!dev_ || !ctx_)
            {
                error = "no ID3D11Device / immediate context";
                return false;
            }
            if (dev_->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
            {
                error = "the d3d11 backend needs feature level 11_0 (shader model 5); use the d3d10 backend";
                return false;
            }
            if (!d3d::ShaderCompilerAvailable(error))
                return false;
            if (log_.DebugLayer() && FAILED(dev_.As(info_)))
                log_.Log(LogLevel::Warning, "debug messages requested, but the device has no ID3D11InfoQueue (created without D3D11_CREATE_DEVICE_DEBUG?)");

            auto blend = [&](bool enable, D3D11_BLEND src, D3D11_BLEND dst, D3D11_BLEND srcA, D3D11_BLEND dstA, ComPtr<ID3D11BlendState>& out) {
                D3D11_BLEND_DESC b = {};
                b.RenderTarget[0].BlendEnable = enable;
                b.RenderTarget[0].SrcBlend = src;
                b.RenderTarget[0].DestBlend = dst;
                b.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                b.RenderTarget[0].SrcBlendAlpha = srcA;
                b.RenderTarget[0].DestBlendAlpha = dstA;
                b.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                b.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                return log_.Check(dev_->CreateBlendState(&b, &out), "CreateBlendState");
            };
            bool ok = blend(false, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, blends_[(int)BlendMode::Opaque]);
            ok = ok && blend(true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, blends_[(int)BlendMode::Straight]);
            ok = ok && blend(true, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, blends_[(int)BlendMode::Premultiplied]);
            ok = ok && blend(true, D3D11_BLEND_SRC1_COLOR, D3D11_BLEND_INV_SRC1_COLOR, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_INV_SRC1_ALPHA,
                             blends_[(int)BlendMode::DualSourceLcd]);

            D3D11_RASTERIZER_DESC rd = {};
            rd.FillMode = D3D11_FILL_SOLID;
            rd.CullMode = D3D11_CULL_NONE;
            rd.ScissorEnable = TRUE;
            rd.DepthClipEnable = TRUE;
            ok = ok && log_.Check(dev_->CreateRasterizerState(&rd, &raster_), "CreateRasterizerState");

            D3D11_DEPTH_STENCIL_DESC dsd = {};
            dsd.DepthEnable = FALSE;
            dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
            dsd.FrontFace.StencilFailOp = dsd.FrontFace.StencilDepthFailOp = dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            dsd.BackFace = dsd.FrontFace;
            ok = ok && log_.Check(dev_->CreateDepthStencilState(&dsd, &depth_), "CreateDepthStencilState");

            // s0 linear clamp, s1 point clamp; one mip level everywhere
            D3D11_SAMPLER_DESC sd = {};
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
            sd.MaxLOD = 0.0f;
            ok = ok && log_.Check(dev_->CreateSamplerState(&sd, &samplers_[0]), "CreateSamplerState");
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            ok = ok && log_.Check(dev_->CreateSamplerState(&sd, &samplers_[1]), "CreateSamplerState");

            // Frame (192), Pass (32), Draw (32) bytes: dynamic, rewritten with WRITE_DISCARD at every SetConstants
            static const UINT kSizes[3] = {192, 32, 32};
            for (int i = 0; ok && i < 3; ++i)
            {
                D3D11_BUFFER_DESC b = {};
                b.ByteWidth = kSizes[i];
                b.Usage = D3D11_USAGE_DYNAMIC;
                b.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                b.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                ok = log_.Check(dev_->CreateBuffer(&b, nullptr, &cbs_[i]), "CreateBuffer (constants)");
            }

            for (ProfileSlot& s : profile_)
            {
                D3D11_QUERY_DESC q = {D3D11_QUERY_TIMESTAMP_DISJOINT, 0};
                if (FAILED(dev_->CreateQuery(&q, &s.disjoint)))
                    break;
                q.Query = D3D11_QUERY_TIMESTAMP;
                for (ComPtr<ID3D11Query>& t : s.stamps)
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
                error = "creating the D3D11 device objects failed";
            return ok;
        }

        const char* Name() const override { return "d3d11"; }
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
            D3D11_TEXTURE2D_DESC td = {};
            td.Width = (UINT)desc.width;
            td.Height = (UINT)desc.height;
            td.MipLevels = td.ArraySize = 1;
            td.Format = f.storage;
            td.SampleDesc.Count = (UINT)desc.samples;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = (t.desc.usage & TextureUsage_Sampled ? D3D11_BIND_SHADER_RESOURCE : 0u) | (rt ? D3D11_BIND_RENDER_TARGET : 0u);
            D3D11_SUBRESOURCE_DATA init = {data, (UINT)(rowPitch > 0 ? rowPitch : desc.width * BytesPerPixel(desc.format)), 0};
            if (!log_.Check(dev_->CreateTexture2D(&td, data ? &init : nullptr, &t.tex), "CreateTexture2D") || !CreateViews(t, f.srv, f.rtv))
            {
                DrainMessages();
                return {};
            }
            if (rt && !data)
            {
                // render targets start transparent black: a Load pass over fresh contents is then deterministic
                const float zero[4] = {};
                ctx_->ClearRenderTargetView(t.rtv.Get(), zero);
            }
            SetDebugName(t.tex.Get(), desc.debugName);
            return Texture{textures_.Add(std::move(t))};
        }

        bool CreateViews(Tex& t, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvFormat)
        {
            if (t.desc.usage & TextureUsage_Sampled)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = srvFormat;
                sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                sv.Texture2D.MipLevels = 1;
                if (!log_.Check(dev_->CreateShaderResourceView(t.tex.Get(), &sv, &t.srv), "CreateShaderResourceView"))
                    return false;
            }
            if ((t.desc.usage & TextureUsage_RenderTarget) && !t.rtv)
            {
                D3D11_RENDER_TARGET_VIEW_DESC rv = {};
                rv.Format = rtvFormat;
                rv.ViewDimension = t.desc.samples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
                if (!log_.Check(dev_->CreateRenderTargetView(t.tex.Get(), &rv, &t.rtv), "CreateRenderTargetView"))
                    return false;
            }
            return true;
        }

        Texture Wrap(ID3D11RenderTargetView* rtv)
        {
            if (!rtv)
                return {};
            auto it = wrapped_.find(rtv);
            if (it != wrapped_.end())
                return Texture{it->second};
            D3D11_RENDER_TARGET_VIEW_DESC rv;
            rtv->GetDesc(&rv);
            ComPtr<ID3D11Resource> res;
            rtv->GetResource(&res);
            Tex t;
            if (FAILED(res.As(t.tex)) || (rv.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2D && rv.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2DMS) ||
                (rv.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D && rv.Texture2D.MipSlice != 0))
            {
                log_.Log(LogLevel::Error, "WrapRenderTarget: not a view of mip 0 of a 2D texture");
                return {};
            }
            D3D11_TEXTURE2D_DESC td;
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
                           ((td.BindFlags & D3D11_BIND_SHADER_RESOURCE) && td.SampleDesc.Count == 1 && rawView ? TextureUsage_Sampled : 0u);
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
            const D3D11_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            ctx_->UpdateSubresource(t->tex.Get(), 0, &box, data, (UINT)(rowPitch > 0 ? rowPitch : r.Width() * BytesPerPixel(t->desc.format)), 0);
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
            D3D11_BUFFER_DESC bd = {};
            bd.ByteWidth = (UINT)((desc.size + 15) & ~std::size_t(15));
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            switch (desc.kind)
            {
            case BufferKind::Vertex: bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; break;
            case BufferKind::Index: bd.BindFlags = D3D11_BIND_INDEX_BUFFER; break;
            case BufferKind::FxInstances:
                bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                bd.StructureByteStride = 16;   // StructuredBuffer<float4>
                break;
            }
            if (!log_.Check(dev_->CreateBuffer(&bd, nullptr, &b.buf), "CreateBuffer"))
            {
                DrainMessages();
                return {};
            }
            if (desc.kind == BufferKind::FxInstances)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = DXGI_FORMAT_UNKNOWN;
                sv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                sv.Buffer.NumElements = bd.ByteWidth / 16;
                if (!log_.Check(dev_->CreateShaderResourceView(b.buf.Get(), &sv, &b.srv), "CreateShaderResourceView (FX instances)"))
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
            D3D11_MAPPED_SUBRESOURCE m;
            if (log_.Check(ctx_->Map(b->buf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m), "Map (buffer)"))
            {
                std::memcpy(m.pData, data, size);
                ctx_->Unmap(b->buf.Get(), 0);
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
            vr.model = d3d::ShaderModel::Sm5;
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
            p.topology = desc.topology == Topology::TriangleStrip ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            p.vs = VertexShader(vs);
            p.ps = PixelShader(ps);
            if (!p.vs || !p.ps)
                return {};
            if (desc.layout == VertexLayout::UiVertex)
            {
                if (!uiLayout_)
                {
                    // esia::Vertex: float2 pos, float2 uv, RGBA8 color
                    const D3D11_INPUT_ELEMENT_DESC e[] = {
                        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
                host_.Capture(ctx_.Get());
            else
                ctx_->ClearState();
            if (caps_.timestampQueries)
            {
                ProfileSlot& s = profile_[frame_ % kProfileSlots];
                if (s.pending)
                    ReadSlot(s);   // the GPU is kProfileSlots frames behind: the old numbers are dropped if not ready
                s.used = 0;
                s.pending = false;
                s.frame.Reset(frame_);
                ctx_->Begin(s.disjoint.Get());
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
                ctx_->End(s.disjoint.Get());
                s.pending = s.frame.frameEnd > 0;
                // submitted now, so the queries complete even when nothing presents (headless, several targets)
                ctx_->Flush();
            }
            if (restore_)
                host_.Restore(ctx_.Get());
            else
                ctx_->ClearState();
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
            ID3D11ShaderResourceView* none[kTextureSlots] = {};
            ctx_->PSSetShaderResources(0, kTextureSlots, none);
            ctx_->VSSetShaderResources(0, kTextureSlots, none);
            ApplyPassState();
            if (desc.load == LoadOp::Clear)
            {
                float c[4];
                d3d::ClearValueFor(t->desc.format, desc.clearColor, c);
                ctx_->ClearRenderTargetView(t->rtv.Get(), c);
            }
            // LoadOp::DontCare needs nothing on an immediate-mode desktop API
        }

        void EndPass() override
        {
            ctx_->OMSetRenderTargets(0, nullptr, nullptr);
            pass_ = nullptr;
        }

        void SetPipeline(Pipeline p) override
        {
            const Pipe* pipe = pipelines_.Find(p.id);
            if (!pipe)
                return;
            if (hostTouched_)
                ApplyPassState();
            ctx_->IASetInputLayout(pipe->layout.Get());
            ctx_->IASetPrimitiveTopology(pipe->topology);
            ctx_->VSSetShader(pipe->vs.Get(), nullptr, 0);
            ctx_->PSSetShader(pipe->ps.Get(), nullptr, 0);
            const float factor[4] = {};
            ctx_->OMSetBlendState(pipe->blend, factor, 0xFFFFFFFFu);
        }

        void SetScissor(const IRect& r) override
        {
            const D3D11_RECT s = {r.x0, r.y0, r.x1, r.y1};
            ctx_->RSSetScissorRects(1, &s);
        }

        void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) override
        {
            ID3D11Buffer* cb = cbs_[(int)slot].Get();
            D3D11_MAPPED_SUBRESOURCE m;
            if (!data || size == 0 || !log_.Check(ctx_->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &m), "Map (constants)"))
                return;
            std::memcpy(m.pData, data, std::min<std::uint32_t>(size, slot == ConstantSlot::Frame ? 192u : 32u));
            ctx_->Unmap(cb, 0);
        }

        void SetTexture(int slot, Texture tex) override
        {
            if (slot < 0 || slot >= kTextureSlots)
                return;
            const Tex* t = textures_.Find(tex.id);
            ID3D11ShaderResourceView* srv = t ? t->srv.Get() : nullptr;
            ctx_->PSSetShaderResources((UINT)slot, 1, &srv);
        }

        void SetFxBuffer(Buffer buf) override
        {
            const Buf* b = buffers_.Find(buf.id);
            ID3D11ShaderResourceView* srv = b ? b->srv.Get() : nullptr;
            // both stages fetch instances (FxVS for the bounds, FxPS for the material)
            ctx_->VSSetShaderResources(kSlotFxData, 1, &srv);
            ctx_->PSSetShaderResources(kSlotFxData, 1, &srv);
        }

        void SetVertexBuffer(Buffer buf) override
        {
            const Buf* b = buffers_.Find(buf.id);
            ID3D11Buffer* vb = b ? b->buf.Get() : nullptr;
            const UINT stride = 20, offset = 0;   // sizeof(esia::Vertex)
            ctx_->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        }

        void SetIndexBuffer(Buffer buf) override
        {
            const Buf* b = buffers_.Find(buf.id);
            ctx_->IASetIndexBuffer(b ? b->buf.Get() : nullptr, DXGI_FORMAT_R32_UINT, 0);
        }

        void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override { ctx_->Draw(vertexCount, firstVertex); }
        void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) override { ctx_->DrawIndexed(indexCount, firstIndex, 0); }
        void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) override { ctx_->DrawInstanced(vertexCount, instanceCount, 0, 0); }

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
                    ctx_->ResolveSubresource(d->tex.Get(), 0, s->tex.Get(), 0, s->resolveFormat);
                    return;
                }
                ComPtr<ID3D11Texture2D> tmp = Resolved(*s);
                if (!tmp)
                    return;
                const D3D11_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
                ctx_->CopySubresourceRegion(d->tex.Get(), 0, (UINT)dstX, (UINT)dstY, 0, tmp.Get(), 0, &box);
                return;
            }
            const D3D11_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            ctx_->CopySubresourceRegion(d->tex.Get(), 0, (UINT)dstX, (UINT)dstY, 0, s->tex.Get(), 0, &box);
        }

        void* NativeRenderState() override
        {
            hostTouched_ = true;   // the host may change anything: the pass state is applied again at the next pipeline
            return ctx_.Get();
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
            ComPtr<ID3D11Texture2D> src = t->tex;
            if (t->desc.samples > 1 && !(src = Resolved(*t)))
                return false;
            D3D11_TEXTURE2D_DESC sd = {};
            sd.Width = (UINT)r.Width();
            sd.Height = (UINT)r.Height();
            sd.MipLevels = sd.ArraySize = 1;
            sd.Format = d3d::DxgiRaw(t->resolveFormat);
            sd.SampleDesc.Count = 1;
            sd.Usage = D3D11_USAGE_STAGING;
            sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            ComPtr<ID3D11Texture2D> staging;
            if (!log_.Check(dev_->CreateTexture2D(&sd, nullptr, &staging), "CreateTexture2D (readback)"))
                return false;
            const D3D11_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            ctx_->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, src.Get(), 0, &box);
            D3D11_MAPPED_SUBRESOURCE m;
            if (!log_.Check(ctx_->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &m), "Map (readback)"))
                return false;
            d3d::ConvertToRgba8(t->desc.format, m.pData, m.RowPitch, r.Width(), r.Height(), rgba8);
            ctx_->Unmap(staging.Get(), 0);
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
            ID3D11RenderTargetView* rtv = pass_->rtv.Get();
            ctx_->OMSetRenderTargets(1, &rtv, nullptr);
            const D3D11_VIEWPORT vp = {0.0f, 0.0f, (float)pass_->desc.width, (float)pass_->desc.height, 0.0f, 1.0f};
            ctx_->RSSetViewports(1, &vp);
            const D3D11_RECT all = {0, 0, pass_->desc.width, pass_->desc.height};
            ctx_->RSSetScissorRects(1, &all);
            ctx_->RSSetState(raster_.Get());
            ctx_->OMSetDepthStencilState(depth_.Get(), 0);
            ID3D11SamplerState* s[2] = {samplers_[0].Get(), samplers_[1].Get()};
            ctx_->PSSetSamplers(0, 2, s);
            ID3D11Buffer* cbs[3] = {cbs_[0].Get(), cbs_[1].Get(), cbs_[2].Get()};
            ctx_->VSSetConstantBuffers(0, 3, cbs);
            ctx_->PSSetConstantBuffers(0, 3, cbs);
            ctx_->GSSetShader(nullptr, nullptr, 0);
            ctx_->HSSetShader(nullptr, nullptr, 0);
            ctx_->DSSetShader(nullptr, nullptr, 0);
        }

        // A single-sampled copy of a multisampled texture.
        ComPtr<ID3D11Texture2D> Resolved(const Tex& t)
        {
            D3D11_TEXTURE2D_DESC td = {};
            td.Width = (UINT)t.desc.width;
            td.Height = (UINT)t.desc.height;
            td.MipLevels = td.ArraySize = 1;
            td.Format = d3d::DxgiTypeless(t.resolveFormat);
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            ComPtr<ID3D11Texture2D> tmp;
            if (!log_.Check(dev_->CreateTexture2D(&td, nullptr, &tmp), "CreateTexture2D (resolve)"))
                return nullptr;
            ctx_->ResolveSubresource(tmp.Get(), 0, t.tex.Get(), 0, t.resolveFormat);
            return tmp;
        }

        ComPtr<ID3D11VertexShader> VertexShader(const d3d::Bytecode& code)
        {
            ComPtr<ID3D11VertexShader>& vs = vertexShaders_[code.get()];
            if (!vs)
                log_.Check(dev_->CreateVertexShader(code->data(), code->size(), nullptr, &vs), "CreateVertexShader");
            return vs;
        }

        ComPtr<ID3D11PixelShader> PixelShader(const d3d::Bytecode& code)
        {
            ComPtr<ID3D11PixelShader>& ps = pixelShaders_[code.get()];
            if (!ps)
                log_.Check(dev_->CreatePixelShader(code->data(), code->size(), nullptr, &ps), "CreatePixelShader");
            return ps;
        }

        int Stamp()
        {
            ProfileSlot& s = profile_[frame_ % kProfileSlots];
            if (s.used >= kMaxStamps)
                return -1;
            ctx_->End(s.stamps[s.used].Get());
            return s.used++;
        }

        void ReadSlot(ProfileSlot& s)
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dj = {};
            std::uint64_t ticks[kMaxStamps];
            if (ctx_->GetData(s.disjoint.Get(), &dj, sizeof(dj), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
                return;   // not yet: try again later
            for (int i = 0; i < s.used; ++i)
                if (ctx_->GetData(s.stamps[i].Get(), &ticks[i], sizeof(std::uint64_t), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
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
                auto* m = reinterpret_cast<D3D11_MESSAGE*>(buf.data());
                if (size == 0 || FAILED(info_->GetMessage(i, m, &size)))
                    continue;
                const LogLevel level = m->Severity <= D3D11_MESSAGE_SEVERITY_ERROR ? LogLevel::Error
                                       : m->Severity == D3D11_MESSAGE_SEVERITY_WARNING ? LogLevel::Warning
                                                                                         : LogLevel::Info;
                log_.Printf(level, "D3D11 debug layer: %.*s", (int)m->DescriptionByteLength, m->pDescription);
            }
            info_->ClearStoredMessages();
        }

        ComPtr<ID3D11Device> dev_;
        ComPtr<ID3D11DeviceContext> ctx_;
        bool restore_ = true;
        d3d::Logger log_;
        ComPtr<ID3D11InfoQueue> info_;
        Caps caps_;

        d3d::HandleTable<Tex> textures_;
        d3d::HandleTable<Buf> buffers_;
        d3d::HandleTable<Pipe> pipelines_;
        std::unordered_map<ID3D11RenderTargetView*, std::uint32_t> wrapped_;
        // shader objects per bytecode (the process-wide cache keeps the bytecode, and so these keys, alive)
        std::unordered_map<const std::vector<std::uint8_t>*, ComPtr<ID3D11VertexShader>> vertexShaders_;
        std::unordered_map<const std::vector<std::uint8_t>*, ComPtr<ID3D11PixelShader>> pixelShaders_;
        ComPtr<ID3D11InputLayout> uiLayout_;
        ComPtr<ID3D11BlendState> blends_[4];
        ComPtr<ID3D11RasterizerState> raster_;
        ComPtr<ID3D11DepthStencilState> depth_;
        ComPtr<ID3D11SamplerState> samplers_[2];
        ComPtr<ID3D11Buffer> cbs_[3];

        HostState host_;
        bool hostTouched_ = false;
        Tex* pass_ = nullptr;
        std::uint64_t frame_ = 0;
        ProfileSlot profile_[kProfileSlots];
        GpuProfile latest_;
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        auto dev = std::make_unique<D3D11Device>(desc);
        std::string e;
        if (!dev->Init(e))
        {
            if (error)
                *error = e;
            return nullptr;
        }
        return dev;
    }

    Texture WrapRenderTarget(Device& device, ID3D11RenderTargetView* rtv)
    {
        auto* d = dynamic_cast<D3D11Device*>(&device);
        return d ? d->Wrap(rtv) : Texture{};
    }

    // ------------------------------------------------------------------ headless (conformance suite)
    namespace
    {
        HeadlessDevice CreateHeadless(const HeadlessDesc& hd, std::string& error)
        {
            const int debug = d3d::DebugLevelFromEnvironment();
            const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
            const D3D_DRIVER_TYPE driver = d3d::UseWarpFromEnvironment() ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
            ComPtr<ID3D11Device> device;
            ComPtr<ID3D11DeviceContext> context;
            HRESULT hr = E_FAIL;
            for (int attempt = debug > 0 ? 0 : 1; attempt < 2 && FAILED(hr); ++attempt)
            {
                // the debug layer needs the Graphics Tools optional feature: without it, run without
                const UINT flags = attempt == 0 ? D3D11_CREATE_DEVICE_DEBUG : 0u;
                hr = D3D11CreateDevice(nullptr, driver, nullptr, flags, levels, 2, D3D11_SDK_VERSION, &device, nullptr, &context);
                if (FAILED(hr))   // runtimes before 11.1 reject the 11_1 level
                    hr = D3D11CreateDevice(nullptr, driver, nullptr, flags, levels + 1, 1, D3D11_SDK_VERSION, &device, nullptr, &context);
            }
            if (FAILED(hr))
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "D3D11CreateDevice failed: 0x%08lX (%s)", (unsigned long)hr, d3d::HResultName(hr));
                error = buf;
                return {};
            }
            Desc d;
            d.device = device.Get();
            d.context = context.Get();
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

void EsiaRegisterBackend_d3d11()
{
    esia::rhi::BackendInfo info;
    info.name = "d3d11";
    info.createHeadless = &esia::rhi::d3d11::CreateHeadless;
    esia::rhi::RegisterBackend(info);
}
