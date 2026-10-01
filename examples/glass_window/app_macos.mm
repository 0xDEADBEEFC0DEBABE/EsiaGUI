// glass_window - the examples' frame on macOS (app.hpp): options, an NSWindow with an MTKView, the Metal backend and
// the frame loop. The options are app.cpp's; --api takes metal only, --no-shader-cache is accepted and ignored.
//
// Threads: AppKit wants its windows and views on the main thread, and MTKView calls its delegate there once per display
// refresh, so the frame loop (Context, renderer, the example's frame) runs on the main thread too - input needs no
// crossing over. The drawable's CAMetalLayer has framebufferOnly = NO: glass reads the target.
//
// --vsync off: no cap. A window cannot present faster than the display refreshes (the window server hands a drawable
// back once per refresh, displaySyncEnabled or not), so rendering and presenting are split, like a game's "mailbox"
// mode: every frame renders into an offscreen texture as soon as the main run loop is idle (events first), at most
// three frames in flight on the GPU, and once per display refresh the latest one is copied into a drawable and
// presented. The frame rate is then what the CPU and the GPU allow.
//
// Icons: esia::ui's icons are code points of Windows' icon fonts (Segoe Fluent Icons / MDL2 Assets), which macOS does not
// have. An example that sets UiDesc::iconFontFile = kSystemSymbolsFont gets them as SF Symbols instead: the frame's text
// system draws that "font" by rasterizing the matching symbol (AppKit) into an Alpha8 atlas page of its own.
//
// Input: mouse (UI units = points x backing scale / UI scale), wheel, keys (esia::Key from the virtual key codes),
// modifiers, text through NSTextInputClient (the IME composes inline: InputState::Composition, the candidate window at
// PlatformRequests::imeRect), clipboard (NSPasteboard), cursor shapes.
#include "app.hpp"
#include "image.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/metal.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#endif
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace glass
{
    namespace
    {
        struct Options
        {
            std::string api = "metal";
            float width = 1280.0f, height = 800.0f;   // UI units
            float scale = 0.0f;                       // pixels per UI unit (0 = the screen's backing scale)
            bool vsync = true, debug = false, stats = false, fullscreen = false;
            int fullscreenAt = -1;   // --fullscreen-at N: toggle full screen after frame N (tests)
            double fixedDt = 0.0;
            int frames = 0;
            std::string screenshot;
            std::vector<std::string> fonts;
        };

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

        esia::Key KeyOf(unsigned short code)
        {
            using K = esia::Key;
            switch (code)
            {
            case kVK_Tab: return K::Tab;
            case kVK_LeftArrow: return K::Left;
            case kVK_RightArrow: return K::Right;
            case kVK_UpArrow: return K::Up;
            case kVK_DownArrow: return K::Down;
            case kVK_PageUp: return K::PageUp;
            case kVK_PageDown: return K::PageDown;
            case kVK_Home: return K::Home;
            case kVK_End: return K::End;
            case kVK_Help: return K::Insert;
            case kVK_ForwardDelete: return K::Delete;
            case kVK_Delete: return K::Backspace;
            case kVK_Space: return K::Space;
            case kVK_Return:
            case kVK_ANSI_KeypadEnter: return K::Enter;
            case kVK_Escape: return K::Escape;
            case kVK_ANSI_A: return K::A;
            case kVK_ANSI_B: return K::B;
            case kVK_ANSI_C: return K::C;
            case kVK_ANSI_D: return K::D;
            case kVK_ANSI_E: return K::E;
            case kVK_ANSI_F: return K::F;
            case kVK_ANSI_G: return K::G;
            case kVK_ANSI_H: return K::H;
            case kVK_ANSI_I: return K::I;
            case kVK_ANSI_J: return K::J;
            case kVK_ANSI_K: return K::K;
            case kVK_ANSI_L: return K::L;
            case kVK_ANSI_M: return K::M;
            case kVK_ANSI_N: return K::N;
            case kVK_ANSI_O: return K::O;
            case kVK_ANSI_P: return K::P;
            case kVK_ANSI_Q: return K::Q;
            case kVK_ANSI_R: return K::R;
            case kVK_ANSI_S: return K::S;
            case kVK_ANSI_T: return K::T;
            case kVK_ANSI_U: return K::U;
            case kVK_ANSI_V: return K::V;
            case kVK_ANSI_W: return K::W;
            case kVK_ANSI_X: return K::X;
            case kVK_ANSI_Y: return K::Y;
            case kVK_ANSI_Z: return K::Z;
            case kVK_ANSI_0: return K::Num0;
            case kVK_ANSI_1: return K::Num1;
            case kVK_ANSI_2: return K::Num2;
            case kVK_ANSI_3: return K::Num3;
            case kVK_ANSI_4: return K::Num4;
            case kVK_ANSI_5: return K::Num5;
            case kVK_ANSI_6: return K::Num6;
            case kVK_ANSI_7: return K::Num7;
            case kVK_ANSI_8: return K::Num8;
            case kVK_ANSI_9: return K::Num9;
            case kVK_F1: return K::F1;
            case kVK_F2: return K::F2;
            case kVK_F3: return K::F3;
            case kVK_F4: return K::F4;
            case kVK_F5: return K::F5;
            case kVK_F6: return K::F6;
            case kVK_F7: return K::F7;
            case kVK_F8: return K::F8;
            case kVK_F9: return K::F9;
            case kVK_F10: return K::F10;
            case kVK_F11: return K::F11;
            case kVK_F12: return K::F12;
            case kVK_Control: return K::LeftCtrl;
            case kVK_RightControl: return K::RightCtrl;
            case kVK_Shift: return K::LeftShift;
            case kVK_RightShift: return K::RightShift;
            case kVK_Option: return K::LeftAlt;
            case kVK_RightOption: return K::RightAlt;
            case kVK_Command: return K::LeftSuper;
            case kVK_RightCommand: return K::RightSuper;
            default: return K::None;
            }
        }

        std::uint32_t ModsOf(NSEventModifierFlags f)
        {
            std::uint32_t m = 0;
            if (f & NSEventModifierFlagControl)
                m |= esia::Mod_Ctrl;
            if (f & NSEventModifierFlagShift)
                m |= esia::Mod_Shift;
            if (f & NSEventModifierFlagOption)
                m |= esia::Mod_Alt;
            if (f & NSEventModifierFlagCommand)
                m |= esia::Mod_Super;
            return m;
        }

        NSCursor* CursorOf(esia::MouseCursor c)
        {
            switch (c)
            {
            case esia::MouseCursor::TextInput: return NSCursor.IBeamCursor;
            case esia::MouseCursor::Hand: return NSCursor.pointingHandCursor;
            case esia::MouseCursor::ResizeEW: return NSCursor.resizeLeftRightCursor;
            case esia::MouseCursor::ResizeNS: return NSCursor.resizeUpDownCursor;
            case esia::MouseCursor::ResizeNWSE:
                if (@available(macOS 15.0, *))
                    return [NSCursor frameResizeCursorFromPosition:NSCursorFrameResizePositionTopLeft inDirections:NSCursorFrameResizeDirectionsAll];
                return NSCursor.crosshairCursor;
            case esia::MouseCursor::ResizeNESW:
                if (@available(macOS 15.0, *))
                    return [NSCursor frameResizeCursorFromPosition:NSCursorFrameResizePositionTopRight inDirections:NSCursorFrameResizeDirectionsAll];
                return NSCursor.crosshairCursor;
            case esia::MouseCursor::ResizeAll: return NSCursor.openHandCursor;
            default: return NSCursor.arrowCursor;
            }
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

        // The frame's text system: FreeType for text, SF Symbols for kSystemSymbolsFont (everything else is forwarded).
        class SymbolTextSystem final : public esia::text::TextSystem
        {
        public:
            SymbolTextSystem(std::unique_ptr<esia::text::TextSystem> inner, esia::TextureRegistry& textures) : inner_(std::move(inner)), textures_(textures) {}
            ~SymbolTextSystem() override
            {
                if (page_)
                    textures_.Destroy(page_);
            }

            esia::text::FontId AddFontFile(const char* path, int faceIndex) override
            {
                return path && std::strcmp(path, kSystemSymbolsFont) == 0 ? kSymbols : inner_->AddFontFile(path, faceIndex);
            }
            esia::text::FontId AddFontMemory(const void* data, std::size_t size, int faceIndex) override { return inner_->AddFontMemory(data, size, faceIndex); }
            void AddFallback(esia::text::FontId font) override
            {
                if (font != kSymbols)
                    inner_->AddFallback(font);
            }
            void NewFrame(const esia::text::RasterParams& params) override
            {
                pixelsPerUnit_ = params.pixelsPerUnit > 0.0f ? params.pixelsPerUnit : 1.0f;
                inner_->NewFrame(params);
            }
            esia::text::TextMetrics Measure(esia::text::FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags) override
            {
                if (font.id == kSymbols)
                    return {esia::Vec2(font.size, font.size), font.size * 0.8f, 1};
                return inner_->Measure(font, text, wrapWidth, flags);
            }
            esia::Vec2 Draw(esia::DrawList& dl, esia::text::FontRef font, esia::Vec2 pos, esia::Color color, std::string_view text, float wrapWidth,
                            std::uint32_t flags, float scale) override
            {
                if (font.id == kSymbols)
                    return esia::Vec2(font.size, font.size);
                return inner_->Draw(dl, font, pos, color, text, wrapWidth, flags, scale);
            }
            void DrawGlyph(esia::DrawList& dl, esia::text::FontRef font, char32_t codepoint, esia::Vec2 center, esia::Color color) override
            {
                if (font.id != kSymbols)
                    return inner_->DrawGlyph(dl, font, codepoint, center, color);
                const int px = std::clamp((int)std::lround(font.size * pixelsPerUnit_), 1, 256);
                const Slot* slot = Find(codepoint, px);
                if (!slot)
                    return;
                const float w = (float)slot->w / pixelsPerUnit_, h = (float)slot->h / pixelsPerUnit_;
                const float x = std::round((center.x - w * 0.5f) * pixelsPerUnit_) / pixelsPerUnit_;
                const float y = std::round((center.y - h * 0.5f) * pixelsPerUnit_) / pixelsPerUnit_;
                const float inv = 1.0f / (float)kPage;
                dl.AddImage(page_, esia::Rect(x, y, x + w, y + h), esia::Vec2((float)slot->x * inv, (float)slot->y * inv),
                            esia::Vec2((float)(slot->x + slot->w) * inv, (float)(slot->y + slot->h) * inv), color.ToRgba8());
            }

        private:
            static constexpr esia::text::FontId kSymbols = 0x7FFF5F01;
            static constexpr int kPage = 1024;
            struct Slot
            {
                int x = 0, y = 0, w = 0, h = 0;
                bool ok = false;
            };

            // The symbol for `c` at an em of `px` pixels, rasterized once into the atlas page (shelf packing).
            const Slot* Find(char32_t c, int px)
            {
                const std::uint64_t key = ((std::uint64_t)c << 16) | (std::uint64_t)px;
                if (auto it = slots_.find(key); it != slots_.end())
                    return it->second.ok ? &it->second : nullptr;
                Slot& slot = slots_[key];
                const char* name = SymbolOf(c);
                if (!name)
                    return nullptr;
                std::vector<std::uint8_t> coverage;
                int w = 0, h = 0;
                if (!Rasterize(name, px, coverage, w, h))
                    return nullptr;
                if (!page_)
                    page_ = textures_.Create({esia::TextureFormat::Alpha8, kPage, kPage});
                if (shelfX_ + w + 1 > kPage)
                {
                    shelfX_ = 1;
                    shelfY_ += shelfH_ + 1;
                    shelfH_ = 0;
                }
                if (shelfY_ + h + 1 > kPage)
                    return nullptr;   // a full page: the rest stays undrawn (the showcase needs a few dozen)
                slot = {shelfX_, shelfY_, w, h, true};
                textures_.Update(page_, slot.x, slot.y, w, h, coverage.data());
                shelfX_ += w + 1;
                shelfH_ = std::max(shelfH_, h);
                return &slot;
            }

            // Coverage of the symbol fitted into a px x px em box (Segoe's icons fill about 85 % of theirs).
            static bool Rasterize(const char* name, int px, std::vector<std::uint8_t>& out, int& w, int& h)
            {
                @autoreleasepool
                {
                    NSImage* base = [NSImage imageWithSystemSymbolName:[NSString stringWithUTF8String:name] accessibilityDescription:nil];
                    if (!base)
                        return false;
                    NSImageSymbolConfiguration* cfg = [NSImageSymbolConfiguration configurationWithPointSize:(CGFloat)px weight:NSFontWeightRegular];
                    NSImage* img = [base imageWithSymbolConfiguration:cfg];
                    const NSSize sz = img.size;
                    if (sz.width <= 0 || sz.height <= 0)
                        return false;
                    const double box = px * 0.85, k = std::min(box / sz.width, box / sz.height);
                    w = std::max(1, (int)std::ceil(sz.width * k));
                    h = std::max(1, (int)std::ceil(sz.height * k));
                    // AppKit draws nothing into an alpha-only context: RGBA, then its alpha
                    std::vector<std::uint8_t> rgba((std::size_t)w * (std::size_t)h * 4, 0);
                    CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                    CGContextRef cg = CGBitmapContextCreate(rgba.data(), (size_t)w, (size_t)h, 8, (size_t)w * 4, srgb,
                                                            (CGBitmapInfo)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
                    CGColorSpaceRelease(srgb);
                    if (!cg)
                        return false;
                    NSGraphicsContext* g = [NSGraphicsContext graphicsContextWithCGContext:cg flipped:NO];
                    [NSGraphicsContext saveGraphicsState];
                    NSGraphicsContext.currentContext = g;
                    [img drawInRect:NSMakeRect(0, 0, w, h) fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1.0];
                    [NSGraphicsContext restoreGraphicsState];
                    CGContextRelease(cg);
                    out.resize((std::size_t)w * (std::size_t)h);
                    for (std::size_t i = 0; i < out.size(); ++i)
                        out[i] = rgba[i * 4 + 3];
                    return true;
                }
            }

            std::unique_ptr<esia::text::TextSystem> inner_;
            esia::TextureRegistry& textures_;
            float pixelsPerUnit_ = 1.0f;
            esia::TextureId page_ = 0;
            int shelfX_ = 1, shelfY_ = 1, shelfH_ = 0;
            std::unordered_map<std::uint64_t, Slot> slots_;
        };

        std::string Utf8(id string)
        {
            NSString* s = [string isKindOfClass:[NSAttributedString class]] ? [(NSAttributedString*)string string] : (NSString*)string;
            return s ? std::string(s.UTF8String) : std::string();
        }

        // --vsync off: three render targets between the frame loop, the GPU and the display. A frame renders into a
        // free texture (or replaces the oldest finished one); the display takes the newest finished one. Completion
        // handlers run on Metal's threads: everything is under the mutex.
        struct Mailbox
        {
            enum State { Free, Rendering, Ready, Presenting };
            std::mutex mutex;
            id<MTLTexture> textures[3];
            State state[3] = {Free, Free, Free};
            std::uint64_t frame[3] = {};
            std::uint64_t next = 0, shown = 0;
            bool copying = false;           // a blit is still reading its texture

            // A texture for the next frame, `width` x `height` (recreated when the size changed).
            int Acquire(id<MTLDevice> device, NSUInteger width, NSUInteger height)
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
                    MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:width height:height
                                                                                              mipmapped:NO];
                    td.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
                    td.storageMode = MTLStorageModePrivate;
                    textures[pick] = [device newTextureWithDescriptor:td];
                }
                state[pick] = Rendering;
                frame[pick] = ++next;
                return pick;
            }

            void Finished(int i)
            {
                std::lock_guard lock(mutex);
                if (state[i] == Rendering)
                    state[i] = Ready;
            }

            // The newest finished frame not shown yet, now Presenting; -1 while the last copy is still running. (Not
            // gated on the last present reaching the screen: the display link already paces one per refresh, and its
            // callback races the presented handler of the previous frame.)
            int TakeNewest()
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

            void Copied(int i)
            {
                std::lock_guard lock(mutex);
                copying = false;
                if (state[i] == Presenting)
                    state[i] = Free;
            }

            void Presented(int i, bool viaDrawable)
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
        };

        // --stats: drawables the window server showed (presentedTime > 0) and dropped (0), from presented handlers
        std::atomic<int> g_presented{0}, g_dropped{0};

        void CountPresented(id<MTLDrawable> d)
        {
            [d addPresentedHandler:^(id<MTLDrawable> shown) {
                (shown.presentedTime > 0.0 ? g_presented : g_dropped).fetch_add(1);
            }];
        }

        // Device, Context, text system, renderer and the frame loop (the render thread of app.cpp).
        class Runner
        {
        public:
            Runner(App& app, const Options& opt) : app_(app), opt_(opt) {}

            bool Init(id<MTLDevice> mtlDevice, id<MTLCommandQueue> queue, int width, int height, float scale)
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

                esia::ContextDesc cd;
                cd.getClipboard = [] {
                    NSString* s = [NSPasteboard.generalPasteboard stringForType:NSPasteboardTypeString];
                    return s ? std::string(s.UTF8String) : std::string();
                };
                cd.setClipboard = [](const std::string& s) {
                    [NSPasteboard.generalPasteboard clearContents];
                    [NSPasteboard.generalPasteboard setString:[NSString stringWithUTF8String:s.c_str()] forType:NSPasteboardTypeString];
                };
                cd.input.doubleClickTime = (float)NSEvent.doubleClickInterval;
                cd.input.keyRepeatDelay = (float)NSEvent.keyRepeatDelay;
                cd.input.keyRepeatRate = (float)NSEvent.keyRepeatInterval;
                ctx_ = std::make_unique<esia::Context>(cd);
#if defined(GLASS_TEXT)
                if (std::unique_ptr<esia::text::TextSystem> ft = esia::text::CreateFreeTypeTextSystem(ctx_->Textures()))
                    textSystem_ = std::make_unique<SymbolTextSystem>(std::move(ft), ctx_->Textures());
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

            esia::Context* Ctx() const { return ctx_.get(); }
            int FrameCount() const { return frame_; }
            int FullscreenAt() const { return opt_.fullscreenAt; }
            const esia::PlatformRequests* Requests() const { return ctx_ ? &ctx_->Requests() : nullptr; }

            // One frame into `texture`; `finish` adds to the command buffer before it is committed (present, copy).
            // `waitMs`: how long the caller waited for a drawable or a free frame slot. Returns false once the last
            // frame (--frames) was drawn.
            bool Frame(id<MTLTexture> texture, void (^finish)(id<MTLCommandBuffer> cb), int width, int height, float scale, float waitMs)
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
                    app_.frame(*ctx_, {"Metal", adapter_, width, height, scale, fps_, cpuMs_, &renderer_->Stats()});
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

            // Everything goes in app.cpp's order: the example's shutdown, then renderer, text system, Context, device.
            int Shutdown()
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

        private:
            using Clock = std::chrono::steady_clock;

            // --stats: once a second, the averages of the renderer's numbers (GPU times per category where the device
            // has timestamps: on Apple GPUs whole encoders, the split inside a render pass approximate)
            void Accumulate(int width, int height)
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
    }
}

