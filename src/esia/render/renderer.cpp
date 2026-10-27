// Esia - renderer (see esia/render/renderer.hpp): executes a FramePlan through an rhi::Device.
// The capture, pyramid and glow-layer logic is the one of WGT's src/backends/d3d11_backend.cpp, on the RHI.
#include "esia/render/renderer.hpp"
#include "gpu_constants.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <unordered_map>

namespace esia::render
{
    namespace
    {
        // Scissor of a pixel rect: the ImGui / WGT convention (truncate both edges, clamp to the target).
        rhi::IRect ToScissor(const PxRect& r, int w, int h)
        {
            rhi::IRect s;
            s.x0 = (int)std::max(0.0f, r.x0);
            s.y0 = (int)std::max(0.0f, r.y0);
            s.x1 = (int)std::min((float)w, r.x1);
            s.y1 = (int)std::min((float)h, r.y1);
            s.x1 = std::max(s.x1, s.x0);
            s.y1 = std::max(s.y1, s.y0);
            return s;
        }

        rhi::Format FormatOf(TextureFormat f) { return f == TextureFormat::Alpha8 ? rhi::Format::R8_UNORM : rhi::Format::RGBA8_UNORM; }

        enum Run { RunNone = -1, RunCapture = (int)rhi::ProfileCategory::Capture, RunLayer = (int)rhi::ProfileCategory::Layer,
                   RunFx = (int)rhi::ProfileCategory::Fx, RunFxGlass = (int)rhi::ProfileCategory::FxGlass,
                   RunGeometry = (int)rhi::ProfileCategory::Geometry };

        struct DeviceTexture
        {
            rhi::Texture tex;
            TextureInfo info;
        };

        struct PipelineKey
        {
            rhi::ShaderProgram program;
            rhi::BlendMode blend;
            rhi::Format format;
            int samples;
            EffectId effect;
            std::uint32_t features;   // FX shader variant (Caps::fxFeatureVariants), 0 = every feature
            bool operator==(const PipelineKey&) const = default;
        };

        struct PipelineKeyHash
        {
            std::size_t operator()(const PipelineKey& k) const
            {
                const std::uint64_t a = (std::uint64_t)k.program | ((std::uint64_t)k.blend << 8) | ((std::uint64_t)k.format << 16) |
                                        ((std::uint64_t)(k.samples & 0xFF) << 24) | ((std::uint64_t)k.effect << 32);
                return IntHash()(a ^ ((std::uint64_t)k.features * 0x9E3779B97F4A7C15ull));
            }
        };
    }

    struct Renderer::Impl
    {
        explicit Impl(rhi::Device& d) : dev(d), caps(d.GetCaps()) {}

        rhi::Device& dev;
        const rhi::Caps caps;
        FramePlan plan;
        RenderStats stats;

        std::unordered_map<TextureId, DeviceTexture, IntHash> textures;
        std::vector<TextureChange> changes;
        std::unordered_map<PipelineKey, rhi::Pipeline, PipelineKeyHash> pipelines;
        struct Effect
        {
            std::string name, source;
        };
        std::unordered_map<EffectId, Effect, IntHash> effects;
        std::unordered_map<EffectId, std::uint64_t> effectTried;   // frame of the last pipeline attempt
        std::vector<EffectId> prewarm;                             // effects to start compiling at the next frame
        std::vector<EffectId> warmUp;                              // prewarmed effects not drawn yet (WarmUp)
        std::uint64_t frame = 0;

        rhi::Texture white;
        rhi::Buffer vb, ib, fxBuf;
        std::size_t vbCap = 0, ibCap = 0, fxCap = 0;
        rhi::Texture fxTex;
        int fxTexW = 0, fxTexH = 0, fxPerRow = 1;
        float quadStart = 0.0f;   // where the quads start in the instance data (gConv.w): a row of the buffer or texture
        std::vector<float> staging;

        // size-dependent surfaces
        rhi::Texture copy, levels[kBackdropLevels], layer;
        int surfW = 0, surfH = 0;
        rhi::Format surfFormat = rhi::Format::Unknown;
        // whether all of a pyramid level holds defined values: not when new, nor after a DontCare pass (a tiler
        // stores garbage where the pass did not write)
        bool levelDefined[kBackdropLevels] = {};

        // Temporal reuse (RenderParams::targetStatic / backdropReuseFrames). Every capture - a region and pyramid,
        // RenderOp::captureShape - is kept from frame to frame: what it saw, whether its levels are still in the copy
        // and the pyramid (nothing wrote over the region its glass reads since), and a copy of them aside, which brings
        // them back when something did: neighbouring captures share the pyramid and write over each other's margins,
        // so most captures of a page of glass would be taken again every frame. The copies aside are made, before a
        // frame's passes, for the captures whose content did not change since the last frame.
        struct KeptCapture
        {
            std::uint64_t shape = 0, key = 0;   // RenderOp::captureShape / the captureKey of what its levels hold
            PxRect wrote, read, level0;         // where it writes the pyramid; where its glass reads; what it copied
            int levels = 0;
            int age = 0;                        // frames since it was taken (a reuse of the same content is as new)
            bool intact = false;                // its levels are in the copy and the pyramid
            bool cached = false;                // ... and in `cache`
            bool planned = false, done = false;   // in this frame's plan; served this frame
            rhi::Texture cache[kBackdropLevels];   // [0] its part of the copy, [l] its texels of level l
        };
        static constexpr std::size_t kMaxKept = 64;
        std::vector<KeptCapture> kept;
        bool reuseExact = false;
        int reuseFrames = 0;
        float light[3] = {0, 0, 0};   // RenderParams::light in the Draw constants (gDrawInfo.yzw, esia_fx.hlsl)

        // this frame
        rhi::Texture target;
        rhi::TextureDesc targetDesc;
        int W = 0, H = 0;
        FrameConstants frameMain{}, frameLayer{};
        int layerDepth = 0;
        bool subpixelText = true;       // RenderParams::subpixelText
        bool layerPending = false;      // the open layer has not been cleared yet (its pass did not start)
        PxRect layerRegion;             // ... and the region to clear then
        fx::LayerParams activeLayer{};
        int captures = 0, budget = 64;
        // where glass gets this target's backdrop: a copy (CopySrc), a direct read (Sampled), or nowhere
        bool canCopy = false, canRead = false, backdrop = false;
        // the pyramid passes keep what the levels hold outside the region they write (section "keepLevels" below)
        bool keepLevels = false;
        int run = RunNone;
        bool profile = true;
        int profileScopes = 0, maxProfileScopes = 32;

        // what the current pass has bound (every pass starts empty, rhi.hpp)
        rhi::Texture passTarget;
        rhi::Format passFormat = rhi::Format::Unknown;
        int passSamples = 1;
        const FrameConstants* passFrame = nullptr;
        bool inPass = false, frameSet = false, geometryBound = false, fxDataBound = false;
        rhi::Pipeline boundPipeline;
        rhi::Texture boundTex[rhi::kTextureSlots];
        rhi::IRect boundScissor;
        bool scissorSet = false;
        DrawConstants boundDraw{};
        bool drawSet = false;

