// glass_window - esia::ui's icons on Linux (symbol_text.hpp): the desktop's symbolic icons (Adwaita, else Yaru: GNOME's
// and Ubuntu's themes, the freedesktop.org names), rasterized by librsvg into a coverage mask. librsvg, cairo and
// GObject are loaded at run time (every GNOME desktop has them): nothing to install for the build, and without them the
// icons are simply not drawn.
#include "icons_linux.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace glass::linux_icons
{
    namespace
    {
        // esia/ui/icons.hpp (Segoe Fluent code points) -> freedesktop.org symbolic icon names, the first one installed
        struct Names
        {
            char32_t c;
            const char* names[3];
        };
        constexpr Names kIcons[] = {
            {0xE700, {"open-menu"}},
            {0xE701, {"network-wireless-signal-excellent", "network-wireless"}},
            {0xE702, {"bluetooth-active", "bluetooth"}},
            {0xE703, {"network-transmit-receive", "network-wired"}},
            {0xE705, {"network-vpn"}},
            {0xE706, {"display-brightness", "weather-clear"}},
            {0xE707, {"mark-location", "find-location"}},
            {0xE708, {"weather-clear-night", "night-light"}},
            {0xE709, {"airplane-mode"}},
            {0xE70D, {"pan-down", "go-down"}},
            {0xE70E, {"pan-up", "go-up"}},
            {0xE70F, {"document-edit"}},
            {0xE710, {"list-add"}},
            {0xE711, {"window-close"}},
            {0xE712, {"view-more-horizontal", "view-more"}},
            {0xE713, {"emblem-system", "preferences-system"}},
            {0xE714, {"camera-video", "video-x-generic"}},
            {0xE715, {"mail-unread", "mail-message-new"}},
            {0xE716, {"system-users"}},
            {0xE717, {"call-start", "phone"}},
            {0xE718, {"view-pin"}},
            {0xE719, {"software-store", "package-x-generic"}},
            {0xE71A, {"media-playback-stop"}},
            {0xE71B, {"insert-link"}},
            {0xE71C, {"nautilus-search-filters", "view-sort-descending"}},
            {0xE71D, {"view-app-grid", "view-grid"}},
            {0xE71E, {"zoom-in"}},
            {0xE71F, {"zoom-out"}},
            {0xE720, {"audio-input-microphone"}},
            {0xE721, {"system-search", "edit-find"}},
            {0xE722, {"camera-photo"}},
            {0xE723, {"mail-attachment"}},
            {0xE724, {"document-send", "mail-send"}},
            {0xE72A, {"go-next"}},
            {0xE72B, {"go-previous"}},
            {0xE72C, {"view-refresh"}},
            {0xE72D, {"send-to", "emblem-shared"}},
            {0xE72E, {"changes-prevent", "system-lock-screen"}},
            {0xE734, {"non-starred"}},
            {0xE735, {"starred"}},
            {0xE738, {"list-remove"}},
            {0xE73E, {"object-select", "emblem-ok"}},
            {0xE740, {"view-fullscreen"}},
            {0xE74D, {"user-trash", "edit-delete"}},
            {0xE74E, {"document-save"}},
            {0xE74F, {"audio-volume-muted"}},
            {0xE753, {"weather-overcast"}},
            {0xE76B, {"pan-start", "go-previous"}},
            {0xE76C, {"pan-end", "go-next"}},
            {0xE767, {"audio-volume-high"}},
            {0xE768, {"media-playback-start"}},
            {0xE769, {"media-playback-pause"}},
            {0xE892, {"media-skip-backward"}},
            {0xE893, {"media-skip-forward"}},
            {0xE771, {"applications-graphics", "color-select"}},
            {0xE774, {"globe", "web-browser"}},
            {0xE77B, {"avatar-default", "user-info"}},
            {0xE783, {"dialog-error"}},
            {0xE785, {"changes-allow"}},
            {0xE787, {"x-office-calendar"}},
            {0xE790, {"preferences-color", "color-select"}},
            {0xE7BA, {"dialog-warning"}},
            {0xE7C1, {"emoji-flags", "flag-outline-thin"}},
            {0xE7E8, {"system-shutdown"}},
            {0xE7FC, {"input-gaming", "applications-games"}},
            {0xE80F, {"go-home", "user-home"}},
            {0xE81C, {"document-open-recent"}},
            {0xE81D, {"find-location", "location-services-active"}},
            {0xE823, {"clock", "preferences-system-time"}},
            {0xE890, {"view-reveal"}},
            {0xE895, {"emblem-synchronizing", "media-playlist-repeat"}},
            {0xE896, {"folder-download", "go-down"}},
            {0xE897, {"dialog-question", "help-browser"}},
            {0xE898, {"go-up"}},
            {0xE8A5, {"text-x-generic", "x-office-document"}},
            {0xE8B7, {"folder"}},
            {0xE8C8, {"edit-copy"}},
            {0xE8D6, {"audio-x-generic", "emblem-music"}},
            {0xE909, {"globe-centered", "web-browser"}},
            {0xE91B, {"image-x-generic", "emblem-photos"}},
            {0xE943, {"utilities-terminal", "text-x-generic"}},
            {0xE945, {"thunderbolt", "battery-full-charging"}},
            {0xE946, {"dialog-information", "help-about"}},
            {0xE962, {"input-mouse"}},
            {0xE9D9, {"utilities-system-monitor", "power-profile-performance"}},
            {0xE9E9, {"multimedia-equalizer", "multimedia-volume-control"}},
            {0xEA18, {"security-high"}},
            {0xEA80, {"weather-clear", "dialog-information"}},
            {0xEA8F, {"preferences-system-notifications"}},
            {0xEB51, {"emote-love"}},
            {0xEB52, {"heart-filled", "emote-love"}},
            {0xEBE8, {"applications-engineering", "dialog-warning"}},
            {0xED1A, {"view-conceal"}},
        };

        // The symbolic icons installed: name (without "-symbolic") -> file, Adwaita's before Yaru's, for every data
        // directory (XDG_DATA_DIRS)
        const std::unordered_map<std::string, std::string>& Installed()
        {
            static const std::unordered_map<std::string, std::string> files = [] {
                std::unordered_map<std::string, std::string> m;
                std::string dirs = std::getenv("XDG_DATA_DIRS") ? std::getenv("XDG_DATA_DIRS") : "";
                if (dirs.empty())
                    dirs = "/usr/local/share:/usr/share";
                const std::string_view suffix = "-symbolic.svg";
                for (const char* theme : {"Adwaita", "Yaru"})
                    for (std::size_t at = 0; at <= dirs.size();)
                    {
                        const std::size_t end = std::min(dirs.find(':', at), dirs.size());
                        const std::filesystem::path root = std::filesystem::path(dirs.substr(at, end - at)) / "icons" / theme;
                        at = end + 1;
                        std::error_code ec;
                        for (auto it = std::filesystem::recursive_directory_iterator(root, ec); !ec && it != std::filesystem::recursive_directory_iterator();
                             it.increment(ec))
                        {
                            const std::string file = it->path().filename().string();
                            if (file.size() > suffix.size() && file.ends_with(suffix))
                                m.emplace(file.substr(0, file.size() - suffix.size()), it->path().string());   // the first one stays
                        }
                    }
                return m;
            }();
            return files;
        }

        // librsvg 2.46+ and cairo, loaded once
        struct Rsvg
        {
            struct Rect
            {
                double x, y, width, height;
            };
            void* (*newFromFile)(const char*, void**) = nullptr;
            int (*renderDocument)(void*, void*, const Rect*, void**) = nullptr;
            void (*unref)(void*) = nullptr;
            void* (*surfaceCreate)(int, int, int) = nullptr;
            void* (*create)(void*) = nullptr;
            void (*destroy)(void*) = nullptr;
            void (*surfaceFlush)(void*) = nullptr;
            unsigned char* (*surfaceData)(void*) = nullptr;
            int (*surfaceStride)(void*) = nullptr;
            void (*surfaceDestroy)(void*) = nullptr;
            bool ok = false;

            static const Rsvg& Get()
            {
                static const Rsvg r = [] {
                    Rsvg r;
                    void* rsvg = ::dlopen("librsvg-2.so.2", RTLD_NOW | RTLD_LOCAL);
                    void* cairo = ::dlopen("libcairo.so.2", RTLD_NOW | RTLD_LOCAL);
                    void* gobject = ::dlopen("libgobject-2.0.so.0", RTLD_NOW | RTLD_LOCAL);
                    if (!rsvg || !cairo || !gobject)
                        return r;
                    const auto load = [](void* lib, const char* name, auto& fn) {
                        fn = reinterpret_cast<std::remove_reference_t<decltype(fn)>>(::dlsym(lib, name));
                        return fn != nullptr;
                    };
                    r.ok = load(rsvg, "rsvg_handle_new_from_file", r.newFromFile) && load(rsvg, "rsvg_handle_render_document", r.renderDocument) &&
                           load(gobject, "g_object_unref", r.unref) && load(cairo, "cairo_image_surface_create", r.surfaceCreate) &&
                           load(cairo, "cairo_create", r.create) && load(cairo, "cairo_destroy", r.destroy) &&
                           load(cairo, "cairo_surface_flush", r.surfaceFlush) && load(cairo, "cairo_image_surface_get_data", r.surfaceData) &&
                           load(cairo, "cairo_image_surface_get_stride", r.surfaceStride) &&
                           load(cairo, "cairo_surface_destroy", r.surfaceDestroy);
                    return r;
                }();
                return r;
            }
        };
    }

    bool Rasterize(char32_t c, int px, std::vector<std::uint8_t>& coverage, int& w, int& h)
    {
        const Rsvg& rsvg = Rsvg::Get();
        if (!rsvg.ok || px <= 0)
            return false;
        const std::string* file = nullptr;
        for (const Names& n : kIcons)
            if (n.c == c)
            {
                for (const char* name : n.names)
                    if (name)
                        if (auto it = Installed().find(name); it != Installed().end())
                        {
                            file = &it->second;
                            break;
                        }
                break;
            }
        if (!file)
            return false;
        void* handle = rsvg.newFromFile(file->c_str(), nullptr);
        if (!handle)
            return false;
        constexpr int kArgb32 = 0;   // CAIRO_FORMAT_ARGB32: premultiplied, the alpha in each pixel's high byte
        void* surface = rsvg.surfaceCreate(kArgb32, px, px);
        void* cr = rsvg.create(surface);
        const Rsvg::Rect box = {0.0, 0.0, (double)px, (double)px};   // the icon's view box fitted into the em box
        const bool ok = rsvg.renderDocument(handle, cr, &box, nullptr) != 0;
        rsvg.destroy(cr);
        rsvg.surfaceFlush(surface);
        if (ok)
        {
            const unsigned char* data = rsvg.surfaceData(surface);
            const int stride = rsvg.surfaceStride(surface);
            w = h = px;
            coverage.resize((std::size_t)px * px);
            for (int y = 0; y < px; ++y)
                for (int x = 0; x < px; ++x)
                {
                    std::uint32_t pixel;
                    std::memcpy(&pixel, data + (std::size_t)y * stride + (std::size_t)x * 4, 4);
                    coverage[(std::size_t)y * px + x] = (std::uint8_t)(pixel >> 24);
                }
        }
        rsvg.surfaceDestroy(surface);
        rsvg.unref(handle);
        return ok;
    }
}