@interface GlassView : MTKView <MTKViewDelegate, NSTextInputClient>
- (instancetype)initWithFrame:(NSRect)frame device:(id<MTLDevice>)device runner:(glass::Runner*)runner scale:(float)scale;
- (void)startUncapped;
@end

@implementation GlassView
{
    glass::Runner* runner_;
    float scale_;   // pixels per UI unit (0 = the backing scale)
    NSTrackingArea* tracking_;
    NSMutableAttributedString* marked_;
    esia::MouseCursor cursor_;
    bool inside_;
    bool done_;
    // --vsync off
    id<MTLCommandQueue> presentQueue_;
    CAMetalLayer* metalLayer_;
    dispatch_semaphore_t inflight_;
    std::shared_ptr<glass::Mailbox> mailbox_;
    id displayLink_;   // CADisplayLink (macOS 14+), on the present thread's run loop
    std::shared_ptr<std::atomic<bool>> stopPresenting_;
}

- (instancetype)initWithFrame:(NSRect)frame device:(id<MTLDevice>)device runner:(glass::Runner*)runner scale:(float)scale
{
    if ((self = [super initWithFrame:frame device:device]))
    {
        runner_ = runner;
        scale_ = scale;
        marked_ = [NSMutableAttributedString new];
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.framebufferOnly = NO;   // glass reads the drawable
        CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        self.colorspace = srgb;
        CGColorSpaceRelease(srgb);
        self.layer.opaque = YES;
        self.delegate = self;
    }
    return self;
}