        // ------------------------------------------------------------ resources
        void Release(rhi::Texture& t)
        {
            if (t)
                dev.DestroyTexture(t);
            t = {};
        }

        void ReleaseSurfaces()
        {
            for (KeptCapture& k : kept)   // the captures they held are gone
                for (rhi::Texture& t : k.cache)
                    Release(t);
            kept.clear();
            Release(copy);
            for (rhi::Texture& l : levels)
                Release(l);
            for (bool& d : levelDefined)
                d = false;
            Release(layer);
            surfW = surfH = 0;
            surfFormat = rhi::Format::Unknown;
        }

        void ReleaseAll()
        {
            ReleaseSurfaces();
            for (auto& [id, t] : textures)
                Release(t.tex);
            textures.clear();
            Release(white);
            Release(fxTex);
            for (rhi::Buffer* b : {&vb, &ib, &fxBuf})
                if (*b)
                {
                    dev.DestroyBuffer(*b);
                    *b = {};
                }
            for (auto& [key, p] : pipelines)
                if (p)
                    dev.DestroyPipeline(p);
            pipelines.clear();
        }

        rhi::Format LayerFormat() const { return caps.floatRenderTargets ? rhi::Format::RGBA16_FLOAT : rhi::Format::RGBA8_UNORM; }

        rhi::Texture MakeTexture(int w, int h, rhi::Format f, std::uint32_t usage, const char* name, const void* data = nullptr)
        {
            rhi::TextureDesc d;
            d.width = w;
            d.height = h;
            d.format = f;
            d.usage = usage;
            d.debugName = name;
            return dev.CreateTexture(d, data, 0);
        }

        // The backdrop copy (glass on a copyable target) and the pyramid + layer (glass and glow layers), created
        // before the first pass of the frame that needs them, at the target's size.
        bool EnsureSurfaces(bool copyNeeded, bool glowLayer)
        {
            if (surfW != W || surfH != H || surfFormat != targetDesc.format)
                ReleaseSurfaces();
            surfW = W;
            surfH = H;
            surfFormat = targetDesc.format;
            static const char* kLevelNames[kBackdropLevels] = {"", "pyramid-1", "pyramid-2", "pyramid-3", "pyramid-4", "pyramid-5"};
            bool ok = true;
            if (copyNeeded && !copy)
            {
                // zeroed: glass past the capture budget or a user effect may read where no capture copied yet
                const rhi::Format f = rhi::RawFormat(targetDesc.format);
                const std::vector<std::uint8_t> zeros((std::size_t)W * (std::size_t)H * (std::size_t)rhi::BytesPerPixel(f), 0);
                ok = (bool)(copy = MakeTexture(W, H, f, rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst | rhi::TextureUsage_CopySrc, "backdrop-copy",
                                               zeros.data()));
            }
            // copied from and to as well: captures kept aside for temporal reuse
            const std::uint32_t levelUsage = rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled | rhi::TextureUsage_CopySrc | rhi::TextureUsage_CopyDst;
            for (int l = 1; ok && l < kBackdropLevels; ++l)
                if (!levels[l])
                {
                    ok = (bool)(levels[l] = MakeTexture(LevelSize(W, l), LevelSize(H, l), LayerFormat(), levelUsage, kLevelNames[l]));
                    levelDefined[l] = false;
                }
            if (ok && glowLayer && !layer)
                ok = (bool)(layer = MakeTexture(W, H, LayerFormat(), rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled, "glow-layer"));
            if (!ok)
                ReleaseSurfaces();
            return ok;
        }

        void ApplyTextureChanges(TextureRegistry* registry)
        {
            if (!registry)
                return;
            registry->TakeChanges(changes);
            for (TextureChange& c : changes)
            {
                switch (c.kind)
                {
                case TextureChange::Kind::Create:
                {
                    rhi::TextureDesc d;
                    d.width = c.width;
                    d.height = c.height;
                    d.format = FormatOf(c.info.format);
                    d.usage = rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst;
                    d.debugName = c.info.format == TextureFormat::Alpha8 ? "glyphs" : "image";
                    const rhi::Texture t = dev.CreateTexture(d, c.pixels.data(), 0);
                    if (t)
                        textures[c.id] = {t, c.info};
                    break;
                }
                case TextureChange::Kind::Update:
                {
                    auto it = textures.find(c.id);
                    if (it != textures.end())
                        dev.UpdateTexture(it->second.tex, rhi::IRect{c.x, c.y, c.x + c.width, c.y + c.height}, c.pixels.data(), 0);
                    break;
                }
                case TextureChange::Kind::Destroy:
                {
                    auto it = textures.find(c.id);
                    if (it != textures.end())
                    {
                        dev.DestroyTexture(it->second.tex);
                        textures.erase(it);
                    }
                    break;
                }
                }
            }
            changes.clear();
        }

        bool EnsureBuffer(rhi::Buffer& buf, std::size_t& cap, std::size_t bytes, rhi::BufferKind kind, const char* name)
        {
            if (buf && cap >= bytes)
                return true;
            if (buf)
                dev.DestroyBuffer(buf);
            cap = std::max<std::size_t>(bytes + bytes / 2, 4096);
            rhi::BufferDesc d;
            d.kind = kind;
            d.size = cap;
            d.debugName = name;
            buf = dev.CreateBuffer(d);
            return (bool)buf;
        }

        // An instance as the GPU reads it: its integer row (flags) as float values (FxFetch reads every row as float4).
        static void WriteInstance(float* dst, const fx::Instance& in)
        {
            std::memcpy(dst, &in, sizeof(fx::Instance));
            for (int k = 0; k < 4; ++k)
                dst[(fx::kInstanceVec4Count - 1) * 4 + (std::size_t)k] = (float)in.flags[k];
        }

        // The plan's instances, in op order, into `dst` (`plan.fxCount` of them)
        void WriteInstances(float* dst) const
        {
            if (!plan.direct)
            {
                for (const fx::Instance& in : plan.instances)
                {
                    WriteInstance(dst, in);
                    dst += fx::kInstanceVec4Count * 4;
                }
                return;
            }
            for (const FramePlan::InstanceRun& span : plan.instanceRuns)
                for (std::uint32_t i = 0; i < span.count; ++i)
                {
                    WriteInstance(dst, span.first[i]);
                    dst += fx::kInstanceVec4Count * 4;
                }
        }

        // The plan's quads, in op order, into `dst` (`plan.quadCount` of them)
        void WriteQuads(unsigned char* dst) const
        {
            for (const FramePlan::QuadRun& r : plan.quadRuns)
            {
                std::memcpy(dst, r.first, (std::size_t)r.count * sizeof(QuadInstance));
                dst += (std::size_t)r.count * sizeof(QuadInstance);
            }
        }

