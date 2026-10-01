// glass_window - the examples' frame on iOS (app.hpp): a UIKit app whose one scene is a full-screen MTKView, with the
// frame loop, options, text system and --vsync off mailbox of app_apple.hpp.
//
// Options are the launch arguments (xcrun devicectl device process launch --console --device <phone> <bundle id>
// --vsync off --stats): app.cpp's, with --size ignored (the screen's) and --scale in pixels per UI unit (default: the
// screen's, so UI units are points). A relative --screenshot path is in the app's Documents directory.
//
// Touch: the first finger is the mouse - down is the left button, lifted it leaves (nothing stays hovered). A touch the
// system takes over (a two-finger scroll, a system gesture) cancels the press as a focus loss does, so it is no click.
// The press is marked as a finger's (InputEvent::touch): esia::ui scrolls a drag along a scroll area from anywhere, a
// row or a slider included, without pressing them; two fingers scroll anywhere (the wheel). Text: while the UI
// wants text input, a hidden text field has the keyboard: what the keyboard commits goes to the UI as text, an input
// method's composition (pinyin, kana ...) as InputState::Composition, Backspace on the empty field and Return as keys.
// The field holds nothing but the composition. The keys of a hardware keyboard (UIPress: HID usages) go to the UI
// whatever has focus. Clipboard: UIPasteboard. The safe area (sensor housing, home indicator) is SceneInfo::safeArea.
//
// Threads: UIKit and the frame loop on the main thread, as on macOS. In the background the loop stops (iOS allows no
// GPU work there) and resumes in the foreground. Frames come at the display's rate up to 120 Hz on ProMotion screens
// (CADisableMinimumFrameDurationOnPhone in the Info.plist); --vsync off renders as fast as the GPU allows and the
// display link presents the newest, as on macOS.
#include "app_apple.hpp"
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <set>

namespace glass
{
    namespace
    {
        App* g_app = nullptr;
        apple::Options g_options;

        esia::Key KeyOf(UIKeyboardHIDUsage usage)
        {
            using K = esia::Key;
            if (usage >= UIKeyboardHIDUsageKeyboardA && usage <= UIKeyboardHIDUsageKeyboardZ)
                return (K)((int)K::A + (int)(usage - UIKeyboardHIDUsageKeyboardA));
            if (usage >= UIKeyboardHIDUsageKeyboard1 && usage <= UIKeyboardHIDUsageKeyboard9)
                return (K)((int)K::Num1 + (int)(usage - UIKeyboardHIDUsageKeyboard1));
            if (usage >= UIKeyboardHIDUsageKeyboardF1 && usage <= UIKeyboardHIDUsageKeyboardF12)
                return (K)((int)K::F1 + (int)(usage - UIKeyboardHIDUsageKeyboardF1));
            switch (usage)
            {
            case UIKeyboardHIDUsageKeyboard0: return K::Num0;
            case UIKeyboardHIDUsageKeyboardTab: return K::Tab;
            case UIKeyboardHIDUsageKeyboardLeftArrow: return K::Left;
            case UIKeyboardHIDUsageKeyboardRightArrow: return K::Right;
            case UIKeyboardHIDUsageKeyboardUpArrow: return K::Up;
            case UIKeyboardHIDUsageKeyboardDownArrow: return K::Down;
            case UIKeyboardHIDUsageKeyboardPageUp: return K::PageUp;
            case UIKeyboardHIDUsageKeyboardPageDown: return K::PageDown;
            case UIKeyboardHIDUsageKeyboardHome: return K::Home;
            case UIKeyboardHIDUsageKeyboardEnd: return K::End;
            case UIKeyboardHIDUsageKeyboardInsert: return K::Insert;
            case UIKeyboardHIDUsageKeyboardDeleteForward: return K::Delete;
            case UIKeyboardHIDUsageKeyboardDeleteOrBackspace: return K::Backspace;
            case UIKeyboardHIDUsageKeyboardSpacebar: return K::Space;
            case UIKeyboardHIDUsageKeyboardReturnOrEnter:
            case UIKeyboardHIDUsageKeypadEnter: return K::Enter;
            case UIKeyboardHIDUsageKeyboardEscape: return K::Escape;
            case UIKeyboardHIDUsageKeyboardLeftControl: return K::LeftCtrl;
            case UIKeyboardHIDUsageKeyboardRightControl: return K::RightCtrl;
            case UIKeyboardHIDUsageKeyboardLeftShift: return K::LeftShift;
            case UIKeyboardHIDUsageKeyboardRightShift: return K::RightShift;
            case UIKeyboardHIDUsageKeyboardLeftAlt: return K::LeftAlt;
            case UIKeyboardHIDUsageKeyboardRightAlt: return K::RightAlt;
            case UIKeyboardHIDUsageKeyboardLeftGUI: return K::LeftSuper;
            case UIKeyboardHIDUsageKeyboardRightGUI: return K::RightSuper;
            default: return K::None;
            }
        }

