// glass_window - the examples' frame on macOS (app.hpp): an NSWindow with an MTKView, AppKit's input, and the frame
// loop, options, text system and --vsync off mailbox of app_apple.hpp. The options are app.cpp's; --api takes metal
// only, --no-shader-cache is accepted and ignored.
//
// Threads: AppKit wants its windows and views on the main thread, and MTKView calls its delegate there once per display
// refresh, so the frame loop (Context, renderer, the example's frame) runs on the main thread too - input needs no
// crossing over. The drawable's CAMetalLayer has framebufferOnly = NO: glass reads the target.
//
// --vsync off: frames render as fast as they can, one per pass of the main run loop, in every mode (a live resize or a
// menu keeps it going); a display link on a thread of its own presents the newest each refresh (app_apple.hpp).
//
// Input: mouse (UI units = points x backing scale / UI scale), wheel, keys (esia::Key from the virtual key codes),
// modifiers, text through NSTextInputClient (the IME composes inline: InputState::Composition, the candidate window at
// PlatformRequests::imeRect), clipboard (NSPasteboard), cursor shapes.
#include "app_apple.hpp"
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <MetalKit/MetalKit.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace glass
{
    using apple::Options;
    using apple::Runner;

    namespace
    {
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

        std::string Utf8(id string)
        {
            NSString* s = [string isKindOfClass:[NSAttributedString class]] ? [(NSAttributedString*)string string] : (NSString*)string;
            return s ? std::string(s.UTF8String) : std::string();
        }

    }

    // The coverage of an SF Symbol (app_apple.hpp), fitted into a px x px em box (Segoe's icons fill about 85 % of theirs).
    bool apple::RasterizeSymbol(const char* name, int px, std::vector<std::uint8_t>& out, int& w, int& h)
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
    std::unique_ptr<glass::apple::Uncapped> uncapped_;   // --vsync off
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
            glass::apple::CountPresented(drawable);
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
    CAMetalLayer* layer = (CAMetalLayer*)self.layer;
    layer.displaySyncEnabled = NO;
    self.paused = YES;
    self.enableSetNeedsDisplay = NO;
    uncapped_ = std::make_unique<glass::apple::Uncapped>(self.device, layer);
    if (@available(macOS 14.0, *))
        uncapped_->StartPresentThread([self displayLinkWithTarget:self selector:@selector(onDisplay:)]);
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
            const bool more = uncapped_->Render(*runner_, px, [self pixelScale]);
            [self applyRequests];
            if (@available(macOS 14.0, *))
                ;
            else
                uncapped_->Present();   // no display link: present what is ready after every frame
            if (!more)
            {
                done_ = true;
                uncapped_->Stop();
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
    uncapped_->Present();
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}

- (void)applyRequests
{
    if (runner_->FrameCount() == runner_->Opt().fullscreenAt)
        [self.window toggleFullScreen:nil];
    if (runner_->FrameCount() == runner_->Opt().hideAt)
        [NSApp hide:nil];
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
        if (!apple::Parse(app, argc, argv, opt, problem))
            return apple::Usage(app, problem.c_str());
        if (opt.debug)
            ::setenv("MTL_DEBUG_LAYER", "1", 1);   // Metal reads it when the first device is created

        @autoreleasepool
        {
            // A frame loop is user-initiated and latency critical, as a game's: without this, macOS naps the app
            // once its window is hidden or covered for half a minute, and its main thread runs on efficiency cores
            // (UI and encoding 4 - 6 times slower; measured with --hide-at). Idle system sleep stays allowed.
            static id<NSObject> activity = [NSProcessInfo.processInfo beginActivityWithOptions:NSActivityUserInitiatedAllowingIdleSystemSleep | NSActivityLatencyCritical
                                                                                       reason:@"Esia frame loop"];
            (void)activity;
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
            if (!runner.Init(device, queue, (int)px.width, (int)px.height, [view pixelScale], cd))
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