- (float)pixelScale
{
    const NSSize pts = self.bounds.size;
    const CGSize px = self.drawableSize;
    return scale_ > 0.0f ? scale_ : (pts.width > 0 ? (float)(px.width / pts.width) : 1.0f);
}

// points -> UI units
- (float)unitsPerPoint
{
    const NSSize pts = self.bounds.size;
    const CGSize px = self.drawableSize;
    return pts.width > 0 ? (float)(px.width / pts.width) / [self pixelScale] : 1.0f;
}

- (void)drawInMTKView:(MTKView*)view
{
    (void)view;
    if (done_)
        return;
    @autoreleasepool
    {
        const CGSize px = self.drawableSize;
        if (px.width < 1 || px.height < 1)
            return;
        const auto waitStart = std::chrono::steady_clock::now();
        id<CAMetalDrawable> drawable = self.currentDrawable;
        if (!drawable)
            return;
        const float waitMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - waitStart).count();
        const bool more = runner_->Frame(drawable.texture, ^(id<MTLCommandBuffer> cb) {
            glass::CountPresented(drawable);
            [cb presentDrawable:drawable];
        },
                                         (int)px.width, (int)px.height, [self pixelScale], waitMs);
        [self applyRequests];
        if (!more)
        {
            done_ = true;
            [NSApp terminate:nil];
        }
    }
}