        std::uint32_t ModsOf(UIKeyModifierFlags f)
        {
            std::uint32_t m = 0;
            if (f & UIKeyModifierControl)
                m |= esia::Mod_Ctrl;
            if (f & UIKeyModifierShift)
                m |= esia::Mod_Shift;
            if (f & UIKeyModifierAlternate)
                m |= esia::Mod_Alt;
            if (f & UIKeyModifierCommand)
                m |= esia::Mod_Super;
            return m;
        }
    }

    // The coverage of an SF Symbol (app_apple.hpp), fitted into a px x px em box (Segoe's icons fill about 85 % of theirs).
    bool apple::RasterizeSymbol(const char* name, int px, std::vector<std::uint8_t>& out, int& w, int& h)
    {
        @autoreleasepool
        {
            UIImageSymbolConfiguration* cfg = [UIImageSymbolConfiguration configurationWithPointSize:(CGFloat)px weight:UIImageSymbolWeightRegular];
            UIImage* img = [UIImage systemImageNamed:[NSString stringWithUTF8String:name] withConfiguration:cfg];
            if (!img)
                return false;
            const CGSize sz = img.size;
            if (sz.width <= 0 || sz.height <= 0)
                return false;
            const double box = px * 0.85, k = std::min(box / sz.width, box / sz.height);
            w = std::max(1, (int)std::ceil(sz.width * k));
            h = std::max(1, (int)std::ceil(sz.height * k));
            // RGBA as on macOS, then its alpha; UIKit draws top-down: the context is flipped to its coordinates
            std::vector<std::uint8_t> rgba((std::size_t)w * (std::size_t)h * 4, 0);
            CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
            CGContextRef cg = CGBitmapContextCreate(rgba.data(), (size_t)w, (size_t)h, 8, (size_t)w * 4, srgb,
                                                    (CGBitmapInfo)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
            CGColorSpaceRelease(srgb);
            if (!cg)
                return false;
            CGContextTranslateCTM(cg, 0, h);
            CGContextScaleCTM(cg, 1, -1);
            UIGraphicsPushContext(cg);
            [img drawInRect:CGRectMake(0, 0, w, h)];
            UIGraphicsPopContext();
            CGContextRelease(cg);
            out.resize((std::size_t)w * (std::size_t)h);
            for (std::size_t i = 0; i < out.size(); ++i)
                out[i] = rgba[i * 4 + 3];
            return true;
        }
    }
}

// ------------------------------------------------------------------ the view: frames and input
@interface GlassView : MTKView <MTKViewDelegate, UITextFieldDelegate>
- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device;
- (void)setActive:(BOOL)active;
- (void)handlePresses:(NSSet<UIPress*>*)presses down:(bool)down;
- (void)typedBackspace;
@end

// The keyboard's end of text input: the system's text field, with its input methods. Hidden: the UI draws the text.
@interface GlassTextField : UITextField
@property(nonatomic, weak) GlassView* owner;
@end

@implementation GlassTextField
- (void)deleteBackward
{
    if (!self.markedTextRange && self.text.length == 0)
        [self.owner typedBackspace];   // nothing of the field's to delete: the UI's text
    [super deleteBackward];
}
- (void)pressesBegan:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    [self.owner handlePresses:presses down:true];
    [super pressesBegan:presses withEvent:event];   // the text: the field's change
}
- (void)pressesEnded:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    [self.owner handlePresses:presses down:false];
    [super pressesEnded:presses withEvent:event];
}
- (void)pressesCancelled:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    [self.owner handlePresses:presses down:false];
    [super pressesCancelled:presses withEvent:event];
}
@end

