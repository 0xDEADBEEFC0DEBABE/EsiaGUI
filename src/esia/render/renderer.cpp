// Esia - renderer (see esia/render/renderer.hpp): executes a FramePlan through an rhi::Device.
// The capture, pyramid and glow-layer logic is the one of WGT's src/backends/d3d11_backend.cpp, on the RHI.
#include "esia/render/renderer.hpp"
#include "gpu_constants.hpp"
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
                return std::hash<std::uint64_t>()(a ^ ((std::uint64_t)k.features * 0x9E3779B97F4A7C15ull));
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

        std::unordered_map<TextureId, DeviceTexture> textures;
        std::vector<TextureChange> changes;
        std::unordered_map<PipelineKey, rhi::Pipeline, PipelineKeyHash> pipelines;
        struct Effect
        {
            std::string name, source;
        };
        std::unordered_map<EffectId, Effect> effects;
        std::unordered_map<EffectId, std::uint64_t> effectTried;   // frame of the last pipeline attempt
        std::uint64_t frame = 0;

        rhi::Texture white;
        rhi::Buffer vb, ib, fxBuf;
        std::size_t vbCap = 0, ibCap = 0, fxCap = 0;
        rhi::Texture fxTex;
        int fxTexW = 0, fxTexH = 0, fxPerRow = 1;
        std::vector<float> staging;

        // size-dependent surfaces
        rhi::Texture copy, levels[kBackdropLevels], layer;
        int surfW = 0, surfH = 0;
        rhi::Format surfFormat = rhi::Format::Unknown;

        // this frame
        rhi::Texture target;
        rhi::TextureDesc targetDesc;
        int W = 0, H = 0;
        FrameConstants frameMain{}, frameLayer{};
        int layerDepth = 0;
        bool layerPending = false;      // the open layer has not been cleared yet (its pass did not start)
        PxRect layerRegion;             // ... and the region to clear then
        fx::LayerParams activeLayer{};
        int captures = 0, budget = 64;
        int run = RunNone;

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
            Release(copy);
            for (rhi::Texture& l : levels)
                Release(l);
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

        rhi::Texture MakeTexture(int w, int h, rhi::Format f, std::uint32_t usage, const char* name)
        {
            rhi::TextureDesc d;
            d.width = w;
            d.height = h;
            d.format = f;
            d.usage = usage;
            d.debugName = name;
            return dev.CreateTexture(d, nullptr, 0);
        }

        // The backdrop copy (glass) and the pyramid + layer (glass and glow layers), created before the first pass
        // of the frame that needs them, at the target's size.
        bool EnsureSurfaces(bool glass, bool glowLayer)
        {
            if (surfW != W || surfH != H || surfFormat != targetDesc.format)
                ReleaseSurfaces();
            surfW = W;
            surfH = H;
            surfFormat = targetDesc.format;
            static const char* kLevelNames[kBackdropLevels] = {"", "pyramid-1", "pyramid-2", "pyramid-3", "pyramid-4", "pyramid-5"};
            bool ok = true;
            if (glass && !copy)
                ok = (bool)(copy = MakeTexture(W, H, rhi::RawFormat(targetDesc.format), rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst, "backdrop-copy"));
            for (int l = 1; ok && l < kBackdropLevels; ++l)
                if (!levels[l])
                    ok = (bool)(levels[l] = MakeTexture(LevelSize(W, l), LevelSize(H, l), LayerFormat(), rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled,
                                                        kLevelNames[l]));
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

        // Vertex, index and FX instance data, once per frame before the first pass. Instances go to the GPU with
        // their integer row (flags) as float values: the shaders read every row as float4 (FxLoad).
        bool Upload()
        {
            if (!plan.vertices.empty())
            {
                const std::size_t vbytes = plan.vertices.size() * sizeof(Vertex), ibytes = plan.indices.size() * sizeof(std::uint32_t);
                if (!EnsureBuffer(vb, vbCap, vbytes, rhi::BufferKind::Vertex, "ui-vertices") ||
                    !EnsureBuffer(ib, ibCap, std::max<std::size_t>(ibytes, 4), rhi::BufferKind::Index, "ui-indices"))
                    return false;
                dev.UpdateBuffer(vb, plan.vertices.data(), vbytes);
                if (ibytes)
                    dev.UpdateBuffer(ib, plan.indices.data(), ibytes);
            }
            const std::size_t n = plan.instances.size();
            if (n == 0)
                return true;
            const bool texture = caps.fxStorage == rhi::FxStorage::Texture;
            fxPerRow = texture ? std::max(1, caps.maxFxDataWidth / (int)fx::kInstanceVec4Count) : 1;
            const std::size_t rows = texture ? (n + (std::size_t)fxPerRow - 1) / (std::size_t)fxPerRow : 1;
            const std::size_t slots = texture ? rows * (std::size_t)fxPerRow : n;
            staging.assign(slots * fx::kInstanceVec4Count * 4, 0.0f);
            for (std::size_t i = 0; i < n; ++i)
            {
                float* dst = staging.data() + i * fx::kInstanceVec4Count * 4;
                std::memcpy(dst, &plan.instances[i], sizeof(fx::Instance));
                for (int k = 0; k < 4; ++k)
                    dst[(fx::kInstanceVec4Count - 1) * 4 + (std::size_t)k] = (float)plan.instances[i].flags[k];
            }
            if (!texture)
            {
                const std::size_t bytes = n * sizeof(fx::Instance);
                if (!EnsureBuffer(fxBuf, fxCap, bytes, rhi::BufferKind::FxInstances, "fx-instances"))
                    return false;
                dev.UpdateBuffer(fxBuf, staging.data(), bytes);
                return true;
            }
            const int w = fxPerRow * (int)fx::kInstanceVec4Count, h = (int)rows;
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
            dev.UpdateTexture(fxTex, rhi::IRect{0, 0, w, h}, staging.data(), 0);
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
            fc.time[2] = copy ? 1.0f : 0.0f;
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
            fc.conv[3] = 0.0f;
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

        rhi::Pipeline GetPipeline(rhi::ShaderProgram program, rhi::BlendMode blend, EffectId effect = 0, std::uint32_t features = 0)
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
            const bool ui = program == rhi::ShaderProgram::UiGeometry || program == rhi::ShaderProgram::TextGray ||
                            program == rhi::ShaderProgram::TextLcd || program == rhi::ShaderProgram::TextLcdGray;
            d.layout = ui ? rhi::VertexLayout::UiVertex : rhi::VertexLayout::None;
            d.topology = program == rhi::ShaderProgram::Fx ? rhi::Topology::TriangleStrip : rhi::Topology::TriangleList;
            d.blend = blend;
            d.targetFormat = passFormat;
            d.samples = passSamples;
            d.fxFeatures = features;
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

        // t1..t6: the backdrop pyramid (white while there is none: every declared texture must be bound)
        void BindBackdrop()
        {
            BindTexture(rhi::kSlotBackdrop0, copy ? copy : white);
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
            if (!drawSet || std::memcmp(&dc, &boundDraw, sizeof(dc)) != 0)
            {
                dev.SetConstants(rhi::ConstantSlot::Draw, &dc, sizeof(dc));
                boundDraw = dc;
                drawSet = true;
            }
        }

        void ProfileRun(int kind)
        {
            if (!caps.timestampQueries || kind == run)
                return;
            if (run != RunNone)
                dev.EndProfile();
            if (kind != RunNone)
                dev.BeginProfile((rhi::ProfileCategory)kind);
            run = kind;
        }

        // ------------------------------------------------------------ post passes
        // Downsamples `source` (full resolution) into pyramid levels 1..levels inside `region` (pixels).
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
                // the pass only writes `written`, and every read is clamped to the region refreshed last: nothing
                // outside it is ever read, so tilers need not load the level
                BeginPass(levels[l], rhi::LoadOp::DontCare, frameMain, "pyramid");
                BindPipeline(GetPipeline(rhi::ShaderProgram::Downsample, rhi::BlendMode::Opaque));
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

        void CaptureBackdrop(const PxRect& wanted, int count, bool needsLevel0)
        {
            const PxRect region = AlignCaptureRegion(wanted, W, H);
            if (region.Empty() || !copy)
                return;
            ProfileRun(RunCapture);
            EndPass();
            // Glass that only reads blurred levels (frost) needs no full-resolution copy: the first downsample reads
            // the render target directly - the same values, a copy's bandwidth saved
            const bool direct = !needsLevel0 && caps.sampleRenderTarget && targetDesc.samples == 1 && (targetDesc.usage & rhi::TextureUsage_Sampled);
            if (direct)
            {
                BuildPyramid(target, region, count);
                ++stats.directCaptures;
            }
            else
            {
                dev.CopyTexture(copy, (int)region.x0, (int)region.y0, target, rhi::IRect{(int)region.x0, (int)region.y0, (int)region.x1, (int)region.y1});
                BuildPyramid(copy, region, count);
            }
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

        // ------------------------------------------------------------ ops
        void DrawGeometry(const RenderOp& op)
        {
            ProfileRun(RunGeometry);
            rhi::ShaderProgram program = rhi::ShaderProgram::UiGeometry;
            rhi::BlendMode blend = rhi::BlendMode::Straight;
            if (op.lcd)
            {
                // sub-pixel pages: per-channel alpha on the target; inside glow layers (alpha targets) and without
                // dual-source blending the grayscale coverage kept in A
                const bool dual = caps.dualSourceBlend && layerDepth == 0;
                program = dual ? rhi::ShaderProgram::TextLcd : rhi::ShaderProgram::TextLcdGray;
                blend = dual ? rhi::BlendMode::DualSourceLcd : rhi::BlendMode::Straight;
            }
            else if (op.coverage)
                program = rhi::ShaderProgram::TextGray;
            EnsureContentPass();
            if (!BindPipeline(GetPipeline(program, blend)) || !SetScissor(op.clip))
                return;
            EnsureFrame();
            BindTexture(rhi::kSlotTexture, TextureOf(op.texture));
            BindDraw(op.fade, 0);
            if (!geometryBound)
            {
                dev.SetVertexBuffer(vb);
                dev.SetIndexBuffer(ib);
                geometryBound = true;
            }
            dev.DrawIndexed(op.idxCount, op.idxOffset);
            ++stats.drawCalls;
        }

        void DrawFx(const RenderOp& op)
        {
            // captures are planned per frame (FramePlan::PlanCaptures): one serves many glass batches
            if (op.glass && !op.captureRegion.Empty() && copy)
            {
                if (captures < budget)
                {
                    CaptureBackdrop(op.captureRegion, op.captureLevels, op.captureLevel0);
                    ++captures;
                }
                else
                    stats.overBudget = true;
            }
            ProfileRun(op.glass ? RunFxGlass : RunFx);
            EnsureContentPass();
            // a shader specialized to the features this batch uses, where the backend builds variants
            const std::uint32_t features = caps.fxFeatureVariants ? op.features : 0u;
            rhi::Pipeline p = op.effect != 0 && caps.runtimeEffects ? GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, op.effect, features)
                                                                    : rhi::Pipeline{};
            if (!p)
                p = GetPipeline(rhi::ShaderProgram::Fx, rhi::BlendMode::Premultiplied, 0, features);
            if (!BindPipeline(p) || !SetScissor(op.clip))
                return;
            EnsureFrame();
            BindTexture(rhi::kSlotTexture, TextureOf(op.texture));
            BindBackdrop();
            if (!fxDataBound)
            {
                if (caps.fxStorage == rhi::FxStorage::Buffer)
                    dev.SetFxBuffer(fxBuf);
                else
                    BindTexture(rhi::kSlotFxData, fxTex);
                fxDataBound = true;
            }
            BindDraw(op.fade, op.instStart);
            dev.DrawInstanced(4, op.instCount);
            ++stats.drawCalls;
            ++stats.fxBatches;
        }

        void RunCallback(const RenderOp& op)
        {
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
            if (!dev.BeginFrame(params.frame))
                return false;
            target = t;

            ApplyTextureChanges(registry);
            plan.Build(dd, [this](TextureId id, TextureInfo& out) {
                auto it = textures.find(id);
                if (it == textures.end())
                    return false;
                out = it->second.info;
                return true;
            });
            stats.fxInstances = plan.fxCount;
            stats.vertices = (int)plan.vertices.size();
            stats.indices = (int)plan.indices.size();

            if (plan.anyGlass || plan.anyLayer)
                EnsureSurfaces(plan.anyGlass, plan.anyLayer);
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
            const bool uploaded = Upload();
            FillFrameConstants(dd, params);

            captures = 0;
            budget = std::max(1, params.maxBackdropCaptures);
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
