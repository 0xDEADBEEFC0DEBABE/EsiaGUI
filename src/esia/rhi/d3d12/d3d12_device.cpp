// Esia - Direct3D 12 backend (see esia/rhi/d3d12.hpp). The techniques are those of WGT's
// src/backends/d3d12_backend.cpp: recording into the host's command list, a root signature of root CBVs, an SRV
// table and static samplers, a shader-visible descriptor ring and an upload ring per frame in flight, a PSO per
// PipelineDesc, tracked resource states with barriers where the RHI implies them (render target <-> copy source /
// pixel-shader resource, copy destination -> pixel-shader resource), deferred releases, and timestamps resolved into
// a readback buffer per frame slot.
#include "esia/rhi/d3d12.hpp"
#include "d3d_shader.hpp"
#include "esia/rhi/backend_registry.hpp"
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esia::rhi::d3d12
{
    using d3d::ComPtr;
    using d3d::LogLevel;

    namespace
    {
        constexpr int kMaxStamps = 256;                // timestamps per frame
        constexpr UINT kCpuSrvCapacity = 4096;         // SRVs of live textures (non shader-visible)
        constexpr UINT kCpuRtvCapacity = 1024;         // RTVs of the device's own render targets
        constexpr UINT kTableDescriptorsPerFrame = 16384;
        constexpr UINT64 kUploadChunk = 4ull << 20;

        // Root signature: b0..b2 root CBVs, t0..t6 one SRV table, t7 a root SRV (StructuredBuffer<float4>)
        enum RootParam : UINT { RootFrame = 0, RootPass = 1, RootDraw = 2, RootTextures = 3, RootFxData = 4, RootCount = 5 };
        constexpr UINT kTableSlots = kSlotBackdrop0 + kBackdropLevels;   // t0 .. t6

        UINT64 AlignUp(UINT64 v, UINT64 a) { return (v + a - 1) & ~(a - 1); }

        D3D12_RESOURCE_BARRIER Transition(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
        {
            D3D12_RESOURCE_BARRIER b = {};
            b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            b.Transition.pResource = r;
            b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            b.Transition.StateBefore = from;
            b.Transition.StateAfter = to;
            return b;
        }

        D3D12_RESOURCE_DESC BufferResource(UINT64 size)
        {
            D3D12_RESOURCE_DESC d = {};
            d.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            d.Width = size;
            d.Height = d.DepthOrArraySize = d.MipLevels = 1;
            d.SampleDesc.Count = 1;
            d.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            return d;
        }

        // A linear allocator in persistently mapped upload-heap chunks; reset when its frame slot comes around.
        struct UploadRing
        {
            struct Chunk
            {
                ComPtr<ID3D12Resource> res;
                std::uint8_t* cpu = nullptr;
                UINT64 size = 0, used = 0;
            };
            struct Alloc
            {
                ID3D12Resource* res = nullptr;
                UINT64 offset = 0;
                std::uint8_t* cpu = nullptr;
                D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
            };
            std::vector<Chunk> chunks;
            std::size_t current = 0;

            void Reset()
            {
                for (Chunk& c : chunks)
                    c.used = 0;
                current = 0;
            }

            Alloc Allocate(ID3D12Device* dev, UINT64 size, UINT64 align)
            {
                for (; current < chunks.size(); ++current)
                {
                    Chunk& c = chunks[current];
                    const UINT64 at = AlignUp(c.used, align);
                    if (at + size <= c.size)
                    {
                        c.used = at + size;
                        return {c.res.Get(), at, c.cpu + at, c.res->GetGPUVirtualAddress() + at};
                    }
                }
                Chunk c;
                c.size = std::max(kUploadChunk, AlignUp(size + align, 65536));
                const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_UPLOAD};
                const D3D12_RESOURCE_DESC d = BufferResource(c.size);
                if (FAILED(dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &d, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&c.res))))
                    return {};
                const D3D12_RANGE none = {0, 0};
                if (FAILED(c.res->Map(0, &none, reinterpret_cast<void**>(&c.cpu))))
                    return {};
                c.used = size;
                chunks.push_back(std::move(c));
                current = chunks.size() - 1;
                Chunk& n = chunks.back();
                return {n.res.Get(), 0, n.cpu, n.res->GetGPUVirtualAddress()};
            }
        };

        // Fixed-size descriptor heap with a free list (CPU-only heaps: SRVs and RTVs of textures).
        struct DescriptorPool
        {
            ComPtr<ID3D12DescriptorHeap> heap;
            UINT inc = 0;
            std::vector<UINT> free;

            bool Init(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
            {
                D3D12_DESCRIPTOR_HEAP_DESC d = {};
                d.Type = type;
                d.NumDescriptors = count;
                if (FAILED(dev->CreateDescriptorHeap(&d, IID_PPV_ARGS(&heap))))
                    return false;
                inc = dev->GetDescriptorHandleIncrementSize(type);
                for (UINT i = count; i-- > 0;)
                    free.push_back(i);
                return true;
            }
            int Alloc()
            {
                if (free.empty())
                    return -1;
                const UINT i = free.back();
                free.pop_back();
                return (int)i;
            }
            void Free(int i)
            {
                if (i >= 0)
                    free.push_back((UINT)i);
            }
            D3D12_CPU_DESCRIPTOR_HANDLE Cpu(int i) const { return {heap->GetCPUDescriptorHandleForHeapStart().ptr + (SIZE_T)i * inc}; }
        };

        struct Tex
        {
            ComPtr<ID3D12Resource> res;
            TextureDesc desc;
            D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
            DXGI_FORMAT storage = DXGI_FORMAT_UNKNOWN, srvFormat = DXGI_FORMAT_UNKNOWN, rtvFormat = DXGI_FORMAT_UNKNOWN;
            DXGI_FORMAT resolveFormat = DXGI_FORMAT_UNKNOWN;   // typed format for ResolveSubresource
            int srv = -1, rtv = -1;                            // pool slots (the device's own views)
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
            bool wrapped = false;
            D3D12_RESOURCE_STATES hostState = D3D12_RESOURCE_STATE_COMMON;
        };

        struct Buf
        {
            BufferDesc desc;
            std::vector<std::uint8_t> data;   // what UpdateBuffer wrote; copied into the frame's upload ring when bound
            std::size_t valid = 0;
            std::uint64_t frame = 0;          // device frame of `gpu`
            D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
        };

        struct Pipe
        {
            ComPtr<ID3D12PipelineState> pso;
            D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            PipelineDesc desc;
        };

        // Work recorded at the start of the next command list: uploads of textures created / updated outside a frame
        // and the first clear of new render targets.
        struct PendingOp
        {
            std::uint32_t tex = 0;
            IRect rect;
            std::vector<std::uint8_t> pixels;   // tightly packed; empty = clear to transparent black
        };

        struct FrameSlot
        {
            UploadRing ring;
            UINT tableOffset = 0;
            std::vector<ComPtr<IUnknown>> garbage;     // released when the slot is reused
            std::vector<int> freeSrvs, freeRtvs;       // descriptors of destroyed textures, freed then too
            ComPtr<ID3D12CommandAllocator> allocator;  // own-list frames
            UINT64 fence = 0;                          // value signaled after the slot's own-list frame
            int stamps = 0;
            bool profilePending = false;
            d3d::ProfileFrame profile;
        };

        Caps MakeCaps()
        {
            Caps c;
            c.fxStorage = FxStorage::Buffer;
            c.shaderFormat = (std::uint8_t)shaders::Format::DxbcSm5;
            c.dualSourceBlend = true;
            c.floatRenderTargets = true;
            c.sampleRenderTarget = true;
            c.timestampQueries = true;
            c.readback = true;
            c.runtimeEffects = true;
            c.fxFeatureVariants = false;   // the whole FX shader fits SM5
            c.maxTextureSize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            c.maxFxDataWidth = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            return c;
        }
    }

    class D3D12Device final : public Device
    {
    public:
        explicit D3D12Device(const Desc& d)
            : dev_(d.device), queue_(d.queue), log_(d.debug), caps_(MakeCaps()), slots_((std::size_t)std::max(1, d.framesInFlight))
        {
        }

        ~D3D12Device() override { WaitIdle(); }

        bool Init(std::string& error)
        {
            if (!dev_)
            {
                error = "no ID3D12Device";
                return false;
            }
            if (!d3d::ShaderCompilerAvailable(error))
                return false;
            if (log_.DebugLayer() && FAILED(dev_.As(info_)))
                log_.Log(LogLevel::Warning, "debug messages requested, but the device has no ID3D12InfoQueue (debug layer not enabled?)");
            bool ok = CreateRootSignature();
            ok = ok && srvs_.Init(dev_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kCpuSrvCapacity);
            ok = ok && rtvs_.Init(dev_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kCpuRtvCapacity);
            if (ok)
            {
                D3D12_DESCRIPTOR_HEAP_DESC d = {};
                d.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                d.NumDescriptors = kTableDescriptorsPerFrame * (UINT)slots_.size();
                d.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                ok = log_.Check(dev_->CreateDescriptorHeap(&d, IID_PPV_ARGS(&tables_)), "CreateDescriptorHeap (tables)");
            }
            if (ok)
            {
                // unbound table slots read a null view (every declared texture must be valid, rhi.hpp: the renderer
                // binds white, this only covers slots of programs that do not declare them)
                nullSrv_ = srvs_.Alloc();
                D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                sv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                sv.Texture2D.MipLevels = 1;
                dev_->CreateShaderResourceView(nullptr, &sv, srvs_.Cpu(nullSrv_));
            }
            if (ok && queue_)
            {
                ok = log_.Check(dev_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)), "CreateFence");
                event_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                for (FrameSlot& s : slots_)
                    ok = ok && log_.Check(dev_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&s.allocator)), "CreateCommandAllocator");
                ok = ok && log_.Check(dev_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&immediateAllocator_)), "CreateCommandAllocator");
                ok = ok && log_.Check(dev_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, immediateAllocator_.Get(), nullptr, IID_PPV_ARGS(&ownList_)),
                                      "CreateCommandList");
                if (ok)
                    ownList_->Close();
            }
            if (ok)
                InitTimestamps();
            caps_.readback = (bool)queue_;
            DrainMessages();
            if (!ok)
                error = "creating the D3D12 device objects failed";
            return ok;
        }

        const char* Name() const override { return "d3d12"; }
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
            if (desc.samples > 1)
            {
                D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS q = {};
                q.Format = f.rtv;
                q.SampleCount = (UINT)desc.samples;
                if (!rt || FAILED(dev_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &q, sizeof(q))) || q.NumQualityLevels == 0)
                {
                    log_.Printf(LogLevel::Error, "CreateTexture: %d samples of %s are not supported", desc.samples, FormatName(desc.format));
                    return {};
                }
            }
            Tex t;
            t.desc = desc;
            t.desc.debugName = nullptr;
            if (desc.samples > 1)
                t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;   // Texture2DMS is not in the binding model
            t.storage = f.storage;
            t.srvFormat = f.srv;
            t.rtvFormat = t.resolveFormat = f.rtv;
            D3D12_RESOURCE_DESC rd = {};
            rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            rd.Width = (UINT64)desc.width;
            rd.Height = (UINT)desc.height;
            rd.DepthOrArraySize = rd.MipLevels = 1;
            rd.Format = f.storage;
            rd.SampleDesc.Count = (UINT)desc.samples;
            rd.Flags = rt ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
            D3D12_CLEAR_VALUE clear = {};
            clear.Format = f.rtv;
            // render targets start as render targets (and are cleared first), the others as copy destinations
            t.state = rt ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_COPY_DEST;
            const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
            if (!log_.Check(dev_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, t.state, rt ? &clear : nullptr, IID_PPV_ARGS(&t.res)),
                            "CreateCommittedResource (texture)"))
            {
                DrainMessages();
                return {};
            }
            if (!CreateViews(t))
            {
                ReleaseViews(t, nullptr);
                return {};
            }
            SetDebugName(t.res.Get(), desc.debugName);
            const std::uint32_t id = textures_.Add(std::move(t));
            if (data)
            {
                PendingOp op;
                op.tex = id;
                op.rect = IRect{0, 0, desc.width, desc.height};
                op.pixels = Packed(data, rowPitch, desc.width, desc.height, BytesPerPixel(desc.format));
                Schedule(std::move(op));
            }
            else if (rt)
            {
                PendingOp op;
                op.tex = id;
                Schedule(std::move(op));   // start transparent black: a Load pass over fresh contents is deterministic
            }
            return Texture{id};
        }

        Texture Wrap(ID3D12Resource* res, D3D12_CPU_DESCRIPTOR_HANDLE rtv, DXGI_FORMAT format, D3D12_RESOURCE_STATES state)
        {
            if (!res)
                return {};
            auto it = wrapped_.find(res);
            if (it != wrapped_.end())
            {
                // the host may hand in another view of the same buffer: keep the latest
                Tex* t = textures_.Find(it->second);
                if (t && t->rtvHandle.ptr == rtv.ptr && t->rtvFormat == format && t->hostState == state)
                    return Texture{it->second};
                DestroyTexture(Texture{it->second});
            }
            const D3D12_RESOURCE_DESC rd = res->GetDesc();
            Tex t;
            t.desc.format = d3d::FormatFromDxgi(format);
            if (rd.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || t.desc.format == Format::Unknown)
            {
                log_.Printf(LogLevel::Error, "WrapRenderTarget: not a 2D texture in a format the renderer can target (DXGI %d)", (int)format);
                return {};
            }
            t.res = res;
            t.wrapped = true;
            t.desc.width = (int)rd.Width;
            t.desc.height = (int)rd.Height;
            t.desc.samples = (int)rd.SampleDesc.Count;
            const bool sampleable = !(rd.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) && rd.SampleDesc.Count == 1 && !d3d::DxgiIsSrgb(rd.Format);
            t.desc.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (sampleable ? TextureUsage_Sampled : 0u);
            t.storage = rd.Format;
            t.srvFormat = d3d::DxgiRaw(format);
            t.rtvFormat = format;
            const bool typeless = d3d::DxgiRaw(rd.Format) != rd.Format && !d3d::DxgiIsSrgb(rd.Format);
            t.resolveFormat = typeless ? format : rd.Format;
            t.rtvHandle = rtv;
            t.state = t.hostState = state;
            if (sampleable && !CreateViews(t))
                return {};
            const std::uint32_t id = textures_.Add(std::move(t));
            wrapped_[res] = id;
            return Texture{id};
        }

        void UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch) override
        {
            Tex* t = textures_.Find(tex.id);
            if (!t || !data || r.Empty())
                return;
            PendingOp op;
            op.tex = tex.id;
            op.rect = r;
            op.pixels = Packed(data, rowPitch, r.Width(), r.Height(), BytesPerPixel(t->desc.format));
            Schedule(std::move(op));
        }

        void DestroyTexture(Texture tex) override
        {
            Tex* t = textures_.Find(tex.id);
            if (!t)
                return;
            if (t->wrapped)
                wrapped_.erase(t->res.Get());
            FrameSlot& s = Slot();
            ReleaseViews(*t, &s);
            s.garbage.push_back(ComPtr<IUnknown>(t->res.Get()));
            std::erase_if(pending_, [&](const PendingOp& op) { return op.tex == tex.id; });
            textures_.Remove(tex.id);
        }

        TextureDesc GetTextureDesc(Texture tex) const override
        {
            const Tex* t = textures_.Find(tex.id);
            return t ? t->desc : TextureDesc{};
        }

        Buffer CreateBuffer(const BufferDesc& desc) override
        {
            if (desc.size == 0)
                return {};
            Buf b;
            b.desc = desc;
            b.data.resize(desc.size);
            return Buffer{buffers_.Add(std::move(b))};
        }

        void UpdateBuffer(Buffer buf, const void* data, std::size_t size) override
        {
            Buf* b = buffers_.Find(buf.id);
            if (!b || !data || size == 0 || size > b->desc.size)
                return;
            std::memcpy(b->data.data(), data, size);
            b->valid = std::max(b->valid, size);
            b->frame = 0;   // copy it into the ring again at the next bind
        }

        void DestroyBuffer(Buffer buf) override { buffers_.Remove(buf.id); }

        Pipeline CreatePipeline(const PipelineDesc& desc) override
        {
            if (desc.program >= ShaderProgram::Count || (desc.blend == BlendMode::DualSourceLcd) != (desc.program == ShaderProgram::TextLcd))
                return {};
            const d3d::DxgiFormats f = d3d::DxgiFormatsOf(desc.targetFormat);
            if (f.rtv == DXGI_FORMAT_UNKNOWN)
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

            D3D12_GRAPHICS_PIPELINE_STATE_DESC d = {};
            d.pRootSignature = root_.Get();
            d.VS = {vs->data(), vs->size()};
            d.PS = {ps->data(), ps->size()};
            D3D12_RENDER_TARGET_BLEND_DESC& b = d.BlendState.RenderTarget[0];
            b.BlendOp = b.BlendOpAlpha = D3D12_BLEND_OP_ADD;
            b.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            b.LogicOp = D3D12_LOGIC_OP_NOOP;
            switch (desc.blend)
            {
            case BlendMode::Opaque:
                b.SrcBlend = b.SrcBlendAlpha = D3D12_BLEND_ONE;
                b.DestBlend = b.DestBlendAlpha = D3D12_BLEND_ZERO;
                break;
            case BlendMode::Straight:
                b.BlendEnable = TRUE;
                b.SrcBlend = D3D12_BLEND_SRC_ALPHA;
                b.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                b.SrcBlendAlpha = D3D12_BLEND_ONE;
                b.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
                break;
            case BlendMode::Premultiplied:
                b.BlendEnable = TRUE;
                b.SrcBlend = b.SrcBlendAlpha = D3D12_BLEND_ONE;
                b.DestBlend = b.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
                break;
            case BlendMode::DualSourceLcd:
                b.BlendEnable = TRUE;
                b.SrcBlend = D3D12_BLEND_SRC1_COLOR;
                b.DestBlend = D3D12_BLEND_INV_SRC1_COLOR;
                b.SrcBlendAlpha = D3D12_BLEND_SRC1_ALPHA;
                b.DestBlendAlpha = D3D12_BLEND_INV_SRC1_ALPHA;
                break;
            }
            d.SampleMask = 0xFFFFFFFFu;
            d.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            d.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            d.RasterizerState.DepthClipEnable = TRUE;
            d.DepthStencilState.DepthEnable = FALSE;
            d.DepthStencilState.StencilEnable = FALSE;
            // esia::Vertex: float2 pos, float2 uv, RGBA8 color
            const D3D12_INPUT_ELEMENT_DESC ui[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            };
            if (desc.layout == VertexLayout::UiVertex)
                d.InputLayout = {ui, 3};
            d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            d.NumRenderTargets = 1;
            d.RTVFormats[0] = f.rtv;
            d.SampleDesc.Count = (UINT)desc.samples;
            Pipe p;
            p.desc = desc;
            p.desc.effectSource = nullptr;
            p.topology = desc.topology == Topology::TriangleStrip ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            if (!log_.Check(dev_->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&p.pso)), "CreateGraphicsPipelineState"))
            {
                DrainMessages();
                return {};
            }
            return Pipeline{pipelines_.Add(std::move(p))};
        }

        void DestroyPipeline(Pipeline p) override
        {
            if (Pipe* pipe = pipelines_.Find(p.id))
                Slot().garbage.push_back(ComPtr<IUnknown>(pipe->pso.Get()));
            pipelines_.Remove(p.id);
        }

        // ------------------------------------------------------------------ frame
        bool BeginFrame(const FrameDesc& desc) override
        {
            ownFrame_ = desc.nativeContext == nullptr;
            if (ownFrame_ && !queue_)
            {
                log_.Log(LogLevel::Error, "BeginFrame: no command list (FrameDesc::nativeContext) and no queue (Desc::queue)");
                return false;
            }
            ++frame_;
            FrameSlot& s = Slot();
            if (ownFrame_)
                Wait(s.fence);   // the slot's last own-list frame; host lists: the host waited (framesInFlight)
            RecycleSlot(s);
            if (ownFrame_)
            {
                s.allocator->Reset();
                ownList_->Reset(s.allocator.Get(), nullptr);
                cl_ = ownList_.Get();
            }
            else
                cl_ = static_cast<ID3D12GraphicsCommandList*>(desc.nativeContext);
            s.profile.Reset(frame_);
            s.stamps = 0;
            if (caps_.timestampQueries)
                s.profile.frameStart = Stamp();
            FlushPending(cl_, s.ring);
            return true;
        }

        void EndFrame() override
        {
            FrameSlot& s = Slot();
            if (caps_.timestampQueries)
            {
                s.profile.frameEnd = Stamp();
                const UINT base = SlotIndex() * kMaxStamps;
                cl_->ResolveQueryData(queries_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base, (UINT)s.stamps, timestamps_.Get(), (UINT64)base * sizeof(std::uint64_t));
                s.profilePending = s.profile.frameEnd > 0;
            }
            // wrapped targets go back to the state the host handed them in
            textures_.ForEach([&](std::uint32_t, Tex& t) {
                if (t.wrapped)
                    Transition(cl_, t, t.hostState);
            });
            if (ownFrame_)
            {
                if (log_.Check(ownList_->Close(), "Close (command list)"))
                {
                    ID3D12CommandList* lists[] = {ownList_.Get()};
                    queue_->ExecuteCommandLists(1, lists);
                    s.fence = Signal();
                }
            }
            cl_ = nullptr;
            pass_ = nullptr;
            DrainMessages();
        }

        // ------------------------------------------------------------------ commands
        void BeginPass(const PassDesc& desc) override
        {
            Tex* t = textures_.Find(desc.target.id);
            if (!t)
                return;
            Transition(cl_, *t, D3D12_RESOURCE_STATE_RENDER_TARGET);
            pass_ = t;
            ApplyPassState();
            if (desc.load == LoadOp::Clear)
            {
                float c[4];
                d3d::ClearValueFor(t->desc.format, desc.clearColor, c);
                cl_->ClearRenderTargetView(RtvOf(*t), c, 0, nullptr);
            }
            // LoadOp::DontCare: nothing (no render-pass API on this path: DiscardResource buys nothing on desktop GPUs)
        }

        void EndPass() override { pass_ = nullptr; }

        void SetPipeline(Pipeline p) override
        {
            const Pipe* pipe = pipelines_.Find(p.id);
            if (!pipe)
                return;
            if (hostTouched_)
                ApplyPassState();
            cl_->SetPipelineState(pipe->pso.Get());
            cl_->IASetPrimitiveTopology(pipe->topology);
        }

        void SetScissor(const IRect& r) override
        {
            const D3D12_RECT s = {r.x0, r.y0, r.x1, r.y1};
            cl_->RSSetScissorRects(1, &s);
        }

        void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) override
        {
            if (!data || size == 0)
                return;
            // a fresh 256-byte aligned copy per call: every draw sees the values set last
            const UploadRing::Alloc a = Slot().ring.Allocate(dev_.Get(), size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
            if (!a.cpu)
                return;
            std::memcpy(a.cpu, data, size);
            cl_->SetGraphicsRootConstantBufferView((UINT)slot, a.gpu);
        }

        void SetTexture(int slot, Texture tex) override
        {
            if (slot < 0 || slot >= (int)kTableSlots)
                return;
            Tex* t = textures_.Find(tex.id);
            if (t && t->srv >= 0)
            {
                // sampled after its pass (rhi.hpp): a render target or copy destination becomes a shader resource here
                Transition(cl_, *t, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                bound_[slot] = t->srv;
            }
            else
                bound_[slot] = nullSrv_;
            tableDirty_ = true;
        }

        void SetFxBuffer(Buffer buf) override
        {
            if (Buf* b = buffers_.Find(buf.id))
                if (const D3D12_GPU_VIRTUAL_ADDRESS va = Upload(*b))
                    cl_->SetGraphicsRootShaderResourceView(RootFxData, va);
        }

        void SetVertexBuffer(Buffer buf) override
        {
            Buf* b = buffers_.Find(buf.id);
            if (!b)
                return;
            const D3D12_VERTEX_BUFFER_VIEW v = {Upload(*b), (UINT)b->valid, 20};   // sizeof(esia::Vertex)
            cl_->IASetVertexBuffers(0, 1, &v);
        }

        void SetIndexBuffer(Buffer buf) override
        {
            Buf* b = buffers_.Find(buf.id);
            if (!b)
                return;
            const D3D12_INDEX_BUFFER_VIEW v = {Upload(*b), (UINT)b->valid, DXGI_FORMAT_R32_UINT};
            cl_->IASetIndexBuffer(&v);
        }

        void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override
        {
            if (FlushTable())
                cl_->DrawInstanced(vertexCount, 1, firstVertex, 0);
        }

        void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) override
        {
            if (FlushTable())
                cl_->DrawIndexedInstanced(indexCount, 1, firstIndex, 0, 0);
        }

        void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) override
        {
            if (FlushTable())
                cl_->DrawInstanced(vertexCount, instanceCount, 0, 0);
        }

        void CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r) override
        {
            Tex* d = textures_.Find(dst.id);
            Tex* s = textures_.Find(src.id);
            if (!d || !s || r.Empty())
                return;
            if (s->desc.samples > 1)
            {
                // ResolveSubresource has no region: resolve the whole target straight into a destination of the same
                // size at the same place (the backdrop copy: texels outside the region get the same, newer source
                // texels, which nothing reads), else through a temporary
                if (d->desc.width == s->desc.width && d->desc.height == s->desc.height && dstX == r.x0 && dstY == r.y0)
                {
                    Transition(cl_, *s, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
                    Transition(cl_, *d, D3D12_RESOURCE_STATE_RESOLVE_DEST);
                    cl_->ResolveSubresource(d->res.Get(), 0, s->res.Get(), 0, s->resolveFormat);
                    return;
                }
                ComPtr<ID3D12Resource> tmp = Resolved(cl_, *s);
                if (!tmp)
                    return;
                Transition(cl_, *d, D3D12_RESOURCE_STATE_COPY_DEST);
                CopyRegion(cl_, d->res.Get(), dstX, dstY, tmp.Get(), r);
                Slot().garbage.push_back(ComPtr<IUnknown>(tmp.Get()));
                return;
            }
            Transition(cl_, *s, D3D12_RESOURCE_STATE_COPY_SOURCE);
            Transition(cl_, *d, D3D12_RESOURCE_STATE_COPY_DEST);
            CopyRegion(cl_, d->res.Get(), dstX, dstY, s->res.Get(), r);
        }

        void* NativeRenderState() override
        {
            hostTouched_ = true;   // the host may change anything: the pass state is applied again at the next pipeline
            return cl_;
        }

        // ------------------------------------------------------------------ profiling
        void BeginProfile(ProfileCategory category) override
        {
            if (!caps_.timestampQueries)
                return;
            d3d::ProfileFrame& f = Slot().profile;
            f.openStart = Stamp();
            f.openCategory = category;
        }

        void EndProfile() override
        {
            if (!caps_.timestampQueries)
                return;
            d3d::ProfileFrame& f = Slot().profile;
            const int end = Stamp();
            if (f.openStart >= 0 && end >= 0)
                f.intervals.push_back({f.openStart, end, f.openCategory});
            f.openStart = -1;
        }

        bool ReadProfile(GpuProfile& out) override
        {
            // own-list frames: every slot whose fence passed; host lists: slots are read when they are reused
            if (queue_ && fence_)
            {
                const UINT64 done = fence_->GetCompletedValue();
                for (FrameSlot& s : slots_)
                    if (s.profilePending && s.fence != 0 && s.fence <= done)
                        ReadTimestamps(s);
            }
            out = latest_;
            return latest_.valid;
        }

        // ------------------------------------------------------------------ readback
        bool ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& rgba8) override
        {
            rgba8.clear();
            Tex* t = textures_.Find(tex.id);
            if (!queue_ || cl_ || !t || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height)
                return false;
            const int bpp = BytesPerPixel(t->desc.format);
            const UINT pitch = (UINT)AlignUp((UINT64)r.Width() * (UINT64)bpp, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
            const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
            const D3D12_RESOURCE_DESC bd = BufferResource((UINT64)pitch * (UINT64)r.Height());
            ComPtr<ID3D12Resource> readback;
            if (!log_.Check(dev_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)),
                            "CreateCommittedResource (readback)"))
                return false;
            UploadRing ring;
            ID3D12GraphicsCommandList* cl = BeginImmediate();
            FlushPending(cl, ring);
            ComPtr<ID3D12Resource> src = t->res;
            IRect sr = r;
            if (t->desc.samples > 1)
                src = Resolved(cl, *t);
            else
                Transition(cl, *t, D3D12_RESOURCE_STATE_COPY_SOURCE);
            if (!src)
                return EndImmediate(), false;
            D3D12_TEXTURE_COPY_LOCATION dl = {};
            dl.pResource = readback.Get();
            dl.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dl.PlacedFootprint.Footprint = {t->desc.samples > 1 ? d3d::DxgiTypeless(t->resolveFormat) : t->storage, (UINT)r.Width(), (UINT)r.Height(), 1, pitch};
            D3D12_TEXTURE_COPY_LOCATION sl = {};
            sl.pResource = src.Get();
            sl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            const D3D12_BOX box = {(UINT)sr.x0, (UINT)sr.y0, 0, (UINT)sr.x1, (UINT)sr.y1, 1};
            cl->CopyTextureRegion(&dl, 0, 0, 0, &sl, &box);
            if (t->wrapped)
                Transition(cl, *t, t->hostState);
            EndImmediate();
            void* p = nullptr;
            const D3D12_RANGE range = {0, (SIZE_T)bd.Width};
            if (!log_.Check(readback->Map(0, &range, &p), "Map (readback)"))
                return false;
            d3d::ConvertToRgba8(t->desc.format, p, pitch, r.Width(), r.Height(), rgba8);
            const D3D12_RANGE none = {0, 0};
            readback->Unmap(0, &none);
            DrainMessages();
            return true;
        }

    private:
        // ------------------------------------------------------------------ setup
        bool CreateRootSignature()
        {
            D3D12_DESCRIPTOR_RANGE range = {};
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = kTableSlots;
            range.BaseShaderRegister = 0;
            D3D12_ROOT_PARAMETER params[RootCount] = {};
            for (UINT i = 0; i < 3; ++i)
            {
                params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                params[i].Descriptor.ShaderRegister = i;
                params[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }
            params[RootTextures].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[RootTextures].DescriptorTable.NumDescriptorRanges = 1;
            params[RootTextures].DescriptorTable.pDescriptorRanges = &range;
            params[RootTextures].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            params[RootFxData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;   // both stages read the instances
            params[RootFxData].Descriptor.ShaderRegister = kSlotFxData;
            params[RootFxData].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            D3D12_STATIC_SAMPLER_DESC samplers[2] = {};
            for (UINT i = 0; i < 2; ++i)
            {
                samplers[i].Filter = i == 0 ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
                samplers[i].AddressU = samplers[i].AddressV = samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
                samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                samplers[i].MaxLOD = 0.0f;
                samplers[i].ShaderRegister = i;
                samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            }
            D3D12_ROOT_SIGNATURE_DESC desc = {};
            desc.NumParameters = RootCount;
            desc.pParameters = params;
            desc.NumStaticSamplers = 2;
            desc.pStaticSamplers = samplers;
            desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                         D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            ComPtr<ID3DBlob> blob, err;
            if (FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err)))
            {
                log_.Printf(LogLevel::Error, "D3D12SerializeRootSignature: %s", err ? static_cast<const char*>(err->GetBufferPointer()) : "failed");
                return false;
            }
            return log_.Check(dev_->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_)), "CreateRootSignature");
        }

        void InitTimestamps()
        {
            // the frequency belongs to the engine: the host's queue, or a throwaway direct queue
            ComPtr<ID3D12CommandQueue> q = queue_;
            if (!q)
            {
                D3D12_COMMAND_QUEUE_DESC qd = {};
                qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                dev_->CreateCommandQueue(&qd, IID_PPV_ARGS(&q));
            }
            D3D12_QUERY_HEAP_DESC hd = {};
            hd.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            hd.Count = (UINT)slots_.size() * kMaxStamps;
            const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
            const D3D12_RESOURCE_DESC bd = BufferResource((UINT64)hd.Count * sizeof(std::uint64_t));
            caps_.timestampQueries = q && SUCCEEDED(q->GetTimestampFrequency(&frequency_)) && frequency_ != 0 &&
                                     SUCCEEDED(dev_->CreateQueryHeap(&hd, IID_PPV_ARGS(&queries_))) &&
                                     SUCCEEDED(dev_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                                             IID_PPV_ARGS(&timestamps_)));
        }

        bool CreateViews(Tex& t)
        {
            if (t.desc.usage & TextureUsage_Sampled)
            {
                t.srv = srvs_.Alloc();
                if (t.srv < 0)
                {
                    log_.Log(LogLevel::Error, "CreateTexture: out of SRV descriptors");
                    return false;
                }
                D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = t.srvFormat;
                sv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                sv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                sv.Texture2D.MipLevels = 1;
                dev_->CreateShaderResourceView(t.res.Get(), &sv, srvs_.Cpu(t.srv));
            }
            if ((t.desc.usage & TextureUsage_RenderTarget) && !t.wrapped)
            {
                t.rtv = rtvs_.Alloc();
                if (t.rtv < 0)
                {
                    log_.Log(LogLevel::Error, "CreateTexture: out of RTV descriptors");
                    return false;
                }
                D3D12_RENDER_TARGET_VIEW_DESC rv = {};
                rv.Format = t.rtvFormat;
                rv.ViewDimension = t.desc.samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;
                t.rtvHandle = rtvs_.Cpu(t.rtv);
                dev_->CreateRenderTargetView(t.res.Get(), &rv, t.rtvHandle);
            }
            return true;
        }

        // Descriptors go back to their pools when the GPU is done with the frame (slot), or now.
        void ReleaseViews(Tex& t, FrameSlot* s)
        {
            if (t.srv >= 0)
                s ? s->freeSrvs.push_back(t.srv) : srvs_.Free(t.srv);
            if (t.rtv >= 0)
                s ? s->freeRtvs.push_back(t.rtv) : rtvs_.Free(t.rtv);
            t.srv = t.rtv = -1;
        }

        // ------------------------------------------------------------------ frames and submissions
        FrameSlot& Slot() { return slots_[SlotIndex()]; }
        UINT SlotIndex() const { return (UINT)(frame_ % slots_.size()); }

        void RecycleSlot(FrameSlot& s)
        {
            if (s.profilePending)
                ReadTimestamps(s);
            s.garbage.clear();
            for (int i : s.freeSrvs)
                srvs_.Free(i);
            for (int i : s.freeRtvs)
                rtvs_.Free(i);
            s.freeSrvs.clear();
            s.freeRtvs.clear();
            s.ring.Reset();
            s.tableOffset = 0;
        }

        UINT64 Signal()
        {
            queue_->Signal(fence_.Get(), ++fenceValue_);
            return fenceValue_;
        }

        void Wait(UINT64 value)
        {
            if (!fence_ || value == 0 || fence_->GetCompletedValue() >= value)
                return;
            fence_->SetEventOnCompletion(value, event_);
            WaitForSingleObject(event_, INFINITE);
        }

        void WaitIdle()
        {
            if (queue_ && fence_)
                Wait(Signal());
            if (event_)
                CloseHandle(event_);
            event_ = nullptr;
        }

        // A one-off list on the device's queue (readback, and the uploads it needs first), executed synchronously.
        ID3D12GraphicsCommandList* BeginImmediate()
        {
            immediateAllocator_->Reset();
            ownList_->Reset(immediateAllocator_.Get(), nullptr);
            return ownList_.Get();
        }

        void EndImmediate()
        {
            if (!log_.Check(ownList_->Close(), "Close (immediate list)"))
                return;
            ID3D12CommandList* lists[] = {ownList_.Get()};
            queue_->ExecuteCommandLists(1, lists);
            Wait(Signal());
        }

        void Schedule(PendingOp&& op)
        {
            if (cl_)
            {
                // inside a frame (before its first pass): record now
                std::vector<PendingOp> one;
                one.push_back(std::move(op));
                Record(cl_, Slot().ring, one);
            }
            else
                pending_.push_back(std::move(op));
        }

        void FlushPending(ID3D12GraphicsCommandList* cl, UploadRing& ring)
        {
            if (pending_.empty())
                return;
            std::vector<PendingOp> ops;
            ops.swap(pending_);
            Record(cl, ring, ops);
        }

        void Record(ID3D12GraphicsCommandList* cl, UploadRing& ring, std::vector<PendingOp>& ops)
        {
            for (PendingOp& op : ops)
            {
                Tex* t = textures_.Find(op.tex);
                if (!t)
                    continue;
                if (op.pixels.empty())
                {
                    Transition(cl, *t, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    const float zero[4] = {};
                    cl->ClearRenderTargetView(RtvOf(*t), zero, 0, nullptr);
                    continue;
                }
                const int bpp = BytesPerPixel(t->desc.format);
                const UINT w = (UINT)op.rect.Width(), h = (UINT)op.rect.Height();
                const UINT pitch = (UINT)AlignUp((UINT64)w * (UINT64)bpp, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
                const UploadRing::Alloc a = ring.Allocate(dev_.Get(), (UINT64)pitch * h, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                if (!a.cpu)
                    continue;
                for (UINT y = 0; y < h; ++y)
                    std::memcpy(a.cpu + (std::size_t)pitch * y, op.pixels.data() + (std::size_t)w * (std::size_t)bpp * y, (std::size_t)w * (std::size_t)bpp);
                Transition(cl, *t, D3D12_RESOURCE_STATE_COPY_DEST);
                D3D12_TEXTURE_COPY_LOCATION dl = {};
                dl.pResource = t->res.Get();
                dl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                D3D12_TEXTURE_COPY_LOCATION sl = {};
                sl.pResource = a.res;
                sl.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                sl.PlacedFootprint.Offset = a.offset;
                sl.PlacedFootprint.Footprint = {t->storage, w, h, 1, pitch};
                cl->CopyTextureRegion(&dl, (UINT)op.rect.x0, (UINT)op.rect.y0, 0, &sl, nullptr);
            }
        }

        static std::vector<std::uint8_t> Packed(const void* data, int rowPitch, int w, int h, int bpp)
        {
            const std::size_t row = (std::size_t)w * (std::size_t)bpp, pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            std::vector<std::uint8_t> out(row * (std::size_t)h);
            for (int y = 0; y < h; ++y)
                std::memcpy(out.data() + row * (std::size_t)y, static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
            return out;
        }

        // The buffer's bytes in this frame's upload ring (copied once per frame, at the first bind).
        D3D12_GPU_VIRTUAL_ADDRESS Upload(Buf& b)
        {
            if (b.frame == frame_ && b.gpu)
                return b.gpu;
            const UploadRing::Alloc a = Slot().ring.Allocate(dev_.Get(), std::max<std::size_t>(b.valid, 16), 256);
            if (!a.cpu)
                return 0;
            std::memcpy(a.cpu, b.data.data(), b.valid);
            b.frame = frame_;
            b.gpu = a.gpu;
            return a.gpu;
        }

        // ------------------------------------------------------------------ state
        void Transition(ID3D12GraphicsCommandList* cl, Tex& t, D3D12_RESOURCE_STATES to)
        {
            if (t.state == to)
                return;
            const D3D12_RESOURCE_BARRIER b = d3d12::Transition(t.res.Get(), t.state, to);
            cl->ResourceBarrier(1, &b);
            t.state = to;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE RtvOf(const Tex& t) const { return t.rtvHandle; }

        // Everything a pass relies on (and host callbacks may change).
        void ApplyPassState()
        {
            hostTouched_ = false;
            if (!pass_)
                return;
            const D3D12_CPU_DESCRIPTOR_HANDLE rtv = RtvOf(*pass_);
            cl_->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            const D3D12_VIEWPORT vp = {0.0f, 0.0f, (float)pass_->desc.width, (float)pass_->desc.height, 0.0f, 1.0f};
            cl_->RSSetViewports(1, &vp);
            const D3D12_RECT all = {0, 0, pass_->desc.width, pass_->desc.height};
            cl_->RSSetScissorRects(1, &all);
            ID3D12DescriptorHeap* heaps[] = {tables_.Get()};
            cl_->SetDescriptorHeaps(1, heaps);
            cl_->SetGraphicsRootSignature(root_.Get());
            for (int& b : bound_)
                b = nullSrv_;
            tableDirty_ = true;
        }

        // The t0..t6 table of the next draw: a fresh copy in the frame's part of the shader-visible heap when a
        // binding changed since the last draw.
        bool FlushTable()
        {
            if (!tableDirty_)
                return true;
            FrameSlot& s = Slot();
            if (s.tableOffset + kTableSlots > kTableDescriptorsPerFrame)
            {
                log_.Log(LogLevel::Error, "out of shader-visible descriptors this frame: draw skipped");
                return false;
            }
            const UINT inc = srvs_.inc;
            const UINT first = SlotIndex() * kTableDescriptorsPerFrame + s.tableOffset;
            s.tableOffset += kTableSlots;
            D3D12_CPU_DESCRIPTOR_HANDLE dst = {tables_->GetCPUDescriptorHandleForHeapStart().ptr + (SIZE_T)first * inc};
            for (UINT i = 0; i < kTableSlots; ++i)
                dev_->CopyDescriptorsSimple(1, {dst.ptr + (SIZE_T)i * inc}, srvs_.Cpu(bound_[i]), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            cl_->SetGraphicsRootDescriptorTable(RootTextures, {tables_->GetGPUDescriptorHandleForHeapStart().ptr + (UINT64)first * inc});
            tableDirty_ = false;
            return true;
        }

        // A single-sampled copy of a multisampled texture (the caller retires it or waits).
        ComPtr<ID3D12Resource> Resolved(ID3D12GraphicsCommandList* cl, Tex& t)
        {
            D3D12_RESOURCE_DESC rd = t.res->GetDesc();
            rd.SampleDesc.Count = 1;
            rd.SampleDesc.Quality = 0;
            rd.Format = d3d::DxgiTypeless(t.resolveFormat);
            rd.Flags = D3D12_RESOURCE_FLAG_NONE;
            const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
            ComPtr<ID3D12Resource> tmp;
            if (!log_.Check(dev_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_RESOLVE_DEST, nullptr, IID_PPV_ARGS(&tmp)),
                            "CreateCommittedResource (resolve)"))
                return nullptr;
            Transition(cl, t, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
            cl->ResolveSubresource(tmp.Get(), 0, t.res.Get(), 0, t.resolveFormat);
            const D3D12_RESOURCE_BARRIER b = d3d12::Transition(tmp.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
            cl->ResourceBarrier(1, &b);
            return tmp;
        }

        static void CopyRegion(ID3D12GraphicsCommandList* cl, ID3D12Resource* dst, int x, int y, ID3D12Resource* src, const IRect& r)
        {
            D3D12_TEXTURE_COPY_LOCATION dl = {};
            dl.pResource = dst;
            dl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_TEXTURE_COPY_LOCATION sl = {};
            sl.pResource = src;
            sl.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            const D3D12_BOX box = {(UINT)r.x0, (UINT)r.y0, 0, (UINT)r.x1, (UINT)r.y1, 1};
            cl->CopyTextureRegion(&dl, (UINT)x, (UINT)y, 0, &sl, &box);
        }

        // ------------------------------------------------------------------ timestamps
        int Stamp()
        {
            FrameSlot& s = Slot();
            if (!cl_ || s.stamps >= kMaxStamps)
                return -1;
            cl_->EndQuery(queries_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, SlotIndex() * kMaxStamps + (UINT)s.stamps);
            return s.stamps++;
        }

        void ReadTimestamps(FrameSlot& s)
        {
            s.profilePending = false;
            const std::size_t index = (std::size_t)(&s - slots_.data());
            const SIZE_T base = index * kMaxStamps * sizeof(std::uint64_t);
            const D3D12_RANGE range = {base, base + (SIZE_T)s.stamps * sizeof(std::uint64_t)};
            void* data = nullptr;
            if (FAILED(timestamps_->Map(0, &range, &data)))
                return;
            const auto* ticks = reinterpret_cast<const std::uint64_t*>(static_cast<std::uint8_t*>(data) + base);
            const GpuProfile p = s.profile.Resolve(ticks, s.stamps, (double)frequency_);
            const D3D12_RANGE none = {0, 0};
            timestamps_->Unmap(0, &none);
            if (p.valid && p.frame > latest_.frame)
                latest_ = p;
        }

        template <class T>
        static void SetDebugName(T* object, const char* name)
        {
            if (!object || !name)
                return;
            wchar_t w[128];
            std::size_t n = 0;
            for (; name[n] && n + 1 < 128; ++n)
                w[n] = (wchar_t)(unsigned char)name[n];
            w[n] = 0;
            object->SetName(w);
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
                auto* m = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
                if (size == 0 || FAILED(info_->GetMessage(i, m, &size)))
                    continue;
                const LogLevel level = m->Severity <= D3D12_MESSAGE_SEVERITY_ERROR ? LogLevel::Error
                                       : m->Severity == D3D12_MESSAGE_SEVERITY_WARNING ? LogLevel::Warning
                                                                                         : LogLevel::Info;
                log_.Printf(level, "D3D12 debug layer: %.*s", (int)m->DescriptionByteLength, m->pDescription);
            }
            info_->ClearStoredMessages();
        }

        ComPtr<ID3D12Device> dev_;
        ComPtr<ID3D12CommandQueue> queue_;
        d3d::Logger log_;
        ComPtr<ID3D12InfoQueue> info_;
        Caps caps_;

        ComPtr<ID3D12RootSignature> root_;
        DescriptorPool srvs_, rtvs_;
        ComPtr<ID3D12DescriptorHeap> tables_;
        int nullSrv_ = -1;
        d3d::HandleTable<Tex> textures_;
        d3d::HandleTable<Buf> buffers_;
        d3d::HandleTable<Pipe> pipelines_;
        std::unordered_map<ID3D12Resource*, std::uint32_t> wrapped_;
        std::vector<PendingOp> pending_;

        std::vector<FrameSlot> slots_;
        std::uint64_t frame_ = 0;
        ComPtr<ID3D12Fence> fence_;
        UINT64 fenceValue_ = 0;
        HANDLE event_ = nullptr;
        ComPtr<ID3D12GraphicsCommandList> ownList_;
        ComPtr<ID3D12CommandAllocator> immediateAllocator_;

        ComPtr<ID3D12QueryHeap> queries_;
        ComPtr<ID3D12Resource> timestamps_;
        UINT64 frequency_ = 0;
        GpuProfile latest_;

        // this frame
        ID3D12GraphicsCommandList* cl_ = nullptr;
        bool ownFrame_ = false, hostTouched_ = false, tableDirty_ = true;
        Tex* pass_ = nullptr;
        int bound_[kTableSlots] = {};
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        auto dev = std::make_unique<D3D12Device>(desc);
        std::string e;
        if (!dev->Init(e))
        {
            if (error)
                *error = e;
            return nullptr;
        }
        return dev;
    }

    Texture WrapRenderTarget(Device& device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtv, DXGI_FORMAT format,
                             D3D12_RESOURCE_STATES stateOnEntryAndExit)
    {
        auto* d = dynamic_cast<D3D12Device*>(&device);
        return d ? d->Wrap(resource, rtv, format, stateOnEntryAndExit) : Texture{};
    }

    // ------------------------------------------------------------------ headless (conformance suite)
    namespace
    {
        HeadlessDevice CreateHeadless(const HeadlessDesc& hd, std::string& error)
        {
            const int debug = d3d::DebugLevelFromEnvironment();
            if (debug > 0)
            {
                ComPtr<ID3D12Debug> dbg;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbg))))
                {
                    dbg->EnableDebugLayer();
                    ComPtr<ID3D12Debug1> dbg1;
                    if (debug > 1 && SUCCEEDED(dbg.As(dbg1)))
                        dbg1->SetEnableGPUBasedValidation(TRUE);
                }
            }
            ComPtr<IDXGIAdapter> adapter;
            if (d3d::UseWarpFromEnvironment())
            {
                ComPtr<IDXGIFactory4> factory;
                if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) || FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))))
                {
                    error = "no WARP adapter";
                    return {};
                }
            }
            ComPtr<ID3D12Device> device;
            const HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
            if (FAILED(hr))
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "D3D12CreateDevice failed: 0x%08lX (%s)", (unsigned long)hr, d3d::HResultName(hr));
                error = buf;
                return {};
            }
            ComPtr<ID3D12CommandQueue> queue;
            D3D12_COMMAND_QUEUE_DESC qd = {};
            qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            if (FAILED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))))
            {
                error = "CreateCommandQueue failed";
                return {};
            }
            Desc d;
            d.device = device.Get();
            d.queue = queue.Get();
            d.framesInFlight = 2;
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

void EsiaRegisterBackend_d3d12()
{
    esia::rhi::BackendInfo info;
    info.name = "d3d12";
    info.createHeadless = &esia::rhi::d3d12::CreateHeadless;
    esia::rhi::RegisterBackend(info);
}
