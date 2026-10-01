// glass_window - the examples' frame on Android (app.hpp): a NativeActivity (android_native_app_glue) on the Android
// platform layer (esia/platform/android.hpp), OpenGL ES through EGL (host_opengl_egl.cpp) or Vulkan through
// VK_KHR_android_surface (host_vulkan.cpp).
//
// The example's main runs as glass_main (CMake renames it); its options come from the launch intent's "args" extra:
//   adb shell "am start -n org.esia.showcase/android.app.NativeActivity -e args '--api vulkan --open-all'"
// (one string: the device's shell would split the options)
// stdout and stderr go to the log (adb logcat -s esia), a --screenshot to the app's files directory
// (adb exec-out run-as org.esia.showcase cat files/shot.png > shot.png). The window can go (the app in the background)
// and come back: the device, the UI and its textures stay, only the surface is made again. The icons are Material Icons
// from the APK's assets (icons_material.hpp, icon_font_text.hpp).
#include "app.hpp"
#include "host.hpp"
#include "image.hpp"
#include "esia/platform/android.hpp"
#include "esia/render/renderer.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include "icon_font_text.hpp"
#include "icons_material.hpp"
#endif
#include <android/asset_manager.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <thread>

int glass_main(int argc, char** argv);   // the example's main (CMake: main=glass_main)

namespace glass
{
    namespace
    {
        android_app* g_app = nullptr;

        struct Options
        {
            std::string api;
            float scale = 0.0f;   // pixels per UI unit (0 = the screen's density: UI units are dp)
            bool vsync = true, debug = false;
            double fixedDt = 0.0;
            int frames = 0;
            std::string screenshot;
            std::vector<std::string> fonts;
        };

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
                else if (a == "--api" && needs())
                    o.api = value;
                else if (a == "--vsync" && needs())
                    o.vsync = std::strcmp(value, "off") != 0;
                else if (a == "--scale" && needs())
                    o.scale = (float)std::atof(value);
                else if (a == "--fixed-dt" && needs())
                    o.fixedDt = std::atof(value);
                else if (a == "--frames" && needs())
                    o.frames = std::atoi(value);
                else if (a == "--screenshot" && needs())
                    o.screenshot = value;
                else if (a == "--font" && needs())
                    o.fonts.push_back(value);
                else if (a == "--size" && needs())
                    ;   // the window is the screen
                else if (app.option && app.option(a, value, usedValue))
                    i += usedValue ? 1 : 0;
                else if (problem.empty())
                    problem = "unknown option " + a;
                if (!problem.empty())
                    return false;
            }
            if (o.api.empty())
                o.api = DefaultApi();
            if (!CreateHost(o.api))
                problem = "--api " + o.api + " is not built in (built: " + BuiltApis() + ")";
            else if (!o.screenshot.empty() && o.frames <= 0)
                o.frames = 60;
            return problem.empty();
        }