@interface GlassViewController : UIViewController
@property(nonatomic, strong) GlassView* glassView;
@end

@implementation GlassView
{
    std::unique_ptr<glass::apple::Runner> runner_;
    std::unique_ptr<glass::apple::Uncapped> uncapped_;   // --vsync off
    bool active_, scheduled_, done_;
    UITouch* finger_;                // the touch that is the mouse
    std::set<long> keysDown_;        // HID usages of the hardware keys held
    GlassTextField* field_;          // the keyboard's, while the UI wants text input
    bool composing_;                 // an input method's composition is in the field
    NSUInteger sent_;                // UTF-16 units of the field's text already sent as text
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    if ((self = [super initWithFrame:frame device:device]))
    {
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.framebufferOnly = NO;   // glass reads the drawable
        self.layer.opaque = YES;
        self.multipleTouchEnabled = YES;
        self.paused = YES;           // until the runner exists (the first layout)
        self.enableSetNeedsDisplay = NO;
        self.delegate = self;
        active_ = true;

        // two fingers scroll (the wheel); the touches they began as are cancelled, so they press nothing
        UIPanGestureRecognizer* pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onScroll:)];
        pan.minimumNumberOfTouches = 2;
        pan.maximumNumberOfTouches = 2;
        [self addGestureRecognizer:pan];

        // text: the characters as typed, nothing the keyboard would change on its own
        field_ = [[GlassTextField alloc] initWithFrame:CGRectMake(0, 0, 1, 1)];
        field_.owner = self;
        field_.hidden = YES;
        field_.delegate = self;
        field_.autocorrectionType = UITextAutocorrectionTypeNo;
        field_.autocapitalizationType = UITextAutocapitalizationTypeNone;
        field_.spellCheckingType = UITextSpellCheckingTypeNo;
        field_.smartQuotesType = UITextSmartQuotesTypeNo;
        field_.smartDashesType = UITextSmartDashesTypeNo;
        field_.smartInsertDeleteType = UITextSmartInsertDeleteTypeNo;
        if (@available(iOS 17.0, *))
            field_.inlinePredictionType = UITextInlinePredictionTypeNo;
        field_.returnKeyType = UIReturnKeyDone;
        [field_ addTarget:self action:@selector(fieldChanged:) forControlEvents:UIControlEventEditingChanged];
        [self addSubview:field_];
    }
    return self;
}

- (float)pixelScale
{
    const float s = glass::g_options.scale;
    return s > 0.0f ? s : (float)self.contentScaleFactor;
}

// points -> UI units
- (float)unitsPerPoint { return (float)self.contentScaleFactor / [self pixelScale]; }

- (CGSize)pixelSize
{
    const CGFloat k = self.contentScaleFactor;
    return CGSizeMake(std::round(self.bounds.size.width * k), std::round(self.bounds.size.height * k));
}

- (esia::Rect)safeArea
{
    const UIEdgeInsets in = self.safeAreaInsets;
    const CGSize b = self.bounds.size;
    const float k = [self unitsPerPoint];
    return esia::Rect((float)in.left * k, (float)in.top * k, (float)(b.width - in.right) * k, (float)(b.height - in.bottom) * k);
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    if (runner_ || done_)
        return;
    const CGSize px = [self pixelSize];
    if (px.width < 1 || px.height < 1)
        return;
    // the runner, once the view has its size (and a screen)
    const NSInteger fps = self.window.windowScene.screen.maximumFramesPerSecond;
    self.preferredFramesPerSecond = fps;
    runner_ = std::make_unique<glass::apple::Runner>(*glass::g_app, glass::g_options);
    esia::ContextDesc cd;
    cd.getClipboard = [] {
        NSString* s = UIPasteboard.generalPasteboard.string;
        return s ? std::string(s.UTF8String) : std::string();
    };
    cd.setClipboard = [](const std::string& s) { UIPasteboard.generalPasteboard.string = [NSString stringWithUTF8String:s.c_str()]; };
    cd.input.dragThreshold = 8.0f;   // a finger is less steady than a mouse
    if (!runner_->Init(self.device, [self.device newCommandQueue], (int)px.width, (int)px.height, [self pixelScale], cd))
    {
        runner_.reset();
        done_ = true;
        return;
    }
    if (glass::g_options.vsync)
        self.paused = NO;
    else
    {
        uncapped_ = std::make_unique<glass::apple::Uncapped>(self.device, (CAMetalLayer*)self.layer);
        CADisplayLink* link = [CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplay:)];
        link.preferredFrameRateRange = CAFrameRateRangeMake((float)fps, (float)fps, (float)fps);
        uncapped_->StartPresentThread(link);
        [self tick];
    }
}

