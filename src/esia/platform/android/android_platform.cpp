// Esia - the Android platform layer (include/esia/platform/android.hpp): an activity's input into the core's input, the
// core's requests onto the activity (the soft keyboard), the clipboard through Java.
#include "esia/platform/android.hpp"
#include "esia/base/utf8.hpp"
#include <android/configuration.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <jni.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace esia::platform::android
{
    namespace
    {
        Key KeyOf(int32_t code)
        {
            if (code >= AKEYCODE_A && code <= AKEYCODE_Z)
                return static_cast<Key>((int)Key::A + (code - AKEYCODE_A));
            if (code >= AKEYCODE_0 && code <= AKEYCODE_9)
                return static_cast<Key>((int)Key::Num0 + (code - AKEYCODE_0));
            if (code >= AKEYCODE_F1 && code <= AKEYCODE_F12)
                return static_cast<Key>((int)Key::F1 + (code - AKEYCODE_F1));
            if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9)
                return static_cast<Key>((int)Key::Keypad0 + (code - AKEYCODE_NUMPAD_0));
            switch (code)
            {
            case AKEYCODE_TAB: return Key::Tab;
            case AKEYCODE_DPAD_LEFT: return Key::Left;
            case AKEYCODE_DPAD_RIGHT: return Key::Right;
            case AKEYCODE_DPAD_UP: return Key::Up;
            case AKEYCODE_DPAD_DOWN: return Key::Down;
            case AKEYCODE_PAGE_UP: return Key::PageUp;
            case AKEYCODE_PAGE_DOWN: return Key::PageDown;
            case AKEYCODE_MOVE_HOME: return Key::Home;
            case AKEYCODE_MOVE_END: return Key::End;
            case AKEYCODE_INSERT: return Key::Insert;
            case AKEYCODE_FORWARD_DEL: return Key::Delete;
            case AKEYCODE_DEL: return Key::Backspace;
            case AKEYCODE_SPACE: return Key::Space;
            case AKEYCODE_ENTER:
            case AKEYCODE_NUMPAD_ENTER: return Key::Enter;
            case AKEYCODE_ESCAPE:
            case AKEYCODE_BACK: return Key::Escape;
            case AKEYCODE_CTRL_LEFT: return Key::LeftCtrl;
            case AKEYCODE_CTRL_RIGHT: return Key::RightCtrl;
            case AKEYCODE_SHIFT_LEFT: return Key::LeftShift;
            case AKEYCODE_SHIFT_RIGHT: return Key::RightShift;
            case AKEYCODE_ALT_LEFT: return Key::LeftAlt;
            case AKEYCODE_ALT_RIGHT: return Key::RightAlt;
            case AKEYCODE_META_LEFT: return Key::LeftSuper;
            case AKEYCODE_META_RIGHT: return Key::RightSuper;
            case AKEYCODE_NUMPAD_DOT: return Key::KeypadDecimal;
            case AKEYCODE_NUMPAD_DIVIDE: return Key::KeypadDivide;
            case AKEYCODE_NUMPAD_MULTIPLY: return Key::KeypadMultiply;
            case AKEYCODE_NUMPAD_SUBTRACT: return Key::KeypadSubtract;
            case AKEYCODE_NUMPAD_ADD: return Key::KeypadAdd;
            case AKEYCODE_APOSTROPHE: return Key::Apostrophe;
            case AKEYCODE_COMMA: return Key::Comma;
            case AKEYCODE_MINUS: return Key::Minus;
            case AKEYCODE_PERIOD: return Key::Period;
            case AKEYCODE_SLASH: return Key::Slash;
            case AKEYCODE_SEMICOLON: return Key::Semicolon;
            case AKEYCODE_EQUALS: return Key::Equal;
            case AKEYCODE_LEFT_BRACKET: return Key::LeftBracket;
            case AKEYCODE_BACKSLASH: return Key::Backslash;
            case AKEYCODE_RIGHT_BRACKET: return Key::RightBracket;
            case AKEYCODE_GRAVE: return Key::GraveAccent;
            case AKEYCODE_CAPS_LOCK: return Key::CapsLock;
            case AKEYCODE_SCROLL_LOCK: return Key::ScrollLock;
            case AKEYCODE_NUM_LOCK: return Key::NumLock;
            case AKEYCODE_SYSRQ: return Key::PrintScreen;
            case AKEYCODE_BREAK: return Key::Pause;
            case AKEYCODE_MENU: return Key::Menu;
            default: return Key::None;
            }
        }

        std::uint32_t ModsOf(int32_t meta)
        {
            return ((meta & AMETA_CTRL_ON) ? Mod_Ctrl : 0u) | ((meta & AMETA_SHIFT_ON) ? Mod_Shift : 0u) | ((meta & AMETA_ALT_ON) ? Mod_Alt : 0u) |
                   ((meta & AMETA_META_ON) ? Mod_Super : 0u);
        }
    }

    struct Platform::Impl
    {
        ANativeActivity* activity = nullptr;
        ANativeWindow* window = nullptr;
        JNIEnv* env = nullptr;
        bool attached = false;          // this thread was attached to the VM here
        Context* ctx = nullptr;
        float density = 1.0f, uiScale = 1.0f;
        bool focused = false;
        bool keyboardShown = false;
        bool captureMouse = false, captureKeyboard = false;

        // touch: the finger that is the mouse, and two fingers that scroll
        int32_t mouseFinger = -1;
        bool scrolling = false;
        Vec2 scrollLast;

        // Java: KeyEvent for the text of a key, the clipboard
        jclass keyEventClass = nullptr;
        jmethodID keyEventCtor = nullptr, getUnicodeChar = nullptr;

        // the window's insets (pixels: left, top, right, bottom), read every kInsetFrames frames and when the window
        // or the configuration changes
        static constexpr int kInsetFrames = 30;
        int insets[4] = {0, 0, 0, 0};
        int insetCountdown = 0;

        float Scale() const { return density * uiScale; }
        Vec2 Units(float x, float y) const { return Vec2(x / Scale(), y / Scale()); }

        void Queue(InputEvent e)
        {
            if (ctx)
                ctx->QueueInput(std::move(e));
        }

        void ReadDensity()
        {
            AConfiguration* config = AConfiguration_new();
            AConfiguration_fromAssetManager(config, activity->assetManager);
            const int32_t dpi = AConfiguration_getDensity(config);
            AConfiguration_delete(config);
            density = dpi > 0 && dpi != ACONFIGURATION_DENSITY_NONE ? (float)dpi / 160.0f : 1.0f;
        }

        void InitJava()
        {
            if (activity->vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED)
            {
                activity->vm->AttachCurrentThread(&env, nullptr);
                attached = true;
            }
            if (!env)
                return;
            jclass local = env->FindClass("android/view/KeyEvent");
            if (local)
            {
                keyEventClass = static_cast<jclass>(env->NewGlobalRef(local));
                env->DeleteLocalRef(local);
                keyEventCtor = env->GetMethodID(keyEventClass, "<init>", "(JJIIIIIIII)V");
                getUnicodeChar = env->GetMethodID(keyEventClass, "getUnicodeChar", "(I)I");
            }
            env->ExceptionClear();
        }

        // The character a key types (0: none), as Java's KeyEvent tells it for the device's key map.
        char32_t UnicodeOf(const AInputEvent* e)
        {
            if (!env || !keyEventCtor || !getUnicodeChar)
                return 0;
            jobject ke = env->NewObject(keyEventClass, keyEventCtor, (jlong)AKeyEvent_getDownTime(e), (jlong)AKeyEvent_getEventTime(e),
                                        (jint)AKeyEvent_getAction(e), (jint)AKeyEvent_getKeyCode(e), (jint)AKeyEvent_getRepeatCount(e),
                                        (jint)AKeyEvent_getMetaState(e), (jint)AInputEvent_getDeviceId(e), (jint)AKeyEvent_getScanCode(e),
                                        (jint)AKeyEvent_getFlags(e), (jint)AInputEvent_getSource(e));
            if (!ke)
            {
                env->ExceptionClear();
                return 0;
            }
            const jint c = env->CallIntMethod(ke, getUnicodeChar, (jint)AKeyEvent_getMetaState(e));
            env->DeleteLocalRef(ke);
            env->ExceptionClear();
            return c > 0 && (c & 0x80000000) == 0 ? (char32_t)c : 0;   // the combining-accent bit: a dead key
        }

        // ---- the safe area: the decor view's WindowInsets - Type.systemBars() | displayCutout() (API 30), else the
        // system window insets and the cutout's safe insets (API 28, 29). Null before the view is attached: kept as is.
        void ReadInsets()
        {
            if (!env)
                return;
            env->PushLocalFrame(16);
            const auto call = [&](jobject o, const char* name, const char* sig) -> jobject {
                if (!o || env->ExceptionCheck())
                    return nullptr;
                jclass c = env->GetObjectClass(o);
                jmethodID m = env->GetMethodID(c, name, sig);
                return m ? env->CallObjectMethod(o, m) : nullptr;
            };
            const auto callInt = [&](jobject o, const char* name) -> int {
                if (!o || env->ExceptionCheck())
                    return 0;
                jmethodID m = env->GetMethodID(env->GetObjectClass(o), name, "()I");
                return m ? env->CallIntMethod(o, m) : 0;
            };
            jobject javaWindow = call(activity->clazz, "getWindow", "()Landroid/view/Window;");
            jobject decor = call(javaWindow, "getDecorView", "()Landroid/view/View;");
            jobject wi = call(decor, "getRootWindowInsets", "()Landroid/view/WindowInsets;");
            int in[4] = {0, 0, 0, 0};
            bool read = false;
            if (wi && !env->ExceptionCheck())
            {
                jclass type = env->FindClass("android/view/WindowInsets$Type");
                if (type && !env->ExceptionCheck())
                {
                    jmethodID bars = env->GetStaticMethodID(type, "systemBars", "()I");
                    jmethodID cutout = env->GetStaticMethodID(type, "displayCutout", "()I");
                    jmethodID getInsets = env->GetMethodID(env->GetObjectClass(wi), "getInsets", "(I)Landroid/graphics/Insets;");
                    if (bars && cutout && getInsets)
                    {
                        const jint mask = env->CallStaticIntMethod(type, bars) | env->CallStaticIntMethod(type, cutout);
                        jobject ins = env->ExceptionCheck() ? nullptr : env->CallObjectMethod(wi, getInsets, mask);
                        if (ins && !env->ExceptionCheck())
                        {
                            jclass ic = env->GetObjectClass(ins);
                            const char* names[4] = {"left", "top", "right", "bottom"};
                            for (int i = 0; i < 4; ++i)
                                if (jfieldID f = env->GetFieldID(ic, names[i], "I"))
                                    in[i] = env->GetIntField(ins, f);
                            read = !env->ExceptionCheck();
                        }
                    }
                }
                else
                {
                    env->ExceptionClear();   // API 28, 29
                    const char* system[4] = {"getSystemWindowInsetLeft", "getSystemWindowInsetTop", "getSystemWindowInsetRight", "getSystemWindowInsetBottom"};
                    const char* safe[4] = {"getSafeInsetLeft", "getSafeInsetTop", "getSafeInsetRight", "getSafeInsetBottom"};
                    jobject cut = call(wi, "getDisplayCutout", "()Landroid/view/DisplayCutout;");
                    for (int i = 0; i < 4; ++i)
                        in[i] = std::max(callInt(wi, system[i]), callInt(cut, safe[i]));
                    read = !env->ExceptionCheck();
                }
            }
            env->ExceptionClear();
            env->PopLocalFrame(nullptr);
            if (read)
                for (int i = 0; i < 4; ++i)
                    insets[i] = std::max(in[i], 0);
        }

        // ---- the clipboard: Context.getSystemService("clipboard"), ClipData as plain text
        jobject Clipboard()
        {
            if (!env)
                return nullptr;
            jclass ctxClass = env->GetObjectClass(activity->clazz);
            jmethodID service = env->GetMethodID(ctxClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
            jstring name = env->NewStringUTF("clipboard");
            jobject cm = service ? env->CallObjectMethod(activity->clazz, service, name) : nullptr;
            env->DeleteLocalRef(name);
            env->DeleteLocalRef(ctxClass);
            env->ExceptionClear();
            return cm;
        }

        std::string GetClipboard()
        {
            std::string out;
            jobject cm = Clipboard();
            if (!cm)
                return out;
            jclass cmClass = env->GetObjectClass(cm);
            jmethodID getClip = env->GetMethodID(cmClass, "getPrimaryClip", "()Landroid/content/ClipData;");
            jobject clip = getClip ? env->CallObjectMethod(cm, getClip) : nullptr;
            env->ExceptionClear();
            if (clip)
            {
                jclass clipClass = env->GetObjectClass(clip);
                jmethodID count = env->GetMethodID(clipClass, "getItemCount", "()I");
                jmethodID itemAt = env->GetMethodID(clipClass, "getItemAt", "(I)Landroid/content/ClipData$Item;");
                if (count && itemAt && env->CallIntMethod(clip, count) > 0)
                {
                    jobject item = env->CallObjectMethod(clip, itemAt, 0);
                    if (item)
                    {
                        jclass itemClass = env->GetObjectClass(item);
                        jmethodID coerce = env->GetMethodID(itemClass, "coerceToText", "(Landroid/content/Context;)Ljava/lang/CharSequence;");
                        jobject text = coerce ? env->CallObjectMethod(item, coerce, activity->clazz) : nullptr;
                        if (text)
                        {
                            jclass csClass = env->GetObjectClass(text);
                            jmethodID toString = env->GetMethodID(csClass, "toString", "()Ljava/lang/String;");
                            jstring s = toString ? static_cast<jstring>(env->CallObjectMethod(text, toString)) : nullptr;
                            if (s)
                            {
                                if (const char* utf = env->GetStringUTFChars(s, nullptr))
                                {
                                    out = utf;   // modified UTF-8: the same as UTF-8 outside the null character and supplementary planes
                                    env->ReleaseStringUTFChars(s, utf);
                                }
                                env->DeleteLocalRef(s);
                            }
                            env->DeleteLocalRef(csClass);
                            env->DeleteLocalRef(text);
                        }
                        env->DeleteLocalRef(itemClass);
                        env->DeleteLocalRef(item);
                    }
                }
                env->DeleteLocalRef(clipClass);
                env->DeleteLocalRef(clip);
            }
            env->DeleteLocalRef(cmClass);
            env->DeleteLocalRef(cm);
            env->ExceptionClear();
            return out;
        }

        void SetClipboard(const std::string& text)
        {
            jobject cm = Clipboard();
            if (!cm)
                return;
            jclass clipDataClass = env->FindClass("android/content/ClipData");
            jmethodID newPlain = clipDataClass ? env->GetStaticMethodID(clipDataClass, "newPlainText",
                                                                         "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;")
                                               : nullptr;
            jstring label = env->NewStringUTF("text");
            jstring value = env->NewStringUTF(text.c_str());
            jobject clip = newPlain ? env->CallStaticObjectMethod(clipDataClass, newPlain, label, value) : nullptr;
            jclass cmClass = env->GetObjectClass(cm);
            jmethodID setClip = env->GetMethodID(cmClass, "setPrimaryClip", "(Landroid/content/ClipData;)V");
            if (clip && setClip)
                env->CallVoidMethod(cm, setClip, clip);
            env->DeleteLocalRef(cmClass);
            if (clip)
                env->DeleteLocalRef(clip);
            env->DeleteLocalRef(value);
            env->DeleteLocalRef(label);
            if (clipDataClass)
                env->DeleteLocalRef(clipDataClass);
            env->DeleteLocalRef(cm);
            env->ExceptionClear();
        }

        // ---- input
        bool Motion(const AInputEvent* e)
        {
            const int32_t action = AMotionEvent_getAction(e);
            const int32_t kind = action & AMOTION_EVENT_ACTION_MASK;
            const std::size_t index = (std::size_t)((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
            const bool mouse = (AInputEvent_getSource(e) & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE &&
                               AMotionEvent_getToolType(e, 0) == AMOTION_EVENT_TOOL_TYPE_MOUSE;
            const auto pos = [&](std::size_t i) { return Units(AMotionEvent_getX(e, i), AMotionEvent_getY(e, i)); };
            if (mouse)
            {
                switch (kind)
                {
                case AMOTION_EVENT_ACTION_HOVER_MOVE:
                case AMOTION_EVENT_ACTION_MOVE: Queue(InputEvent::MouseMove(pos(0))); break;
                case AMOTION_EVENT_ACTION_HOVER_EXIT: Queue(InputEvent::MouseLeave()); break;
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_UP:
                {
                    Queue(InputEvent::MouseMove(pos(0)));
                    const int32_t buttons = AMotionEvent_getButtonState(e);
                    const bool down = kind == AMOTION_EVENT_ACTION_DOWN;
                    const MouseButton b = (buttons & AMOTION_EVENT_BUTTON_SECONDARY) ? MouseButton::Right
                                          : (buttons & AMOTION_EVENT_BUTTON_TERTIARY) ? MouseButton::Middle
                                                                                      : MouseButton::Left;
                    Queue(InputEvent::Button(down ? b : MouseButton::Left, down));
                    break;
                }
                case AMOTION_EVENT_ACTION_SCROLL:
                    Queue(InputEvent::MouseMove(pos(0)));
                    Queue(InputEvent::Wheel(-AMotionEvent_getAxisValue(e, AMOTION_EVENT_AXIS_HSCROLL, 0), AMotionEvent_getAxisValue(e, AMOTION_EVENT_AXIS_VSCROLL, 0)));
                    break;
                default: break;
                }
                return true;
            }
            // touch: the first finger is the mouse; a second one turns the gesture into scrolling
            switch (kind)
            {
            case AMOTION_EVENT_ACTION_DOWN:
                mouseFinger = AMotionEvent_getPointerId(e, 0);
                scrolling = false;
                Queue(InputEvent::MouseMove(pos(0)));
                Queue(InputEvent::Button(MouseButton::Left, true, true));   // a finger's: scroll areas may take it over
                break;
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                if (!scrolling && AMotionEvent_getPointerCount(e) >= 2)
                {
                    // the press becomes a scroll: cancelled, not clicked (a focus loss releases it as canceled)
                    scrolling = true;
                    Queue(InputEvent::FocusEvent(false));
                    Queue(InputEvent::FocusEvent(true));
                    scrollLast = (pos(0) + pos(1)) * 0.5f;
                    Queue(InputEvent::MouseMove(scrollLast));
                }
                (void)index;
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                if (scrolling && AMotionEvent_getPointerCount(e) >= 2)
                {
                    const Vec2 c = (pos(0) + pos(1)) * 0.5f;
                    const Vec2 d = c - scrollLast;
                    scrollLast = c;
                    Queue(InputEvent::MouseMove(c));
                    Queue(InputEvent::Wheel(d.x * 0.1f, d.y * 0.1f));   // a dp is a tenth of a notch: the content follows the fingers
                }
                else if (!scrolling)
                    for (std::size_t i = 0; i < AMotionEvent_getPointerCount(e); ++i)
                        if (AMotionEvent_getPointerId(e, i) == mouseFinger)
                            Queue(InputEvent::MouseMove(pos(i)));
                break;
            case AMOTION_EVENT_ACTION_UP:
                if (!scrolling)
                {
                    Queue(InputEvent::MouseMove(pos(0)));
                    Queue(InputEvent::Button(MouseButton::Left, false, true));
                }
                Queue(InputEvent::MouseLeave());   // no hover is left behind a finger
                mouseFinger = -1;
                scrolling = false;
                break;
            case AMOTION_EVENT_ACTION_CANCEL:
                Queue(InputEvent::FocusEvent(false));
                Queue(InputEvent::FocusEvent(true));
                Queue(InputEvent::MouseLeave());
                mouseFinger = -1;
                scrolling = false;
                break;
            default: break;
            }
            return true;
        }

        bool KeyEvent(const AInputEvent* e)
        {
            const int32_t code = AKeyEvent_getKeyCode(e);
            const int32_t action = AKeyEvent_getAction(e);
            if (code == AKEYCODE_BACK && !captureKeyboard)
                return false;   // nothing in the UI wants it: the activity goes back
            const Key key = KeyOf(code);
            const std::uint32_t mods = ModsOf(AKeyEvent_getMetaState(e));
            if (action == AKEY_EVENT_ACTION_DOWN)
            {
                if (key != Key::None && AKeyEvent_getRepeatCount(e) == 0)   // a repeat is the core's to make
                    Queue(InputEvent::KeyEvent(key, true, mods));
                if (!(mods & (Mod_Ctrl | Mod_Alt | Mod_Super)))
                    if (const char32_t c = UnicodeOf(e); c >= 0x20 && c != 0x7F)
                    {
                        std::string text;
                        EncodeUtf8(text, c);
                        Queue(InputEvent::TextEvent(std::move(text)));
                    }
            }
            else if (action == AKEY_EVENT_ACTION_UP)
            {
                if (key != Key::None)
                    Queue(InputEvent::KeyEvent(key, false, mods));
            }
            return key != Key::None || code == AKEYCODE_UNKNOWN;
        }
    };

    Platform::Platform(void* activity) : impl_(std::make_unique<Impl>())
    {
        impl_->activity = static_cast<ANativeActivity*>(activity);
        impl_->ReadDensity();
        impl_->InitJava();
    }

    Platform::~Platform()
    {
        Impl& m = *impl_;
        if (m.env && m.keyEventClass)
            m.env->DeleteGlobalRef(m.keyEventClass);
        if (m.attached)
            m.activity->vm->DetachCurrentThread();
    }

    void Platform::SetWindow(void* window)
    {
        impl_->window = static_cast<ANativeWindow*>(window);
        impl_->insetCountdown = 0;   // the insets are read again with the next frame
    }

    void Platform::ConfigurationChanged()
    {
        impl_->ReadDensity();
        impl_->insetCountdown = 0;
    }

    void Platform::SetFocused(bool focused)
    {
        if (impl_->focused != focused)
            impl_->Queue(InputEvent::FocusEvent(focused));
        impl_->focused = focused;
    }

    bool Platform::HandleInputEvent(const void* event)
    {
        const AInputEvent* e = static_cast<const AInputEvent*>(event);
        switch (AInputEvent_getType(e))
        {
        case AINPUT_EVENT_TYPE_MOTION: return impl_->Motion(e);
        case AINPUT_EVENT_TYPE_KEY: return impl_->KeyEvent(e);
        default: return false;
        }
    }

    void Platform::SetContext(Context* ctx) { impl_->ctx = ctx; }

    void Platform::Configure(ContextDesc& desc)
    {
        Impl* m = impl_.get();
        desc.getClipboard = [m] { return m->GetClipboard(); };
        desc.setClipboard = [m](const std::string& s) { m->SetClipboard(s); };
    }

    FrameInfo Platform::Frame() const
    {
        Impl& m = *impl_;
        FrameInfo f;
        f.visible = m.window != nullptr;
        f.width = m.window ? ANativeWindow_getWidth(m.window) : 0;
        f.height = m.window ? ANativeWindow_getHeight(m.window) : 0;
        f.scale = m.Scale();
        f.focused = m.focused;
        if (m.window && --m.insetCountdown <= 0)
        {
            m.ReadInsets();
            m.insetCountdown = Impl::kInsetFrames;
        }
        f.safeLeft = m.insets[0];
        f.safeTop = m.insets[1];
        f.safeRight = m.insets[2];
        f.safeBottom = m.insets[3];
        return f;
    }

    void Platform::ApplyRequests(const PlatformRequests& requests)
    {
        Impl& m = *impl_;
        m.captureMouse = requests.wantCaptureMouse;
        m.captureKeyboard = requests.wantCaptureKeyboard;
        if (requests.wantTextInput != m.keyboardShown)
        {
            m.keyboardShown = requests.wantTextInput;
            if (m.keyboardShown)
                ANativeActivity_showSoftInput(m.activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
            else
                ANativeActivity_hideSoftInput(m.activity, ANATIVEACTIVITY_HIDE_SOFT_INPUT_NOT_ALWAYS);
        }
    }

    void Platform::SetUiScale(float scale) { impl_->uiScale = scale > 0.0f ? scale : 1.0f; }
    bool Platform::WantCaptureMouse() const { return impl_->captureMouse; }
    bool Platform::WantCaptureKeyboard() const { return impl_->captureKeyboard; }
    std::string Platform::FilesDirectory() const { return impl_->activity->internalDataPath ? impl_->activity->internalDataPath : ""; }
}