// --vsync off: rendering runs as fast as it can, one frame per pass of the main run loop, in every mode (a live
// resize or a menu keeps it going); presenting follows the display (a display link), always the newest finished frame
- (void)startUncapped
{
    // a queue of its own: a present must not wait behind the frames still rendering (it copies a finished one)
    presentQueue_ = [self.device newCommandQueue];
    metalLayer_ = (CAMetalLayer*)self.layer;
    metalLayer_.displaySyncEnabled = NO;
    self.paused = YES;
    self.enableSetNeedsDisplay = NO;
    inflight_ = dispatch_semaphore_create(2);   // frames rendering on the GPU; the third texture can be on screen
    mailbox_ = std::make_shared<glass::Mailbox>();
    stopPresenting_ = std::make_shared<std::atomic<bool>>(false);
    if (@available(macOS 14.0, *))
    {
        // presenting runs on a thread of its own: the main thread is busy rendering (or waiting for a free frame)
        // and would delay the display link past refreshes
        CADisplayLink* link = [self displayLinkWithTarget:self selector:@selector(onDisplay:)];
        displayLink_ = link;
        std::shared_ptr<std::atomic<bool>> stop = stopPresenting_;
        NSThread* thread = [[NSThread alloc] initWithBlock:^{
            [link addToRunLoop:NSRunLoop.currentRunLoop forMode:NSRunLoopCommonModes];
            while (!stop->load())
                @autoreleasepool
                {
                    [NSRunLoop.currentRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
                }
        }];
        thread.name = @"glass present";
        thread.qualityOfService = NSQualityOfServiceUserInteractive;
        [thread start];
    }
    [self tick];
}

- (void)tick
{
    if (done_)
        return;
    @autoreleasepool
    {
        const CGSize px = self.drawableSize;
        if (px.width >= 1 && px.height >= 1)
        {
            // back-pressure: at most two frames rendering on the GPU
            const auto waitStart = std::chrono::steady_clock::now();
            dispatch_semaphore_wait(inflight_, DISPATCH_TIME_FOREVER);
            const float waitMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - waitStart).count();
            std::shared_ptr<glass::Mailbox> box = mailbox_;
            const int slot = box->Acquire(self.device, (NSUInteger)px.width, (NSUInteger)px.height);
            dispatch_semaphore_t inflight = inflight_;
            const bool more = runner_->Frame(box->textures[slot], ^(id<MTLCommandBuffer> cb) {
                [cb addCompletedHandler:^(id<MTLCommandBuffer>) {
                    box->Finished(slot);
                    dispatch_semaphore_signal(inflight);
                }];
            }, (int)px.width, (int)px.height, [self pixelScale], waitMs);
            [self applyRequests];
            if (@available(macOS 14.0, *))
                ;
            else
                [self present];   // no display link: present what is ready after every frame
            if (!more)
            {
                done_ = true;
                stopPresenting_->store(true);
                [displayLink_ invalidate];
                [NSApp terminate:nil];
                return;
            }
        }
    }
    [self performSelector:@selector(tick) withObject:nil afterDelay:0 inModes:@[NSRunLoopCommonModes]];
}