// Frames stop in the background (no GPU work there) and resume in the foreground.
- (void)setActive:(BOOL)active
{
    active_ = active;
    if (!runner_ || done_)
        return;
    if (glass::g_options.vsync)
        self.paused = !active;
    else if (active && !scheduled_)
        [self tick];
}

- (void)finish
{
    done_ = true;
    if (uncapped_)
        uncapped_->Stop();
    self.paused = YES;
    const int result = runner_->Shutdown();
    std::fflush(stdout);
    std::exit(result);   // --frames: the run is over (the exit code of the frame loop)
}

- (void)drawInMTKView:(MTKView*)view
{
    (void)view;
    if (done_ || !runner_ || !active_)
        return;
    @autoreleasepool
    {
        const auto waitStart = std::chrono::steady_clock::now();
        id<CAMetalDrawable> drawable = self.currentDrawable;
        if (!drawable)
            return;
        const float waitMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - waitStart).count();
        const bool more = runner_->Frame(drawable.texture, ^(id<MTLCommandBuffer> cb) {
            glass::apple::CountPresented(drawable);
            [cb presentDrawable:drawable];
        },
                                         (int)drawable.texture.width, (int)drawable.texture.height, [self pixelScale], waitMs, [self safeArea]);
        [self applyRequests];
        if (!more)
            [self finish];
    }
}

// --vsync off: one frame per pass of the main run loop (touches first); the display link presents the newest
- (void)tick
{
    scheduled_ = false;
    if (done_ || !active_)
        return;   // setActive resumes
    @autoreleasepool
    {
        const CGSize px = [self pixelSize];
        if (px.width >= 1 && px.height >= 1)
        {
            const bool more = uncapped_->Render(*runner_, px, [self pixelScale], [self safeArea]);
            [self applyRequests];
            if (!more)
            {
                [self finish];
                return;
            }
        }
    }
    scheduled_ = true;
    [self performSelector:@selector(tick) withObject:nil afterDelay:0 inModes:@[NSRunLoopCommonModes]];
}

- (void)onDisplay:(CADisplayLink*)link
{
    (void)link;
    if (active_)
        uncapped_->Present();
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}

// The keyboard follows the UI's text input (the text field has it); otherwise the view controller has focus
// (hardware keys). A composition the UI no longer wants is dropped, as on macOS.
- (void)applyRequests
{
    const esia::PlatformRequests* r = runner_ ? runner_->Requests() : nullptr;
    if (!r)
        return;
    if (r->wantTextInput && !field_.isFirstResponder)
        [field_ becomeFirstResponder];
    else if (!r->wantTextInput && field_.isFirstResponder)
    {
        [field_ resignFirstResponder];
        [self clearField];
        [self.nextResponder becomeFirstResponder];   // the view controller
    }
}

- (void)queue:(esia::InputEvent)e
{
    if (esia::Context* ctx = runner_ ? runner_->Ctx() : nullptr)
        ctx->QueueInput(std::move(e));
}