        // Vertex, index and instance data (FX instances, then quads), once per frame before the first pass. A direct
        // plan's (a device with Caps::baseVertex) go from the draw lists into the mapped buffers: each list's vertices
        // and indices as they are, at its offsets.
        bool Upload(int maxPerRow)
        {
            const std::size_t vcount = plan.direct ? plan.totalVertices : plan.vertices.size();
            const std::size_t icount = plan.direct ? plan.totalIndices : plan.indices.size();
            if (vcount > 0)
            {
                const std::size_t vbytes = vcount * sizeof(Vertex), ibytes = icount * sizeof(std::uint32_t);
                if (!EnsureBuffer(vb, vbCap, vbytes, rhi::BufferKind::Vertex, "ui-vertices") ||
                    !EnsureBuffer(ib, ibCap, std::max<std::size_t>(ibytes, 4), rhi::BufferKind::Index, "ui-indices"))
                    return false;
                if (plan.direct)
                {
                    auto* v = static_cast<unsigned char*>(dev.MapBuffer(vb, vbytes));
                    if (!v)
                        return false;
                    for (const FramePlan::ListGeometry& l : plan.lists)
                        if (!l.list->Vertices().empty())
                            std::memcpy(v + (std::size_t)l.firstVertex * sizeof(Vertex), l.list->Vertices().data(), l.list->Vertices().size() * sizeof(Vertex));
                    dev.UnmapBuffer(vb);
                    if (ibytes)
                    {
                        auto* x = static_cast<unsigned char*>(dev.MapBuffer(ib, ibytes));
                        if (!x)
                            return false;
                        for (const FramePlan::ListGeometry& l : plan.lists)
                            if (!l.list->Indices().empty())
                                std::memcpy(x + (std::size_t)l.firstIndex * sizeof(std::uint32_t), l.list->Indices().data(),
                                            l.list->Indices().size() * sizeof(std::uint32_t));
                        dev.UnmapBuffer(ib);
                    }
                }
                else
                {
                    dev.UpdateBuffer(vb, plan.vertices.data(), vbytes);
                    if (ibytes)
                        dev.UpdateBuffer(ib, plan.indices.data(), ibytes);
                }
            }
            const std::size_t n = (std::size_t)plan.fxCount, nq = (std::size_t)plan.quadCount;
            quadStart = 0.0f;
            if (n == 0 && nq == 0)
                return true;
            const bool texture = caps.fxStorage == rhi::FxStorage::Texture;
            fxPerRow = texture ? std::max(1, caps.maxFxDataWidth / (int)fx::kInstanceVec4Count) : 1;
            if (texture && maxPerRow > 0)
                fxPerRow = std::min(fxPerRow, maxPerRow);
            if (!texture)
            {
                // written in place: one copy from the draw lists (or the plan) to the GPU, the quads after the FX
                // instances
                const std::size_t fxBytes = n * sizeof(fx::Instance), bytes = fxBytes + nq * sizeof(QuadInstance);
                if (!EnsureBuffer(fxBuf, fxCap, bytes, rhi::BufferKind::FxInstances, "fx-instances"))
                    return false;
                auto* dst = static_cast<unsigned char*>(dev.MapBuffer(fxBuf, bytes));
                if (!dst)
                    return false;
                WriteInstances(reinterpret_cast<float*>(dst));
                WriteQuads(dst + fxBytes);
                dev.UnmapBuffer(fxBuf);
                quadStart = (float)(n * fx::kInstanceVec4Count);
                return true;
            }
            // The texture: fxPerRow FX instances to a row (24 texels each), then the quads from the next row on, 8 in
            // the room of an FX instance - 16 of its 24 texels, so that a row holds a power of two of them (SM3 finds
            // a quad's row with a float modulo, exact only then; esia_quad.hlsl)
            const std::size_t perRow = (std::size_t)fxPerRow, quadsPerRow = perRow * 8;
            const std::size_t fxRows = (n + perRow - 1) / perRow, quadRows = (nq + quadsPerRow - 1) / quadsPerRow;
            const int w = fxPerRow * (int)fx::kInstanceVec4Count, h = (int)(fxRows + quadRows);
            const std::size_t rowFloats = (std::size_t)w * 4;
            staging.resize((std::size_t)h * rowFloats);
            WriteInstances(staging.data());
            std::size_t q = 0;
            for (const FramePlan::QuadRun& r : plan.quadRuns)
                for (std::uint32_t i = 0; i < r.count; ++i, ++q)
                    std::memcpy(staging.data() + (fxRows + q / quadsPerRow) * rowFloats + (q % quadsPerRow) * 8, &r.first[i], sizeof(QuadInstance));
            quadStart = (float)fxRows;
            if (!fxTex || fxTexW != w || fxTexH < h)
            {
                Release(fxTex);
                fxTexW = w;
                fxTexH = std::max(h, fxTexH * 2);
                fxTexH = std::min(std::max(fxTexH, h), caps.maxTextureSize);
                if (h > fxTexH)
                    return false;
                fxTex = MakeTexture(fxTexW, fxTexH, rhi::Format::RGBA32_FLOAT, rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst, "fx-instances");
                if (!fxTex)
                    return false;
            }
            // the full rows, then the used part of the last one: the texels past the last instance or quad are never read
            const int rowPitch = w * (int)sizeof(float) * 4;
            if (h > 1)
                dev.UpdateTexture(fxTex, rhi::IRect{0, 0, w, h - 1}, staging.data(), rowPitch);
            const std::size_t used = nq > 0 ? (nq - (quadRows - 1) * quadsPerRow) * 2 : (n - (fxRows - 1) * perRow) * fx::kInstanceVec4Count;
            dev.UpdateTexture(fxTex, rhi::IRect{0, h - 1, (int)used, h}, staging.data() + (std::size_t)(h - 1) * rowFloats, rowPitch);
            return true;
        }

        void FillFrameConstants(const DrawData& dd, const RenderParams& p)
        {
            FrameConstants& fc = frameMain;
            const float sx = dd.framebufferScale.x, sy = dd.framebufferScale.y;
            const float L = dd.displayPos.x, T = dd.displayPos.y;
            // the viewport is the whole target: pixel = (ui - displayPos) * framebufferScale
            fc.xform[0] = 2.0f * sx / (float)W;
            fc.xform[1] = -2.0f * sy / (float)H;
            fc.xform[2] = -1.0f - 2.0f * sx * L / (float)W;
            fc.xform[3] = 1.0f + 2.0f * sy * T / (float)H;
            if (caps.clipSpaceYDown)
            {
                fc.xform[1] = -fc.xform[1];
                fc.xform[3] = -fc.xform[3];
            }
            fc.target[0] = (float)W;
            fc.target[1] = (float)H;
            fc.target[2] = 1.0f / (float)W;
            fc.target[3] = 1.0f / (float)H;
            fc.display[0] = L;
            fc.display[1] = T;
            fc.display[2] = sx;
            fc.display[3] = sy;
            fc.time[0] = (float)std::fmod(dd.time, 3600.0);
            fc.time[1] = dd.deltaTime;
            fc.time[2] = backdrop ? 1.0f : 0.0f;
            fc.time[3] = rhi::IsSrgb(targetDesc.format) ? 1.0f : 0.0f;
            for (int l = 0; l < kBackdropLevels; ++l)
            {
                const int w = LevelSize(W, l), h = LevelSize(H, l);
                fc.level[l][0] = (float)w;
                fc.level[l][1] = (float)h;
                fc.level[l][2] = 1.0f / (float)w;
                fc.level[l][3] = 1.0f / (float)h;
            }
            fc.text[0] = p.text.gamma;
            fc.text[1] = p.text.grayscaleContrast;
            fc.text[2] = p.text.clearTypeContrast;
            fc.text[3] = p.text.clearTypeLevel;
            fc.conv[0] = caps.framebufferOriginBottomLeft ? 1.0f : 0.0f;
            fc.conv[1] = caps.halfPixelOffset ? 0.5f : 0.0f;
            fc.conv[2] = caps.fxStorage == rhi::FxStorage::Texture ? (float)fxPerRow : 0.0f;
            fc.conv[3] = quadStart;
            frameLayer = fc;
            frameLayer.time[3] = 0.0f;   // layers are composed in gamma space and converted on composite
        }