- (void)onDisplay:(id)link
{
    (void)link;
    if (!stopPresenting_->load())
        [self present];
}

// Copies the newest finished frame into a drawable and presents it, unless the last one is not on screen yet.
- (void)present
{
    @autoreleasepool
    {
        std::shared_ptr<glass::Mailbox> box = mailbox_;
        const int slot = box->TakeNewest();
        if (slot < 0)
            return;
        id<MTLTexture> src = box->textures[slot];
        CAMetalLayer* layer = metalLayer_;
        // a paused MTKView stops resizing its layer's drawables (after entering full screen nothing was presented any
        // more): the layer follows the frames here
        const CGSize size = CGSizeMake(src.width, src.height);
        if (!CGSizeEqualToSize(layer.drawableSize, size))
            layer.drawableSize = size;
        id<CAMetalDrawable> d = [layer nextDrawable];
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
        glass::CountPresented(d);
        [cb presentDrawable:d];
        [cb commit];
    }
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}

- (void)applyRequests
{
    if (runner_->FrameCount() == runner_->FullscreenAt())
        [self.window toggleFullScreen:nil];
    const esia::PlatformRequests* r = runner_->Requests();
    if (!r)
        return;
    if (r->cursor != cursor_ && inside_)
        [glass::CursorOf(r->cursor) set];
    cursor_ = r->cursor;
    if (!r->wantTextInput && marked_.length > 0)
    {
        [self.inputContext discardMarkedText];
        [self unmarkText];
    }
}

