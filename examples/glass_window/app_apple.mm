// glass_window - the frame's parts shared by macOS and iOS (app_apple.hpp): options, the frame's text system, the
// frame loop on the Metal backend, the --vsync off mailbox.
#include "app_apple.hpp"
#include "symbol_text.hpp"
#include "image.hpp"
#include "esia/rhi/metal.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#endif
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace glass::apple
{
    int Usage(const App& app, const char* problem)
    {
        std::fprintf(stderr,
                     "%s: %s\n"
                     "usage: %s [--api metal] [--size WxH] [--scale s] [--vsync on|off] [--debug] [--fixed-dt seconds]\n"
                     "       [--frames N] [--screenshot out.png] [--font file]... [--stats] [--fullscreen]\n",
                     app.name, problem, app.name);
        return 2;
    }

    bool Parse(App& app, int argc, char** argv, Options& o, std::string& problem)
    {
        for (int i = 1; i < argc; ++i)
        {
            const std::string a = argv[i];
            const char* value = i + 1 < argc ? argv[i + 1] : nullptr;
            const auto needs = [&]() {
                if (!value)
                    problem = a + " needs a value";
                else
                    ++i;
                return value != nullptr;
            };
            bool usedValue = false;
            if (a == "--debug")
                o.debug = true;
            else if (a == "--stats")
                o.stats = true;
            else if (a == "--fullscreen")
                o.fullscreen = true;
            else if (a == "--fullscreen-at" && needs())
                o.fullscreenAt = std::atoi(value);
            else if (a == "--hide-at" && needs())
                o.hideAt = std::atoi(value);
            else if (a == "--no-shader-cache")
                ;
            else if (a == "--api" && needs())
                o.api = value;
            else if (a == "--vsync" && needs())
                o.vsync = std::strcmp(value, "off") != 0;
            else if (a == "--size" && needs())
            {
                if (std::sscanf(value, "%fx%f", &o.width, &o.height) != 2 || !(o.width > 0.0f) || !(o.height > 0.0f))
                    problem = "--size wants WxH";
            }
            else if (a == "--scale" && needs())
            {
                o.scale = (float)std::atof(value);
                if (!(o.scale > 0.0f))
                    problem = "--scale wants a number above 0";
            }
            else if (a == "--fixed-dt" && needs())
                o.fixedDt = std::atof(value);
            else if (a == "--frames" && needs())
                o.frames = std::atoi(value);
            else if (a == "--screenshot" && needs())
                o.screenshot = value;
            else if (a == "--font" && needs())
                o.fonts.push_back(value);
            else if (app.option && app.option(a, value, usedValue))
                i += usedValue ? 1 : 0;
            else if (problem.empty())
                problem = "unknown option " + a;
            if (!problem.empty())
                return false;
        }
        if (o.api != "metal")
            problem = "--api " + o.api + " is not built in (built: metal)";
        else if (!o.screenshot.empty() && o.frames <= 0)
            o.frames = 60;
        return problem.empty();
    }

    namespace
    {
#if defined(GLASS_TEXT)
        // The main font and its fallbacks: --font, or the platform's chain (SF, Helvetica Neue, Hiragino Sans GB ...).
        esia::text::FontId LoadFonts(esia::text::TextSystem& text, const std::vector<std::string>& files)
        {
            esia::text::FontId main = 0;
            if (files.empty())
            {
                const std::vector<esia::text::FontId> chain = esia::text::AddFallbackFonts(text, esia::text::FindDefaultFallbackFonts());
                return chain.empty() ? 0 : chain.front();
            }
            for (const std::string& path : files)
                if (const esia::text::FontId id = text.AddFontFile(path.c_str()))
                {
                    if (!main)
                        main = id;
                    else
                        text.AddFallback(id);
                }
            return main;
        }
#endif

        // The frame's text system draws esia::ui's icons with SF Symbols (symbol_text.hpp).
        bool RasterizeSfSymbol(char32_t c, int px, std::vector<std::uint8_t>& coverage, int& w, int& h)
        {
            const char* name = SymbolOf(c);
            return name && RasterizeSymbol(name, px, coverage, w, h);
        }

        // --stats: drawables the display showed (presentedTime > 0) and dropped (0), from presented handlers
        std::atomic<int> g_presented{0}, g_dropped{0};
    }

    // ------------------------------------------------------------------ SF Symbols for the icon font
    // esia/ui/icons.hpp (Segoe Fluent code points) -> SF Symbol names
    const char* SymbolOf(char32_t c)
    {
        switch (c)
        {
        case 0xE700: return "line.3.horizontal";
        case 0xE701: return "wifi";
        case 0xE702: return "antenna.radiowaves.left.and.right";   // no Bluetooth logo in SF Symbols
        case 0xE703: return "point.3.connected.trianglepath.dotted";
        case 0xE705: return "lock.shield";
        case 0xE706: return "sun.max";
        case 0xE707: return "mappin";
        case 0xE708: return "moon";
        case 0xE709: return "airplane";
        case 0xE70D: return "chevron.down";
        case 0xE70E: return "chevron.up";
        case 0xE70F: return "pencil";
        case 0xE710: return "plus";
        case 0xE711: return "xmark";
        case 0xE712: return "ellipsis";
        case 0xE713: return "gearshape";
        case 0xE714: return "video";
        case 0xE715: return "envelope";
        case 0xE716: return "person.2";
        case 0xE717: return "phone";
        case 0xE718: return "pin";
        case 0xE719: return "bag";
        case 0xE71A: return "stop";
        case 0xE71B: return "link";
        case 0xE71C: return "line.3.horizontal.decrease";
        case 0xE71D: return "square.grid.2x2";
        case 0xE71E: return "plus.magnifyingglass";
        case 0xE71F: return "minus.magnifyingglass";
        case 0xE720: return "mic";
        case 0xE721: return "magnifyingglass";
        case 0xE722: return "camera";
        case 0xE723: return "paperclip";
        case 0xE724: return "paperplane";
        case 0xE72A: return "arrow.right";
        case 0xE72B: return "arrow.left";
        case 0xE72C: return "arrow.clockwise";
        case 0xE72D: return "square.and.arrow.up";
        case 0xE72E: return "lock";
        case 0xE734: return "star";
        case 0xE735: return "star.fill";
        case 0xE738: return "minus";
        case 0xE73E: return "checkmark";
        case 0xE740: return "arrow.up.left.and.arrow.down.right";
        case 0xE74D: return "trash";
        case 0xE74E: return "square.and.arrow.down";
        case 0xE74F: return "speaker.slash";
        case 0xE753: return "cloud";
        case 0xE76B: return "chevron.left";
        case 0xE76C: return "chevron.right";
        case 0xE767: return "speaker.wave.2";
        case 0xE768: return "play.fill";
        case 0xE769: return "pause.fill";
        case 0xE892: return "backward.fill";
        case 0xE893: return "forward.fill";
        case 0xE771: return "paintbrush";
        case 0xE774: return "globe";
        case 0xE77B: return "person.crop.circle";
        case 0xE783: return "xmark.octagon";
        case 0xE785: return "lock.open";
        case 0xE787: return "calendar";
        case 0xE790: return "paintpalette";
        case 0xE7BA: return "exclamationmark.triangle";
        case 0xE7C1: return "flag";
        case 0xE7E8: return "power";
        case 0xE7FC: return "gamecontroller";
        case 0xE80F: return "house";
        case 0xE81C: return "clock.arrow.circlepath";
        case 0xE81D: return "location";
        case 0xE823: return "clock";
        case 0xE890: return "eye";
        case 0xE895: return "arrow.triangle.2.circlepath";
        case 0xE896: return "arrow.down.circle";
        case 0xE897: return "questionmark.circle";
        case 0xE898: return "arrow.up.circle";
        case 0xE8A5: return "doc";
        case 0xE8B7: return "folder";
        case 0xE8C8: return "doc.on.doc";
        case 0xE8D6: return "music.note";
        case 0xE909: return "globe.asia.australia";
        case 0xE91B: return "photo";
        case 0xE943: return "chevron.left.forwardslash.chevron.right";
        case 0xE945: return "bolt";
        case 0xE946: return "info.circle";
        case 0xE962: return "computermouse";
        case 0xE9D9: return "waveform.path.ecg";
        case 0xE9E9: return "slider.horizontal.3";
        case 0xEA18: return "shield";
        case 0xEA80: return "lightbulb";
        case 0xEA8F: return "bell";
        case 0xEB51: return "heart";
        case 0xEB52: return "heart.fill";
        case 0xEBE8: return "ladybug";
        case 0xED1A: return "eye.slash";
        default: return nullptr;
        }
    }

    void CountPresented(id<MTLDrawable> drawable)
    {
#if TARGET_OS_SIMULATOR
        (void)drawable;   // the simulator's drawables have no presented handlers: --stats shows 0
#else
        [drawable addPresentedHandler:^(id<MTLDrawable> shown) {
            (shown.presentedTime > 0.0 ? g_presented : g_dropped).fetch_add(1);
        }];
#endif
    }

    // ------------------------------------------------------------------ Runner
    Runner::~Runner() = default;

    bool Runner::Init(id<MTLDevice> mtlDevice, id<MTLCommandQueue> queue, int width, int height, float scale, const esia::ContextDesc& cd)
    {
        queue_ = queue;
        device_ = esia::rhi::metal::CreateDevice(mtlDevice, queue);
        if (!device_)
        {
            std::fprintf(stderr, "%s: metal: the device could not be created\n", app_.name);
            return false;
        }
        adapter_ = mtlDevice.name.UTF8String;
        std::printf("%s: Metal on %s, %d x %d px, scale %.2f\n", app_.name, adapter_.c_str(), width, height, scale);
        std::fflush(stdout);

        ctx_ = std::make_unique<esia::Context>(cd);
#if defined(GLASS_TEXT)
        if (std::unique_ptr<esia::text::TextSystem> ft = esia::text::CreateFreeTypeTextSystem(ctx_->Textures()))
            textSystem_ = std::make_unique<SymbolTextSystem>(std::move(ft), ctx_->Textures(), &RasterizeSfSymbol);
        if (textSystem_ && !app_.loadFonts)
            text_ = textSystem_.get();
        else if (textSystem_ && (font_ = LoadFonts(*textSystem_, opt_.fonts)) != 0)
            text_ = textSystem_.get();
        else
            std::fprintf(stderr, "%s: no font loaded (--font): labels are left out\n", app_.name);
#endif
        renderer_ = std::make_unique<esia::render::Renderer>(*device_);
        if (app_.init)
            app_.init(*ctx_, text_, font_, opt_.fonts);
        if (app_.renderer)
            app_.renderer(*renderer_);
        start_ = Clock::now();
        fpsStart_ = start_;
        return true;
    }

    bool Runner::Frame(id<MTLTexture> texture, void (^finish)(id<MTLCommandBuffer> cb), int width, int height, float scale, float waitMs, esia::Rect safeArea)
    {
        drawableWaitMs_ = waitMs;
        if (!ctx_)
            return false;
        id<MTLCommandBuffer> cb = [queue_ commandBuffer];
        const double now = std::chrono::duration<double>(Clock::now() - start_).count();
        esia::FrameParams fp;
        fp.displaySize = esia::Vec2((float)width / scale, (float)height / scale);
        fp.framebufferScale = esia::Vec2(scale, scale);
        fp.time = opt_.fixedDt > 0.0 ? (frame_ + 1) * opt_.fixedDt : now;
        ctx_->NewFrame(fp);
        if (text_ && app_.loadFonts)
            text_->NewFrame({scale});
        const Clock::time_point uiStart = Clock::now();
        if (app_.frame)
        {
            SceneInfo info{"Metal", adapter_, width, height, scale, fps_, cpuMs_, &renderer_->Stats()};
            info.safeArea = safeArea;
            app_.frame(*ctx_, info);
        }
        ctx_->EndFrame();
        cpuMs_ = std::chrono::duration<float, std::milli>(Clock::now() - uiStart).count();

        const Clock::time_point encodeStart = Clock::now();
        const esia::rhi::Texture target = esia::rhi::metal::WrapTexture(*device_, texture);
        esia::render::RenderParams rp;
        rp.frame.nativeContext = (__bridge void*)cb;
        if (!target || !renderer_->Render(ctx_->GetDrawData(), &ctx_->Textures(), target, rp))
            std::fprintf(stderr, "%s: the device refused frame %d\n", app_.name, frame_);
        encodeMs_ = std::chrono::duration<float, std::milli>(Clock::now() - encodeStart).count();
        if (opt_.stats)
        {
            // the whole command buffer on the GPU, gaps between encoders included
            std::shared_ptr<GpuSpan> span = gpuSpan_;
            [cb addCompletedHandler:^(id<MTLCommandBuffer> done) {
                span->us.fetch_add((std::uint64_t)((done.GPUEndTime - done.GPUStartTime) * 1e6));
                span->count.fetch_add(1);
            }];
        }
        finish(cb);
        [cb commit];

        ++frame_;
        const bool last = opt_.frames > 0 && frame_ >= opt_.frames;
        if (last && !opt_.screenshot.empty() && target)
        {
            esia::testkit::Image image(width, height);
            std::string error;
            if (!device_->ReadPixels(target, {0, 0, width, height}, image.rgba))
            {
                std::fprintf(stderr, "%s: reading the drawable back failed\n", app_.name);
                result_ = 1;
            }
            else
            {
                for (std::size_t i = 3; i < image.rgba.size(); i += 4)
                    image.rgba[i] = 255;   // a window has no alpha
                if (!esia::testkit::WritePng(opt_.screenshot, image, &error))
                {
                    std::fprintf(stderr, "%s: %s\n", app_.name, error.c_str());
                    result_ = 1;
                }
                else
                    std::printf("%s: frame %d written to %s\n", app_.name, frame_, opt_.screenshot.c_str());
            }
        }
        if (target)
            device_->DestroyTexture(target);

        ++fpsFrames_;
        const double since = std::chrono::duration<double>(Clock::now() - fpsStart_).count();
        if (opt_.stats)
            Accumulate(width, height);
        if (since >= 0.5)
        {
            fps_ = (float)(fpsFrames_ / since);
            fpsFrames_ = 0;
            fpsStart_ = Clock::now();
        }
        return !last;
    }

    int Runner::Shutdown()
    {
        if (!ctx_)
            return result_;
        if (app_.shutdown)
            app_.shutdown();
        id<MTLCommandBuffer> drain = [queue_ commandBuffer];
        [drain commit];
        [drain waitUntilCompleted];
        const std::uint32_t messages = device_->ValidationErrors();
        renderer_.reset();
        textSystem_.reset();
        text_ = nullptr;
        ctx_.reset();
        device_.reset();
        std::printf("%s: %d frames, %u validation messages%s\n", app_.name, frame_, messages, opt_.debug ? "" : " (--debug to enable Metal's API validation)");
        std::fflush(stdout);
        return (opt_.debug && messages > 0 && result_ == 0) ? 3 : result_;
    }

    // --stats: once a second, the averages of the renderer's numbers (GPU times per category where the device has
    // timestamps: on Apple GPUs whole encoders, the split inside a render pass approximate)
    void Runner::Accumulate(int width, int height)
    {
        const esia::render::RenderStats& st = renderer_->Stats();
        ++sum_.frames;
        sum_.cpuMs += cpuMs_;
        sum_.encodeMs += encodeMs_;
        sum_.waitMs += drawableWaitMs_;
        sum_.draws += st.drawCalls;
        sum_.passes += st.passes;
        sum_.captures += st.backdropCaptures;
        sum_.layers += st.glowLayers;
        if (st.gpu.valid)
        {
            ++sum_.gpuFrames;
            sum_.gpuMs += st.gpu.totalMs;
            for (int c = 0; c < (int)esia::rhi::ProfileCategory::Count; ++c)
                sum_.categoryMs[c] += st.gpu.categoryMs[c];
        }
        const double since = std::chrono::duration<double>(Clock::now() - sum_.start).count();
        if (since < 1.0)
            return;
        const double n = sum_.frames, g = std::max(1, sum_.gpuFrames);
        const std::uint64_t spans = gpuSpan_->count.exchange(0), spanUs = gpuSpan_->us.exchange(0);
        const int shown = g_presented.exchange(0), dropped = g_dropped.exchange(0);
        std::printf("shown %d/s, dropped %d/s | ", (int)(shown / since), (int)(dropped / since));
        std::printf("stats %dx%d: %.0f fps | ui %.2f ms, encode %.2f ms, wait %.2f ms | command buffer on the GPU %.2f ms | "
                    "timestamps %.2f ms (capture %.2f, layer %.2f, fx %.2f, fx glass %.2f, geometry %.2f) | "
                    "%.0f draws, %.0f passes, %.1f captures, %.1f layers\n",
                    width, height, n / since, sum_.cpuMs / n, sum_.encodeMs / n, sum_.waitMs / n, spans ? spanUs / 1000.0 / (double)spans : 0.0,
                    sum_.gpuMs / g, sum_.categoryMs[0] / g, sum_.categoryMs[1] / g,
                    sum_.categoryMs[2] / g, sum_.categoryMs[3] / g, sum_.categoryMs[4] / g, sum_.draws / n, sum_.passes / n,
                    sum_.captures / n, sum_.layers / n);
        std::fflush(stdout);
        sum_ = {};
        sum_.start = Clock::now();
    }

    // ------------------------------------------------------------------ Mailbox
    int Mailbox::Acquire(id<MTLDevice> device, NSUInteger width, NSUInteger height)
    {
        std::lock_guard lock(mutex);
        int pick = -1;
        for (int i = 0; i < 3 && pick < 0; ++i)
            if (state[i] == Free)
                pick = i;
        for (int i = 0; i < 3; ++i)   // else drop the oldest finished frame
            if (pick < 0 ? state[i] == Ready : (state[pick] != Free && state[i] == Ready && frame[i] < frame[pick]))
                pick = i;
        if (pick < 0)
            pick = 0;   // cannot happen: two rendering at most, one presenting
        if (!textures[pick] || textures[pick].width != width || textures[pick].height != height)
        {
            MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height mipmapped:NO];
            td.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            td.storageMode = MTLStorageModePrivate;
            textures[pick] = [device newTextureWithDescriptor:td];
        }
        state[pick] = Rendering;
        frame[pick] = ++next;
        return pick;
    }

    void Mailbox::Finished(int i)
    {
        std::lock_guard lock(mutex);
        if (state[i] == Rendering)
            state[i] = Ready;
    }

    int Mailbox::TakeNewest()
    {
        std::lock_guard lock(mutex);
        if (copying)
            return -1;
        int pick = -1;
        for (int i = 0; i < 3; ++i)
            if (state[i] == Ready && frame[i] > shown && (pick < 0 || frame[i] > frame[pick]))
                pick = i;
        if (pick >= 0)
        {
            state[pick] = Presenting;
            shown = frame[pick];
            copying = true;
        }
        return pick;
    }

    void Mailbox::Copied(int i)
    {
        std::lock_guard lock(mutex);
        copying = false;
        if (state[i] == Presenting)
            state[i] = Free;
    }

    void Mailbox::Presented(int i, bool viaDrawable)
    {
        std::lock_guard lock(mutex);
        if (!viaDrawable)   // no drawable this time: the frame stays ready for the next try
        {
            copying = false;
            if (state[i] == Presenting)
                state[i] = Ready;
            shown = 0;
        }
    }

    // ------------------------------------------------------------------ Uncapped
    Uncapped::Uncapped(id<MTLDevice> device, CAMetalLayer* layer)
        : device_(device), layer_(layer), presentQueue_([device newCommandQueue]),
          inflight_(dispatch_semaphore_create(2))   // frames rendering on the GPU; the third texture can be on screen
    {
    }

    bool Uncapped::Render(Runner& runner, CGSize px, float scale, esia::Rect safeArea)
    {
        const auto waitStart = std::chrono::steady_clock::now();
        dispatch_semaphore_wait(inflight_, DISPATCH_TIME_FOREVER);
        const float waitMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - waitStart).count();
        std::shared_ptr<Mailbox> box = mailbox_;
        const int slot = box->Acquire(device_, (NSUInteger)px.width, (NSUInteger)px.height);
        dispatch_semaphore_t inflight = inflight_;
        return runner.Frame(box->textures[slot], ^(id<MTLCommandBuffer> cb) {
            [cb addCompletedHandler:^(id<MTLCommandBuffer>) {
                box->Finished(slot);
                dispatch_semaphore_signal(inflight);
            }];
        }, (int)px.width, (int)px.height, scale, waitMs, safeArea);
    }

    void Uncapped::Present()
    {
        if (stop_->load())
            return;
        @autoreleasepool
        {
            std::shared_ptr<Mailbox> box = mailbox_;
            const int slot = box->TakeNewest();
            if (slot < 0)
                return;
            id<MTLTexture> src = box->textures[slot];
            // a paused MTKView stops resizing its layer's drawables (after entering full screen nothing was presented
            // any more): the layer follows the frames here
            const CGSize size = CGSizeMake(src.width, src.height);
            if (!CGSizeEqualToSize(layer_.drawableSize, size))
                layer_.drawableSize = size;
            id<CAMetalDrawable> d = [layer_ nextDrawable];
            if (!d || d.texture.width != src.width || d.texture.height != src.height)
            {
                box->Presented(slot, false);
                return;
            }
            id<MTLCommandBuffer> cb = [presentQueue_ commandBuffer];
            id<MTLBlitCommandEncoder> blit = [cb blitCommandEncoder];
            [blit copyFromTexture:src toTexture:d.texture];
            [blit endEncoding];
            [cb addCompletedHandler:^(id<MTLCommandBuffer>) { box->Copied(slot); }];
            CountPresented(d);
            [cb presentDrawable:d];
            [cb commit];
        }
    }

    void Uncapped::StartPresentThread(CADisplayLink* link)
    {
        std::shared_ptr<std::atomic<bool>> stop = stop_;
        NSThread* thread = [[NSThread alloc] initWithBlock:^{
            [link addToRunLoop:NSRunLoop.currentRunLoop forMode:NSRunLoopCommonModes];
            while (!stop->load())
                @autoreleasepool
                {
                    [NSRunLoop.currentRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
                }
            [link invalidate];
        }];
        thread.name = @"glass present";
        thread.qualityOfService = NSQualityOfServiceUserInteractive;
        [thread start];
    }
}
