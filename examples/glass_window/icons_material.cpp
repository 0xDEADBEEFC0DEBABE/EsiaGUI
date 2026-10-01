// glass_window - esia/ui/icons.hpp (Segoe Fluent code points) -> Material Icons Outlined code points (icons_material.hpp),
// the Material icon of the same meaning; Windows' outline and filled pairs (Star / StarFill, Heart / HeartFill) map to
// Material's outline and filled ones.
#include "icons_material.hpp"
#include <algorithm>
#include <iterator>

namespace glass::material_icons
{
    namespace
    {
        struct Pair
        {
            char32_t segoe, material;
        };
        constexpr Pair kIcons[] = {   // sorted by the Segoe code point
            {0xE700, 0xE5D2},   // Menu: menu
            {0xE701, 0xE63E},   // Wifi: wifi
            {0xE702, 0xE1A7},   // Bluetooth: bluetooth
            {0xE703, 0xE8BE},   // Connect: settings_ethernet
            {0xE705, 0xE0DA},   // Vpn: vpn_key
            {0xE706, 0xE430},   // Brightness: wb_sunny
            {0xE707, 0xE55F},   // MapPin: place
            {0xE708, 0xEA46},   // Moon: nights_stay
            {0xE709, 0xE539},   // Airplane: flight
            {0xE70D, 0xE5CF},   // ChevronDown: expand_more
            {0xE70E, 0xE5CE},   // ChevronUp: expand_less
            {0xE70F, 0xE3C9},   // Edit: edit
            {0xE710, 0xE145},   // Add: add
            {0xE711, 0xE5CD},   // Close: close
            {0xE712, 0xE5D3},   // More: more_horiz
            {0xE713, 0xE8B8},   // Settings: settings
            {0xE714, 0xE04B},   // Video: videocam
            {0xE715, 0xE158},   // Mail: mail
            {0xE716, 0xE7FB},   // People: people
            {0xE717, 0xE0CD},   // Phone: phone
            {0xE718, 0xF10D},   // Pin: push_pin
            {0xE719, 0xF1CC},   // Shop: shopping_bag
            {0xE71A, 0xE047},   // Stop: stop
            {0xE71B, 0xE157},   // Link: link
            {0xE71C, 0xE152},   // Filter: filter_list
            {0xE71D, 0xE5C3},   // Apps: apps
            {0xE71E, 0xE8FF},   // ZoomIn: zoom_in
            {0xE71F, 0xE900},   // ZoomOut: zoom_out
            {0xE720, 0xE029},   // Microphone: mic
            {0xE721, 0xE8B6},   // Search: search
            {0xE722, 0xE412},   // Camera: photo_camera
            {0xE723, 0xE226},   // Attach: attach_file
            {0xE724, 0xE163},   // Send: send
            {0xE72A, 0xE5C8},   // Forward: arrow_forward
            {0xE72B, 0xE5C4},   // Back: arrow_back
            {0xE72C, 0xE5D5},   // Refresh: refresh
            {0xE72D, 0xE80D},   // Share: share
            {0xE72E, 0xE897},   // Lock: lock
            {0xE734, 0xE83A},   // Star: star_border
            {0xE735, 0xE838},   // StarFill: star
            {0xE738, 0xE15B},   // Remove: remove
            {0xE73E, 0xE5CA},   // Checkmark: check
            {0xE740, 0xE5D0},   // FullScreen: fullscreen
            {0xE74D, 0xE872},   // Delete: delete
            {0xE74E, 0xE161},   // Save: save
            {0xE74F, 0xE04F},   // Mute: volume_off
            {0xE753, 0xE2BD},   // Cloud: cloud
            {0xE767, 0xE050},   // Volume: volume_up
            {0xE768, 0xE037},   // Play: play_arrow
            {0xE769, 0xE034},   // Pause: pause
            {0xE76B, 0xE5CB},   // ChevronLeft: chevron_left
            {0xE76C, 0xE5CC},   // ChevronRight: chevron_right
            {0xE771, 0xE3AE},   // Brush: brush
            {0xE774, 0xE894},   // Globe: language
            {0xE77B, 0xE7FD},   // Contact: person
            {0xE783, 0xE000},   // Error: error
            {0xE785, 0xE898},   // Unlock: lock_open
            {0xE787, 0xE935},   // Calendar: calendar_today
            {0xE790, 0xE40A},   // Palette: palette
            {0xE7BA, 0xE002},   // Warning: warning
            {0xE7C1, 0xE153},   // Flag: flag
            {0xE7E8, 0xE8AC},   // Power: power_settings_new
            {0xE7FC, 0xEA28},   // Game: sports_esports
            {0xE80F, 0xE88A},   // Home: home
            {0xE81C, 0xE889},   // History: history
            {0xE81D, 0xE55C},   // Location: my_location
            {0xE823, 0xE8B5},   // Recent: schedule
            {0xE890, 0xE8F4},   // View: visibility
            {0xE892, 0xE045},   // Previous: skip_previous
            {0xE893, 0xE044},   // Next: skip_next
            {0xE895, 0xE627},   // Sync: sync
            {0xE896, 0xF090},   // Download: download
            {0xE897, 0xE887},   // Help: help
            {0xE898, 0xF09B},   // Upload: upload
            {0xE8A5, 0xE873},   // Document: description
            {0xE8B7, 0xE2C7},   // Folder: folder
            {0xE8C8, 0xE14D},   // Copy: content_copy
            {0xE8D6, 0xE405},   // Music: music_note
            {0xE909, 0xE80B},   // World: public
            {0xE91B, 0xE3F4},   // Photo: image
            {0xE943, 0xE86F},   // Code: code
            {0xE945, 0xE3E7},   // Lightning: flash_on
            {0xE946, 0xE88E},   // Info: info
            {0xE962, 0xE323},   // Mouse: mouse
            {0xE9D9, 0xE6E1},   // Diagnostic: show_chart
            {0xE9E9, 0xE01D},   // Equalizer: equalizer
            {0xEA18, 0xE32A},   // Shield: security
            {0xEA80, 0xE0F0},   // Lightbulb: lightbulb
            {0xEA8F, 0xE7F4},   // Bell: notifications
            {0xEB51, 0xE87E},   // Heart: favorite_border
            {0xEB52, 0xE87D},   // HeartFill: favorite
            {0xEBE8, 0xE868},   // Bug: bug_report
            {0xED1A, 0xE8F5},   // Hide: visibility_off
        };
    }

    char32_t Map(char32_t c)
    {
        const auto it = std::lower_bound(std::begin(kIcons), std::end(kIcons), c, [](const Pair& p, char32_t v) { return p.segoe < v; });
        return it != std::end(kIcons) && it->segoe == c ? it->material : 0;
    }
}