- (void)queue:(esia::InputEvent)e
{
    if (esia::Context* ctx = runner_->Ctx())
        ctx->QueueInput(std::move(e));
}

// ---- mouse
- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)acceptsFirstMouse:(NSEvent*)event { (void)event; return YES; }

- (void)updateTrackingAreas
{
    if (tracking_)
        [self removeTrackingArea:tracking_];
    tracking_ = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                             options:NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways | NSTrackingInVisibleRect
                                               owner:self
                                            userInfo:nil];
    [self addTrackingArea:tracking_];
    [super updateTrackingAreas];
}

- (void)move:(NSEvent*)e
{
    const NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    const float k = [self unitsPerPoint];
    [self queue:esia::InputEvent::MouseMove(esia::Vec2((float)p.x * k, (float)(self.bounds.size.height - p.y) * k))];
}

- (void)button:(NSEvent*)e button:(esia::MouseButton)b down:(bool)down
{
    [self move:e];
    [self queue:esia::InputEvent::Button(b, down)];
}

- (esia::MouseButton)otherButton:(NSEvent*)e
{
    return e.buttonNumber == 2 ? esia::MouseButton::Middle : e.buttonNumber == 3 ? esia::MouseButton::X1 : esia::MouseButton::X2;
}

- (void)mouseMoved:(NSEvent*)e { [self move:e]; }
- (void)mouseDragged:(NSEvent*)e { [self move:e]; }
- (void)rightMouseDragged:(NSEvent*)e { [self move:e]; }
- (void)otherMouseDragged:(NSEvent*)e { [self move:e]; }
- (void)mouseDown:(NSEvent*)e { [self button:e button:esia::MouseButton::Left down:true]; }
- (void)mouseUp:(NSEvent*)e { [self button:e button:esia::MouseButton::Left down:false]; }
- (void)rightMouseDown:(NSEvent*)e { [self button:e button:esia::MouseButton::Right down:true]; }
- (void)rightMouseUp:(NSEvent*)e { [self button:e button:esia::MouseButton::Right down:false]; }
- (void)otherMouseDown:(NSEvent*)e { [self button:e button:[self otherButton:e] down:true]; }
- (void)otherMouseUp:(NSEvent*)e { [self button:e button:[self otherButton:e] down:false]; }