        // ------------------------------------------------------------ passes and bindings
        void BeginPass(rhi::Texture t, rhi::LoadOp load, const FrameConstants& fc, const char* name)
        {
            rhi::PassDesc d;
            d.target = t;
            d.load = load;
            d.debugName = name;
            dev.BeginPass(d);
            const rhi::TextureDesc td = dev.GetTextureDesc(t);
            passTarget = t;
            passFormat = td.format;
            passSamples = td.samples;
            passFrame = &fc;
            inPass = true;
            ForgetBindings();
            ++stats.passes;
        }

        void EndPass()
        {
            if (inPass)
                dev.EndPass();
            inPass = false;
        }

        void ForgetBindings()
        {
            frameSet = geometryBound = fxDataBound = scissorSet = drawSet = false;
            boundPipeline = {};
            for (rhi::Texture& t : boundTex)
                t = {};
        }

        // Passes start lazily, at the first draw that needs them: a capture or a layer ends the current pass, and
        // nothing is loaded and stored again until something is drawn (an empty pass costs a full-target load and
        // store on tile-based GPUs). Content goes to the open glow layer or to the target.
        void EnsureContentPass()
        {
            const bool toLayer = layerDepth > 0 && layer;
            const rhi::Texture want = toLayer ? layer : target;
            if (inPass && passTarget == want)
                return;
            EndPass();
            if (toLayer && layerPending)
            {
                // a new layer: nothing outside the region is ever read, so nothing is loaded; the region is cleared
                BeginPass(layer, rhi::LoadOp::DontCare, frameLayer, "glow-layer");
                if (BindPipeline(GetPipeline(rhi::ShaderProgram::Clear, rhi::BlendMode::Opaque)) && SetScissor(layerRegion))
                {
                    EnsureFrame();
                    dev.Draw(3, 0);
                    ++stats.drawCalls;
                }
                layerPending = false;
            }
            else if (toLayer)
                BeginPass(layer, rhi::LoadOp::Load, frameLayer, "glow-layer");
            else
                BeginPass(target, rhi::LoadOp::Load, frameMain, "ui");
        }

        void EnsureFrame()
        {
            if (!frameSet)
            {
                dev.SetConstants(rhi::ConstantSlot::Frame, passFrame, sizeof(FrameConstants));
                frameSet = true;
            }
        }

        rhi::Pipeline GetPipeline(rhi::ShaderProgram program, rhi::BlendMode blend, EffectId effect = 0, std::uint32_t features = 0,
                                  bool background = false)
        {
            const PipelineKey key{program, blend, passFormat, passSamples, effect, features};
            auto it = pipelines.find(key);
            if (it != pipelines.end() && (it->second || effect == 0))
                return it->second;   // built-in programs are asked once; a refusal is final
            if (effect != 0)
            {
                // an effect compiling in the background (or failed): ask again on a later frame, not on every draw
                auto tried = effectTried.find(effect);
                if (tried != effectTried.end() && tried->second == frame)
                    return {};
                effectTried[effect] = frame;
            }
            rhi::PipelineDesc d;
            d.program = program;
            d.layout = rhi::UsesUiVertices(program) ? rhi::VertexLayout::UiVertex : rhi::VertexLayout::None;
            d.topology = rhi::DrawsInstances(program) ? rhi::Topology::TriangleStrip : rhi::Topology::TriangleList;
            d.blend = blend;
            d.targetFormat = passFormat;
            d.samples = passSamples;
            d.fxFeatures = features;
            d.background = background;
            if (effect != 0)
            {
                auto e = effects.find(effect);
                if (e == effects.end())
                    return {};
                d.effect = effect;
                d.effectSource = e->second.source.c_str();
            }
            const rhi::Pipeline p = dev.CreatePipeline(d);
            pipelines[key] = p;
            return p;
        }

