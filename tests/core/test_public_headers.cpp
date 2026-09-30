// Esia - every public header compiles the way a Windows host includes it: after <windows.h>, without NOMINMAX or
// WIN32_LEAN_AND_MEAN (Esia's own targets define both; hosts often do not). <windows.h> then defines min / max and
// CreateWindow, DrawText, LoadImage ... as function-like macros, which break `std::min(a, b)`, `min(x)` member
// initializers and member functions of those names. Elsewhere this is a plain all-headers compile.
#if defined(_WIN32)
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "esia/base/config.hpp"
#include "esia/base/hash.hpp"
#include "esia/base/math.hpp"
#include "esia/base/utf8.hpp"
#include "esia/core/context.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/core/fx.hpp"
#include "esia/core/input.hpp"
#include "esia/core/layout.hpp"
#include "esia/core/state.hpp"
#include "esia/core/texture.hpp"
#include "esia/render/frame_plan.hpp"
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/render/shader_library.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/null_device.hpp"
#include "esia/rhi/rhi.hpp"
#include "esia/text/freetype.hpp"
#include "esia/text/glyph_atlas.hpp"
#include "esia/text/glyph_raster.hpp"
#include "esia/text/system_fonts.hpp"
#include "esia/text/text.hpp"
#include "esia_test.hpp"

// the inline code the macros used to break
ESIA_TEST(PublicHeaders, InlineCodeAfterWindowsH)
{
    const esia::Rect a(0, 0, 10, 10), b(esia::Vec2(5, 5), esia::Vec2(20, 20));
    const esia::Rect i = a.Intersect(b);
    ESIA_CHECK(i.min.x == 5 && i.max.x == 10);
    const esia::render::PxRect p{0, 0, 8, 8}, q{4, 4, 16, 16};
    ESIA_CHECK(p.Union(q).x1 == 16 && p.Intersect(q).x0 == 4);
}

// A member named like a <windows.h> macro (FindWindow, CreateWindow ...) is renamed in this file only
// (FindWindowW) and no longer links against the library, which was compiled without the macro: call them.
ESIA_TEST(PublicHeaders, ContextMembersLinkAfterWindowsH)
{
    esia::Context ctx;
    ctx.NewFrame({{800, 600}, {1, 1}, 1.0});
    ctx.Begin("W");
    ctx.End();
    ctx.EndFrame();
    ESIA_CHECK(ctx.FindWindowByName("W") != nullptr);
}
