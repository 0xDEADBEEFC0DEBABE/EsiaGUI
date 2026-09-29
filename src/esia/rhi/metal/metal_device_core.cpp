// Esia - Metal backend: the rhi::Device logic (see metal_device_core.hpp). UNVERIFIED: needs macOS.
#include "metal_device_core.hpp"
#include "esia/render/shader_library.hpp"
#include <algorithm>
#include <cstring>

namespace esia::rhi::metal
{
    namespace
    {
        constexpr std::uint32_t kMaxCounterBuffers = 8;   // frames profiled in flight; later frames go unprofiled

        std::uint32_t ConstantLimit(ConstantSlot s)
        {
            // the sizes of WgtFrame / WgtPass / WgtDraw in the MSL (esia_common.hlsli); smaller data would make
            // the shader read past the bytes Metal copied
            switch (s)
            {
            case ConstantSlot::Frame: return 192;
            case ConstantSlot::Pass: return 32;
            case ConstantSlot::Draw: return 32;
            }
            return 0;
        }
    }

    MetalDevice::MetalDevice(std::unique_ptr<Gpu> gpu, const DeviceOptions& options)
        : gpu_(std::move(gpu)),
          options_(options),
          // not make_shared: its control block compares type_info, which clang with mingw-w64's libstdc++ 13 defines
          // twice (a duplicate symbol at link time in the Windows cross builds that compile this file)
          tracker_(new FrameTracker),
          staging_(StagingArena::Callbacks{[this](std::size_t bytes) { return gpu_->CreateBuffer(bytes, "esia-staging"); },
                                           [this](std::uint32_t b) { return gpu_->BufferContents(b); },
                                           [this](std::uint32_t b) { gpu_->ReleaseBuffer(b); }})
    {
        const GpuCaps g = gpu_->Caps();
        caps_.fxStorage = FxStorage::Buffer;
        caps_.shaderFormat = (std::uint8_t)shaders::Format::Msl;
        caps_.framebufferOriginBottomLeft = false;   // Metal: top-left framebuffer and texture origins
        caps_.clipSpaceYDown = false;                // clip-space +y up, as Direct3D
        caps_.halfPixelOffset = false;
        caps_.floatRenderTargets = true;             // RGBA16Float is renderable and blendable on every Metal GPU
        caps_.sampleRenderTarget = true;             // encoders are ordered; Metal tracks the hazards of private textures
        caps_.timestampQueries = g.timestamps && options_.timestamps;
        caps_.readback = true;
        caps_.runtimeEffects = false;
        caps_.fxFeatureVariants = false;
        caps_.maxTextureSize = g.maxTextureSize;
        caps_.maxFxDataWidth = g.maxTextureSize;     // unused: FxStorage::Buffer
    }

    MetalDevice::~MetalDevice()
    {
        if (inFrame_)
            EndFrame();
        // Command buffers retain what they reference, but a host command buffer may be unretained: wait for the
        // frames that can still be waited for before releasing anything.
        WaitForFramesInFlight();
        profiler_.Drop([this](std::uint32_t b) { counterFree_.push_back(b); });
        for (std::uint32_t b : counterFree_)
            gpu_->ReleaseCounterBuffer(b);
        for (auto& [id, t] : textures_)
            Doom(t);
        if (scratch_.gpu)
            Doom(scratch_);
        for (auto& [id, b] : buffers_)
            for (const auto& v : b.ring.All())
                doomed_.Push({false, v.object}, 0);
        doomed_.Collect(0, [this](Doomed d) { d.texture ? gpu_->ReleaseTexture(d.id) : gpu_->ReleaseBuffer(d.id); }, true);
        for (auto& [key, pso] : psoCache_)
            gpu_->ReleasePipeline(pso);
        staging_.ReleaseAll();
    }

    // ------------------------------------------------------------------ helpers
    void MetalDevice::Error(const std::string& what)
    {
        errors_.push_back(what);
        if (options_.onError)
            options_.onError(what);
    }

    bool MetalDevice::InFrame(const char* call)
    {
        if (!inFrame_)
            Error(std::string(call) + " outside BeginFrame / EndFrame");
        return inFrame_;
    }

    bool MetalDevice::InPass(const char* call)
    {
        if (!InFrame(call))
            return false;
        if (!inPass_)
            Error(std::string(call) + " outside a pass");
        return inPass_;
    }

    MetalDevice::TextureRec* MetalDevice::FindTexture(Texture t)
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    const MetalDevice::TextureRec* MetalDevice::FindTexture(Texture t) const
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    MetalDevice::BufferRec* MetalDevice::FindBuffer(Buffer b)
    {
        auto it = buffers_.find(b.id);
        return it != buffers_.end() ? &it->second : nullptr;
    }