// ---- touch
- (esia::Vec2)unitsOf:(CGPoint)p
{
    const float k = [self unitsPerPoint];
    return esia::Vec2((float)p.x * k, (float)p.y * k);
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)event;
    if (finger_)
        return;
    finger_ = touches.anyObject;
    [self queue:esia::InputEvent::MouseMove([self unitsOf:[finger_ locationInView:self]])];
    [self queue:esia::InputEvent::Button(esia::MouseButton::Left, true, true)];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)event;
    if (finger_ && [touches containsObject:finger_])
        [self queue:esia::InputEvent::MouseMove([self unitsOf:[finger_ locationInView:self]])];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)event;
    if (!finger_ || ![touches containsObject:finger_])
        return;
    [self queue:esia::InputEvent::MouseMove([self unitsOf:[finger_ locationInView:self]])];
    [self queue:esia::InputEvent::Button(esia::MouseButton::Left, false, true)];
    [self queue:esia::InputEvent::MouseLeave()];
    finger_ = nil;
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
    (void)event;
    if (!finger_ || ![touches containsObject:finger_])
        return;
    // the system took the touch: the press is cancelled (a focus loss releases it as canceled), not clicked
    [self queue:esia::InputEvent::FocusEvent(false)];
    [self queue:esia::InputEvent::FocusEvent(true)];
    [self queue:esia::InputEvent::MouseLeave()];
    finger_ = nil;
}

- (void)onScroll:(UIPanGestureRecognizer*)pan
{
    switch (pan.state)
    {
    case UIGestureRecognizerStateBegan:
    case UIGestureRecognizerStateChanged:
    {
        // the content follows the fingers; a point is a tenth of a notch, as a trackpad's on macOS
        const CGPoint d = [pan translationInView:self];
        [pan setTranslation:CGPointZero inView:self];
        [self queue:esia::InputEvent::MouseMove([self unitsOf:[pan locationInView:self]])];
        [self queue:esia::InputEvent::Wheel((float)d.x * 0.1f, (float)d.y * 0.1f)];
        break;
    }
    default: [self queue:esia::InputEvent::MouseLeave()]; break;
    }
}

// ---- keys
- (void)handlePresses:(NSSet<UIPress*>*)presses down:(bool)down
{
    for (UIPress* press in presses)
    {
        UIKey* key = press.key;
        if (!key)
            continue;
        if (down)
            keysDown_.insert((long)key.keyCode);
        else
            keysDown_.erase((long)key.keyCode);
        const esia::Key k = glass::KeyOf(key.keyCode);
        if (k != esia::Key::None)
            [self queue:esia::InputEvent::KeyEvent(k, down, glass::ModsOf(key.modifierFlags))];
    }
}

- (void)queueKey:(esia::Key)key unlessHeld:(UIKeyboardHIDUsage)usage
{
    if (keysDown_.count((long)usage))
        return;   // a hardware key: already sent as itself
    [self queue:esia::InputEvent::KeyEvent(key, true)];
    [self queue:esia::InputEvent::KeyEvent(key, false)];
}

- (void)typedBackspace { [self queueKey:esia::Key::Backspace unlessHeld:UIKeyboardHIDUsageKeyboardDeleteOrBackspace]; }

// ---- text: the hidden field's changes
- (void)queueText:(NSString*)text
{
    std::string utf8;
    for (unsigned char c : std::string(text.UTF8String))
        if (c >= 0x20 && c != 0x7F)
            utf8 += (char)c;
    if (!utf8.empty())
        [self queue:esia::InputEvent::TextEvent(utf8)];
}

- (void)endComposition
{
    if (composing_)
        [self queue:esia::InputEvent::Composition(std::string(), 0)];
    composing_ = false;
}

- (void)clearField
{
    [self endComposition];
    field_.text = @"";
    sent_ = 0;
}

// The field's text is what was committed (sent once) and the composition after it, if any. Once nothing is being
// composed the field is emptied again.
- (void)fieldChanged:(UITextField*)field
{
    NSString* text = field.text ? field.text : @"";
    UITextRange* marked = field.markedTextRange;
    const NSUInteger markedAt = marked ? (NSUInteger)[field offsetFromPosition:field.beginningOfDocument toPosition:marked.start] : text.length;
    const NSUInteger committed = std::min(markedAt, (NSUInteger)text.length);
    if (committed > sent_)
    {
        [self endComposition];   // the composition became this text
        [self queueText:[text substringWithRange:NSMakeRange(sent_, committed - sent_)]];
    }
    sent_ = committed;
    if (!marked)
    {
        [self clearField];
        return;
    }
    // the composition, its caret in bytes of its UTF-8
    NSString* composition = [field textInRange:marked];
    if (!composition)
        composition = @"";
    const NSInteger caret = [field offsetFromPosition:marked.start toPosition:field.selectedTextRange.start];
    NSString* before = [composition substringToIndex:(NSUInteger)std::clamp<NSInteger>(caret, 0, (NSInteger)composition.length)];
    [self queue:esia::InputEvent::Composition(std::string(composition.UTF8String), (int)std::strlen(before.UTF8String))];
    composing_ = true;
}