- (void)mouseEntered:(NSEvent*)e
{
    inside_ = true;
    [glass::CursorOf(cursor_) set];
    [self move:e];
}

- (void)mouseExited:(NSEvent*)e
{
    (void)e;
    inside_ = false;
    [NSCursor.arrowCursor set];
    [self queue:esia::InputEvent::MouseMove(esia::Vec2(esia::kNoMousePos, esia::kNoMousePos))];
}

- (void)scrollWheel:(NSEvent*)e
{
    // notches: a wheel click is 1 line; trackpads report points (about 10 per line)
    const float k = e.hasPreciseScrollingDeltas ? 0.1f : 1.0f;
    [self queue:esia::InputEvent::Wheel((float)e.scrollingDeltaX * k, (float)e.scrollingDeltaY * k)];
}

// ---- keyboard
- (void)keyDown:(NSEvent*)e
{
    const esia::Key k = glass::KeyOf(e.keyCode);
    const std::uint32_t mods = glass::ModsOf(e.modifierFlags);
    const bool composing = marked_.length > 0;
    const esia::PlatformRequests* r = runner_->Requests();
    if (k != esia::Key::None && !composing && !e.isARepeat)
    {
        [self queue:esia::InputEvent::KeyEvent(k, true, mods)];
        // AppKit sends no key-up for a key pressed with Command held
        if (mods & esia::Mod_Super)
            [self queue:esia::InputEvent::KeyEvent(k, false, mods)];
    }
    if (mods & (esia::Mod_Super | esia::Mod_Ctrl))
        return;   // shortcuts are keys, not text
    if (r && r->wantTextInput)
        [self interpretKeyEvents:@[e]];   // insertText / setMarkedText (the IME) / doCommandBySelector
}

- (void)keyUp:(NSEvent*)e
{
    const esia::Key k = glass::KeyOf(e.keyCode);
    if (k != esia::Key::None)
        [self queue:esia::InputEvent::KeyEvent(k, false, glass::ModsOf(e.modifierFlags))];
}

- (void)flagsChanged:(NSEvent*)e
{
    const esia::Key k = glass::KeyOf(e.keyCode);
    NSEventModifierFlags bit = 0;
    switch (k)
    {
    case esia::Key::LeftShift:
    case esia::Key::RightShift: bit = NSEventModifierFlagShift; break;
    case esia::Key::LeftCtrl:
    case esia::Key::RightCtrl: bit = NSEventModifierFlagControl; break;
    case esia::Key::LeftAlt:
    case esia::Key::RightAlt: bit = NSEventModifierFlagOption; break;
    case esia::Key::LeftSuper:
    case esia::Key::RightSuper: bit = NSEventModifierFlagCommand; break;
    default: return;
    }
    [self queue:esia::InputEvent::KeyEvent(k, (e.modifierFlags & bit) != 0, glass::ModsOf(e.modifierFlags))];
}

