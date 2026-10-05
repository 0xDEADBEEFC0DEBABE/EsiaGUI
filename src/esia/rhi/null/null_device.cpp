// Esia - null RHI backend (see esia/rhi/null_device.hpp)
#include "esia/rhi/null_device.hpp"
#include "esia/rhi/backend_registry.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace esia::rhi
{
    namespace
    {
        std::string Fmt(const char* f, ...) ESIA_PRINTF(1, 2);
        std::string Fmt(const char* f, ...)
        {
            char buf[512];
            va_list ap;
            va_start(ap, f);
            std::vsnprintf(buf, sizeof(buf), f, ap);
            va_end(ap);
            return buf;
        }

        std::string Rect(const IRect& r) { return Fmt("[%d,%d %dx%d]", r.x0, r.y0, r.Width(), r.Height()); }

        const char* BlendName(BlendMode b)
        {
            switch (b)
            {
            case BlendMode::Opaque: return "opaque";
            case BlendMode::Straight: return "straight";
            case BlendMode::Premultiplied: return "premul";
            }
            return "?";
        }

        std::string Floats(const void* data, std::uint32_t size)
        {
            std::string s;
            const float* f = static_cast<const float*>(data);
            for (std::uint32_t i = 0; i < size / 4; ++i)
            {
                // the renderer's constants are float4 rows; a %g per value keeps the logs short and stable
                s += Fmt(i == 0 ? "%g" : (i % 4 == 0 ? " | %g" : " %g"), (double)f[i]);
            }
            return s;
        }
    }

    NullDevice::NullDevice(const NullOptions& options) : caps_(options.caps), options_(options), record_(options.record), keepData_(options.keepData) {}

    const std::vector<std::uint8_t>* NullDevice::Data(Buffer b) const
    {
        auto it = data_.find(b.id);
        return it != data_.end() && buffers_.count(b.id) ? &it->second : nullptr;
    }

    const std::vector<std::uint8_t>* NullDevice::Data(Texture t) const
    {
        auto it = data_.find(t.id);
        return it != data_.end() && textures_.count(t.id) ? &it->second : nullptr;
    }

    void NullDevice::Record(const std::string& line)
    {
        if (record_)
            log_.push_back(line);
    }

    void NullDevice::Error(const std::string& what)
    {
        errors_.push_back(what);
        if (record_)
            Record("!! " + what);
    }

    bool NullDevice::InFrame(const char* call)
    {
        if (!inFrame_)
            Error(Fmt("%s outside BeginFrame / EndFrame", call));
        return inFrame_;
    }

    bool NullDevice::InPass(const char* call)
    {
        if (!InFrame(call))
            return false;
        if (!inPass_)
            Error(Fmt("%s outside a pass", call));
        return inPass_;
    }

    std::string NullDevice::TexName(Texture t) const
    {
        auto it = textures_.find(t.id);
        if (it == textures_.end())
            return Fmt("#%u(dead)", t.id);
        return Fmt("#%u", t.id);
    }

    // ------------------------------------------------------------------ resources
    Texture NullDevice::CreateTexture(const TextureDesc& desc, const void* data, int rowPitch)
    {
        if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize)
        {
            Error(Fmt("CreateTexture: invalid size %dx%d", desc.width, desc.height));
            return {};
        }
        if ((desc.usage & TextureUsage_RenderTarget) && desc.format == Format::RGBA16_FLOAT && !caps_.floatRenderTargets)
            Error("CreateTexture: RGBA16F render target without Caps::floatRenderTargets");
        if (inFrame_ && passStarted_)
            Error("CreateTexture after the frame's first pass");
        const Texture t{next_++};
        textures_[t.id] = desc;
        if (keepData_)
        {
            const std::size_t row = (std::size_t)desc.width * (std::size_t)BytesPerPixel(desc.format);
            std::vector<std::uint8_t>& d = data_[t.id];
            d.assign(row * (std::size_t)desc.height, 0);
            const std::size_t pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            if (data)
                for (int y = 0; y < desc.height; ++y)
                    std::memcpy(d.data() + row * (std::size_t)y, static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
        }
        if (record_)
            Record(Fmt("create texture #%u %dx%d %s usage=%x%s%s", t.id, desc.width, desc.height, FormatName(desc.format), desc.usage,
                       desc.samples > 1 ? " msaa" : "", desc.debugName ? (std::string(" ") + desc.debugName).c_str() : ""));
        return t;
    }

    Texture NullDevice::CreateHostTarget(int width, int height, Format format, bool sampleable, int samples)
    {
        TextureDesc d;
        d.width = width;
        d.height = height;
        d.format = format;
        d.samples = samples;
        d.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (sampleable ? TextureUsage_Sampled : 0u);
        d.debugName = "host-target";
        return CreateTexture(d, nullptr, 0);
    }

    void NullDevice::UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch)
    {
        auto it = textures_.find(tex.id);
        if (it == textures_.end())
        {
            Error("UpdateTexture: unknown texture");
            return;
        }
        if (inFrame_ && passStarted_)
            Error("UpdateTexture after the frame's first pass");
        if (r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > it->second.width || r.y1 > it->second.height || !data)
            Error("UpdateTexture: bad rect or data " + Rect(r));
        ++stats_.textureUpdates;
        if (record_)
            Record(Fmt("update texture #%u ", tex.id) + Rect(r));
        if (keepData_ && data && !r.Empty() && r.x0 >= 0 && r.y0 >= 0 && r.x1 <= it->second.width && r.y1 <= it->second.height)
        {
            const std::size_t bpp = (std::size_t)BytesPerPixel(it->second.format);
            const std::size_t row = (std::size_t)r.Width() * bpp, pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            std::vector<std::uint8_t>& d = data_[tex.id];
            for (int y = 0; y < r.Height(); ++y)
                std::memcpy(d.data() + ((std::size_t)(r.y0 + y) * (std::size_t)it->second.width + (std::size_t)r.x0) * bpp,
                            static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
        }
    }

    void NullDevice::DestroyTexture(Texture tex)
    {
        if (textures_.erase(tex.id) == 0)
            Error("DestroyTexture: unknown texture");
        data_.erase(tex.id);
        if (record_)
            Record(Fmt("destroy texture #%u", tex.id));
    }

    TextureDesc NullDevice::GetTextureDesc(Texture tex) const
    {
        auto it = textures_.find(tex.id);
        return it != textures_.end() ? it->second : TextureDesc{};
    }

    Buffer NullDevice::CreateBuffer(const BufferDesc& desc)
    {
        if (desc.size == 0)
        {
            Error("CreateBuffer: size 0");
            return {};
        }
        if (desc.kind == BufferKind::FxInstances && caps_.fxStorage != FxStorage::Buffer)
            Error("CreateBuffer: FxInstances buffer on a FxStorage::Texture device");
        if (inFrame_ && passStarted_)
            Error("CreateBuffer after the frame's first pass");
        const Buffer b{next_++};
        buffers_[b.id] = desc;
        static const char* kinds[] = {"vertex", "index", "fx"};
        if (record_)
            Record(Fmt("create buffer #%u %s %zu bytes", b.id, kinds[(int)desc.kind], desc.size));
        return b;
    }

    void NullDevice::UpdateBuffer(Buffer buf, const void* data, std::size_t size)
    {
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end())
        {
            Error("UpdateBuffer: unknown buffer");
            return;
        }
        if (size > it->second.size || !data)
            Error(Fmt("UpdateBuffer: %zu bytes into a %zu byte buffer", size, it->second.size));
        if (inFrame_ && passStarted_)
            Error("UpdateBuffer after the frame's first pass");
        ++stats_.bufferUpdates;
        if (record_)
            Record(Fmt("update buffer #%u %zu bytes", buf.id, size));
        if (keepData_ && data && size <= it->second.size)
        {
            std::vector<std::uint8_t>& d = data_[buf.id];
            d.resize(it->second.size);
            std::memcpy(d.data(), data, size);
        }
    }

    void NullDevice::DestroyBuffer(Buffer buf)
    {
        if (buffers_.erase(buf.id) == 0)
            Error("DestroyBuffer: unknown buffer");
        data_.erase(buf.id);
        if (record_)
            Record(Fmt("destroy buffer #%u", buf.id));
    }

    Pipeline NullDevice::CreatePipeline(const PipelineDesc& d)
    {
        if (d.effect != 0 && !caps_.runtimeEffects)
        {
            if (record_)
                Record(Fmt("create pipeline refused: runtime effect %u unsupported", d.effect));
            return {};
        }
        if (options_.refusePrograms & (1u << (unsigned)d.program))
        {
            if (record_)
                Record(Fmt("create pipeline refused: %s", ShaderProgramName(d.program)));
            return {};
        }
        if (d.fxFeatures != 0 && options_.refuseFxVariants)
        {
            if (record_)
                Record(Fmt("create pipeline refused: fx variant 0x%x", d.fxFeatures));
            return {};
        }
        if (d.program >= ShaderProgram::Count)
        {
            Error("CreatePipeline: bad program");
            return {};
        }
        const bool wantsVertices = d.program == ShaderProgram::UiGeometry || d.program == ShaderProgram::TextGray;
        if (wantsVertices != (d.layout == VertexLayout::UiVertex))
            Error(Fmt("CreatePipeline: %s with the wrong vertex layout", ShaderProgramName(d.program)));
        const Pipeline p{next_++};
        pipelines_[p.id] = d;
        if (d.fxFeatures != 0 && (!caps_.fxFeatureVariants || d.program != ShaderProgram::Fx))
            Error("CreatePipeline: fxFeatures without Caps::fxFeatureVariants or on a non-Fx program");
        const bool background = d.background && caps_.asyncPipelines;
        if (background)
            pipelineReadyAt_[p.id] = frame_ + (std::uint64_t)std::max(options_.pendingFrames, 0);
        if (record_)
            Record(Fmt("create pipeline #%u %s %s %s %s%s%s%s", p.id, ShaderProgramName(d.program), d.topology == Topology::TriangleStrip ? "strip" : "list",
                       BlendName(d.blend), FormatName(d.targetFormat), d.effect ? Fmt(" effect=%u", d.effect).c_str() : "",
                       d.fxFeatures ? Fmt(" features=0x%x", d.fxFeatures).c_str() : "", background ? " background" : ""));
        return p;
    }

    void NullDevice::DestroyPipeline(Pipeline p)
    {
        if (pipelines_.erase(p.id) == 0)
            Error("DestroyPipeline: unknown pipeline");
        pipelineReadyAt_.erase(p.id);
        if (record_)
            Record(Fmt("destroy pipeline #%u", p.id));
    }

    PipelineStatus NullDevice::GetPipelineStatus(Pipeline p) const
    {
        if (!pipelines_.count(p.id))
            return PipelineStatus::Failed;
        auto it = pipelineReadyAt_.find(p.id);
        if (it == pipelineReadyAt_.end())
            return PipelineStatus::Ready;   // built in CreatePipeline
        if (frame_ < it->second)
            return PipelineStatus::Pending;
        return options_.failBackground ? PipelineStatus::Failed : PipelineStatus::Ready;
    }

    // ------------------------------------------------------------------ frame
    bool NullDevice::BeginFrame(const FrameDesc& desc)
    {
        if (inFrame_)
            Error("BeginFrame inside a frame");
        inFrame_ = true;
        passStarted_ = false;
        pipeline_ = {};
        for (Texture& t : bound_)
            t = {};
        fxBuffer_ = vertexBuffer_ = indexBuffer_ = {};
        for (bool& c : constantsSet_)
            c = false;
        ++frame_;
        if (record_)
            Record(Fmt("begin frame %llu", (unsigned long long)frame_) + (desc.hostFrame ? Fmt(" host %llu", (unsigned long long)desc.hostFrame) : ""));
        return true;
    }

    void NullDevice::EndFrame()
    {
        if (!InFrame("EndFrame"))
            return;
        if (inPass_)
            Error("EndFrame inside a pass");
        if (profileDepth_ != 0)
            Error("EndFrame with an open profile scope");
        inFrame_ = false;
        if (record_)
            Record("end frame");
    }

    void NullDevice::BeginPass(const PassDesc& d)
    {
        if (!InFrame("BeginPass"))
            return;
        if (inPass_)
            Error("BeginPass inside a pass");
        auto it = textures_.find(d.target.id);
        if (it == textures_.end() || !(it->second.usage & TextureUsage_RenderTarget))
            Error("BeginPass: target is not a render target");
        // every pass starts with nothing bound (Metal encoders and Vulkan render passes carry no state over)
        pipeline_ = {};
        for (Texture& t : bound_)
            t = {};
        fxBuffer_ = vertexBuffer_ = indexBuffer_ = {};
        for (bool& c : constantsSet_)
            c = false;
        inPass_ = true;
        passStarted_ = true;
        passTarget_ = d.target;
        ++stats_.passes;
        static const char* loads[] = {"load", "clear", "dont-care"};
        if (record_)
            Record(Fmt("pass %s %s%s", TexName(d.target).c_str(), loads[(int)d.load], d.debugName ? (std::string(" ") + d.debugName).c_str() : ""));
    }

    void NullDevice::EndPass()
    {
        if (!InPass("EndPass"))
            return;
        inPass_ = false;
        passTarget_ = {};
        if (record_)
            Record("end pass");
    }

    void NullDevice::SetPipeline(Pipeline p)
    {
        if (!InPass("SetPipeline"))
            return;
        auto it = pipelines_.find(p.id);
        if (it == pipelines_.end())
        {
            Error("SetPipeline: unknown pipeline");
            return;
        }
        if (GetPipelineStatus(p) != PipelineStatus::Ready)
            Error(Fmt("SetPipeline: pipeline #%u is not ready (pending or failed)", p.id));
        const TextureDesc& t = textures_[passTarget_.id];
        if (it->second.targetFormat != t.format)
            Error(Fmt("SetPipeline: pipeline format %s, target %s", FormatName(it->second.targetFormat), FormatName(t.format)));
        if (it->second.samples != t.samples)
            Error(Fmt("SetPipeline: pipeline for %d samples, target has %d", it->second.samples, t.samples));
        pipeline_ = p;
        if (record_)
            Record(Fmt("pipeline #%u", p.id));
    }

    void NullDevice::SetScissor(const IRect& r)
    {
        if (!InPass("SetScissor"))
            return;
        const TextureDesc& t = textures_[passTarget_.id];
        if (r.x0 < 0 || r.y0 < 0 || r.x1 > t.width || r.y1 > t.height || r.x1 < r.x0 || r.y1 < r.y0)
            Error("SetScissor outside the target " + Rect(r));
        if (record_)
            Record("scissor " + Rect(r));
    }

    void NullDevice::SetConstants(ConstantSlot slot, const void* data, std::uint32_t size)
    {
        if (!InFrame("SetConstants"))
            return;
        if (size == 0 || size % 16 != 0 || !data)
            Error(Fmt("SetConstants: size %u is not a multiple of 16", size));
        constantsSet_[(int)slot] = true;
        static const char* names[] = {"frame", "pass", "draw"};
        if (record_)
            Record(Fmt("constants %s: ", names[(int)slot]) + Floats(data, size));
    }

    void NullDevice::SetTexture(int slot, Texture tex)
    {
        if (!InFrame("SetTexture"))
            return;
        if (slot < 0 || slot >= kTextureSlots)
        {
            Error(Fmt("SetTexture: slot %d", slot));
            return;
        }
        if (tex)
        {
            auto it = textures_.find(tex.id);
            if (it == textures_.end())
                Error("SetTexture: unknown texture");
            else if (!(it->second.usage & TextureUsage_Sampled))
                Error(Fmt("SetTexture: texture #%u is not sampleable", tex.id));
            if (inPass_ && tex == passTarget_)
                Error(Fmt("SetTexture: texture #%u is the bound render target", tex.id));
            if (slot == kSlotFxData && caps_.fxStorage != FxStorage::Texture)
                Error("SetTexture: t7 is a buffer on this device (SetFxBuffer)");
        }
        bound_[slot] = tex;
        if (record_)
            Record(Fmt("texture t%d %s", slot, tex ? TexName(tex).c_str() : "-"));
    }

    void NullDevice::SetFxBuffer(Buffer buf)
    {
        if (!InFrame("SetFxBuffer"))
            return;
        if (caps_.fxStorage != FxStorage::Buffer)
            Error("SetFxBuffer on a FxStorage::Texture device");
        auto it = buffers_.find(buf.id);
        if (buf && (it == buffers_.end() || it->second.kind != BufferKind::FxInstances))
            Error("SetFxBuffer: not an FX instance buffer");
        fxBuffer_ = buf;
        if (record_)
            Record(Fmt("fx buffer #%u", buf.id));
    }

    void NullDevice::SetVertexBuffer(Buffer buf)
    {
        if (!InFrame("SetVertexBuffer"))
            return;
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end() || it->second.kind != BufferKind::Vertex)
            Error("SetVertexBuffer: not a vertex buffer");
        vertexBuffer_ = buf;
        if (record_)
            Record(Fmt("vertex buffer #%u", buf.id));
    }

    void NullDevice::SetIndexBuffer(Buffer buf)
    {
        if (!InFrame("SetIndexBuffer"))
            return;
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end() || it->second.kind != BufferKind::Index)
            Error("SetIndexBuffer: not an index buffer");
        indexBuffer_ = buf;
        if (record_)
            Record(Fmt("index buffer #%u", buf.id));
    }

    void NullDevice::CheckDraw(const char* call, bool instanced)
    {
        if (!InPass(call))
            return;
        auto it = pipelines_.find(pipeline_.id);
        if (it == pipelines_.end())
        {
            Error(Fmt("%s without a pipeline", call));
            return;
        }
        const PipelineDesc& p = it->second;
        if ((p.program == ShaderProgram::Fx) != instanced)
            Error(Fmt("%s: program %s", call, ShaderProgramName(p.program)));
        // What each program's shaders declare must be bound: Vulkan and D3D12 have no "unbound" descriptors, and
        // Metal / GL would read whatever an earlier draw left.
        const bool ui = p.program == ShaderProgram::UiGeometry || p.program == ShaderProgram::TextGray;
        const bool fx = p.program == ShaderProgram::Fx;
        const bool composite = p.program == ShaderProgram::LayerComposite;
        const bool usesPass = p.program == ShaderProgram::Downsample || composite;
        if (!constantsSet_[(int)ConstantSlot::Frame])
            Error(Fmt("%s before the frame constants were set", call));
        if (usesPass && !constantsSet_[(int)ConstantSlot::Pass])
            Error(Fmt("%s (%s) before the pass constants were set", call, ShaderProgramName(p.program)));
        if ((ui || fx) && !constantsSet_[(int)ConstantSlot::Draw])
            Error(Fmt("%s (%s) before the draw constants were set", call, ShaderProgramName(p.program)));
        const int slots = (fx || composite) ? kSlotBackdrop0 + kBackdropLevels : (p.program == ShaderProgram::Clear ? 0 : 1);
        for (int sl = 0; sl < slots; ++sl)
            if (!bound_[sl])
                Error(Fmt("%s (%s): nothing bound at t%d", call, ShaderProgramName(p.program), sl));
        if (fx)
        {
            const bool data = caps_.fxStorage == FxStorage::Buffer ? (bool)fxBuffer_ : (bool)bound_[kSlotFxData];
            if (!data)
                Error(Fmt("%s: no FX instance data bound", call));
        }
        for (int sl = 0; sl < kTextureSlots; ++sl)
            if (bound_[sl] && !textures_.count(bound_[sl].id))
                Error(Fmt("%s: slot t%d holds a destroyed texture", call, sl));
    }

    void NullDevice::Draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
    {
        CheckDraw("Draw", false);
        ++stats_.draws;
        if (record_)
            Record(Fmt("draw %u from %u", vertexCount, firstVertex));
    }

    void NullDevice::DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex)
    {
        CheckDraw("DrawIndexed", false);
        if (!vertexBuffer_ || !indexBuffer_)
            Error("DrawIndexed without vertex / index buffers");
        ++stats_.draws;
        ++stats_.indexedDraws;
        if (record_)
            Record(Fmt("draw indexed %u from %u", indexCount, firstIndex));
    }

    void NullDevice::DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount)
    {
        CheckDraw("DrawInstanced", true);
        ++stats_.draws;
        ++stats_.instancedDraws;
        if (record_)
            Record(Fmt("draw instanced %u x %u", vertexCount, instanceCount));
    }

    void NullDevice::DrawInstancedFrom(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstInstance)
    {
        if (!caps_.drawFirstInstance)
            Error("DrawInstancedFrom without Caps::drawFirstInstance");
        CheckDraw("DrawInstancedFrom", true);
        ++stats_.draws;
        ++stats_.instancedDraws;
        if (record_)
            Record(Fmt("draw instanced %u x %u from %u", vertexCount, instanceCount, firstInstance));
    }

    void NullDevice::CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r)
    {
        if (!InFrame("CopyTexture"))
            return;
        if (inPass_)
            Error("CopyTexture inside a pass");
        auto d = textures_.find(dst.id), s = textures_.find(src.id);
        if (d == textures_.end() || s == textures_.end())
        {
            Error("CopyTexture: unknown texture");
            return;
        }
        if (!(d->second.usage & TextureUsage_CopyDst) || !(s->second.usage & TextureUsage_CopySrc))
            Error("CopyTexture: missing CopySrc / CopyDst usage");
        if (d->second.format != s->second.format && d->second.format != RawFormat(s->second.format))
            Error(Fmt("CopyTexture: %s into %s", FormatName(s->second.format), FormatName(d->second.format)));
        if (d->second.samples != 1)
            Error("CopyTexture: multisampled destination");
        if (r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > s->second.width || r.y1 > s->second.height || dstX < 0 || dstY < 0 ||
            dstX + r.Width() > d->second.width || dstY + r.Height() > d->second.height)
            Error("CopyTexture: region out of bounds " + Rect(r));
        ++stats_.copies;
        if (record_)
            Record(Fmt("copy %s -> #%u at %d,%d ", TexName(src).c_str(), dst.id, dstX, dstY) + Rect(r));
    }

    void NullDevice::BeginProfile(ProfileCategory c)
    {
        if (!InFrame("BeginProfile"))
            return;
        if (profileDepth_++ != 0)
            Error("BeginProfile: scopes do not nest");
        static const char* names[] = {"capture", "layer", "fx", "fx-glass", "geometry"};
        if (record_)
            Record(Fmt("profile %s", names[(int)c]));
    }

    void NullDevice::EndProfile()
    {
        if (!InFrame("EndProfile"))
            return;
        if (--profileDepth_ != 0)
            Error("EndProfile without BeginProfile");
        if (record_)
            Record("end profile");
    }

    bool NullDevice::ReadProfile(GpuProfile& out)
    {
        out = GpuProfile();
        return false;
    }

    bool NullDevice::ReadPixels(Texture, const IRect&, std::vector<std::uint8_t>& rgba8)
    {
        rgba8.clear();
        return false;
    }

    void* NullDevice::NativeRenderState()
    {
        InPass("NativeRenderState");
        // host code may have changed anything: the renderer must bind everything again
        pipeline_ = {};
        for (Texture& t : bound_)
            t = {};
        fxBuffer_ = vertexBuffer_ = indexBuffer_ = {};
        for (bool& c : constantsSet_)
            c = false;
        if (record_)
            Record("native render state");
        return nullptr;
    }

    // ------------------------------------------------------------------ registration
    namespace
    {
        HeadlessDevice CreateNullHeadless(const HeadlessDesc& desc, std::string&)
        {
            NullOptions o;
            auto dev = std::make_unique<NullDevice>(o);
            HeadlessDevice h;
            h.target = dev->CreateHostTarget(desc.width, desc.height, desc.format, desc.sampleable, desc.samples);
            h.device = std::move(dev);
            return h;
        }
    }
}

void EsiaRegisterBackend_null()
{
    esia::rhi::BackendInfo info;
    info.name = "null";
    info.createHeadless = &esia::rhi::CreateNullHeadless;
    esia::rhi::RegisterBackend(info);
}