        // An FX batch's pipeline: its feature variant (Caps::fxFeatureVariants) when it is ready. While the variant
        // compiles in the background (Caps::asyncPipelines) a ready variant that covers the batch's features stands
        // in - the smallest one - else the full shader; a refused or failed variant falls back to the full shader.
        // The full shader is only asked for when it is needed: on SM3 it is the slowest compile of all. A batch of
        // glyph shapes on a device whose full shader has no glyph code (Caps::fullFxGlyphs) waits for a variant.
        rhi::Pipeline FxPipeline(std::uint32_t features)
        {
            if (features == 0)
                return GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied);
            // glyph shapes the full shader cannot draw (Caps::fullFxGlyphs): their variant, built at once - nothing
            // could stand in for it while it compiled
            const bool glyphVariant = !caps.fullFxGlyphs && (features & fx::kVariantOnlyFeatures);
            const rhi::Pipeline variant = GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, 0, features, caps.asyncPipelines && !glyphVariant);
            const rhi::PipelineStatus status = variant ? dev.GetPipelineStatus(variant) : rhi::PipelineStatus::Failed;
            if (status == rhi::PipelineStatus::Ready)
                return variant;
            // the full shader may not draw glyph shapes (Caps::fullFxGlyphs): such a batch waits for a variant
            const bool fullDraws = caps.fullFxGlyphs || !(features & fx::kVariantOnlyFeatures);
            if (status == rhi::PipelineStatus::Failed)
            {
                ++stats.fxFallbacks;
                return fullDraws ? GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied) : rhi::Pipeline();
            }
            ++stats.fxPendingVariants;
            rhi::Pipeline best;
            int bestBits = 33;
            for (const auto& [k, p] : pipelines)
            {
                if (k.program != rhi::ShaderProgram::Fx || k.effect != 0 || k.blend != rhi::BlendMode::Premultiplied || k.format != passFormat ||
                    k.samples != passSamples || k.features == 0 || (k.features & features) != features || std::popcount(k.features) >= bestBits || !p ||
                    dev.GetPipelineStatus(p) != rhi::PipelineStatus::Ready)
                    continue;
                best = p;
                bestBits = std::popcount(k.features);
            }
            return best || !fullDraws ? best : GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied);
        }

        bool BindPipeline(rhi::Pipeline p)
        {
            if (!p)
                return false;
            if (!(p == boundPipeline))
            {
                dev.SetPipeline(p);
                boundPipeline = p;
            }
            return true;
        }

        void BindTexture(int slot, rhi::Texture t)
        {
            if (!(boundTex[slot] == t))
            {
                dev.SetTexture(slot, t);
                boundTex[slot] = t;
            }
        }

        rhi::Texture TextureOf(TextureId id) const
        {
            auto it = textures.find(id);
            return it != textures.end() ? it->second.tex : white;
        }

        // t1..t6: the backdrop pyramid (white while there is none: every declared texture must be bound). A target
        // that cannot be copied has no full-resolution level: level 1 stands in for it (blurrier, but valid).
        void BindBackdrop()
        {
            const bool substitute = backdrop && !canCopy && levels[1];
            BindTexture(rhi::kSlotBackdrop0, copy ? copy : (substitute ? levels[1] : white));
            for (int l = 1; l < kBackdropLevels; ++l)
                BindTexture(rhi::kSlotBackdrop0 + l, levels[l] ? levels[l] : white);
        }

        bool SetScissor(const PxRect& r)
        {
            const rhi::IRect s = ToScissor(r, W, H);
            if (s.Empty())
                return false;
            if (!scissorSet || !(s == boundScissor))
            {
                dev.SetScissor(s);
                boundScissor = s;
                scissorSet = true;
            }
            return true;
        }

        void BindDraw(const float fade[4], std::uint32_t firstInstance)
        {
            DrawConstants dc{};
            std::memcpy(dc.fade, fade, sizeof(dc.fade));
            dc.info[0] = (float)firstInstance;
            std::memcpy(dc.info + 1, light, sizeof(light));
            if (!drawSet || std::memcmp(&dc, &boundDraw, sizeof(dc)) != 0)
            {
                dev.SetConstants(rhi::ConstantSlot::Draw, &dc, sizeof(dc));
                boundDraw = dc;
                drawSet = true;
            }
        }

        // Timestamps around runs of one category (non-nesting). They stop after maxProfileScopes runs: two queries
        // per switch between geometry and FX add up to hundreds in a busy frame.
        void ProfileRun(int kind)
        {
            if (!caps.timestampQueries || !profile || kind == run)
                return;
            if (run != RunNone)
                dev.EndProfile();
            run = RunNone;
            if (kind != RunNone && profileScopes < maxProfileScopes)
            {
                dev.BeginProfile((rhi::ProfileCategory)kind);
                ++profileScopes;
                run = kind;
            }
        }

        // ------------------------------------------------------------ post passes
        // Downsamples `source` (full resolution) into pyramid levels 1..levels inside `region` (pixels).
        //
        // keepLevels: a pass only writes its region and every read is clamped to the region refreshed last, so the
        // levels normally start DontCare (tilers need not load them). But glass past the capture budget reuses the
        // last capture, and a user effect may sample the backdrop outside its margin: both read what a level holds
        // outside the region written this frame, which DontCare leaves undefined (garbage, NaN in RGBA16F). In such
        // frames the passes load, and a level whose content is undefined is cleared instead.
        void BuildPyramid(rhi::Texture source, const PxRect& region, int count)
        {
            PyramidStep step = PyramidStep::First(region, W, H);
            rhi::Texture src = source;
            int srcW = W, srcH = H;
            for (int l = 1; l <= std::min(count, kBackdropLevels - 1); ++l)
            {
                const int dw = LevelSize(W, l), dh = LevelSize(H, l);
                const PxRect written = step.Next(region, l, dw, dh);
                if (written.Empty())
                    break;
                const rhi::LoadOp load = !keepLevels ? rhi::LoadOp::DontCare : levelDefined[l] ? rhi::LoadOp::Load : rhi::LoadOp::Clear;
                BeginPass(levels[l], load, frameMain, "pyramid");
                levelDefined[l] = keepLevels;
                if (!BindPipeline(GetPipeline(rhi::ShaderProgram::Downsample, rhi::BlendMode::Opaque)))
                {
                    EndPass();   // no downsample program: the levels keep what they had
                    break;
                }
                EnsureFrame();
                dev.SetScissor(rhi::IRect{(int)written.x0, (int)written.y0, (int)written.x1, (int)written.y1});
                const PassConstants pc = step.Constants(srcW, srcH);
                dev.SetConstants(rhi::ConstantSlot::Pass, &pc, sizeof(pc));
                BindTexture(rhi::kSlotTexture, src);
                dev.Draw(3, 0);
                ++stats.drawCalls;
                EndPass();
                step.valid = written;
                src = levels[l];
                srcW = dw;
                srcH = dh;
            }
        }

        // ------------------------------------------------------------ temporal reuse
        // Something wrote `region` of the copy and the pyramid: the kept captures whose glass reads there lost their
        // levels (`self`, which wrote it, has them now).
        void Wrote(const PxRect& region, const KeptCapture* self)
        {
            for (KeptCapture& k : kept)
                if (&k != self && k.read.Overlaps(region))
                    k.intact = false;
        }

        KeptCapture* FindKept(std::uint64_t shape)
        {
            for (KeptCapture& k : kept)
                if (k.shape == shape)
                    return &k;
            return nullptr;
        }

        // The texels of level l a capture over `region` writes (BuildPyramid's), and the part of the copy it writes
        PxRect LevelRegion(const PxRect& region, int l) const
        {
            return PyramidStep::First(region, W, H).Next(region, l, LevelSize(W, l), LevelSize(H, l));
        }
        PxRect Level0Region(const PxRect& region, const RenderOp& op) const
        {
            if (!canRead)
                return region;
            const PxRect want0 = op.captureLevel0 ? op.captureLevel0Region.Intersect(region) : PxRect{};
            return canCopy && !want0.Empty() ? AlignCopyRegion(want0, W, H) : PxRect{};
        }

        // Before the frame's passes (textures are made then, rhi.hpp): a kept capture for every capture of the plan, a
        // copy aside for those whose content is the one they hold - or for every one, where captures serve frames
        // whatever changed - and the kept captures the frame no longer has released.
        void PrepareKept()
        {
            for (KeptCapture& k : kept)
                k.planned = k.done = false;
            if (!plan.keyCaptures || !backdrop)
            {
                ReleaseKept();
                return;
            }
            for (const RenderOp& op : plan.ops)
            {
                if (op.type != RenderOp::FxBatch || op.captureRegion.Empty())
                    continue;
                KeptCapture* k = FindKept(op.captureShape);
                if (!k)
                {
                    if (kept.size() >= kMaxKept)
                        continue;
                    kept.emplace_back().shape = op.captureShape;
                    k = &kept.back();
                }
                k->planned = true;
                const bool stable = reuseFrames > 0 || (reuseExact && op.captureKey != 0 && k->key == op.captureKey);
                if (!stable || k->cache[1] || k->cache[0])
                    continue;
                const PxRect region = AlignCaptureRegion(op.captureRegion, W, H);
                for (int l = 1; l <= std::min(op.captureLevels, kBackdropLevels - 1); ++l)
                {
                    const PxRect r = LevelRegion(region, l);
                    if (!r.Empty())
                        k->cache[l] = MakeTexture((int)(r.x1 - r.x0), (int)(r.y1 - r.y0), LayerFormat(),
                                                  rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled | rhi::TextureUsage_CopySrc |
                                                      rhi::TextureUsage_CopyDst,
                                                  "kept-capture");
                }
                const PxRect r0 = Level0Region(region, op);
                if (!r0.Empty())
                    k->cache[0] = MakeTexture((int)(r0.x1 - r0.x0), (int)(r0.y1 - r0.y0), rhi::RawFormat(targetDesc.format),
                                              rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled | rhi::TextureUsage_CopySrc | rhi::TextureUsage_CopyDst,
                                              "kept-capture-0");
            }
            for (std::size_t i = 0; i < kept.size();)
            {
                if (kept[i].planned)
                {
                    ++i;
                    continue;
                }
                for (rhi::Texture& t : kept[i].cache)
                    Release(t);
                kept.erase(kept.begin() + (std::ptrdiff_t)i);
            }
        }

        void ReleaseKept()
        {
            for (KeptCapture& k : kept)
                for (rhi::Texture& t : k.cache)
                    Release(t);
            kept.clear();
        }

        // `op`'s capture from an earlier frame: the same content (RenderParams::targetStatic) or, within
        // backdropReuseFrames, the same region - its levels still in the pyramid, or brought back from the copy aside.
        bool ReuseCapture(const RenderOp& op)
        {
            KeptCapture* k = plan.keyCaptures ? FindKept(op.captureShape) : nullptr;
            if (!k || k->done || !(k->intact || k->cached))
                return false;
            const bool same = reuseExact && op.captureKey != 0 && k->key == op.captureKey;
            if (!same && !(reuseFrames > 0 && k->age < reuseFrames))
                return false;
            if (!k->intact)
            {
                ProfileRun(RunCapture);
                EndPass();
                for (int l = 0; l <= k->levels && l < kBackdropLevels; ++l)
                {
                    const PxRect r = l == 0 ? k->level0 : LevelRegion(k->wrote, l);
                    if (k->cache[l] && !r.Empty())
                        dev.CopyTexture(l == 0 ? copy : levels[l], (int)r.x0, (int)r.y0, k->cache[l], rhi::IRect{0, 0, (int)(r.x1 - r.x0), (int)(r.y1 - r.y0)});
                }
                Wrote(k->wrote, k);
                k->intact = true;
                ++stats.restoredCaptures;
            }
            k->age = same ? k->age : k->age + 1;   // its key stays the content the levels hold
            k->done = true;
            return true;
        }

        // `op`'s capture was just taken: kept, and copied aside where PrepareKept made room for it.
        void KeepCapture(const RenderOp& op, const PxRect& region, const PxRect& level0)
        {
            KeptCapture* k = plan.keyCaptures ? FindKept(op.captureShape) : nullptr;
            Wrote(region, k);
            if (!k)
                return;
            k->key = op.captureKey;
            k->wrote = region;
            k->read = op.captureRegion;
            k->level0 = level0;
            k->levels = op.captureLevels;
            k->age = 0;
            k->intact = true;
            k->done = true;
            k->cached = k->cache[1] || k->cache[0];
            for (int l = 0; l <= k->levels && l < kBackdropLevels; ++l)
            {
                const PxRect r = l == 0 ? level0 : LevelRegion(region, l);
                if (k->cache[l] && !r.Empty())
                    dev.CopyTexture(k->cache[l], 0, 0, l == 0 ? copy : levels[l], rhi::IRect{(int)r.x0, (int)r.y0, (int)r.x1, (int)r.y1});
            }
        }

        // `level0`: the part of the capture that glass reads at full resolution (clear glass, user effects), empty
        // when every batch it serves is frosted. Returns the part of the copy it wrote.
        PxRect CaptureBackdrop(const PxRect& wanted, int count, const PxRect& level0)
        {
            const PxRect region = AlignCaptureRegion(wanted, W, H);
            if (region.Empty())
                return {};
            ProfileRun(RunCapture);
            EndPass();
            // A target that can be read builds the pyramid straight from itself - the same values as from a copy,
            // without the copy's bandwidth - and only level 0 is copied, only where glass reads it: a frosted window
            // with one clear control copies the control's region, not the window's. A target that cannot be copied
            // is read directly always; level 1 then stands in for level 0 (BindBackdrop), so it is always built. One
            // that cannot be read (multisampled, not sampleable) is copied whole and the pyramid built from the copy.
            if (canRead)
            {
                const PxRect want0 = level0.Intersect(region);
                const PxRect r0 = canCopy && !want0.Empty() ? AlignCopyRegion(want0, W, H) : PxRect{};
                if (!r0.Empty())
                    dev.CopyTexture(copy, (int)r0.x0, (int)r0.y0, target, rhi::IRect{(int)r0.x0, (int)r0.y0, (int)r0.x1, (int)r0.y1});
                else
                    ++stats.directCaptures;
                BuildPyramid(target, region, canCopy ? count : std::max(count, 1));
                return r0;
            }
            dev.CopyTexture(copy, (int)region.x0, (int)region.y0, target, rhi::IRect{(int)region.x0, (int)region.y0, (int)region.x1, (int)region.y1});
            BuildPyramid(copy, region, count);
            return region;
        }

        void BeginLayer(const RenderOp& op)
        {
            if (layerDepth++ > 0 || !layer)
                return;   // nested layers draw into the outer one; without a layer the content draws unbloomed
            ProfileRun(RunNone);
            // the layer pass starts with its first draw; only the region the layer is sampled in gets cleared (the
            // pyramid clamps to it)
            layerPending = true;
            layerRegion = AlignCaptureRegion(op.bounds, W, H);
            activeLayer = op.layer;
        }

        void EndLayer(const RenderOp& op)
        {
            if (layerDepth == 0)
                return;
            if (--layerDepth > 0 || !layer)
                return;
            if (layerPending)
            {
                layerPending = false;   // nothing was drawn into it: nothing to bloom
                return;
            }
            ProfileRun(RunLayer);
            EndPass();
            const PxRect region = AlignCaptureRegion(op.bounds, W, H);
            BuildPyramid(layer, region, LevelsForBloom(op.blurPx));
            Wrote(region, nullptr);   // the bloom pyramid is the backdrop's
            EnsureContentPass();
            if (BindPipeline(GetPipeline(rhi::ShaderProgram::LayerComposite, rhi::BlendMode::Premultiplied)) && SetScissor(op.bounds))
            {
                EnsureFrame();
                const PassConstants pc = {{activeLayer.color[0], activeLayer.color[1], activeLayer.color[2], activeLayer.color[3]},
                                          {activeLayer.intensity, activeLayer.radius, activeLayer.opacity, 0.0f}};
                dev.SetConstants(rhi::ConstantSlot::Pass, &pc, sizeof(pc));
                BindTexture(rhi::kSlotTexture, layer);
                BindBackdrop();
                dev.Draw(3, 0);
                ++stats.drawCalls;
                ++stats.glowLayers;
            }
        }

        // t7: the frame's instance data (FX instances, then quads)
        void BindInstanceData()
        {
            if (!fxDataBound)
            {
                if (caps.fxStorage == rhi::FxStorage::Buffer)
                    dev.SetFxBuffer(fxBuf);
                else
                    BindTexture(rhi::kSlotFxData, fxTex);
                fxDataBound = true;
            }
        }

        // ------------------------------------------------------------ ops
        void DrawGeometry(const RenderOp& op)
        {
            ProfileRun(RunGeometry);
            EnsureContentPass();
            // the program of the texture's kind, of vertices or of instanced quads
            const bool q = op.quads;
            rhi::Pipeline pipeline;
            if (op.lcd)
            {
                // sub-pixel pages: per-channel alpha on an opaque target; inside glow layers (alpha targets), on targets
                // that are not opaque and without dual-source blending, the grayscale coverage kept in A
                if (caps.dualSourceBlend && subpixelText && !(layerDepth > 0 && layer))
                    pipeline = GetPipeline(q ? rhi::ShaderProgram::QuadLcd : rhi::ShaderProgram::TextLcd, rhi::BlendMode::DualSourceLcd);
                if (!pipeline)
                    pipeline = GetPipeline(q ? rhi::ShaderProgram::QuadLcdGray : rhi::ShaderProgram::TextLcdGray, rhi::BlendMode::Straight);
            }
            else if (op.coverage)
                pipeline = GetPipeline(q ? rhi::ShaderProgram::QuadGray : rhi::ShaderProgram::TextGray, rhi::BlendMode::Straight);
            else
                pipeline = GetPipeline(q ? rhi::ShaderProgram::QuadImage : rhi::ShaderProgram::UiGeometry, rhi::BlendMode::Straight);
            if (!BindPipeline(pipeline) || !SetScissor(op.clip))
                return;
            EnsureFrame();
            BindTexture(rhi::kSlotTexture, TextureOf(op.texture));
            if (q)
            {
                BindInstanceData();
                BindDraw(op.fade, caps.drawFirstInstance ? 0u : op.quadStart);
                if (caps.drawFirstInstance)
                    dev.DrawInstancedFrom(4, op.quadCount, op.quadStart);
                else
                    dev.DrawInstanced(4, op.quadCount);
                ++stats.drawCalls;
                return;
            }
            BindDraw(op.fade, 0);
            if (!geometryBound)
            {
                dev.SetVertexBuffer(vb);
                dev.SetIndexBuffer(ib);
                geometryBound = true;
            }
            if (plan.direct)
            {
                for (std::uint32_t g = op.geometryFirst; g < op.geometryFirst + op.geometryCount; ++g)
                {
                    const FramePlan::GeometryDraw& d = plan.geometry[g];
                    dev.DrawIndexedBase(d.count, d.firstIndex, d.baseVertex);
                    ++stats.drawCalls;
                }
                return;
            }
            dev.DrawIndexed(op.idxCount, op.idxOffset);
            ++stats.drawCalls;
        }

        void DrawFx(const RenderOp& op)
        {
            // captures are planned per frame (FramePlan::PlanCaptures): one serves many glass batches
            if (op.glass && !op.captureRegion.Empty() && backdrop)
            {
                if (ReuseCapture(op))
                    ++stats.reusedCaptures;
                else if (captures < budget)
                {
                    const PxRect level0 = CaptureBackdrop(op.captureRegion, op.captureLevels, op.captureLevel0 ? op.captureLevel0Region : PxRect{});
                    ++captures;
                    KeepCapture(op, AlignCaptureRegion(op.captureRegion, W, H), level0);
                }
                else
                    stats.overBudget = true;
            }
            ProfileRun(op.glass ? RunFxGlass : RunFx);
            EnsureContentPass();
            // a shader specialized to the features this batch uses, where the backend builds variants (glyph shapes on
            // a device whose full shader has no glyph code: there too)
            const bool glyphVariant = !caps.fullFxGlyphs && (op.features & fx::kVariantOnlyFeatures);
            const std::uint32_t features = caps.fxFeatureVariants || glyphVariant ? op.features : 0u;
            rhi::Pipeline p = op.effect != 0 && caps.runtimeEffects ? GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, op.effect, features)
                                                                    : rhi::Pipeline{};
            if (!p)
                p = FxPipeline(features);
            if (!BindPipeline(p) || !SetScissor(op.clip))
                return;
            EnsureFrame();
            BindTexture(rhi::kSlotTexture, TextureOf(op.texture));
            BindBackdrop();
            BindInstanceData();
            // the first instance in the draw where the device takes it there: the Draw constants then stay as they were
            BindDraw(op.fade, caps.drawFirstInstance ? 0u : op.instStart);
            if (!warmUp.empty() && passFormat == targetDesc.format && passSamples == targetDesc.samples && WarmUp(op))
            {
                BindPipeline(p);
                dev.SetScissor(boundScissor);
            }
            if (caps.drawFirstInstance)
                dev.DrawInstancedFrom(4, op.instCount, op.instStart);
            else
                dev.DrawInstanced(4, op.instCount);
            ++stats.drawCalls;
            ++stats.fxBatches;
        }

        // Caps::firstDrawCompiles: a prewarmed effect whose pipeline is ready draws the batch's first instance once
        // where no pixel is written (an empty scissor), with this batch's bindings. The driver compiles it now (NVIDIA's
        // Direct3D 9: 140 ms for an effect on the RTX 4080), not in the frame a shape first shows the effect. Only
        // there: Direct3D 12's debug layer warns about the empty scissor, and its pipelines are complete anyway.
        // True when it drew: the caller binds its pipeline and scissor again.
        bool WarmUp(const RenderOp& op)
        {
            bool drew = false;
            for (auto it = warmUp.begin(); it != warmUp.end();)
            {
                const rhi::Pipeline wp = GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, *it);
                if (!wp)
                {
                    ++it;   // still compiling (asked again next frame)
                    continue;
                }
                if (op.instCount > 0 && BindPipeline(wp))
                {
                    dev.SetScissor(rhi::IRect{0, 0, 0, 0});
                    if (caps.drawFirstInstance)
                        dev.DrawInstancedFrom(4, 1, op.instStart);
                    else
                        dev.DrawInstanced(4, 1);
                    ++stats.drawCalls;
                    drew = true;
                }
                it = warmUp.erase(it);
            }
            return drew;
        }

        void RunCallback(const RenderOp& op)
        {
            // a callback with nothing visible does not run: SetScissor would keep the previous scissor, and the host
            // code would draw unclipped
            if (ToScissor(op.clip, W, H).Empty())
                return;
            ProfileRun(RunNone);
            EnsureContentPass();
            SetScissor(op.clip);
            void* state = dev.NativeRenderState();
            op.cmd->callback(*op.list, *op.cmd, state);
            ForgetBindings();   // the host may have changed anything
        }

        void Execute()
        {
            for (const RenderOp& op : plan.ops)
            {
                switch (op.type)
                {
                case RenderOp::Draw: DrawGeometry(op); break;
                case RenderOp::FxBatch: DrawFx(op); break;
                case RenderOp::LayerBegin: BeginLayer(op); break;
                case RenderOp::LayerEnd: EndLayer(op); break;
                case RenderOp::Callback: RunCallback(op); break;
                }
            }
            // a layer the draw lists left open still composites
            while (layerDepth > 0)
            {
                RenderOp end;
                end.type = RenderOp::LayerEnd;
                end.bounds = {0, 0, (float)W, (float)H};
                end.blurPx = activeLayer.radius * frameMain.display[2];
                layerDepth = 1;
                EndLayer(end);
            }
            ProfileRun(RunNone);
        }

        // Effects set since the last frame start compiling now - their pipeline for this target, on the device's
        // worker - not when a shape first uses one: a compile takes hundreds of milliseconds, and the shape would
        // draw without its effect until then. Where the backend builds feature variants, which one a shape needs is
        // not known before it draws.
        void Prewarm()
        {
            if (prewarm.empty())
                return;
            if (caps.runtimeEffects && !caps.fxFeatureVariants)
            {
                const rhi::Format format = passFormat;
                const int samples = passSamples;
                passFormat = targetDesc.format;
                passSamples = targetDesc.samples;
                for (const EffectId id : prewarm)
                {
                    GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, id);
                    if (caps.firstDrawCompiles && std::find(warmUp.begin(), warmUp.end(), id) == warmUp.end())
                        warmUp.push_back(id);
                }
                passFormat = format;
                passSamples = samples;
            }
            prewarm.clear();
        }

        bool Render(const DrawData& dd, TextureRegistry* registry, rhi::Texture t, const RenderParams& params)
        {
            const rhi::GpuProfile gpu = stats.gpu;
            stats = RenderStats();
            stats.gpu = gpu;
            ++frame;
            targetDesc = dev.GetTextureDesc(t);
            W = targetDesc.width;
            H = targetDesc.height;
            if (W <= 0 || H <= 0 || !(targetDesc.usage & rhi::TextureUsage_RenderTarget))
                return false;
            // a Metal framebufferOnly drawable or a swap-chain image without TRANSFER_SRC cannot be copied: glass then
            // reads it directly if it can be sampled, else it has no backdrop
            canCopy = (targetDesc.usage & rhi::TextureUsage_CopySrc) != 0;
            canRead = caps.sampleRenderTarget && targetDesc.samples == 1 && (targetDesc.usage & rhi::TextureUsage_Sampled) != 0;
            if (!dev.BeginFrame(params.frame))
                return false;
            target = t;
            subpixelText = params.subpixelText;

            ApplyTextureChanges(registry);
            reuseExact = params.targetStatic;
            reuseFrames = std::max(0, params.backdropReuseFrames);
            // the light: its point or direction, and 0 none / 1 a direction / 1 + its height for a point
            light[0] = params.light.position.x;
            light[1] = params.light.position.y;
            light[2] = params.light.enabled ? 1.0f + std::max(params.light.height, 0.0f) : 0.0f;
            plan.lightShadows = params.light.enabled;
            plan.keyCaptures = reuseExact || reuseFrames > 0;
            plan.Build(dd, [this](TextureId id, TextureInfo& out) {
                auto it = textures.find(id);
                if (it == textures.end())
                    return false;
                out = it->second.info;
                return true;
            }, caps.baseVertex);
            stats.fxInstances = plan.fxCount;
            stats.vertices = (int)plan.totalVertices;
            stats.indices = (int)plan.totalIndices;
            stats.quads = (int)plan.quadCount;

            const bool glass = plan.anyGlass && (canCopy || canRead);
            const bool surfaces = (glass || plan.anyLayer) && EnsureSurfaces(glass && canCopy, plan.anyLayer);
            backdrop = glass && surfaces;
            if (!white)
            {
                const std::uint32_t px = 0xFFFFFFFFu;
                rhi::TextureDesc d;
                d.width = d.height = 1;
                d.format = rhi::Format::RGBA8_UNORM;
                d.usage = rhi::TextureUsage_Sampled;
                d.debugName = "white";
                white = dev.CreateTexture(d, &px, 0);
            }
            const bool uploaded = Upload(params.maxFxInstancesPerRow);
            FillFrameConstants(dd, params);
            Prewarm();

            captures = 0;
            budget = std::max(1, params.maxBackdropCaptures);
            bool userEffects = false;
            for (const RenderOp& op : plan.ops)
                userEffects |= op.type == RenderOp::FxBatch && op.effect != 0;
            // the levels keep what they hold outside the regions a frame writes when glass may read it: past the capture
            // budget, user effects, and captures kept for the next frame
            keepLevels = backdrop && (plan.plannedCaptures > budget || (userEffects && caps.runtimeEffects) || plan.keyCaptures);
            PrepareKept();
            profile = params.profile;
            profileScopes = 0;
            maxProfileScopes = std::max(0, params.maxProfileScopes);
            layerDepth = 0;
            layerPending = false;
            inPass = false;
            run = RunNone;
            if (uploaded && white)
            {
                Execute();
                EndPass();
            }
            stats.backdropCaptures = captures;
            dev.EndFrame();
            rhi::GpuProfile latest;
            if (dev.ReadProfile(latest))
                stats.gpu = latest;
            return true;
        }
    };

    Renderer::Renderer(rhi::Device& device) : impl_(std::make_unique<Impl>(device)) {}

    Renderer::~Renderer() { impl_->ReleaseAll(); }

    bool Renderer::Render(const DrawData& dd, TextureRegistry* textures, rhi::Texture target, const RenderParams& params)
    {
        return impl_->Render(dd, textures, target, params);
    }

    void Renderer::SetEffectSource(EffectId id, const std::string& name, const std::string& source)
    {
        impl_->effects[id] = {name, source};
        impl_->effectTried.erase(id);
        impl_->prewarm.push_back(id);
        // a changed source invalidates the pipelines built from the old one
        for (auto it = impl_->pipelines.begin(); it != impl_->pipelines.end();)
        {
            if (it->first.effect == id)
            {
                if (it->second)
                    impl_->dev.DestroyPipeline(it->second);
                it = impl_->pipelines.erase(it);
            }
            else
                ++it;
        }
    }

    void Renderer::ReleaseSurfaces() { impl_->ReleaseSurfaces(); }

    const RenderStats& Renderer::Stats() const { return impl_->stats; }
    const FramePlan& Renderer::Plan() const { return impl_->plan; }

    rhi::Texture Renderer::GpuTexture(TextureId id) const
    {
        auto it = impl_->textures.find(id);
        return it != impl_->textures.end() ? it->second.tex : rhi::Texture{};
    }
}
