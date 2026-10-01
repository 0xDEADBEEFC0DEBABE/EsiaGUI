// Esia UI - icon code points (WGT's set).
//
// Icons are glyphs of the platform's icon font: "Segoe Fluent Icons" on Windows 11, "Segoe MDL2 Assets" on
// Windows 10 (the Ui finds either; no bundled assets). Any code point of those fonts can be used; the constants below
// are the set the widgets and the showcase use. Where neither font is installed, icons are not drawn.
#pragma once
#include <cstdint>

namespace esia::ui
{
    using Icon = std::uint32_t;   // 0 = no icon
}

namespace esia::ui::icons
{
    constexpr Icon Menu            = 0xE700;
    constexpr Icon Wifi            = 0xE701;
    constexpr Icon Bluetooth       = 0xE702;
    constexpr Icon Connect         = 0xE703;
    constexpr Icon Vpn             = 0xE705;
    constexpr Icon Brightness      = 0xE706;
    constexpr Icon MapPin          = 0xE707;
    constexpr Icon Moon            = 0xE708;
    constexpr Icon Airplane        = 0xE709;
    constexpr Icon ChevronDown     = 0xE70D;
    constexpr Icon ChevronUp       = 0xE70E;
    constexpr Icon Edit            = 0xE70F;
    constexpr Icon Add             = 0xE710;
    constexpr Icon Close           = 0xE711;
    constexpr Icon More            = 0xE712;
    constexpr Icon Settings        = 0xE713;
    constexpr Icon Video           = 0xE714;
    constexpr Icon Mail            = 0xE715;
    constexpr Icon People          = 0xE716;
    constexpr Icon Phone           = 0xE717;
    constexpr Icon Pin             = 0xE718;
    constexpr Icon Shop            = 0xE719;
    constexpr Icon Stop            = 0xE71A;
    constexpr Icon Link            = 0xE71B;
    constexpr Icon Filter          = 0xE71C;
    constexpr Icon Apps            = 0xE71D;
    constexpr Icon ZoomIn          = 0xE71E;
    constexpr Icon ZoomOut         = 0xE71F;
    constexpr Icon Microphone      = 0xE720;
    constexpr Icon Search          = 0xE721;
    constexpr Icon Camera          = 0xE722;
    constexpr Icon Attach          = 0xE723;
    constexpr Icon Send            = 0xE724;
    constexpr Icon Forward         = 0xE72A;
    constexpr Icon Back            = 0xE72B;
    constexpr Icon Refresh         = 0xE72C;
    constexpr Icon Share           = 0xE72D;
    constexpr Icon Lock            = 0xE72E;
    constexpr Icon Star            = 0xE734;
    constexpr Icon StarFill        = 0xE735;
    constexpr Icon Remove          = 0xE738;
    constexpr Icon Checkmark       = 0xE73E;
    constexpr Icon FullScreen      = 0xE740;
    constexpr Icon Delete          = 0xE74D;
    constexpr Icon Save            = 0xE74E;
    constexpr Icon Mute            = 0xE74F;
    constexpr Icon Cloud           = 0xE753;
    constexpr Icon ChevronLeft     = 0xE76B;
    constexpr Icon ChevronRight    = 0xE76C;
    constexpr Icon Volume          = 0xE767;
    constexpr Icon Play            = 0xE768;
    constexpr Icon Pause           = 0xE769;
    constexpr Icon Previous        = 0xE892;
    constexpr Icon Next            = 0xE893;
    constexpr Icon Brush           = 0xE771;
    constexpr Icon Globe           = 0xE774;
    constexpr Icon Contact         = 0xE77B;
    constexpr Icon Error           = 0xE783;
    constexpr Icon Unlock          = 0xE785;
    constexpr Icon Calendar        = 0xE787;
    constexpr Icon Palette         = 0xE790;
    constexpr Icon Warning         = 0xE7BA;
    constexpr Icon Flag            = 0xE7C1;
    constexpr Icon Power           = 0xE7E8;
    constexpr Icon Game            = 0xE7FC;
    constexpr Icon Home            = 0xE80F;
    constexpr Icon History         = 0xE81C;
    constexpr Icon Location        = 0xE81D;
    constexpr Icon Recent          = 0xE823;
    constexpr Icon View            = 0xE890;
    constexpr Icon Sync            = 0xE895;
    constexpr Icon Download        = 0xE896;
    constexpr Icon Help            = 0xE897;
    constexpr Icon Upload          = 0xE898;
    constexpr Icon Document        = 0xE8A5;
    constexpr Icon Folder          = 0xE8B7;
    constexpr Icon Copy            = 0xE8C8;
    constexpr Icon Music           = 0xE8D6;
    constexpr Icon World           = 0xE909;
    constexpr Icon Photo           = 0xE91B;
    constexpr Icon Code            = 0xE943;
    constexpr Icon Lightning       = 0xE945;
    constexpr Icon Info            = 0xE946;
    constexpr Icon Mouse           = 0xE962;
    constexpr Icon Diagnostic      = 0xE9D9;
    constexpr Icon Equalizer       = 0xE9E9;
    constexpr Icon Shield          = 0xEA18;
    constexpr Icon Lightbulb       = 0xEA80;
    constexpr Icon Bell            = 0xEA8F;
    constexpr Icon Heart           = 0xEB51;
    constexpr Icon HeartFill       = 0xEB52;
    constexpr Icon Bug             = 0xEBE8;
    constexpr Icon Hide            = 0xED1A;
}