#if defined(GLASS_TEXT)
        // An asset of the APK, whole; empty when it has none by that name.
        std::vector<std::uint8_t> ReadAsset(AAssetManager* assets, const char* name)
        {
            std::vector<std::uint8_t> data;
            if (AAsset* asset = assets ? AAssetManager_open(assets, name, AASSET_MODE_BUFFER) : nullptr)
            {
                data.resize((std::size_t)AAsset_getLength(asset));
                if (AAsset_read(asset, data.data(), data.size()) != (int)data.size())
                    data.clear();
                AAsset_close(asset);
            }
            return data;
        }

        esia::text::FontId LoadFonts(esia::text::TextSystem& text, const std::vector<std::string>& files)
        {
            if (files.empty())
            {
                const std::vector<esia::text::FontId> chain = esia::text::AddFallbackFonts(text, esia::text::FindDefaultFallbackFonts());
                return chain.empty() ? 0 : chain.front();
            }
            esia::text::FontId main = 0;
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

        // stdout and stderr into the log: a pipe read by a thread
        void RedirectOutput(const char* tag)
        {
            static int fds[2];
            static std::string logTag;
            logTag = tag;
            std::setvbuf(stdout, nullptr, _IOLBF, 0);
            std::setvbuf(stderr, nullptr, _IONBF, 0);
            if (pipe(fds) != 0)
                return;
            dup2(fds[1], 1);
            dup2(fds[1], 2);
            std::thread([] {
                char buf[1024];
                std::string line;
                for (ssize_t n; (n = read(fds[0], buf, sizeof(buf))) > 0;)
                    for (ssize_t i = 0; i < n; ++i)
                        if (buf[i] == '\n')
                        {
                            __android_log_write(ANDROID_LOG_INFO, logTag.c_str(), line.c_str());
                            line.clear();
                        }
                        else
                            line += buf[i];
            }).detach();
        }

        // The launch intent's "args" extra, split at spaces (a word in quotes keeps its spaces).
        std::vector<std::string> IntentArgs(ANativeActivity* activity)
        {
            std::vector<std::string> out;
            JNIEnv* env = nullptr;
            const bool attach = activity->vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED;
            if (attach)
                activity->vm->AttachCurrentThread(&env, nullptr);
            if (!env)
                return out;
            jclass act = env->GetObjectClass(activity->clazz);
            jmethodID getIntent = env->GetMethodID(act, "getIntent", "()Landroid/content/Intent;");
            jobject intent = getIntent ? env->CallObjectMethod(activity->clazz, getIntent) : nullptr;
            if (intent)
            {
                jclass ic = env->GetObjectClass(intent);
                jmethodID getExtra = env->GetMethodID(ic, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
                jstring key = env->NewStringUTF("args");
                jstring value = getExtra ? static_cast<jstring>(env->CallObjectMethod(intent, getExtra, key)) : nullptr;
                if (value)
                {
                    if (const char* s = env->GetStringUTFChars(value, nullptr))
                    {
                        std::string word;
                        bool quoted = false, any = false;
                        for (const char* p = s;; ++p)
                        {
                            if (*p == '"')
                                quoted = !quoted, any = true;
                            else if (*p && (quoted || *p != ' '))
                                word += *p, any = true;
                            else if (any)
                            {
                                out.push_back(word);
                                word.clear();
                                any = false;
                            }
                            if (!*p)
                                break;
                        }
                        env->ReleaseStringUTFChars(value, s);
                    }
                    env->DeleteLocalRef(value);
                }
                env->DeleteLocalRef(key);
                env->DeleteLocalRef(ic);
                env->DeleteLocalRef(intent);
            }
            env->DeleteLocalRef(act);
            env->ExceptionClear();
            if (attach)
                activity->vm->DetachCurrentThread();
            return out;
        }

        struct State
        {
            esia::platform::android::Platform* platform = nullptr;
            bool windowChanged = false;
        };

        void OnCommand(android_app* app, int32_t cmd)
        {
            State& s = *static_cast<State*>(app->userData);
            switch (cmd)
            {
            case APP_CMD_INIT_WINDOW:
                s.platform->SetWindow(app->window);
                s.windowChanged = true;
                break;
            case APP_CMD_TERM_WINDOW:
                s.platform->SetWindow(nullptr);
                s.windowChanged = true;
                break;
            case APP_CMD_GAINED_FOCUS: s.platform->SetFocused(true); break;
            case APP_CMD_LOST_FOCUS: s.platform->SetFocused(false); break;
            case APP_CMD_CONFIG_CHANGED: s.platform->ConfigurationChanged(); break;
            default: break;
            }
        }

        int32_t OnInput(android_app* app, AInputEvent* event)
        {
            return static_cast<State*>(app->userData)->platform->HandleInputEvent(event) ? 1 : 0;
        }

        // Every event waiting; `wait`: blocks for one first (no window: nothing to draw). False once the activity ends.
        bool Poll(android_app* app, bool wait)
        {
            for (;;)
            {
                int events = 0;
                android_poll_source* source = nullptr;
                const int r = ALooper_pollOnce(wait ? -1 : 0, nullptr, &events, reinterpret_cast<void**>(&source));
                if (r < 0 && r != ALOOPER_POLL_CALLBACK)
                    return !app->destroyRequested;
                if (source)
                    source->process(app, source);
                if (app->destroyRequested)
                    return false;
                wait = false;
            }
        }
    }

    namespace
    {
        // The Java calls of Wallpaper(): every result checked, nothing called with an exception pending.
        struct Java
        {
            JNIEnv* env;
            bool Failed() const { return env->ExceptionCheck() == JNI_TRUE; }

            // An APK asset decoded by BitmapFactory (JPEG, PNG, HEIF ...); null when there is no such asset.
            jobject DecodeAsset(const char* name)
            {
                AAsset* asset = AAssetManager_open(g_app->activity->assetManager, name, AASSET_MODE_BUFFER);
                if (!asset)
                    return nullptr;
                const jsize size = (jsize)AAsset_getLength(asset);
                jbyteArray bytes = env->NewByteArray(size);
                if (bytes && !Failed())
                    env->SetByteArrayRegion(bytes, 0, size, static_cast<const jbyte*>(AAsset_getBuffer(asset)));
                AAsset_close(asset);
                jclass factory = Failed() || !bytes ? nullptr : env->FindClass("android/graphics/BitmapFactory");
                jmethodID decode = factory && !Failed() ? env->GetStaticMethodID(factory, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;") : nullptr;
                jobject bmp = decode && !Failed() ? env->CallStaticObjectMethod(factory, decode, bytes, (jint)0, size) : nullptr;
                return Failed() ? nullptr : bmp;
            }

            // WallpaperManager.getBuiltInDrawable() drawn into a bitmap of its own size.
            jobject BuiltIn()
            {
                jclass wmClass = env->FindClass("android/app/WallpaperManager");
                jmethodID getInstance = wmClass && !Failed() ? env->GetStaticMethodID(wmClass, "getInstance", "(Landroid/content/Context;)Landroid/app/WallpaperManager;") : nullptr;
                jmethodID builtIn = getInstance && !Failed() ? env->GetMethodID(wmClass, "getBuiltInDrawable", "()Landroid/graphics/drawable/Drawable;") : nullptr;
                jobject wm = builtIn && !Failed() ? env->CallStaticObjectMethod(wmClass, getInstance, g_app->activity->clazz) : nullptr;
                jobject drawable = wm && !Failed() ? env->CallObjectMethod(wm, builtIn) : nullptr;
                if (Failed() || !drawable)
                    return nullptr;
                jclass dClass = env->GetObjectClass(drawable);
                const jint w = env->CallIntMethod(drawable, env->GetMethodID(dClass, "getIntrinsicWidth", "()I"));
                const jint h = Failed() ? 0 : env->CallIntMethod(drawable, env->GetMethodID(dClass, "getIntrinsicHeight", "()I"));
                jobject bmp = Failed() || w <= 0 || h <= 0 ? nullptr : NewBitmap(w, h);
                jclass canvasClass = bmp ? env->FindClass("android/graphics/Canvas") : nullptr;
                jmethodID canvasCtor = canvasClass && !Failed() ? env->GetMethodID(canvasClass, "<init>", "(Landroid/graphics/Bitmap;)V") : nullptr;
                jobject canvas = canvasCtor && !Failed() ? env->NewObject(canvasClass, canvasCtor, bmp) : nullptr;
                if (Failed() || !canvas)
                    return nullptr;
                env->CallVoidMethod(drawable, env->GetMethodID(dClass, "setBounds", "(IIII)V"), (jint)0, (jint)0, w, h);
                if (!Failed())
                    env->CallVoidMethod(drawable, env->GetMethodID(dClass, "draw", "(Landroid/graphics/Canvas;)V"), canvas);
                return Failed() ? nullptr : bmp;
            }

            jobject Argb8888()
            {
                jclass cfg = env->FindClass("android/graphics/Bitmap$Config");
                jfieldID f = cfg && !Failed() ? env->GetStaticFieldID(cfg, "ARGB_8888", "Landroid/graphics/Bitmap$Config;") : nullptr;
                return f && !Failed() ? env->GetStaticObjectField(cfg, f) : nullptr;
            }

            jobject NewBitmap(jint w, jint h)
            {
                jclass bmpClass = env->FindClass("android/graphics/Bitmap");
                jmethodID create = bmpClass && !Failed() ? env->GetStaticMethodID(bmpClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;") : nullptr;
                jobject cfg = create && !Failed() ? Argb8888() : nullptr;
                return cfg ? env->CallStaticObjectMethod(bmpClass, create, w, h, cfg) : nullptr;
            }

            // `bmp` at most `maxSide` on its longer side, in ARGB_8888 (a wide-gamut decode may be F16), as straight
            // RGBA8.
            bool Read(jobject bmp, int maxSide, int& width, int& height, std::vector<std::uint8_t>& rgba)
            {
                AndroidBitmapInfo info{};
                if (Failed() || !bmp || AndroidBitmap_getInfo(env, bmp, &info) != ANDROID_BITMAP_RESULT_SUCCESS || !info.width || !info.height)
                    return false;
                jclass bmpClass = env->GetObjectClass(bmp);
                const float k = maxSide > 0 ? std::min(1.0f, (float)maxSide / (float)std::max(info.width, info.height)) : 1.0f;
                if (k < 1.0f)
                {
                    jmethodID scaled = env->GetStaticMethodID(bmpClass, "createScaledBitmap", "(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
                    bmp = scaled && !Failed() ? env->CallStaticObjectMethod(bmpClass, scaled, bmp, (jint)std::lround((float)info.width * k),
                                                                            (jint)std::lround((float)info.height * k), JNI_TRUE)
                                              : nullptr;
                    if (Failed() || !bmp || AndroidBitmap_getInfo(env, bmp, &info) != ANDROID_BITMAP_RESULT_SUCCESS)
                        return false;
                }
                if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
                {
                    jmethodID copy = env->GetMethodID(bmpClass, "copy", "(Landroid/graphics/Bitmap$Config;Z)Landroid/graphics/Bitmap;");
                    jobject cfg = copy && !Failed() ? Argb8888() : nullptr;
                    bmp = cfg ? env->CallObjectMethod(bmp, copy, cfg, JNI_FALSE) : nullptr;
                    if (Failed() || !bmp || AndroidBitmap_getInfo(env, bmp, &info) != ANDROID_BITMAP_RESULT_SUCCESS ||
                        info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
                        return false;
                }
                void* pixels = nullptr;
                if (AndroidBitmap_lockPixels(env, bmp, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS)
                    return false;
                rgba.resize((std::size_t)info.width * info.height * 4);
                for (std::uint32_t y = 0; y < info.height; ++y)
                {
                    const std::uint8_t* src = static_cast<const std::uint8_t*>(pixels) + (std::size_t)y * info.stride;
                    std::uint8_t* dst = rgba.data() + (std::size_t)y * info.width * 4;
                    for (std::uint32_t x = 0; x < info.width; ++x, src += 4, dst += 4)
                    {
                        // premultiplied in the bitmap, straight in the texture
                        const unsigned a = src[3];
                        for (int c = 0; c < 3; ++c)
                            dst[c] = a == 255 || a == 0 ? src[c] : (std::uint8_t)std::min(255u, (src[c] * 255u + a / 2) / a);
                        dst[3] = (std::uint8_t)a;
                    }
                }
                AndroidBitmap_unlockPixels(env, bmp);
                width = (int)info.width;
                height = (int)info.height;
                return true;
            }
        };
    }

    bool Wallpaper(int maxSide, int& width, int& height, std::vector<std::uint8_t>& rgba)
    {
        if (!g_app)
            return false;
        JavaVM* vm = g_app->activity->vm;
        JNIEnv* env = nullptr;
        bool attached = false;
        if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED)
        {
            if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
                return false;
            attached = true;
        }
        Java java{env};
        bool ok = false;
        for (const char* asset : {"wallpaper.jpg", "wallpaper.png", "wallpaper.heic"})
        {
            env->PushLocalFrame(32);
            ok = java.Read(java.DecodeAsset(asset), maxSide, width, height, rgba);
            env->ExceptionClear();
            env->PopLocalFrame(nullptr);
            if (ok)
                break;
        }
        if (!ok)
        {
            env->PushLocalFrame(32);
            ok = java.Read(java.BuiltIn(), maxSide, width, height, rgba);
            env->ExceptionClear();
            env->PopLocalFrame(nullptr);
        }
        if (attached)
            vm->DetachCurrentThread();
        return ok;
    }

    int RunApp(int argc, char** argv, App& app)
    {
        android_app* aapp = g_app;
        Options opt;
        std::string problem;
        if (!Parse(app, argc, argv, opt, problem))
        {
            std::fprintf(stderr, "%s: %s\n", app.name, problem.c_str());
            return 2;
        }
        esia::platform::android::Platform platform(aapp->activity);
        if (opt.scale > 0.0f)
            platform.SetUiScale(opt.scale / std::max(platform.Frame().scale, 0.01f));
        State state;
        state.platform = &platform;
        aapp->userData = &state;
        aapp->onAppCmd = OnCommand;
        aapp->onInputEvent = OnInput;

        // the first window
        while (!aapp->window)
            if (!Poll(aapp, true))
                return 0;
        state.windowChanged = false;
        esia::platform::android::FrameInfo fi = platform.Frame();
        std::string error;
        std::unique_ptr<Host> host = CreateHost(opt.api);
        NativeWindow native;
        native.window = aapp->window;
        int width = fi.width, height = fi.height;
        if (!host || !host->Init(native, width, height, {opt.vsync, opt.debug}, error))
        {
            std::fprintf(stderr, "%s: %s: %s\n", app.name, opt.api.c_str(), error.c_str());
            return 1;
        }
        std::printf("%s: %s on %s, %d x %d px, scale %.2f\n", app.name, host->Name(), host->Adapter().c_str(), width, height, fi.scale);

        esia::ContextDesc cd;
        platform.Configure(cd);
        esia::Context ctx(cd);
        esia::text::TextSystem* text = nullptr;
        esia::text::FontId font = 0;
#if defined(GLASS_TEXT)
        std::unique_ptr<esia::text::TextSystem> textSystem = esia::text::CreateFreeTypeTextSystem(ctx.Textures());
        if (textSystem)   // kSystemSymbolsFont: Material Icons
            textSystem = std::make_unique<IconFontTextSystem>(std::move(textSystem), ReadAsset(aapp->activity->assetManager, material_icons::kAsset),
                                                              &material_icons::Map, material_icons::kScale);
        if (textSystem && !app.loadFonts)
            text = textSystem.get();
        else if (textSystem && (font = LoadFonts(*textSystem, opt.fonts)) != 0)
            text = textSystem.get();
#endif
        esia::render::Renderer renderer(host->Device());
        if (app.init)
            app.init(ctx, text, font, opt.fonts);
        if (app.renderer)
            app.renderer(renderer);
        platform.SetContext(&ctx);

        using Clock = std::chrono::steady_clock;
        const Clock::time_point start = Clock::now();
        Clock::time_point fpsStart = start;
        int fpsFrames = 0, frame = 0, result = 0;
        float fps = 0.0f, cpuMs = 0.0f;
        bool hasWindow = true;
        while (Poll(aapp, !hasWindow))
        {
            if (state.windowChanged)
            {
                state.windowChanged = false;
                if (aapp->window && !hasWindow)
                {
                    native.window = aapp->window;
                    fi = platform.Frame();
                    width = fi.width;
                    height = fi.height;
                    hasWindow = host->AttachWindow(native, width, height, error);
                    if (!hasWindow)
                        std::fprintf(stderr, "%s: the new window: %s\n", app.name, error.c_str());
                }
                else if (!aapp->window && hasWindow)
                {
                    host->ReleaseWindow();
                    hasWindow = false;
                }
            }
            if (!hasWindow)
                continue;
            fi = platform.Frame();
            if (fi.width != width || fi.height != height)
            {
                width = fi.width;
                height = fi.height;
                host->Resize(width, height);
            }
            if (!host->BeginFrame())
                continue;
            const double now = std::chrono::duration<double>(Clock::now() - start).count();
            ctx.NewFrame(fi.Params(opt.fixedDt > 0.0 ? (frame + 1) * opt.fixedDt : now));
            if (text && app.loadFonts)
                text->NewFrame({fi.scale});
            const Clock::time_point uiStart = Clock::now();
            if (app.frame)
                app.frame(ctx, {host->Name(), host->Adapter(), width, height, fi.scale, fps, cpuMs, &renderer.Stats()});
            ctx.EndFrame();
            cpuMs = std::chrono::duration<float, std::milli>(Clock::now() - uiStart).count();
            esia::render::RenderParams rp;
            rp.frame = host->Frame();
            if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), host->Target(), rp))
                std::fprintf(stderr, "%s: the device refused frame %d\n", app.name, frame);
            platform.ApplyRequests(ctx.Requests());

            ++frame;
            const bool last = opt.frames > 0 && frame >= opt.frames;
            std::vector<std::uint8_t> pixels;
            const bool capture = last && !opt.screenshot.empty();
            if (!host->EndFrame(capture ? &pixels : nullptr))
                result = 1;
            else if (capture)
            {
                esia::testkit::Image image(width, height);
                image.rgba = std::move(pixels);
                for (std::size_t i = 3; i < image.rgba.size(); i += 4)
                    image.rgba[i] = 255;
                const std::string path = opt.screenshot.front() == '/' ? opt.screenshot : platform.FilesDirectory() + "/" + opt.screenshot;
                if (!esia::testkit::WritePng(path, image, &error))
                {
                    std::fprintf(stderr, "%s: %s\n", app.name, error.c_str());
                    result = 1;
                }
                else
                    std::printf("%s: frame %d written to %s\n", app.name, frame, path.c_str());
            }
            ++fpsFrames;
            const double since = std::chrono::duration<double>(Clock::now() - fpsStart).count();
            if (since >= 0.5)
            {
                fps = (float)(fpsFrames / since);
                fpsFrames = 0;
                fpsStart = Clock::now();
            }
            if (last)
                break;
        }
        platform.SetContext(nullptr);
        // what comes after (the activity finishing) reaches no Platform or State of this function
        aapp->onInputEvent = nullptr;
        aapp->onAppCmd = nullptr;
        aapp->userData = nullptr;
        if (app.shutdown)
            app.shutdown();
        std::printf("%s: %d frames, %u validation / debug-layer messages%s\n", app.name, frame, host->ValidationMessages(), opt.debug ? "" : " (--debug to enable the layer)");
        return result;
    }
}

// The activity's thread: the example's main with the intent's arguments; the activity ends with it.
void android_main(android_app* app)
{
    glass::g_app = app;
    const char* tag = "esia";
    glass::RedirectOutput(tag);
    std::vector<std::string> args = glass::IntentArgs(app->activity);
    std::vector<char*> argv;
    static char name[] = "esia";
    argv.push_back(name);
    for (std::string& a : args)
        argv.push_back(a.data());
    argv.push_back(nullptr);
    const int code = glass_main((int)argv.size() - 1, argv.data());
    std::printf("exit %d\n", code);
    ANativeActivity_finish(app->activity);
    // wait for the activity to go (the glue joins this thread on its destruction)
    while (!app->destroyRequested)
    {
        int events = 0;
        android_poll_source* source = nullptr;
        if (ALooper_pollOnce(-1, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0 && source)
            source->process(app, source);
    }
}
