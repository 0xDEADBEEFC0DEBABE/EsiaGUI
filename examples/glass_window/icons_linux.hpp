// glass_window - esia::ui's icons from the Linux desktop's icon theme (icons_linux.cpp), for symbol_text.hpp.
#pragma once
#include <cstdint>
#include <vector>

namespace glass::linux_icons
{
    // The symbolic icon for code point `c` of esia/ui/icons.hpp as a px x px coverage mask; false when the theme has
    // none for it or librsvg is not there.
    bool Rasterize(char32_t c, int px, std::vector<std::uint8_t>& coverage, int& w, int& h);
}
