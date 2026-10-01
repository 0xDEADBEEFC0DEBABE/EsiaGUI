// glass_window - what the frame shares on macOS and iOS (app_macos.mm, app_ios.mm, both on app_apple.mm): options, the
// frame loop on the Metal backend (Runner), the --vsync off mailbox and the frame's text system with SF Symbols for
// esia::ui's icons. Objective-C++.
//
// --vsync off: no cap. A view cannot present faster than the display refreshes (a drawable comes back once per
// refresh), so rendering and presenting are split, like a game's "mailbox" mode: every frame renders into an offscreen
// texture as soon as the main run loop is idle (events first), at most two frames rendering on the GPU, and once per
// display refresh the newest finished one is copied into a drawable and presented (Uncapped).
//
// Icons: esia::ui's icons are code points of Windows' icon fonts (Segoe Fluent Icons / MDL2 Assets), which Apple's
// systems do not have. An example that sets UiDesc::iconFontFile = kSystemSymbolsFont gets them as SF Symbols instead:
// the frame's text system draws that "font" by rasterizing the matching symbol (AppKit / UIKit: RasterizeSymbol) into
// an Alpha8 atlas page of its own.
#pragma once
#include "app.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/rhi.hpp"
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace glass::apple
{
    struct Options
    {
        std::string api = "metal";
        float width = 1280.0f, height = 800.0f;   // UI units (macOS; on iOS the screen)
        float scale = 0.0f;                       // pixels per UI unit (0 = the screen's: UI units are points)
        bool vsync = true, debug = false, stats = false, fullscreen = false;
        int fullscreenAt = -1;   // --fullscreen-at N: toggle full screen after frame N (tests, macOS)
        int hideAt = -1;         // --hide-at N: hide the app after frame N, as Cmd+H does (tests, macOS)
        double fixedDt = 0.0;
        int frames = 0;
        std::string screenshot;
        std::vector<std::string> fonts;
    };

    int Usage(const App& app, const char* problem);
    bool Parse(App& app, int argc, char** argv, Options& o, std::string& problem);

    // The SF Symbol for a code point of esia/ui/icons.hpp (null: none).
    const char* SymbolOf(char32_t c);
    // Per platform (AppKit, UIKit): the coverage of the symbol `name` fitted into a px x px em box, w x h bytes.
    bool RasterizeSymbol(const char* name, int px, std::vector<std::uint8_t>& coverage, int& w, int& h);

    // --stats: a drawable the display showed or dropped (its presented handler)
    void CountPresented(id<MTLDrawable> drawable);

    // Device, Context, text system, renderer and the frame loop (the render thread of app.cpp), on the main thread.
    class Runner
    {
    public:
        Runner(App& app, const Options& opt) : app_(app), opt_(opt) {}
        ~Runner();

        // `cd`: the platform's clipboard and input timings.
        bool Init(id<MTLDevice> mtlDevice, id<MTLCommandQueue> queue, int width, int height, float scale, const esia::ContextDesc& cd);

        esia::Context* Ctx() const { return ctx_.get(); }
        int FrameCount() const { return frame_; }
        const Options& Opt() const { return opt_; }
        const esia::PlatformRequests* Requests() const { return ctx_ ? &ctx_->Requests() : nullptr; }

        // One frame into `texture`; `finish` adds to the command buffer before it is committed (present, copy).
        // `waitMs`: how long the caller waited for a drawable or a free frame slot. `safeArea`: UI units
        // (FrameParams::safeArea). Returns false once the last frame (--frames) was drawn.
        bool Frame(id<MTLTexture> texture, void (^finish)(id<MTLCommandBuffer> cb), int width, int height, float scale, float waitMs,
                   esia::Rect safeArea = esia::Rect());

        // Everything goes in app.cpp's order: the example's shutdown, then renderer, text system, Context, device.
        // Returns the exit code.
        int Shutdown();

    private:
        using Clock = std::chrono::steady_clock;
        void Accumulate(int width, int height);

        struct Sums
        {
            Clock::time_point start = Clock::now();
            int frames = 0, gpuFrames = 0;
            double cpuMs = 0, encodeMs = 0, waitMs = 0, gpuMs = 0, draws = 0, passes = 0, captures = 0, layers = 0;
            double categoryMs[(int)esia::rhi::ProfileCategory::Count] = {};
        };
        Sums sum_;
        struct GpuSpan
        {
            std::atomic<std::uint64_t> us{0}, count{0};
        };
        std::shared_ptr<GpuSpan> gpuSpan_ = std::make_shared<GpuSpan>();
        float encodeMs_ = 0.0f, drawableWaitMs_ = 0.0f;

        App& app_;
        Options opt_;
        id<MTLCommandQueue> queue_ = nil;
        std::unique_ptr<esia::rhi::Device> device_;
        std::unique_ptr<esia::Context> ctx_;
        std::unique_ptr<esia::text::TextSystem> textSystem_;
        esia::text::TextSystem* text_ = nullptr;
        esia::text::FontId font_ = 0;
        std::unique_ptr<esia::render::Renderer> renderer_;
        std::string adapter_;
        Clock::time_point start_, fpsStart_;
        int frame_ = 0, fpsFrames_ = 0, result_ = 0;
        float fps_ = 0.0f, cpuMs_ = 0.0f;
    };

    // --vsync off: three render targets between the frame loop, the GPU and the display. A frame renders into a free
    // texture (or replaces the oldest finished one); the display takes the newest finished one. Completion handlers
    // run on Metal's threads: everything is under the mutex.
    struct Mailbox
    {
        enum State { Free, Rendering, Ready, Presenting };
        std::mutex mutex;
        id<MTLTexture> textures[3];
        State state[3] = {Free, Free, Free};
        std::uint64_t frame[3] = {};
        std::uint64_t next = 0, shown = 0;
        bool copying = false;   // a blit is still reading its texture

        // A texture for the next frame, `width` x `height` (recreated when the size changed).
        int Acquire(id<MTLDevice> device, NSUInteger width, NSUInteger height);
        void Finished(int i);
        // The newest finished frame not shown yet, now Presenting; -1 while the last copy is still running. (Not
        // gated on the last present reaching the screen: the display link already paces one per refresh, and its
        // callback races the presented handler of the previous frame.)
        int TakeNewest();
        void Copied(int i);
        void Presented(int i, bool viaDrawable);
    };

    // --vsync off on a view's CAMetalLayer: Render on the main thread, Present on the display link's.
    class Uncapped
    {
    public:
        Uncapped(id<MTLDevice> device, CAMetalLayer* layer);

        // One frame of `runner` into the mailbox, `px` pixels: waits while two frames are rendering (back-pressure).
        // False once the runner is done.
        bool Render(Runner& runner, CGSize px, float scale, esia::Rect safeArea = esia::Rect());
        // Copies the newest finished frame into a drawable and presents it, unless the last copy is still running.
        void Present();
        // A thread of its own that runs `link`'s callbacks, until Stop: the main thread is busy rendering (or waiting
        // for a free frame) and would delay the display link past refreshes.
        void StartPresentThread(CADisplayLink* link) API_AVAILABLE(macos(14.0), ios(3.1));
        void Stop() { stop_->store(true); }
        bool Stopped() const { return stop_->load(); }

    private:
        id<MTLDevice> device_;
        CAMetalLayer* layer_;
        id<MTLCommandQueue> presentQueue_;   // a present must not wait behind the frames still rendering
        dispatch_semaphore_t inflight_;
        std::shared_ptr<Mailbox> mailbox_ = std::make_shared<Mailbox>();
        std::shared_ptr<std::atomic<bool>> stop_ = std::make_shared<std::atomic<bool>>(false);
    };
}
