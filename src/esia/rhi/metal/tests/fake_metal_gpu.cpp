// Metal backend tests: the fake, rule-enforcing Gpu (see fake_metal_gpu.hpp).
#include "fake_metal_gpu.hpp"
#include "esia/render/shader_library.hpp"
#include "metal_planning.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace esia::rhi::metal::test
{
    namespace
    {
        int BytesPerPixelOf(std::uint32_t pixelFormat) { return InfoOf(FormatOfPixelFormat(pixelFormat)).bytesPerPixel; }

        // the bytes a declared buffer needs (the MSL structs of esia_common.hlsli)
        std::uint32_t RequiredBytes(const std::string& type)
        {
            if (type.find("WgtFrame") != std::string::npos)
                return 192;
            if (type.find("WgtPass") != std::string::npos || type.find("WgtDraw") != std::string::npos)
                return 32;
            return 16;   // gFxData: at least one float4
        }

        bool SrgbTwins(std::uint32_t a, std::uint32_t b)
        {
            const Format fa = FormatOfPixelFormat(a), fb = FormatOfPixelFormat(b);
            return fa != Format::Unknown && fb != Format::Unknown && fa != fb && RawFormat(fa) == RawFormat(fb);
        }
    }

    // not make_shared: see MetalDevice's constructor
    FakeMetalGpu::FakeMetalGpu(Options o) : options_(std::move(o)), shared_(new Shared) {}

    FakeMetalGpu::~FakeMetalGpu()
    {
        // the device released everything it created
        for (const Tex& t : textures_)
            if (t.live)
                Error("texture leaked");
        for (const Buf& b : buffers_)
            if (b.live)
                Error("buffer leaked");
        for (const Pso& p : psos_)
            if (p.live)
                Error("pipeline leaked");
        for (std::uint32_t c : counters_)
            if (c)
                Error("counter buffer leaked");
    }

    void FakeMetalGpu::Error(const std::string& what) { shared_->errors.push_back(what); }

    FakeMetalGpu::Tex* FakeMetalGpu::T(std::uint32_t id, const char* call)
    {
        if (id == 0 || id > textures_.size() || !textures_[id - 1].live)
        {
            Error(std::string(call) + ": dead or unknown texture");
            return nullptr;
        }
        return &textures_[id - 1];
    }

    std::uint32_t FakeMetalGpu::BaseOf(std::uint32_t id) const
    {
        return id && id <= textures_.size() && textures_[id - 1].base ? textures_[id - 1].base : id;
    }

    FakeMetalGpu::Buf* FakeMetalGpu::B(std::uint32_t id, const char* call)
    {
        if (id == 0 || id > buffers_.size() || !buffers_[id - 1].live)
        {
            Error(std::string(call) + ": dead or unknown buffer");
            return nullptr;
        }
        return &buffers_[id - 1];
    }

    bool FakeMetalGpu::InUse(bool texture, std::uint32_t id) const
    {
        for (const auto& [serial, cb] : cbs_)
            if (!cb.completed && (texture ? cb.textures.count(id) : cb.buffers.count(id)))
                return true;
        return false;
    }

    void FakeMetalGpu::UseTexture(std::uint32_t id)
    {
        if (recording_)
            recording_->textures.insert(id);
    }

    void FakeMetalGpu::UseBuffer(std::uint32_t id)
    {
        if (recording_)
            recording_->buffers.insert(id);
    }

    void FakeMetalGpu::Complete(Cb& cb)
    {
        if (cb.completed)
            return;
        cb.completed = true;
        cb.tracker->Complete(cb.serial);
    }

    void FakeMetalGpu::CompleteAll()
    {
        for (auto& [serial, cb] : cbs_)
            if (cb.committed)
                Complete(cb);
    }

    void FakeMetalGpu::CommitHost(std::uint64_t serial)
    {
        auto it = cbs_.find(serial);
        if (it == cbs_.end() || !it->second.host || &it->second == recording_)
            Error("CommitHost: no such host command buffer, or it is still recording");
        else
            it->second.committed = true;
    }

    const std::vector<std::uint8_t>* FakeMetalGpu::Pixels(std::uint32_t texture) const
    {
        const std::uint32_t base = BaseOf(texture);
        return base && base <= textures_.size() ? &textures_[base - 1].pixels : nullptr;
    }

    bool FakeMetalGpu::SupportsSampleCount(int samples) const
    {
        return std::find(options_.sampleCounts.begin(), options_.sampleCounts.end(), samples) != options_.sampleCounts.end();
    }

    // ------------------------------------------------------------------ objects
    std::uint32_t FakeMetalGpu::CreateTexture(const GpuTextureDesc& d)
    {
        const int bpp = BytesPerPixelOf(d.pixelFormat);
        if (d.width <= 0 || d.height <= 0 || d.width > options_.caps.maxTextureSize || d.height > options_.caps.maxTextureSize || bpp == 0 ||
            !SupportsSampleCount(d.samples))
        {
            Error("CreateTexture: invalid descriptor");
            return 0;
        }
        if (d.samples > 1 && !(d.usage & mtl::TextureUsageRenderTarget))
            Error("CreateTexture: a multisampled texture that is not a render target");
        Tex t;
        t.desc = d;
        t.desc.label = nullptr;
        t.pixels.assign((std::size_t)d.width * (std::size_t)d.height * (std::size_t)bpp, 0xCD);   // "undefined" contents
        textures_.push_back(std::move(t));
        ++shared_->stats.liveTextures;
        return (std::uint32_t)textures_.size();
    }

    std::uint32_t FakeMetalGpu::CreateTextureView(std::uint32_t texture, std::uint32_t pixelFormat)
    {
        Tex* t = T(texture, "CreateTextureView");
        if (!t)
            return 0;
        if (t->base)
            Error("CreateTextureView: a view of a view");
        if (!(t->desc.usage & mtl::TextureUsagePixelFormatView))
            Error("CreateTextureView: the texture has no MTLTextureUsagePixelFormatView");
        if (!SrgbTwins(t->desc.pixelFormat, pixelFormat))
            Error("CreateTextureView: the backend only reinterprets sRGB twins");
        Tex v;
        v.desc = t->desc;
        v.desc.pixelFormat = pixelFormat;
        v.base = texture;
        v.framebufferOnly = t->framebufferOnly;
        textures_.push_back(std::move(v));
        ++shared_->stats.views;
        ++shared_->stats.liveTextures;
        return (std::uint32_t)textures_.size();
    }

    void* FakeMetalGpu::MakeHostTexture(const HostTextureInfo& info)
    {
        hostTextures_.push_back(std::make_unique<HostTextureInfo>(info));
        return hostTextures_.back().get();
    }

    std::uint32_t FakeMetalGpu::AdoptTexture(void* native, HostTextureInfo& info)
    {
        for (const auto& h : hostTextures_)
            if (h.get() == native)
            {
                info = *h;
                GpuTextureDesc d;
                d.width = info.width;
                d.height = info.height;
                d.pixelFormat = info.pixelFormat;
                d.samples = info.samples;
                d.usage = info.usage;
                const std::uint32_t id = CreateTexture(d);
                if (id)
                    textures_[id - 1].framebufferOnly = info.framebufferOnly;
                return id;
            }
        return 0;
    }

    void FakeMetalGpu::ReleaseTexture(std::uint32_t texture)
    {
        Tex* t = T(texture, "ReleaseTexture");
        if (!t)
            return;
        if (InUse(true, texture))
            Error("ReleaseTexture: a command buffer that uses it has not completed");
        t->live = false;
        t->pixels.clear();
        --shared_->stats.liveTextures;
    }

    std::uint32_t FakeMetalGpu::CreateBuffer(std::size_t bytes, const char*)
    {
        if (bytes == 0)
        {
            Error("CreateBuffer: 0 bytes");
            return 0;
        }
        Buf b;
        b.data.assign(bytes, 0xCD);
        buffers_.push_back(std::move(b));
        ++shared_->stats.liveBuffers;
        return (std::uint32_t)buffers_.size();
    }

    void* FakeMetalGpu::BufferContents(std::uint32_t buffer)
    {
        Buf* b = B(buffer, "BufferContents");
        return b ? b->data.data() : nullptr;
    }

    void FakeMetalGpu::ReleaseBuffer(std::uint32_t buffer)
    {
        Buf* b = B(buffer, "ReleaseBuffer");
        if (!b)
            return;
        if (InUse(false, buffer))
            Error("ReleaseBuffer: a command buffer that uses it has not completed");
        b->live = false;
        b->data.clear();
        b->data.shrink_to_fit();
        --shared_->stats.liveBuffers;
    }

    std::uint32_t FakeMetalGpu::CreatePipeline(const GpuPipelineDesc& d, std::string& error)
    {
        Pso p;
        p.desc = d;
        p.vs = ParseMsl(d.vertexSource ? d.vertexSource : "");
        p.ps = ParseMsl(d.fragmentSource ? d.fragmentSource : "");
        if (!p.vs.ok || !p.ps.ok || !p.vs.vertex || p.ps.vertex)
        {
            error = "MSL does not parse: " + p.vs.error + p.ps.error;
            return 0;
        }
        const FormatInfo& f = InfoOf(FormatOfPixelFormat(d.pixelFormat));
        if (!f.renderable || !SupportsSampleCount(d.samples))
        {
            error = "not a renderable format / sample count";
            return 0;
        }
        // the vertex descriptor and the shader's stage_in must agree
        if (d.uiVertexLayout != !p.vs.inputs.empty())
            Error("CreatePipeline: vertex descriptor and stage_in disagree");
        // dual-source outputs need Source1 factors, and Source1 factors need them
        bool index1 = false;
        for (const MslVarying& o : p.ps.outputs)
            index1 |= o.attribute.find("index(1)") != std::string::npos;
        const bool source1 = d.blend.rgbSrc >= mtl::BlendFactorSource1Color || d.blend.rgbDst >= mtl::BlendFactorSource1Color;
        if (index1 != (d.blend.enabled && source1))
            Error("CreatePipeline: dual-source outputs and blend factors disagree");
        p.desc.vertexSource = p.desc.fragmentSource = nullptr;
        psos_.push_back(std::move(p));
        ++shared_->stats.pipelines;
        return (std::uint32_t)psos_.size();
    }

    void FakeMetalGpu::ReleasePipeline(std::uint32_t pipeline)
    {
        if (pipeline == 0 || pipeline > psos_.size() || !psos_[pipeline - 1].live)
            Error("ReleasePipeline: unknown");
        else
            psos_[pipeline - 1].live = false;
    }

    // ------------------------------------------------------------------ command buffers
    bool FakeMetalGpu::BeginCommandBuffer(void* native, std::uint64_t serial, const std::shared_ptr<FrameTracker>& tracker)
    {
        if (recording_)
            Error("BeginCommandBuffer while another one records");
        if (cbs_.count(serial))
            Error("BeginCommandBuffer: serial reused");
        Cb& cb = cbs_[serial];
        cb.serial = serial;
        cb.host = native != nullptr;
        cb.tracker = tracker;
        recording_ = &cb;
        ++shared_->stats.commandBuffers;
        return true;
    }

    void FakeMetalGpu::EndCommandBuffer()
    {
        if (!recording_)
        {
            Error("EndCommandBuffer without a command buffer");
            return;
        }
        if (enc_ != Enc::None)
            Error("EndCommandBuffer with an open encoder");
        if (!recording_->host)
        {
            recording_->committed = true;
            if (options_.autoComplete)
                Complete(*recording_);
        }
        recording_ = nullptr;
    }

    bool FakeMetalGpu::WaitForFrame(std::uint64_t serial)
    {
        auto it = cbs_.find(serial);
        if (it == cbs_.end())
            return true;
        if (!it->second.committed)
            return false;
        Complete(it->second);
        return true;
    }

    bool FakeMetalGpu::UseSamples(const EncoderSamples& s, int expected)
    {
        if (!s.buffer)
            return true;
        if (s.buffer > counters_.size() || !counters_[s.buffer - 1] || s.count != expected)
        {
            Error("counter samples: dead buffer or wrong count");
            return false;
        }
        for (int i = 0; i < s.count; ++i)
            if (s.index[i] >= counters_[s.buffer - 1])
                Error("counter samples: index out of range");
        if (recording_)
            recording_->counters.insert(s.buffer);
        return true;
    }

    bool FakeMetalGpu::BeginRenderPass(const RenderPassDesc& d)
    {
        if (!recording_ || enc_ != Enc::None)
        {
            Error("BeginRenderPass without a command buffer or with an open encoder");
            return false;
        }
        Tex* t = T(d.target, "BeginRenderPass");
        if (!t)
            return false;
        if (!(t->desc.usage & mtl::TextureUsageRenderTarget))
            Error("BeginRenderPass: the attachment is not a render target");
        const bool resolve = d.storeAction == mtl::StoreActionStoreAndMultisampleResolve || d.storeAction == mtl::StoreActionMultisampleResolve;
        if (resolve != (d.resolveTarget != 0))
            Error("BeginRenderPass: a resolve store action needs a resolve texture and the other way round");
        if (d.resolveTarget)
        {
            Tex* r = T(d.resolveTarget, "BeginRenderPass resolve");
            if (r && (t->desc.samples == 1 || r->desc.samples != 1 || r->desc.pixelFormat != t->desc.pixelFormat || r->desc.width != t->desc.width ||
                      r->desc.height != t->desc.height || !(r->desc.usage & mtl::TextureUsageRenderTarget)))
                Error("BeginRenderPass: resolve texture does not match the attachment");
            if (r)
            {
                // executed at the end of the pass; the fake keeps one value per pixel, so the resolve is a copy
                textures_[BaseOf(d.resolveTarget) - 1].pixels = textures_[BaseOf(d.target) - 1].pixels;
                UseTexture(d.resolveTarget);
                ++shared_->stats.resolves;
            }
        }
        if (d.loadAction == mtl::LoadActionClear)
        {
            const Format f = FormatOfPixelFormat(t->desc.pixelFormat);
            if (f == Format::RGBA8_UNORM || f == Format::BGRA8_UNORM)
            {
                std::uint8_t c[4];
                for (int i = 0; i < 4; ++i)
                    c[i] = (std::uint8_t)std::lround(std::clamp(d.clearColor[f == Format::BGRA8_UNORM && i != 3 ? 2 - i : i], 0.0f, 1.0f) * 255.0f);
                std::vector<std::uint8_t>& px = textures_[BaseOf(d.target) - 1].pixels;
                for (std::size_t i = 0; i < px.size(); i += 4)
                    std::memcpy(&px[i], c, 4);
            }
        }
        UseSamples(d.samples, 4);
        UseTexture(d.target);
        enc_ = Enc::Render;
        attachment_ = d.target;
        pipeline_ = 0;
        vertexBound_.clear();
        fragmentBound_.clear();
        fragmentTextures_.clear();
        samplers_ = false;
        ++shared_->stats.renderPasses;
        return true;
    }

    bool FakeMetalGpu::BeginBlitPass(const EncoderSamples& samples)
    {
        if (!recording_ || enc_ != Enc::None)
        {
            Error("BeginBlitPass without a command buffer or with an open encoder");
            return false;
        }
        UseSamples(samples, 2);
        enc_ = Enc::Blit;
        ++shared_->stats.blitPasses;
        return true;
    }

    void FakeMetalGpu::EndEncoder()
    {
        if (enc_ == Enc::None)
            Error("EndEncoder without an encoder");
        enc_ = Enc::None;
        attachment_ = 0;
    }

    void* FakeMetalGpu::NativeRenderEncoder()
    {
        if (!Render("NativeRenderEncoder"))
            return nullptr;
        return this;
    }

    // ------------------------------------------------------------------ render encoder
    bool FakeMetalGpu::Render(const char* call)
    {
        if (enc_ != Enc::Render)
            Error(std::string(call) + " outside a render encoder");
        return enc_ == Enc::Render;
    }

    bool FakeMetalGpu::Blit(const char* call)
    {
        if (enc_ != Enc::Blit)
            Error(std::string(call) + " outside a blit encoder");
        return enc_ == Enc::Blit;
    }

    void FakeMetalGpu::SetPipeline(std::uint32_t pipeline)
    {
        if (!Render("SetPipeline"))
            return;
        if (pipeline == 0 || pipeline > psos_.size() || !psos_[pipeline - 1].live)
        {
            Error("SetPipeline: unknown pipeline");
            return;
        }
        const Tex& a = textures_[attachment_ - 1];
        const GpuPipelineDesc& d = psos_[pipeline - 1].desc;
        if (d.pixelFormat != a.desc.pixelFormat || d.samples != a.desc.samples)
            Error("SetPipeline: pixel format / sample count differ from the attachment's");
        pipeline_ = pipeline;
    }

    void FakeMetalGpu::SetViewport(int width, int height)
    {
        if (Render("SetViewport") && (width != textures_[attachment_ - 1].desc.width || height != textures_[attachment_ - 1].desc.height))
            Error("SetViewport: not the whole attachment");
    }

    void FakeMetalGpu::SetScissor(const IRect& r)
    {
        if (!Render("SetScissor"))
            return;
        const Tex& a = textures_[attachment_ - 1];
        if (!RectInside(r, a.desc.width, a.desc.height))
            Error("SetScissor: empty or outside the attachment");
    }

    void FakeMetalGpu::SetBytes(std::uint32_t stages, std::uint32_t index, const void* data, std::uint32_t size)
    {
        if (!Render("SetBytes"))
            return;
        if (!data || size == 0 || size > binding::kMaxInlineBytes)
            Error("SetBytes: no data or more than 4 KB");
        if (stages & StageVertex)
            vertexBound_[index] = {size};
        if (stages & StageFragment)
            fragmentBound_[index] = {size};
    }

    void FakeMetalGpu::SetBuffer(std::uint32_t stages, std::uint32_t index, std::uint32_t buffer, std::size_t offset)
    {
        if (!Render("SetBuffer"))
            return;
        Buf* b = B(buffer, "SetBuffer");
        if (!b)
            return;
        if (offset >= b->data.size())
            Error("SetBuffer: offset past the end");
        UseBuffer(buffer);
        const Bound bound{(std::uint32_t)(b->data.size() - std::min(offset, b->data.size()))};
        if (stages & StageVertex)
            vertexBound_[index] = bound;
        if (stages & StageFragment)
            fragmentBound_[index] = bound;
    }

    void FakeMetalGpu::SetFragmentTexture(std::uint32_t index, std::uint32_t texture)
    {
        if (!Render("SetFragmentTexture"))
            return;
        Tex* t = T(texture, "SetFragmentTexture");
        if (!t)
            return;
        if (BaseOf(texture) == BaseOf(attachment_))
            Error("SetFragmentTexture: the render pass's attachment");
        if (!(t->desc.usage & mtl::TextureUsageShaderRead) || t->desc.samples != 1)
            Error("SetFragmentTexture: no ShaderRead usage, or multisampled");
        if (InfoOf(FormatOfPixelFormat(t->desc.pixelFormat)).srgb)
            Error("SetFragmentTexture: sampled through an sRGB format (the RHI never decodes sRGB)");
        UseTexture(texture);
        fragmentTextures_.insert(index);
    }

    void FakeMetalGpu::SetFragmentSamplers()
    {
        if (Render("SetFragmentSamplers"))
            samplers_ = true;
    }

    void FakeMetalGpu::CheckDraw(const char* call, std::uint32_t primitive)
    {
        if (!Render(call))
            return;
        if (!pipeline_)
        {
            Error(std::string(call) + " without a pipeline");
            return;
        }
        const Pso& p = psos_[pipeline_ - 1];
        const std::uint32_t want = p.desc.program == ShaderProgram::Fx ? mtl::PrimitiveTypeTriangleStrip : mtl::PrimitiveTypeTriangle;
        if (primitive != want)
            Error(std::string(call) + ": primitive type does not fit the program");
        for (const MslInterface* m : {&p.vs, &p.ps})
        {
            const std::map<std::uint32_t, Bound>& bound = m->vertex ? vertexBound_ : fragmentBound_;
            for (const MslResource& r : m->resources)
            {
                const std::string what = std::string(call) + " (" + ShaderProgramName(p.desc.program) + (m->vertex ? " vs" : " ps") + "): " + r.name;
                if (r.kind == MslResource::Buffer)
                {
                    auto it = bound.find((std::uint32_t)r.index);
                    if (it == bound.end())
                        Error(what + " not bound");
                    else if (it->second.size < RequiredBytes(r.type))
                        Error(what + " smaller than its struct");
                }
                else if (r.kind == MslResource::Texture && !fragmentTextures_.count((std::uint32_t)r.index))
                    Error(what + " not bound");
                else if (r.kind == MslResource::Sampler && !samplers_)
                    Error(what + " not bound");
            }
        }
        if (p.desc.uiVertexLayout && !vertexBound_.count(binding::kVertexBuffer))
            Error(std::string(call) + ": no vertex buffer at index 30");
        ++shared_->stats.draws;
    }

    void FakeMetalGpu::Draw(std::uint32_t primitive, std::uint32_t, std::uint32_t count, std::uint32_t instances)
    {
        CheckDraw("Draw", primitive);
        if (count == 0 || instances == 0)
            Error("Draw: nothing to draw");
    }

    void FakeMetalGpu::DrawIndexed(std::uint32_t count, std::uint32_t indexBuffer, std::size_t offset)
    {
        CheckDraw("DrawIndexed", mtl::PrimitiveTypeTriangle);
        Buf* b = B(indexBuffer, "DrawIndexed");
        if (!b)
            return;
        UseBuffer(indexBuffer);
        if (offset % 4 != 0 || offset + (std::size_t)count * 4 > b->data.size() || count % 3 != 0)
            Error("DrawIndexed: misaligned or out-of-range indices");
    }

    // ------------------------------------------------------------------ blit encoder
    void FakeMetalGpu::CopyTexture(std::uint32_t src, const IRect& r, std::uint32_t dst, int dstX, int dstY)
    {
        if (!Blit("CopyTexture"))
            return;
        Tex* s = T(src, "CopyTexture src");
        Tex* d = T(dst, "CopyTexture dst");
        if (!s || !d)
            return;
        if (s->desc.pixelFormat != d->desc.pixelFormat)
            Error("CopyTexture: pixel formats differ (a view is needed)");
        if (s->desc.samples != 1 || d->desc.samples != 1 || s->framebufferOnly || d->framebufferOnly)
            Error("CopyTexture: multisampled or framebufferOnly texture");
        if (!RectInside(r, s->desc.width, s->desc.height) || !RectInside(IRect{dstX, dstY, dstX + r.Width(), dstY + r.Height()}, d->desc.width, d->desc.height))
        {
            Error("CopyTexture: region out of bounds");
            return;
        }
        const std::size_t bpp = (std::size_t)BytesPerPixelOf(s->desc.pixelFormat);
        const std::vector<std::uint8_t>& sp = textures_[BaseOf(src) - 1].pixels;
        std::vector<std::uint8_t>& dp = textures_[BaseOf(dst) - 1].pixels;
        for (int y = 0; y < r.Height(); ++y)
            std::memcpy(&dp[((std::size_t)(dstY + y) * (std::size_t)d->desc.width + (std::size_t)dstX) * bpp],
                        &sp[((std::size_t)(r.y0 + y) * (std::size_t)s->desc.width + (std::size_t)r.x0) * bpp], (std::size_t)r.Width() * bpp);
        UseTexture(src);
        UseTexture(dst);
        ++shared_->stats.copies;
    }

    void FakeMetalGpu::CopyBufferToTexture(std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow, std::uint32_t dst, const IRect& r)
    {
        if (!Blit("CopyBufferToTexture"))
            return;
        Buf* b = B(buffer, "CopyBufferToTexture");
        Tex* d = T(dst, "CopyBufferToTexture");
        if (!b || !d)
            return;
        const std::size_t bpp = (std::size_t)BytesPerPixelOf(d->desc.pixelFormat), row = (std::size_t)r.Width() * bpp;
        if (d->desc.samples != 1 || d->framebufferOnly || !RectInside(r, d->desc.width, d->desc.height) || offset % bpp || bytesPerRow % bpp ||
            bytesPerRow < row || offset + bytesPerRow * (std::size_t)(r.Height() - 1) + row > b->data.size())
        {
            Error("CopyBufferToTexture: bad layout, region or texture");
            return;
        }
        std::vector<std::uint8_t>& dp = textures_[BaseOf(dst) - 1].pixels;
        for (int y = 0; y < r.Height(); ++y)
            std::memcpy(&dp[((std::size_t)(r.y0 + y) * (std::size_t)d->desc.width + (std::size_t)r.x0) * bpp], &b->data[offset + bytesPerRow * (std::size_t)y], row);
        UseBuffer(buffer);
        UseTexture(dst);
        ++shared_->stats.uploads;
    }

    void FakeMetalGpu::CopyTextureToBuffer(std::uint32_t src, const IRect& r, std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow)
    {
        if (!Blit("CopyTextureToBuffer"))
            return;
        Buf* b = B(buffer, "CopyTextureToBuffer");
        Tex* s = T(src, "CopyTextureToBuffer");
        if (!b || !s)
            return;
        const std::size_t bpp = (std::size_t)BytesPerPixelOf(s->desc.pixelFormat), row = (std::size_t)r.Width() * bpp;
        if (s->desc.samples != 1 || s->framebufferOnly || !RectInside(r, s->desc.width, s->desc.height) || offset % bpp || bytesPerRow % bpp ||
            bytesPerRow < row || offset + bytesPerRow * (std::size_t)(r.Height() - 1) + row > b->data.size())
        {
            Error("CopyTextureToBuffer: bad layout, region or texture");
            return;
        }
        const std::vector<std::uint8_t>& sp = textures_[BaseOf(src) - 1].pixels;
        for (int y = 0; y < r.Height(); ++y)
            std::memcpy(&b->data[offset + bytesPerRow * (std::size_t)y], &sp[((std::size_t)(r.y0 + y) * (std::size_t)s->desc.width + (std::size_t)r.x0) * bpp], row);
        UseBuffer(buffer);
        UseTexture(src);
        ++shared_->stats.readbacks;
    }

    // ------------------------------------------------------------------ timestamps
    std::uint32_t FakeMetalGpu::CreateCounterBuffer(std::uint32_t samples)
    {
        if (!options_.caps.timestamps || samples == 0 || samples * 8 > 32768)
        {
            Error("CreateCounterBuffer: unsupported or larger than 32 KB");
            return 0;
        }
        counters_.push_back(samples);
        return (std::uint32_t)counters_.size();
    }

    void FakeMetalGpu::ReleaseCounterBuffer(std::uint32_t buffer)
    {
        if (buffer == 0 || buffer > counters_.size() || !counters_[buffer - 1])
            Error("ReleaseCounterBuffer: unknown");
        else
            counters_[buffer - 1] = 0;
    }

    bool FakeMetalGpu::ResolveCounters(std::uint32_t buffer, std::uint32_t first, std::uint32_t count, std::uint64_t* out)
    {
        if (buffer == 0 || buffer > counters_.size() || !counters_[buffer - 1] || first + count > counters_[buffer - 1])
        {
            Error("ResolveCounters: unknown buffer or range");
            return false;
        }
        for (const auto& [serial, cb] : cbs_)
            if (!cb.completed && cb.counters.count(buffer))
                Error("ResolveCounters before the command buffer completed");
        // every sample 1 us after the previous one: encoder i spans [4i, 4i + 3] us
        for (std::uint32_t i = 0; i < count; ++i)
            out[i] = 1000000 + (std::uint64_t)(first + i) * 1000;
        return true;
    }

    bool FakeMetalGpu::SampleClock(std::uint64_t& cpuNs, std::uint64_t& gpuTicks)
    {
        clock_ += 16000000;   // a frame every 16 ms, GPU ticks in ns
        cpuNs = gpuTicks = clock_;
        return true;
    }
}
