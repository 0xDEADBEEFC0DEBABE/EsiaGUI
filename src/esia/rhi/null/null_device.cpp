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
            case BlendMode::DualSourceLcd: return "dual-source";
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

    NullDevice::NullDevice(const NullOptions& options) : caps_(options.caps), record_(options.record) {}

    void NullDevice::Record(const std::string& line)
    {
        if (record_)
            log_.push_back(line);
    }

    void NullDevice::Error(const std::string& what)
    {
        errors_.push_back(what);
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
    Texture NullDevice::CreateTexture(const TextureDesc& desc, const void*, int)
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

    void NullDevice::UpdateTexture(Texture tex, const IRect& r, const void* data, int)
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
        Record(Fmt("update texture #%u ", tex.id) + Rect(r));
    }

    void NullDevice::DestroyTexture(Texture tex)
    {
        if (textures_.erase(tex.id) == 0)
            Error("DestroyTexture: unknown texture");
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
        Record(Fmt("update buffer #%u %zu bytes", buf.id, size));
    }

    void NullDevice::DestroyBuffer(Buffer buf)
    {
        if (buffers_.erase(buf.id) == 0)
            Error("DestroyBuffer: unknown buffer");
        Record(Fmt("destroy buffer #%u", buf.id));
    }

    Pipeline NullDevice::CreatePipeline(const PipelineDesc& d)
    {
        if (d.blend == BlendMode::DualSourceLcd && !caps_.dualSourceBlend)
        {
            Record("create pipeline refused: dual-source blending unsupported");
            return {};
        }
        if (d.effect != 0 && !caps_.runtimeEffects)
        {
            Record(Fmt("create pipeline refused: runtime effect %u unsupported", d.effect));
            return {};
        }
        if (d.program >= ShaderProgram::Count)
        {
            Error("CreatePipeline: bad program");
            return {};
        }
        const bool wantsVertices = d.program == ShaderProgram::UiGeometry || d.program == ShaderProgram::TextGray ||
                                   d.program == ShaderProgram::TextLcd || d.program == ShaderProgram::TextLcdGray;
        if (wantsVertices != (d.layout == VertexLayout::UiVertex))
            Error(Fmt("CreatePipeline: %s with the wrong vertex layout", ShaderProgramName(d.program)));
        if ((d.program == ShaderProgram::TextLcd) != (d.blend == BlendMode::DualSourceLcd))
            Error("CreatePipeline: TextLcd needs (and only it may use) dual-source blending");
        const Pipeline p{next_++};
        pipelines_[p.id] = d;
        Record(Fmt("create pipeline #%u %s %s %s %s%s", p.id, ShaderProgramName(d.program), d.topology == Topology::TriangleStrip ? "strip" : "list",
                   BlendName(d.blend), FormatName(d.targetFormat), d.effect ? Fmt(" effect=%u", d.effect).c_str() : ""));
        return p;
    }

    void NullDevice::DestroyPipeline(Pipeline p)
    {
        if (pipelines_.erase(p.id) == 0)
            Error("DestroyPipeline: unknown pipeline");
        Record(Fmt("destroy pipeline #%u", p.id));
    }

    // ------------------------------------------------------------------ frame
    bool NullDevice::BeginFrame(const FrameDesc&)
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
        Record(Fmt("begin frame %llu", (unsigned long long)frame_));
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
        for (int s = 0; s < kTextureSlots; ++s)
            if (bound_[s] == d.target)
                bound_[s] = {};   // a target cannot stay bound as a texture: backends unbind it
        inPass_ = true;
        passStarted_ = true;
        passTarget_ = d.target;
        ++stats_.passes;
        static const char* loads[] = {"load", "clear", "dont-care"};
        Record(Fmt("pass %s %s%s", TexName(d.target).c_str(), loads[(int)d.load], d.debugName ? (std::string(" ") + d.debugName).c_str() : ""));
    }

    void NullDevice::EndPass()
    {
        if (!InPass("EndPass"))
            return;
        inPass_ = false;
        passTarget_ = {};
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
        const TextureDesc& t = textures_[passTarget_.id];
        if (it->second.targetFormat != t.format)
            Error(Fmt("SetPipeline: pipeline format %s, target %s", FormatName(it->second.targetFormat), FormatName(t.format)));
        pipeline_ = p;
        Record(Fmt("pipeline #%u", p.id));
    }

    void NullDevice::SetScissor(const IRect& r)
    {
        if (!InPass("SetScissor"))
            return;
        const TextureDesc& t = textures_[passTarget_.id];
        if (r.x0 < 0 || r.y0 < 0 || r.x1 > t.width || r.y1 > t.height || r.x1 < r.x0 || r.y1 < r.y0)
            Error("SetScissor outside the target " + Rect(r));
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
        if (!constantsSet_[(int)ConstantSlot::Frame])
            Error(Fmt("%s before the frame constants were set", call));
        if ((p.program == ShaderProgram::Fx) != instanced)
            Error(Fmt("%s: program %s", call, ShaderProgramName(p.program)));
        if (p.program == ShaderProgram::Fx)
        {
            const bool data = caps_.fxStorage == FxStorage::Buffer ? (bool)fxBuffer_ : (bool)bound_[kSlotFxData];
            if (!data)
                Error(Fmt("%s: no FX instance data bound", call));
        }
        for (int s = 0; s < kTextureSlots; ++s)
            if (bound_[s] && !textures_.count(bound_[s].id))
                Error(Fmt("%s: slot t%d holds a destroyed texture", call, s));
    }

    void NullDevice::Draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
    {
        CheckDraw("Draw", false);
        ++stats_.draws;
        Record(Fmt("draw %u from %u", vertexCount, firstVertex));
    }

    void NullDevice::DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex)
    {
        CheckDraw("DrawIndexed", false);
        if (!vertexBuffer_ || !indexBuffer_)
            Error("DrawIndexed without vertex / index buffers");
        ++stats_.draws;
        ++stats_.indexedDraws;
        Record(Fmt("draw indexed %u from %u", indexCount, firstIndex));
    }

    void NullDevice::DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount)
    {
        CheckDraw("DrawInstanced", true);
        ++stats_.draws;
        ++stats_.instancedDraws;
        Record(Fmt("draw instanced %u x %u", vertexCount, instanceCount));
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
        if (r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > s->second.width || r.y1 > s->second.height || dstX < 0 || dstY < 0 ||
            dstX + r.Width() > d->second.width || dstY + r.Height() > d->second.height)
            Error("CopyTexture: region out of bounds " + Rect(r));
        ++stats_.copies;
        Record(Fmt("copy %s -> #%u at %d,%d ", TexName(src).c_str(), dst.id, dstX, dstY) + Rect(r));
    }

    void NullDevice::BeginProfile(ProfileCategory c)
    {
        if (!InFrame("BeginProfile"))
            return;
        if (profileDepth_++ != 0)
            Error("BeginProfile: scopes do not nest");
        static const char* names[] = {"capture", "layer", "fx", "fx-glass", "geometry"};
        Record(Fmt("profile %s", names[(int)c]));
    }

    void NullDevice::EndProfile()
    {
        if (!InFrame("EndProfile"))
            return;
        if (--profileDepth_ != 0)
            Error("EndProfile without BeginProfile");
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