- (BOOL)textFieldShouldReturn:(UITextField*)field
{
    (void)field;
    [self queueKey:esia::Key::Enter unlessHeld:UIKeyboardHIDUsageKeyboardReturnOrEnter];
    return NO;
}
@end

// ------------------------------------------------------------------ the view controller: full screen, all of it
@implementation GlassViewController
- (void)loadView
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    self.glassView = [[GlassView alloc] initWithFrame:CGRectZero device:device];
    self.view = self.glassView;
}

- (BOOL)prefersStatusBarHidden { return YES; }
- (BOOL)prefersHomeIndicatorAutoHidden { return YES; }
// (no edges deferring system gestures: iOS then holds back every touch near them, and the dock is at the bottom)
- (UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad ? UIInterfaceOrientationMaskAll : UIInterfaceOrientationMaskAllButUpsideDown;
}

// hardware keys while no text field has the keyboard
- (BOOL)canBecomeFirstResponder { return YES; }
- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [self becomeFirstResponder];
}
// (only as the first responder: presses the text field passes on reach here too, and it sent them already)
- (void)pressesBegan:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    if (self.isFirstResponder)
        [self.glassView handlePresses:presses down:true];
    [super pressesBegan:presses withEvent:event];
}
- (void)pressesEnded:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    if (self.isFirstResponder)
        [self.glassView handlePresses:presses down:false];
    [super pressesEnded:presses withEvent:event];
}
- (void)pressesCancelled:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
    if (self.isFirstResponder)
        [self.glassView handlePresses:presses down:false];
    [super pressesCancelled:presses withEvent:event];
}
@end

// ------------------------------------------------------------------ app and scene
@interface GlassSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow* window;
@end

@implementation GlassSceneDelegate
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options
{
    (void)session;
    (void)options;
    UIWindow* window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
    window.rootViewController = [GlassViewController new];
    window.overrideUserInterfaceStyle = UIUserInterfaceStyleDark;
    self.window = window;
    [window makeKeyAndVisible];
}

- (GlassView*)glassView { return ((GlassViewController*)self.window.rootViewController).glassView; }
- (void)sceneDidBecomeActive:(UIScene*)scene { (void)scene; [[self glassView] setActive:YES]; }
- (void)sceneDidEnterBackground:(UIScene*)scene { (void)scene; [[self glassView] setActive:NO]; }
- (void)sceneWillEnterForeground:(UIScene*)scene { (void)scene; [[self glassView] setActive:YES]; }
@end

@interface GlassAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation GlassAppDelegate
- (UISceneConfiguration*)application:(UIApplication*)application
    configurationForConnectingSceneSession:(UISceneSession*)session
                                   options:(UISceneConnectionOptions*)options
{
    (void)application;
    (void)options;
    UISceneConfiguration* c = [[UISceneConfiguration alloc] initWithName:@"Default" sessionRole:session.role];
    c.delegateClass = [GlassSceneDelegate class];
    return c;
}
@end

namespace glass
{
    int RunApp(int argc, char** argv, App& app)
    {
        std::string problem;
        if (!apple::Parse(app, argc, argv, g_options, problem))
            return apple::Usage(app, problem.c_str());
        if (g_options.debug)
            ::setenv("MTL_DEBUG_LAYER", "1", 1);   // Metal reads it when the first device is created
        g_app = &app;
        @autoreleasepool
        {
            if (!g_options.screenshot.empty() && g_options.screenshot.front() != '/')
            {
                NSString* docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
                g_options.screenshot = std::string(docs.UTF8String) + "/" + g_options.screenshot;
            }
            return UIApplicationMain(argc, argv, nil, NSStringFromClass([GlassAppDelegate class]));
        }
    }
}