    std::uint32_t MetalDevice::ViewOf(TextureRec& rec, std::uint32_t pixelFormat)
    {
        if (PixelFormatOf(rec.desc.format) == pixelFormat)
            return rec.gpu;
        for (const auto& [format, view] : rec.views)
            if (format == pixelFormat)
                return view;
        const std::uint32_t view = gpu_->CreateTextureView(rec.gpu, pixelFormat);
        if (view)
            rec.views.push_back({pixelFormat, view});
        else
            Error("newTextureViewWithPixelFormat failed");
        return view;
    }

    void MetalDevice::Doom(TextureRec& rec)
    {
        // any frame begun so far may reference it
        const std::uint64_t last = tracker_->LastBegun();
        for (const auto& [format, view] : rec.views)
            doomed_.Push({true, view}, last);
        if (rec.gpu)
            doomed_.Push({true, rec.gpu}, last);
        rec.views.clear();
        rec.gpu = rec.sampleView = 0;
    }

    void MetalDevice::Collect()
    {
        const std::uint64_t done = Completed();
        doomed_.Collect(done, [this](Doomed d) { d.texture ? gpu_->ReleaseTexture(d.id) : gpu_->ReleaseBuffer(d.id); });
        // a burst of uploads (a new glyph page, images) can leave chunks behind: keep two idle ones
        staging_.Trim(done, 2);
        profiler_.Poll(done, clock_,
                       [this](std::uint32_t b, std::uint32_t first, std::uint32_t count, std::uint64_t* out) { return gpu_->ResolveCounters(b, first, count, out); },
                       [this](std::uint32_t b) { counterFree_.push_back(b); });
    }

    bool MetalDevice::WaitForFramesInFlight()
    {
        bool ok = true;
        for (std::uint64_t s = Completed() + 1; s <= tracker_->LastBegun(); ++s)
            if (!gpu_->WaitForFrame(s))
                ok = false;
        return ok;
    }

    // ------------------------------------------------------------------ command buffers and encoders
    bool MetalDevice::StartCommandBuffer(void* native)
    {
        serial_ = tracker_->Begin();
        if (!gpu_->BeginCommandBuffer(native, serial_, tracker_))
        {
            tracker_->Complete(serial_);   // nothing was recorded: nothing to wait for
            Error("could not start a command buffer");
            return false;
        }
        return true;
    }

    void MetalDevice::EnsureBlit()
    {
        if (encoder_ == Encoder::Blit)
            return;
        EndEncoder();
        EncoderSamples samples;
        profiler_.NextEncoder(false, samples);
        if (gpu_->BeginBlitPass(samples))
            encoder_ = Encoder::Blit;
        else
            Error("blitCommandEncoder failed");
    }

    void MetalDevice::EndEncoder()
    {
        if (encoder_ != Encoder::None)
            gpu_->EndEncoder();
        encoder_ = Encoder::None;
    }