// ---- NSTextInputClient: committed text and the IME's composition
- (void)insertText:(id)string replacementRange:(NSRange)range
{
    (void)range;
    const std::string s = glass::Utf8(string);
    std::string text;
    for (unsigned char c : s)
        if (c >= 0x20 && c != 0x7F)
            text += (char)c;
    if (marked_.length > 0)
        [self queue:esia::InputEvent::Composition(std::string(), 0)];
    [marked_ deleteCharactersInRange:NSMakeRange(0, marked_.length)];
    if (!text.empty())
        [self queue:esia::InputEvent::TextEvent(text)];
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selected replacementRange:(NSRange)range
{
    (void)range;
    NSString* s = [string isKindOfClass:[NSAttributedString class]] ? [(NSAttributedString*)string string] : (NSString*)string;
    [marked_ setAttributedString:[[NSAttributedString alloc] initWithString:s ? s : @""]];
    // the caret in bytes of the UTF-8 composition
    NSString* before = [s substringToIndex:std::min<NSUInteger>(selected.location, s.length)];
    [self queue:esia::InputEvent::Composition(glass::Utf8(s), (int)std::strlen(before.UTF8String))];
}

- (void)unmarkText
{
    if (marked_.length > 0)
        [self queue:esia::InputEvent::Composition(std::string(), 0)];
    [marked_ deleteCharactersInRange:NSMakeRange(0, marked_.length)];
}

- (BOOL)hasMarkedText { return marked_.length > 0; }
- (NSRange)markedRange { return marked_.length > 0 ? NSMakeRange(0, marked_.length) : NSMakeRange(NSNotFound, 0); }
- (NSRange)selectedRange { return NSMakeRange(NSNotFound, 0); }
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText { return @[]; }
- (NSUInteger)characterIndexForPoint:(NSPoint)point { (void)point; return NSNotFound; }
- (void)doCommandBySelector:(SEL)selector { (void)selector; }   // Enter, arrows ...: already sent as keys

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actual
{
    (void)range;
    (void)actual;
    return nil;
}

// The candidate window goes under the caret the UI reported (UI units -> screen points).
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actual
{
    (void)range;
    (void)actual;
    const esia::PlatformRequests* r = runner_->Requests();
    const float k = [self unitsPerPoint];
    const esia::Rect ime = r ? r->imeRect : esia::Rect();
    const NSRect local = NSMakeRect(ime.min.x / k, self.bounds.size.height - ime.max.y / k, std::max(1.0f, ime.Width() / k), ime.Height() / k);
    return [self.window convertRectToScreen:[self convertRect:local toView:nil]];
}
@end

@interface GlassAppDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic) glass::Runner* runner;
@property(nonatomic) int result;
@end

@implementation GlassAppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app { (void)app; return YES; }
- (void)applicationWillTerminate:(NSNotification*)note
{
    (void)note;
    if (self.runner)
    {
        self.result = self.runner->Shutdown();
        self.runner = nullptr;
    }
    std::fflush(stdout);
    std::exit(self.result);   // the exit code of the frame loop (a failed screenshot, validation messages)
}
@end

namespace glass
{
    int RunApp(int argc, char** argv, App& app)
    {
        Options opt;
        std::string problem;
        if (!Parse(app, argc, argv, opt, problem))
            return Usage(app, problem.c_str());
        if (opt.debug)
            ::setenv("MTL_DEBUG_LAYER", "1", 1);   // Metal reads it when the first device is created

        @autoreleasepool
        {
            NSApplication* nsApp = [NSApplication sharedApplication];
            [nsApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            GlassAppDelegate* delegate = [GlassAppDelegate new];
            nsApp.delegate = delegate;

            NSMenu* bar = [NSMenu new];
            NSMenuItem* appItem = [NSMenuItem new];
            [bar addItem:appItem];
            NSMenu* appMenu = [NSMenu new];
            [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %s", app.name] action:@selector(terminate:) keyEquivalent:@"q"];
            appItem.submenu = appMenu;
            nsApp.mainMenu = bar;

            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            if (!device)
            {
                std::fprintf(stderr, "%s: no Metal device\n", app.name);
                return 1;
            }
            // --scale s: the drawable is size x s pixels (points = pixels / backing scale); else UI units are points
            const float backing = (float)(NSScreen.mainScreen ? NSScreen.mainScreen.backingScaleFactor : 2.0);
            const float points = opt.scale > 0.0f ? opt.scale / backing : 1.0f;
            const NSRect frame = NSMakeRect(0, 0, std::round(opt.width * points), std::round(opt.height * points));
            NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                           styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
                                                                     NSWindowStyleMaskResizable
                                                             backing:NSBackingStoreBuffered
                                                               defer:NO];
            window.title = [NSString stringWithFormat:@"Esia %s - Metal", app.name];
            window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];

            Runner runner(app, opt);
            GlassView* view = [[GlassView alloc] initWithFrame:frame device:device runner:&runner scale:opt.scale];
            NSInteger refresh = 60;
            if (@available(macOS 12.0, *))
                refresh = NSScreen.mainScreen.maximumFramesPerSecond;
            view.preferredFramesPerSecond = refresh;
            view.paused = YES;   // until the runner exists
            window.contentView = view;
            [window center];
            [window makeKeyAndOrderFront:nil];
            [window makeFirstResponder:view];

            const CGSize px = view.drawableSize;
            id<MTLCommandQueue> queue = [device newCommandQueue];
            if (!runner.Init(device, queue, (int)px.width, (int)px.height, [view pixelScale]))
                return 1;
            delegate.runner = &runner;
            if (opt.vsync)
                view.paused = NO;
            else
                [view startUncapped];
            if (@available(macOS 14.0, *))
                [nsApp activate];
            else
            {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                [nsApp activateIgnoringOtherApps:YES];
#pragma clang diagnostic pop
            }
            if (opt.fullscreen)
                [window toggleFullScreen:nil];
            [nsApp run];   // returns never: applicationWillTerminate exits with the loop's result
        }
        return 0;
    }
}