    // ------------------------------------------------------------------ textures
    Texture MetalDevice::CreateTexture(const TextureDesc& desc, const void* data, int rowPitch)
    {
        const FormatInfo& f = InfoOf(desc.format);
        if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize || f.format == Format::Unknown)
        {
            Error("CreateTexture: invalid size or format");
            return {};
        }
        if (inFrame_ && passStarted_)
        {
            Error("CreateTexture after the frame's first pass");
            return {};
        }
        if ((desc.usage & TextureUsage_RenderTarget) && !f.renderable)
        {
            Error(std::string("CreateTexture: ") + FormatName(desc.format) + " is not a render-target format on Metal");
            return {};
        }
        if (desc.samples != 1)
        {
            // multisampled textures are render targets that are resolved, never sampled or written by blits
            if (!gpu_->SupportsSampleCount(desc.samples) || !(desc.usage & TextureUsage_RenderTarget) ||
                (desc.usage & (TextureUsage_Sampled | TextureUsage_CopyDst)) || data)
                return {};
        }
        GpuTextureDesc g;
        g.width = desc.width;
        g.height = desc.height;
        g.pixelFormat = f.pixelFormat;
        g.samples = desc.samples;
        g.usage = MetalUsageFor(desc);
        g.label = desc.debugName;
        TextureRec rec;
        rec.gpu = gpu_->CreateTexture(g);
        if (!rec.gpu)
        {
            Error("newTextureWithDescriptor failed");
            return {};
        }
        rec.desc = desc;
        rec.desc.debugName = nullptr;
        rec.metalUsage = g.usage;
        if (desc.usage & TextureUsage_Sampled)
            rec.sampleView = IsSrgb(desc.format) ? ViewOf(rec, PixelFormatOf(RawFormat(desc.format))) : rec.gpu;
        const Texture t{next_++};
        textures_[t.id] = std::move(rec);
        if (data)
            UpdateTexture(t, IRect{0, 0, desc.width, desc.height}, data, rowPitch);
        return t;
    }

    Texture MetalDevice::WrapTexture(void* native)
    {
        if (!native)
            return {};
        auto cached = wrappers_.find(native);
        if (cached != wrappers_.end() && textures_.count(cached->second))
            return Texture{cached->second};
        HostTextureInfo info;
        const std::uint32_t id = gpu_->AdoptTexture(native, info);
        if (!id)
        {
            Error("WrapTexture: not a usable texture");
            return {};
        }
        const Format format = FormatOfPixelFormat(info.pixelFormat);
        if (format == Format::Unknown || !info.is2D || info.width <= 0 || info.height <= 0)
        {
            gpu_->ReleaseTexture(id);
            Error("WrapTexture: the RHI has no format for this pixel format, or the texture is not 2D");
            return {};
        }
        TextureRec rec;
        rec.gpu = id;
        rec.native = native;
        rec.metalUsage = info.usage;
        rec.framebufferOnly = info.framebufferOnly;
        rec.desc.width = info.width;
        rec.desc.height = info.height;
        rec.desc.format = format;
        rec.desc.samples = info.samples;
        rec.desc.usage = RhiUsageOfHostTexture(format, info.usage, info.framebufferOnly, info.samples);
        if (rec.desc.usage & TextureUsage_Sampled)
            rec.sampleView = IsSrgb(format) ? ViewOf(rec, PixelFormatOf(RawFormat(format))) : rec.gpu;
        const Texture t{next_++};
        textures_[t.id] = std::move(rec);
        wrappers_[native] = t.id;
        return t;
    }

    void MetalDevice::UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch)
    {
        TextureRec* rec = FindTexture(tex);
        if (!rec)
        {
            Error("UpdateTexture: unknown texture");
            return;
        }
        if (inFrame_ && passStarted_)
        {
            Error("UpdateTexture after the frame's first pass");
            return;
        }
        if (!data || !RectInside(r, rec->desc.width, rec->desc.height) || rec->desc.samples != 1 || rec->framebufferOnly)
        {
            Error("UpdateTexture: bad rect or data, or a texture blits cannot write");
            return;
        }
        const BufferImageLayout l = LayoutOf(rec->desc.format, r.Width(), r.Height());
        const std::size_t pitch = rowPitch > 0 ? (std::size_t)rowPitch : l.rowBytes;
        if (inFrame_)
        {
            StageUpload(*rec, r, data, pitch);
            return;
        }
        // outside a frame there is no command buffer to blit in: keep the pixels for the next one
        PendingUpload p{tex.id, r, std::vector<std::uint8_t>(l.rowBytes * (std::size_t)r.Height())};
        CopyRows(p.pixels.data(), l.rowBytes, data, pitch, l.rowBytes, r.Height());
        pending_.push_back(std::move(p));
    }

    bool MetalDevice::StageUpload(TextureRec& rec, const IRect& r, const void* data, std::size_t pitch)
    {
        const BufferImageLayout l = LayoutOf(rec.desc.format, r.Width(), r.Height());
        StagingArena::Allocation a;
        if (!staging_.Allocate(l.bytes, kBlitAlignment, serial_, Completed(), a))
        {
            Error("staging buffer allocation failed");
            return false;
        }
        CopyRows(a.cpu, l.bytesPerRow, data, pitch, l.rowBytes, r.Height());
        EnsureBlit();
        if (encoder_ != Encoder::Blit)
            return false;
        gpu_->CopyBufferToTexture(a.buffer, a.offset, l.bytesPerRow, rec.gpu, r);
        profiler_.Work();
        return true;
    }

    void MetalDevice::FlushPendingUploads()
    {
        for (const PendingUpload& p : pending_)
            if (TextureRec* rec = FindTexture(Texture{p.texture}))
                StageUpload(*rec, p.rect, p.pixels.data(), LayoutOf(rec->desc.format, p.rect.Width(), p.rect.Height()).rowBytes);
        pending_.clear();
    }

    void MetalDevice::DestroyTexture(Texture tex)
    {
        auto it = textures_.find(tex.id);
        if (it == textures_.end())
        {
            Error("DestroyTexture: unknown texture");
            return;
        }
        if (inPass_)
            Error("DestroyTexture inside a pass");
        if (it->second.native)
            wrappers_.erase(it->second.native);
        Doom(it->second);
        textures_.erase(it);
        // uploads still waiting for a command buffer have nowhere to go
        pending_.erase(std::remove_if(pending_.begin(), pending_.end(), [&](const PendingUpload& p) { return p.texture == tex.id; }), pending_.end());
    }

    TextureDesc MetalDevice::GetTextureDesc(Texture tex) const
    {
        const TextureRec* rec = FindTexture(tex);
        return rec ? rec->desc : TextureDesc{};
    }

    // ------------------------------------------------------------------ buffers
    Buffer MetalDevice::CreateBuffer(const BufferDesc& desc)
    {
        if (desc.size == 0)
        {
            Error("CreateBuffer: size 0");
            return {};
        }
        if (inFrame_ && passStarted_)
        {
            Error("CreateBuffer after the frame's first pass");
            return {};
        }
        const Buffer b{next_++};
        BufferRec& rec = buffers_[b.id];
        rec.desc = desc;
        rec.desc.debugName = nullptr;
        return b;
    }

    std::uint32_t MetalDevice::CurrentVersion(BufferRec& rec)
    {
        if (!rec.ring.HasCurrent())
            rec.ring.Acquire(Completed(), [&] { return gpu_->CreateBuffer(rec.desc.size, "esia-buffer"); });
        return rec.ring.Current().object;
    }

    void MetalDevice::UpdateBuffer(Buffer buf, const void* data, std::size_t size)
    {
        BufferRec* rec = FindBuffer(buf);
        if (!rec)
        {
            Error("UpdateBuffer: unknown buffer");
            return;
        }
        if (!data || size > rec->desc.size)
        {
            Error("UpdateBuffer: bad size or data");
            return;
        }
        if (inFrame_ && passStarted_)
        {
            Error("UpdateBuffer after the frame's first pass");
            return;
        }
        const int previous = rec->ring.CurrentIndex();
        const int v = rec->ring.Acquire(Completed(), [&] { return gpu_->CreateBuffer(rec->desc.size, "esia-buffer"); });
        const std::uint32_t object = rec->ring.At(v).object;
        void* dst = object ? gpu_->BufferContents(object) : nullptr;
        if (!dst)
        {
            Error("newBufferWithLength failed");
            return;
        }
        std::memcpy(dst, data, size);
        // "replaces the first size bytes": a fresh version inherits the rest from the one it replaces
        if (previous >= 0 && previous != v && size < rec->valid)
        {
            const auto* src = static_cast<const std::uint8_t*>(gpu_->BufferContents(rec->ring.At(previous).object));
            std::memcpy(static_cast<std::uint8_t*>(dst) + size, src + size, rec->valid - size);
        }
        rec->valid = std::max(rec->valid, size);
    }

    void MetalDevice::DestroyBuffer(Buffer buf)
    {
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end())
        {
            Error("DestroyBuffer: unknown buffer");
            return;
        }
        for (const auto& v : it->second.ring.All())
            if (v.object)
                doomed_.Push({false, v.object}, tracker_->LastBegun());
        buffers_.erase(it);
    }

    // ------------------------------------------------------------------ pipelines
    Pipeline MetalDevice::CreatePipeline(const PipelineDesc& desc)
    {
        PipelineKey key;
        std::string why;
        if (!MakePipelineKey(desc, key, why))
            return {};   // the RHI's "impossible combination": the renderer falls back or skips
        if (desc.samples > 1 && !gpu_->SupportsSampleCount(desc.samples))
            return {};
        std::uint32_t pso = 0;
        auto cached = psoCache_.find(key);
        if (cached != psoCache_.end())
            pso = cached->second;
        else
        {
            const shaders::ShaderBlob* vs = shaders::Find(shaders::Format::Msl, desc.program, shaders::Stage::Vertex);
            const shaders::ShaderBlob* ps = shaders::Find(shaders::Format::Msl, desc.program, shaders::Stage::Pixel);
            if (!vs || !ps)
            {
                Error(std::string("no MSL for ") + ShaderProgramName(desc.program));
                return {};
            }
            GpuPipelineDesc g;
            g.program = desc.program;
            g.vertexSource = reinterpret_cast<const char*>(vs->data);
            g.fragmentSource = reinterpret_cast<const char*>(ps->data);
            g.uiVertexLayout = ProgramUsesVertices(desc.program);
            g.pixelFormat = PixelFormatOf(desc.targetFormat);
            g.samples = desc.samples;
            g.blend = BlendStateOf(desc.blend);
            std::string error;
            pso = gpu_->CreatePipeline(g, error);
            if (!pso)
            {
                Error(std::string("pipeline ") + ShaderProgramName(desc.program) + ": " + error);
                return {};
            }
            psoCache_[key] = pso;
        }
        const Pipeline p{next_++};
        pipelines_[p.id] = PipelineRec{key, pso, desc.topology};
        return p;
    }

    void MetalDevice::DestroyPipeline(Pipeline p)
    {
        // the pipeline state stays in the cache: a render pipeline is immutable and the renderer's set is small
        if (pipelines_.erase(p.id) == 0)
            Error("DestroyPipeline: unknown pipeline");
    }

    // ------------------------------------------------------------------ frames and passes
    bool MetalDevice::BeginFrame(const FrameDesc& desc)
    {
        if (inFrame_)
        {
            Error("BeginFrame inside a frame");
            return false;
        }
        Collect();
        if (!StartCommandBuffer(desc.nativeContext))
            return false;
        inFrame_ = true;
        passStarted_ = inPass_ = false;
        ++frame_;
        ResetBindings();
        std::uint32_t counters = 0;
        if (caps_.timestampQueries)
        {
            std::uint64_t cpu = 0, ticks = 0;
            if (gpu_->SampleClock(cpu, ticks))
                clock_.AddSample(cpu, ticks);
            if (!counterFree_.empty())
            {
                counters = counterFree_.back();
                counterFree_.pop_back();
            }
            else if (counterCount_ < kMaxCounterBuffers && (counters = gpu_->CreateCounterBuffer(ProfileRecorder::kSamplesPerFrame)) != 0)
                ++counterCount_;
        }
        profiler_.BeginFrame(serial_, frame_, counters);
        profileScope_ = false;
        FlushPendingUploads();
        return true;
    }

    void MetalDevice::EndFrame()
    {
        if (!InFrame("EndFrame"))
            return;
        if (inPass_)
        {
            Error("EndFrame inside a pass");
            EndPass();
        }
        EndEncoder();
        profiler_.EndFrame();
        gpu_->EndCommandBuffer();
        inFrame_ = false;
    }

    void MetalDevice::ResetBindings()
    {
        pipeline_ = 0;
        pipelineDirty_ = samplersDirty_ = viewportDirty_ = true;
        scissorDirty_ = scissorEmpty_ = false;
        std::fill(std::begin(constantSize_), std::end(constantSize_), 0u);
        std::fill(std::begin(constantDirty_), std::end(constantDirty_), false);
        for (Texture& t : textureSlot_)
            t = {};
        textureDirty_ = 0;
        fxBuffer_ = vertexBuffer_ = indexBuffer_ = {};
        fxDirty_ = vertexDirty_ = false;
    }

    void MetalDevice::BeginPass(const PassDesc& d)
    {
        if (!InFrame("BeginPass"))
            return;
        if (inPass_)
        {
            Error("BeginPass inside a pass");
            EndPass();
        }
        EndEncoder();
        TextureRec* rec = FindTexture(d.target);
        inPass_ = passStarted_ = true;
        passTarget_ = d.target;
        passOk_ = false;
        ResetBindings();
        if (!rec || !(rec->desc.usage & TextureUsage_RenderTarget))
        {
            Error("BeginPass: target is not a render target");
            return;
        }
        RenderPassDesc rp;
        rp.target = rec->gpu;
        rp.loadAction = LoadActionOf(d.load);
        rp.storeAction = mtl::StoreActionStore;
        std::copy(std::begin(d.clearColor), std::end(d.clearColor), rp.clearColor);
        rp.label = d.debugName;
        profiler_.NextEncoder(true, rp.samples);
        if (!gpu_->BeginRenderPass(rp))
        {
            Error("renderCommandEncoderWithDescriptor failed");
            return;
        }
        encoder_ = Encoder::Render;
        passOk_ = true;
    }

    void MetalDevice::EndPass()
    {
        if (!InPass("EndPass"))
            return;
        EndEncoder();
        inPass_ = passOk_ = false;
        passTarget_ = {};
    }

    void MetalDevice::SetPipeline(Pipeline p)
    {
        if (!InPass("SetPipeline"))
            return;
        auto it = pipelines_.find(p.id);
        if (it == pipelines_.end())
        {
            Error("SetPipeline: unknown pipeline");
            return;
        }
        const TextureRec* target = FindTexture(passTarget_);
        if (target && (it->second.key.format != target->desc.format || it->second.key.samples != target->desc.samples))
            Error("SetPipeline: the pipeline was built for another target format or sample count");
        pipeline_ = p.id;
        pipelineDirty_ = true;
    }

    void MetalDevice::SetScissor(const IRect& r)
    {
        if (!InPass("SetScissor"))
            return;
        const TextureRec* target = FindTexture(passTarget_);
        if (!target || r.x0 < 0 || r.y0 < 0 || r.x1 > target->desc.width || r.y1 > target->desc.height || r.x1 < r.x0 || r.y1 < r.y0)
        {
            Error("SetScissor outside the target");
            return;
        }
        // Metal wants a non-empty rect; the RHI allows an empty one when no draw follows
        scissorEmpty_ = r.Empty();
        scissor_ = r;
        scissorDirty_ = !scissorEmpty_;
    }

    void MetalDevice::SetConstants(ConstantSlot slot, const void* data, std::uint32_t size)
    {
        if (!InFrame("SetConstants"))
            return;
        const int s = (int)slot;
        if (s < 0 || s > 2 || !data || size == 0 || size % 16 != 0 || size > kMaxConstantBytes)
        {
            Error("SetConstants: bad slot or size");
            return;
        }
        if (size < ConstantLimit(slot))
            Error("SetConstants: fewer bytes than the shaders' constant block");
        std::memcpy(constants_[s], data, size);
        constantSize_[s] = size;
        constantDirty_[s] = true;
    }

    void MetalDevice::SetTexture(int slot, Texture tex)
    {
        if (!InFrame("SetTexture"))
            return;
        if (slot < 0 || slot >= kTextureSlots)
        {
            Error("SetTexture: bad slot");
            return;
        }
        if (slot == kSlotFxData)
        {
            // t7 is the FX buffer on Metal (FxStorage::Buffer): SetFxBuffer
            if (tex)
                Error("SetTexture: t7 is a buffer on Metal");
            return;
        }
        textureSlot_[slot] = tex;
        textureDirty_ |= 1u << slot;
    }

    void MetalDevice::SetFxBuffer(Buffer buf)
    {
        if (!InFrame("SetFxBuffer"))
            return;
        fxBuffer_ = buf;
        fxDirty_ = true;
    }

    void MetalDevice::SetVertexBuffer(Buffer buf)
    {
        if (!InFrame("SetVertexBuffer"))
            return;
        vertexBuffer_ = buf;
        vertexDirty_ = true;
    }

    void MetalDevice::SetIndexBuffer(Buffer buf)
    {
        if (!InFrame("SetIndexBuffer"))
            return;
        indexBuffer_ = buf;
    }

    // Applies what changed since the last draw. False: skip the draw (reported where it is a contract violation).
    bool MetalDevice::PrepareDraw(const char* call)
    {
        if (!InPass(call) || !passOk_ || scissorEmpty_)
            return false;
        auto p = pipelines_.find(pipeline_);
        if (p == pipelines_.end())
        {
            Error(std::string(call) + " without a pipeline");
            return false;
        }
        if (pipelineDirty_)
        {
            gpu_->SetPipeline(p->second.pso);
            pipelineDirty_ = false;
        }
        if (viewportDirty_)
        {
            const TextureRec* target = FindTexture(passTarget_);
            if (!target)
            {
                Error(std::string(call) + ": the pass target was destroyed");
                return false;
            }
            gpu_->SetViewport(target->desc.width, target->desc.height);
            viewportDirty_ = false;
        }
        if (scissorDirty_)
        {
            gpu_->SetScissor(scissor_);
            scissorDirty_ = false;
        }
        if (samplersDirty_)
        {
            gpu_->SetFragmentSamplers();
            samplersDirty_ = false;
        }
        for (int s = 0; s < 3; ++s)
            if (constantDirty_[s])
            {
                gpu_->SetBytes(StageVertex | StageFragment, binding::ConstantIndex((ConstantSlot)s), constants_[s], constantSize_[s]);
                constantDirty_[s] = false;
            }
        for (int slot = 0; slot < kSlotFxData; ++slot)
        {
            if (!(textureDirty_ & (1u << slot)) || !textureSlot_[slot])
                continue;
            TextureRec* rec = FindTexture(textureSlot_[slot]);
            if (!rec || !rec->sampleView)
            {
                Error(std::string(call) + ": a bound texture was destroyed or is not sampleable");
                return false;
            }
            if (textureSlot_[slot] == passTarget_)
            {
                Error(std::string(call) + ": the pass target is bound as a texture");
                return false;
            }
            gpu_->SetFragmentTexture(binding::TextureIndex(slot), rec->sampleView);
            textureDirty_ &= ~(1u << slot);
        }
        if (fxDirty_ && fxBuffer_)
        {
            BufferRec* rec = FindBuffer(fxBuffer_);
            if (!rec || rec->desc.kind != BufferKind::FxInstances)
            {
                Error(std::string(call) + ": the FX buffer is not an FX instance buffer");
                return false;
            }
            gpu_->SetBuffer(StageVertex | StageFragment, binding::kFxData, CurrentVersion(*rec), 0);
            rec->ring.MarkUsed(serial_);
            fxDirty_ = false;
        }
        if (vertexDirty_ && vertexBuffer_)
        {
            BufferRec* rec = FindBuffer(vertexBuffer_);
            if (!rec || rec->desc.kind != BufferKind::Vertex)
            {
                Error(std::string(call) + ": not a vertex buffer");
                return false;
            }
            gpu_->SetBuffer(StageVertex, binding::kVertexBuffer, CurrentVersion(*rec), 0);
            rec->ring.MarkUsed(serial_);
            vertexDirty_ = false;
        }
        profiler_.Work();
        return true;
    }

    void MetalDevice::Draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
    {
        if (PrepareDraw("Draw"))
            gpu_->Draw(PrimitiveTypeOf(pipelines_[pipeline_].topology), firstVertex, vertexCount, 1);
    }

    void MetalDevice::DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex)
    {
        if (!PrepareDraw("DrawIndexed"))
            return;
        BufferRec* rec = FindBuffer(indexBuffer_);
        if (!rec || rec->desc.kind != BufferKind::Index || ((std::size_t)firstIndex + indexCount) * 4 > rec->desc.size)
        {
            Error("DrawIndexed: no index buffer, or indices out of range");
            return;
        }
        const std::uint32_t object = CurrentVersion(*rec);
        rec->ring.MarkUsed(serial_);
        gpu_->DrawIndexed(indexCount, object, (std::size_t)firstIndex * 4);
    }

    void MetalDevice::DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount)
    {
        if (PrepareDraw("DrawInstanced"))
            gpu_->Draw(PrimitiveTypeOf(pipelines_[pipeline_].topology), 0, vertexCount, instanceCount);
    }

    // ------------------------------------------------------------------ copies
    MetalDevice::TextureRec* MetalDevice::ResolveScratch(const TextureRec& src)
    {
        if (scratch_.gpu && scratch_.desc.width == src.desc.width && scratch_.desc.height == src.desc.height && scratch_.desc.format == src.desc.format)
            return &scratch_;
        if (scratch_.gpu)
            Doom(scratch_);
        // a resolve texture has the multisampled texture's pixel format; PixelFormatView (from MetalUsageFor) lets
        // the blit that follows read it as the destination's sRGB twin
        TextureDesc d;
        d.width = src.desc.width;
        d.height = src.desc.height;
        d.format = src.desc.format;
        d.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc;
        GpuTextureDesc g;
        g.width = d.width;
        g.height = d.height;
        g.pixelFormat = PixelFormatOf(d.format);
        g.usage = MetalUsageFor(d);
        g.label = "esia-resolve";
        scratch_ = TextureRec();
        scratch_.gpu = gpu_->CreateTexture(g);
        if (!scratch_.gpu)
        {
            Error("resolve texture: newTextureWithDescriptor failed");
            return nullptr;
        }
        scratch_.desc = d;
        scratch_.metalUsage = g.usage;
        return &scratch_;
    }

    // A render pass that loads the multisampled texture and stores it resolved into `scratch` (Metal resolves only
    // at the end of a render pass: no blit resolves).
    bool MetalDevice::Resolve(TextureRec& src, TextureRec& scratch)
    {
        EndEncoder();
        RenderPassDesc rp;
        rp.target = src.gpu;
        rp.loadAction = mtl::LoadActionLoad;
        rp.storeAction = mtl::StoreActionStoreAndMultisampleResolve;
        rp.resolveTarget = scratch.gpu;
        rp.label = "esia-resolve";
        profiler_.NextEncoder(true, rp.samples);
        if (!gpu_->BeginRenderPass(rp))
        {
            Error("resolve pass failed");
            return false;
        }
        profiler_.Work();
        gpu_->EndEncoder();
        return true;
    }

    void MetalDevice::CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r)
    {
        if (!InFrame("CopyTexture"))
            return;
        if (inPass_)
        {
            Error("CopyTexture inside a pass");
            return;
        }
        TextureRec* s = FindTexture(src);
        TextureRec* d = FindTexture(dst);
        if (!s || !d)
        {
            Error("CopyTexture: unknown texture");
            return;
        }
        if (!(s->desc.usage & TextureUsage_CopySrc) || !(d->desc.usage & TextureUsage_CopyDst) || !RectInside(r, s->desc.width, s->desc.height) ||
            !RectInside(IRect{dstX, dstY, dstX + r.Width(), dstY + r.Height()}, d->desc.width, d->desc.height))
        {
            Error("CopyTexture: missing CopySrc / CopyDst usage, or region out of bounds");
            return;
        }
        const CopyPlan plan = PlanCopy({s->desc.format, s->desc.samples, (s->metalUsage & mtl::TextureUsagePixelFormatView) != 0, !s->framebufferOnly},
                                       {d->desc.format, d->desc.samples, (d->metalUsage & mtl::TextureUsagePixelFormatView) != 0, !d->framebufferOnly});
        if (!plan.valid)
        {
            Error("CopyTexture: " + plan.why);
            return;
        }
        TextureRec* from = s;
        if (plan.resolve)
        {
            from = ResolveScratch(*s);
            if (!from || !Resolve(*s, *from))
                return;
        }
        const std::uint32_t srcTex = plan.view == CopyView::Source ? ViewOf(*from, PixelFormatOf(plan.viewFormat)) : from->gpu;
        const std::uint32_t dstTex = plan.view == CopyView::Destination ? ViewOf(*d, PixelFormatOf(plan.viewFormat)) : d->gpu;
        if (!srcTex || !dstTex)
            return;
        EnsureBlit();
        if (encoder_ != Encoder::Blit)
            return;
        gpu_->CopyTexture(srcTex, r, dstTex, dstX, dstY);
        profiler_.Work();
    }

    void* MetalDevice::NativeRenderState()
    {
        if (!InPass("NativeRenderState") || !passOk_)
            return nullptr;
        // host code may change anything on the encoder: bind everything again (the renderer does too)
        ResetBindings();
        return gpu_->NativeRenderEncoder();
    }

    // ------------------------------------------------------------------ profiling
    void MetalDevice::BeginProfile(ProfileCategory category)
    {
        if (!caps_.timestampQueries || !InFrame("BeginProfile"))
            return;
        if (profileScope_)
            Error("BeginProfile: scopes do not nest");
        profileScope_ = true;
        profiler_.Scope(category);
    }

    void MetalDevice::EndProfile()
    {
        if (!caps_.timestampQueries || !InFrame("EndProfile"))
            return;
        if (!profileScope_)
            Error("EndProfile without BeginProfile");
        profileScope_ = false;
        profiler_.EndScope();
    }

    bool MetalDevice::ReadProfile(GpuProfile& out)
    {
        out = GpuProfile();
        if (!caps_.timestampQueries)
            return false;
        Collect();
        return profiler_.Latest(out);
    }

    // ------------------------------------------------------------------ readback
    bool MetalDevice::ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& rgba8)
    {
        rgba8.clear();
        if (inFrame_)
        {
            Error("ReadPixels inside a frame");
            return false;
        }
        TextureRec* rec = FindTexture(tex);
        if (!rec || !RectInside(r, rec->desc.width, rec->desc.height) || rec->framebufferOnly)
        {
            Error("ReadPixels: unknown texture, region out of bounds, or a framebufferOnly texture");
            return false;
        }
        // the frames that rendered it (the host must have committed its command buffers)
        if (!WaitForFramesInFlight())
        {
            Error("ReadPixels: a frame's command buffer was not committed");
            return false;
        }
        const BufferImageLayout l = LayoutOf(rec->desc.format, r.Width(), r.Height());
        const std::uint32_t readback = gpu_->CreateBuffer(l.bytes, "esia-readback");
        if (!readback)
        {
            Error("ReadPixels: no readback buffer");
            return false;
        }
        bool ok = StartCommandBuffer(nullptr);
        if (ok)
        {
            FlushPendingUploads();   // pixels uploaded outside a frame must land before they are read
            TextureRec* from = rec;
            if (rec->desc.samples > 1)
            {
                from = ResolveScratch(*rec);
                ok = from && Resolve(*rec, *from);
            }
            if (ok)
            {
                EnsureBlit();
                ok = encoder_ == Encoder::Blit;
                if (ok)
                    gpu_->CopyTextureToBuffer(from->gpu, r, readback, 0, l.bytesPerRow);
            }
            EndEncoder();
            gpu_->EndCommandBuffer();
            ok = gpu_->WaitForFrame(serial_) && ok;
        }
        if (ok)
        {
            rgba8.resize((std::size_t)r.Width() * (std::size_t)r.Height() * 4);
            ok = ConvertToRgba8(rec->desc.format, gpu_->BufferContents(readback), l.bytesPerRow, r.Width(), r.Height(), rgba8.data());
        }
        gpu_->ReleaseBuffer(readback);   // the command buffer completed (or never started)
        Collect();
        if (!ok)
            rgba8.clear();
        return ok;
    }

    // ------------------------------------------------------------------ headless
    HeadlessDevice CreateHeadlessOn(std::unique_ptr<Gpu> gpu, const HeadlessDesc& desc, const DeviceOptions& options, std::string& error)
    {
        HeadlessDevice h;
        if (!gpu)
        {
            error = "no Metal device";
            return h;
        }
        if (desc.samples > 1 && !gpu->SupportsSampleCount(desc.samples))
        {
            error = "the GPU has no " + std::to_string(desc.samples) + "x MSAA";
            return h;
        }
        auto dev = std::make_unique<MetalDevice>(std::move(gpu), options);
        TextureDesc td;
        td.width = desc.width;
        td.height = desc.height;
        td.format = desc.format;
        td.samples = desc.samples;
        td.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (desc.sampleable && desc.samples == 1 ? TextureUsage_Sampled : 0u);
        td.debugName = "headless-target";
        h.target = dev->CreateTexture(td, nullptr, 0);
        if (!h.target)
        {
            error = std::string("cannot create a ") + FormatName(desc.format) + " render target";
            return HeadlessDevice();
        }
        h.device = std::move(dev);
        return h;
    }
}
