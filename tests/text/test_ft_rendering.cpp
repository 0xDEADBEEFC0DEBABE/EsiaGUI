// TextRendering in the FreeType text system: the parser and presets, hinting (light, the fonts' own with either
// TrueType interpreter, the autohinter), FreeType's rasterizer beside the analytic one, stem darkening, hinted advances,
// sub-pixel (LCD) glyphs from both rasterizers in either stripe order, when a new rendering takes over,
// ESIA_TEXT_RENDERING - and golden images: the analytic sub-pixel text (everywhere) and the crisp preset (FreeType's
// own pixels, so only against the FreeType the golden was made with: the bundled one).
#include "esia/text/freetype.hpp"
#include "esia/text/glyph_atlas.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>
#include "font_test_util.hpp"
#include "ft_test_util.hpp"

using namespace esia;
using namespace esia::texttest;

namespace
{
    struct Fixture
    {
        TextureRegistry textures;
        PageStore pages;   // what the registry received, over all draws (TakeChanges hands each change out once)
        std::unique_ptr<text::TextSystem> ts;
        text::FontId droid = 0;

        explicit Fixture(const text::TextRendering& rendering = {}, float pixelsPerUnit = 1.0f)
        {
            text::FreeTypeDesc desc;
            desc.rendering = rendering;
            ts = text::CreateFreeTypeTextSystem(textures, desc);
            ESIA_CHECK(ts != nullptr);
            if (!ts)
                return;
            ts->SetRendering(rendering);   // over ESIA_TEXT_RENDERING, should the environment have it
            droid = ts->AddFontFile((kFonts + "/DroidSans.ttf").c_str());
            ESIA_CHECK(droid != 0);
            ts->NewFrame({pixelsPerUnit});
        }
    };

    // A text drawn in black at `pos` as 8-bit coverage, `w` x `h` pixels (Composite over white, inverted).
    struct Coverage
    {
        int w = 0, h = 0;
        std::vector<std::uint8_t> v;
        std::uint8_t At(int x, int y) const { return v[(std::size_t)y * (std::size_t)w + (std::size_t)x]; }
        long Sum() const
        {
            long s = 0;
            for (std::uint8_t c : v)
                s += c;
            return s;
        }
        // pixels neither empty nor full: the blur of edges between pixels
        int Partial(int lo = 16, int hi = 239) const
        {
            int n = 0;
            for (std::uint8_t c : v)
                n += c > lo && c < hi;
            return n;
        }
    };

    Coverage Draw(Fixture& f, text::FontId font, float size, Vec2 pos, std::string_view s, int w = 160, int h = 48)
    {
        DrawList dl = NewList();
        f.ts->Draw(dl, {font, size}, pos, Color::Black(), s);
        f.pages.Collect(f.textures);
        const testkit::Image img = Composite(dl, f.pages, w, h);
        Coverage c;
        c.w = w;
        c.h = h;
        c.v.resize((std::size_t)w * (std::size_t)h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                c.v[(std::size_t)y * (std::size_t)w + (std::size_t)x] = (std::uint8_t)(255 - img.At(x, y)[0]);
        return c;
    }

    // Of a glyph drawn alone, the share of its fullest row's ink in that row's strongest pixel: 1 for a vertical stem
    // between two pixel edges, about a half for one straddling two pixels.
    float StemSharpness(const Coverage& c)
    {
        int row = 0, rowInk = -1;
        for (int y = 0; y < c.h; ++y)
        {
            int ink = 0;
            for (int x = 0; x < c.w; ++x)
                ink += c.At(x, y);
            if (ink > rowInk)
            {
                row = y;
                rowInk = ink;
            }
        }
        int strongest = 0;
        for (int x = 0; x < c.w; ++x)
            strongest = std::max(strongest, (int)c.At(x, row));
        return rowInk > 0 ? (float)strongest / (float)rowInk : 0.0f;
    }

    void SetEnv(const char* name, const char* value)
    {
#if defined(_WIN32)
        _putenv_s(name, value ? value : "");
#else
        if (value)
            setenv(name, value, 1);
        else
            unsetenv(name);
#endif
    }

    text::TextRendering With(text::Hinting hinting, text::GlyphRasterizer rasterizer = text::GlyphRasterizer::Analytic)
    {
        text::TextRendering r;
        r.hinting = hinting;
        r.rasterizer = rasterizer;
        return r;
    }
}

ESIA_TEST(TextRendering, ParsesPresetsAndSettings)
{
    text::TextRendering r;
    ESIA_CHECK(text::ParseTextRendering("crisp", r) && r == text::TextRendering::Crisp());
    ESIA_CHECK(text::ParseTextRendering("design", r) && r == text::TextRendering::Design());
    ESIA_CHECK(text::ParseTextRendering("native", r) && r == text::NativeTextRendering());
    ESIA_CHECK(text::ParseTextRendering(" light , stem-darkening ", r) && r.hinting == text::Hinting::Light && r.stemDarkening);
    // later words override earlier ones
    ESIA_CHECK(text::ParseTextRendering("crisp,interpreter=40,hinted-advances=0", r));
    ESIA_CHECK(r.hinting == text::Hinting::Full && r.interpreter == text::TrueTypeInterpreter::V40 && !r.hintedAdvances &&
               r.rasterizer == text::GlyphRasterizer::FreeType);
    ESIA_CHECK(text::ParseTextRendering("hinting=auto,rasterizer=analytic", r) && r.hinting == text::Hinting::Auto &&
               r.rasterizer == text::GlyphRasterizer::Analytic);
    ESIA_CHECK(text::ParseTextRendering("cleartype", r) && r == text::TextRendering::ClearType() && r.subpixel == text::SubpixelLayout::Rgb);
    ESIA_CHECK(text::ParseTextRendering("subpixel=bgr,lcd-filter=light", r) && r.subpixel == text::SubpixelLayout::Bgr &&
               r.lcdFilter == text::LcdFilter::Light && r.hinting == text::Hinting::Full);
    ESIA_CHECK(text::ParseTextRendering("subpixel=none,lcd-filter=none", r) && r.subpixel == text::SubpixelLayout::None &&
               r.lcdFilter == text::LcdFilter::None);
    ESIA_CHECK(!text::ParseTextRendering("subpixel=vrgb", r) && !text::ParseTextRendering("lcd-filter=legacy", r));
    ESIA_CHECK(text::ParseTextRendering("design,smooth-motion", r) && r.smoothMotion && r.hinting == text::Hinting::None);
    ESIA_CHECK(text::ParseTextRendering("crisp,smooth-motion=0,distance-field=40", r) && !r.smoothMotion && r.distanceFieldFrom == 40.0f);
    ESIA_CHECK(!text::ParseTextRendering("distance-field=big", r));
    // a word it does not know: false, `out` unchanged, the word named
    const text::TextRendering before = r;
    std::string error;
    ESIA_CHECK(!text::ParseTextRendering("light,hinting=sharp", r, &error) && r == before);
    ESIA_CHECK(error.find("hinting=sharp") != std::string::npos);
    ESIA_CHECK(!text::ParseTextRendering("bold", r) && !text::ParseTextRendering("stem-darkening=maybe", r));
    ESIA_CHECK(text::ParseTextRendering("", r) && r == before);
}

// Hinting that fits vertical stems puts the stem of an "l" into one pixel column wherever the text starts: the classic
// interpreter between two pixel edges, the autohinter (smooth: its standard width, 1.02 px here) leaning into one
// pixel with a sliver beside it. Unhinted, or with v40 (vertical only), it straddles two pixels at some of the
// quarter-pixel phases.
ESIA_TEST(FreeTypeRendering, FullHintingPutsStemsBetweenPixelEdges)
{
    const auto sharpness = [](const text::TextRendering& r, float& weakest, int& strongest) {
        Fixture f(r);
        weakest = 1.0f;
        strongest = 0;
        for (int i = 0; i < 4; ++i)
        {
            const Coverage c = Draw(f, f.droid, 11.0f, Vec2(10.0f + 0.25f * (float)i, 10.0f), "l");
            weakest = std::min(weakest, StemSharpness(c));
            for (std::uint8_t v : c.v)
                strongest = std::max(strongest, (int)v);
        }
    };
    float weakest = 0.0f;
    int strongest = 0;
    sharpness(text::TextRendering::Crisp(), weakest, strongest);
    ESIA_CHECK(weakest > 0.95f && strongest >= 248);
    sharpness(With(text::Hinting::Auto), weakest, strongest);
    ESIA_CHECK(weakest > 0.85f);
    sharpness(With(text::Hinting::Auto, text::GlyphRasterizer::FreeType), weakest, strongest);
    ESIA_CHECK(weakest > 0.85f);
    sharpness({}, weakest, strongest);
    ESIA_CHECK(weakest < 0.75f);
    sharpness(With(text::Hinting::Full, text::GlyphRasterizer::FreeType), weakest, strongest);   // v40
    ESIA_CHECK(weakest < 0.75f);
}

// Hinting fits horizontals too: the bars of "=" and "-" land on rows, so far fewer pixels are partly covered (light:
// about half as many with DroidSans' own hints at v40, which place the bars but keep their thickness; crisp: a third).
ESIA_TEST(FreeTypeRendering, HintingSharpensHorizontals)
{
    const auto partial = [](const text::TextRendering& r) {
        Fixture f(r);
        int n = 0;
        for (float size : {11.0f, 12.0f, 13.0f, 14.0f, 15.0f})
            n += Draw(f, f.droid, size, Vec2(10, 10), "= - =").Partial();
        return n;
    };
    const int design = partial({});
    ESIA_CHECK(partial(text::TextRendering::Light()) * 3 < design * 2);
    ESIA_CHECK(partial(text::TextRendering::Crisp()) * 2 < design);
    ESIA_CHECK(partial(With(text::Hinting::Auto)) * 3 < design * 2);
}

// Light hinting keeps the design's widths; hinted advances (full hinting) put every pen on a whole pixel, at the
// density of the frame: the layout then follows the density.
ESIA_TEST(FreeTypeRendering, HintedAdvancesAreWholePixels)
{
    const std::string_view s = "Waffle Toolkit";
    Fixture design;
    const float width = design.ts->Measure({design.droid, 13.0f}, s).size.x;
    Fixture light(text::TextRendering::Light());
    ESIA_CHECK_NEAR(light.ts->Measure({light.droid, 13.0f}, s).size.x, width, 1e-3f);
    text::TextRendering lightAdvances = text::TextRendering::Light();
    lightAdvances.hintedAdvances = true;   // light hinting does not change advances: still the design's
    Fixture la(lightAdvances);
    ESIA_CHECK_NEAR(la.ts->Measure({la.droid, 13.0f}, s).size.x, width, 1e-3f);

    for (float ppu : {1.0f, 1.5f, 2.0f})
    {
        Fixture crisp(text::TextRendering::Crisp(), ppu);
        const float w = crisp.ts->Measure({crisp.droid, 13.0f}, s).size.x;
        ESIA_CHECK(IsWhole(w * ppu));
        ESIA_CHECK(std::fabs(w - width) < 0.15f * width);   // the same text, a little wider or narrower
        // every glyph on a whole pixel, whatever the origin's fraction
        DrawList dl = NewList();
        crisp.ts->Draw(dl, {crisp.droid, 13.0f}, Vec2(10.3f, 5.6f), Color::Black(), s);
        for (const Quad& q : Quads(dl))
            ESIA_CHECK(IsWhole(q.r.min.x * ppu));
    }
    // one text system, another density: another layout
    Fixture crisp(text::TextRendering::Crisp());
    const float at1 = crisp.ts->Measure({crisp.droid, 13.0f}, s).size.x;
    crisp.ts->NewFrame({1.25f});
    const float at125 = crisp.ts->Measure({crisp.droid, 13.0f}, s).size.x;
    ESIA_CHECK(IsWhole(at125 * 1.25f) && at1 != at125);
}

// FreeType's rasterizer on the design's outline paints what the analytic one does: the same boxes, the same ink.
ESIA_TEST(FreeTypeRendering, FreeTypesRasterizerMatchesTheAnalyticOne)
{
    Fixture analytic;
    Fixture freetype(With(text::Hinting::None, text::GlyphRasterizer::FreeType));
    for (float x : {10.0f, 10.25f, 10.5f, 10.75f})
    {
        const Coverage a = Draw(analytic, analytic.droid, 24.0f, Vec2(x, 6), "Esia AVg");
        const Coverage b = Draw(freetype, freetype.droid, 24.0f, Vec2(x, 6), "Esia AVg");
        ESIA_CHECK(std::labs(a.Sum() - b.Sum()) < a.Sum() / 50);   // within 2 %
        int apart = 0;
        for (std::size_t i = 0; i < a.v.size(); ++i)
            apart += std::abs((int)a.v[i] - (int)b.v[i]) > 48;
        ESIA_CHECK(apart < 12);
    }
}

// The two TrueType interpreters fit DroidSans differently (the classic one moves stems sideways too); the
// autohinter differs from the font's own hints.
ESIA_TEST(FreeTypeRendering, InterpretersAndTheAutohinterDiffer)
{
    const auto image = [](const text::TextRendering& r) { Fixture f(r); return Draw(f, f.droid, 11.0f, Vec2(10, 10), "Wavy lines mmm"); };
    text::TextRendering v35 = text::TextRendering::Crisp(), v40 = v35;
    v40.interpreter = text::TrueTypeInterpreter::V40;
    v35.hintedAdvances = v40.hintedAdvances = false;   // the same layout: only the shapes differ
    const Coverage a = image(v35), b = image(v40), c = image(With(text::Hinting::Auto, text::GlyphRasterizer::FreeType));
    const auto differ = [](const Coverage& p, const Coverage& q) {
        int n = 0;
        for (std::size_t i = 0; i < p.v.size(); ++i)
            n += p.v[i] != q.v[i];
        return n;
    };
    ESIA_CHECK(differ(a, b) > 20);
    ESIA_CHECK(differ(b, c) > 20);
}

// FreeType's stem darkening thickens a CFF font's thin stems at small sizes.
ESIA_TEST(FreeTypeRendering, StemDarkeningThickensCffStems)
{
    const auto ink = [](bool darkening) {
        text::TextRendering r = text::TextRendering::Light();
        r.rasterizer = text::GlyphRasterizer::FreeType;
        r.stemDarkening = darkening;
        Fixture f(r);
        const text::FontId sc = f.ts->AddFontFile((kFonts + "/NotoSansSC-Subset.otf").c_str());
        ESIA_CHECK(sc != 0);
        return Draw(f, sc, 12.0f, Vec2(10, 10), "\xE4\xB8\xAD\xE5\xAD\x97\xE4\xBD\x93\xE5\x9C\xA8").Sum();   // 中字体在
    };
    ESIA_CHECK(ink(true) > ink(false) + ink(false) / 50);
}

// A new rendering takes over at the next NewFrame: until then the glyphs and layouts are the old ones.
ESIA_TEST(FreeTypeRendering, ANewRenderingTakesOverAtTheNextFrame)
{
    Fixture f;
    const Coverage before = Draw(f, f.droid, 11.0f, Vec2(10.25f, 10), "lime");
    const float width = f.ts->Measure({f.droid, 11.0f}, "lime").size.x;
    f.ts->SetRendering(text::TextRendering::Crisp());
    ESIA_CHECK(f.ts->Rendering() == text::TextRendering::Crisp());
    ESIA_CHECK(Draw(f, f.droid, 11.0f, Vec2(10.25f, 10), "lime").v == before.v);
    ESIA_CHECK(f.ts->Measure({f.droid, 11.0f}, "lime").size.x == width);
    f.ts->NewFrame({1.0f});
    ESIA_CHECK(Draw(f, f.droid, 11.0f, Vec2(10.25f, 10), "lime").v != before.v);
    ESIA_CHECK(IsWhole(f.ts->Measure({f.droid, 11.0f}, "lime").size.x));
    // and back
    f.ts->SetRendering({});
    f.ts->NewFrame({1.0f});
    ESIA_CHECK(Draw(f, f.droid, 11.0f, Vec2(10.25f, 10), "lime").v == before.v);
}

// ESIA_TEXT_RENDERING sets a new text system's rendering; a word it does not know leaves the description's.
ESIA_TEST(FreeTypeRendering, TheEnvironmentVariableSetsIt)
{
    TextureRegistry textures;
    SetEnv("ESIA_TEXT_RENDERING", "crisp,interpreter=40");
    std::unique_ptr<text::TextSystem> a = text::CreateFreeTypeTextSystem(textures);
    SetEnv("ESIA_TEXT_RENDERING", "hinting=sharp");
    text::FreeTypeDesc light;
    light.rendering = text::TextRendering::Light();
    std::unique_ptr<text::TextSystem> b = text::CreateFreeTypeTextSystem(textures, light);
    SetEnv("ESIA_TEXT_RENDERING", nullptr);
    std::unique_ptr<text::TextSystem> c = text::CreateFreeTypeTextSystem(textures, light);
    ESIA_CHECK(a && b && c);
    if (!a || !b || !c)
        return;
    text::TextRendering want = text::TextRendering::Crisp();
    want.interpreter = text::TrueTypeInterpreter::V40;
    ESIA_CHECK(a->Rendering() == want);
    ESIA_CHECK(b->Rendering() == text::TextRendering::Light());
    ESIA_CHECK(c->Rendering() == text::TextRendering::Light());
}

// Sub-pixel rendering puts every glyph into sub-pixel pages - with either rasterizer - whose stripes differ at the
// edges; a B-G-R panel gets the same pages with red and blue swapped.
ESIA_TEST(FreeTypeRendering, SubpixelGlyphsGoToLcdPages)
{
    for (text::GlyphRasterizer raster : {text::GlyphRasterizer::Analytic, text::GlyphRasterizer::FreeType})
    {
        text::TextRendering rgb = text::TextRendering::ClearType(), bgr = rgb;
        rgb.rasterizer = bgr.rasterizer = raster;
        bgr.subpixel = text::SubpixelLayout::Bgr;
        Fixture a(rgb), b(bgr);
        DrawList da = NewList(), db = NewList();
        a.ts->Draw(da, {a.droid, 13.0f}, Vec2(10.3f, 5.0f), Color::Black(), "Waffle AVg");
        b.ts->Draw(db, {b.droid, 13.0f}, Vec2(10.3f, 5.0f), Color::Black(), "Waffle AVg");
        a.pages.Collect(a.textures);
        b.pages.Collect(b.textures);
        const std::vector<Quad> qa = Quads(da), qb = Quads(db);
        ESIA_CHECK(qa.size() >= 8 && qb.size() == qa.size());   // the inked glyphs ("ff" may be one)
        for (const Quad& q : qa)
            ESIA_CHECK(a.pages.pages.at(q.texture).info.Lcd() && q.color == Color::Black().ToRgba8());
        ESIA_CHECK(a.pages.pages.size() == 1 && b.pages.pages.size() == 1);
        if (a.pages.pages.size() != 1 || b.pages.pages.size() != 1)
            continue;
        const std::vector<std::uint8_t>& pa = a.pages.pages.begin()->second.pixels;
        const std::vector<std::uint8_t>& pb = b.pages.pages.begin()->second.pixels;
        int fringes = 0, swapped = 0;
        for (std::size_t i = 0; i + 3 < pa.size() && i + 3 < pb.size(); i += 4)
        {
            fringes += pa[i] != pa[i + 2];
            swapped += pa[i] == pb[i + 2] && pa[i + 1] == pb[i + 1] && pa[i + 2] == pb[i] && pa[i + 3] == pb[i + 3];
        }
        ESIA_CHECK(fringes > 100);
        ESIA_CHECK(swapped == (int)(pa.size() / 4));
    }
}

// Switching between grayscale and sub-pixel rendering changes the pages' kind at the next frame; the tiles' generation
// moves with it (the widget layer adds its coverage tiles again).
ESIA_TEST(FreeTypeRendering, SwitchingToSubpixelChangesThePages)
{
    Fixture f;
    const std::uint64_t generation = f.ts->TileGeneration();
    DrawList dl = NewList();
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(10, 5), Color::Black(), "gray");
    f.pages.Collect(f.textures);
    ESIA_CHECK(f.pages.pages.size() == 1 && f.pages.pages.begin()->second.info.format == TextureFormat::Alpha8);
    f.ts->SetRendering(text::TextRendering::ClearType());
    f.ts->NewFrame({1.0f});
    ESIA_CHECK(f.ts->TileGeneration() != generation);
    dl = NewList();
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(10, 5), Color::Black(), "sub-pixel");
    f.pages.Collect(f.textures);
    ESIA_CHECK(f.pages.pages.size() == 1 && f.pages.pages.begin()->second.info.Lcd());   // the gray page is gone
    // a coverage tile goes into the sub-pixel page too, as gray
    text::GlyphBitmap tile;
    tile.width = tile.height = 3;
    tile.pixels.assign(9, 128);
    const text::GlyphSlot* s = f.ts->AddTile(0x8000000000000001ull, tile);
    ESIA_CHECK(s && s->page == Quads(dl)[0].texture);
}

// The analytic sub-pixel text, composed per channel and through its grayscale fallback: the fallback is the grayscale
// text to the last bit.
ESIA_TEST(FreeTypeRendering, RendersTheSubpixelGoldenImage)
{
    text::TextRendering lcd;
    lcd.subpixel = text::SubpixelLayout::Rgb;
    Fixture f(lcd), gray;
    const text::FontId karla = f.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());
    const text::FontId karlaGray = gray.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());
    const auto draw = [](Fixture& x, text::FontId k, DrawList& dl) {
        const Color ink(0.08f, 0.08f, 0.10f, 1.0f);
        x.ts->Draw(dl, {x.droid, 11.0f}, Vec2(8, 6), ink, "Sub-pixel: three stripes per pixel, FreeType's default filter.");
        x.ts->Draw(dl, {x.droid, 13.0f}, Vec2(8.25f, 24), ink, "The quick brown fox jumps over the lazy dog 0123456789");
        x.ts->Draw(dl, {k, 12.0f}, Vec2(8.5f, 44), Color(0.0f, 0.35f, 0.8f, 1.0f), "Karla 12: Waffle Toolkit, liquid glass, AV To");
        x.ts->Draw(dl, {x.droid, 16.0f}, Vec2(8.75f, 62), ink, "Esia \xC2\xB7 AV To Wa fi \xC3\x80\xC3\x89\xC3\x8E\xC3\xB5\xC3\xBC");
        x.ts->Draw(dl, {x.droid, 9.0f}, Vec2(8, 86), Color(0.7f, 0.1f, 0.1f, 1.0f), "Nine pixels per em, red ink: the stripes in color.");
    };
    DrawList dl = NewList(), dg = NewList();
    draw(f, karla, dl);
    draw(gray, karlaGray, dg);
    f.pages.Collect(f.textures);
    gray.pages.Collect(gray.textures);
    CheckGolden(Composite(dl, f.pages, 480, 104), "ft_lcd");
    const testkit::Image fallback = Composite(dl, f.pages, 480, 104, true), grayscale = Composite(dg, gray.pages, 480, 104);
    int worst = 0;
    for (int y = 0; y < 104; ++y)
        for (int x = 0; x < 480; ++x)
            for (int c = 0; c < 3; ++c)
                worst = std::max(worst, std::abs((int)fallback.At(x, y)[c] - (int)grayscale.At(x, y)[c]));
    ESIA_CHECK(worst <= 2);
}

// Distance-field text: every glyph an FX glyph shape on a distance-field page, made once for every size - the outer
// layers of all glyphs first, then the faces; the text's box spans the paints; color glyphs as their bitmaps.
ESIA_TEST(FreeTypeRendering, DrawShapesMakesGlyphShapes)
{
    Fixture f;
    fx::Instance m;
    std::memset(&m, 0, sizeof(m));
    m.fill0[0] = m.fill0[1] = m.fill0[2] = m.fill0[3] = 1.0f;
    m.misc[0] = 1.0f;
    m.stroke[3] = 1.0f;
    m.strokeParams[0] = 2.0f;
    m.strokeParams[1] = 1.0f;   // outside: an outer layer
    m.flags[0] = fx::kFill | fx::kStroke;
    DrawList dl = NewList();
    const auto start = std::chrono::steady_clock::now();
    const Vec2 size = f.ts->DrawShapes(dl, {f.droid, 40.0f}, Vec2(10, 20), m, 0, 0.5f, "Waffle AVg");
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    ESIA_CHECK(size == f.ts->Measure({f.droid, 40.0f}, "Waffle AVg").size);
    const std::vector<fx::Instance>& in = dl.FxInstances();
    ESIA_CHECK(in.size() >= 16 && in.size() % 2 == 0);   // 8 or 9 inked glyphs ("ff" may be one), twice
    const std::size_t n = in.size() / 2;
    f.pages.Collect(f.textures);
    for (std::size_t i = 0; i < in.size(); ++i)
    {
        const fx::Instance& g = in[i];
        ESIA_CHECK(g.flags[2] == (std::uint32_t)fx::ShapeKind::Glyph && (g.flags[0] & fx::kGlyph));
        // the outer layer first (the stroke outside), then the face
        ESIA_CHECK(i < n ? g.flags[0] == (fx::kStroke | fx::kGlyph) : g.flags[0] == (fx::kFill | fx::kGlyph));
        ESIA_CHECK(g.rect[0] == 10.0f && g.rect[1] == 20.0f && g.rect[2] == 10.0f + size.x && g.rect[3] == 20.0f + size.y);
        ESIA_CHECK(g.shape2[2] > g.shape2[0] && g.shape2[3] > g.shape2[1] && g.uvRect[2] > g.uvRect[0]);
        ESIA_CHECK(g.radii[0] == 8.0f && g.radii[1] == 24.0f && std::fabs(g.shape[2] * 40.0f - 48.0f) < 1e-3f && g.shape[3] == 0.5f);
    }
    // the same glyph, the same field: both passes and every size share it
    ESIA_CHECK(in[0].uvRect[0] == in[n].uvRect[0] && in[0].uvRect[1] == in[n].uvRect[1]);
    std::uint32_t cmds = 0;
    for (const DrawCmd& c : dl.Commands())
        if (c.kind == DrawCmdKind::Fx)
        {
            ++cmds;
            ESIA_CHECK(f.pages.pages.count(c.texture) && f.pages.pages.at(c.texture).info.format == TextureFormat::RGBA8 &&
                       (f.pages.pages.at(c.texture).info.flags & TextureFlags_DistanceField) && !f.pages.pages.at(c.texture).info.Coverage());
        }
    ESIA_CHECK(cmds >= 1);
    DrawList again = NewList();
    f.ts->DrawShapes(again, {f.droid, 400.0f}, Vec2(0, 0), m, 0, 0.0f, "Waffle AVg");
    f.pages.Collect(f.textures);
    ESIA_CHECK(again.FxInstances().size() == in.size() && again.FxInstances()[0].uvRect[0] == in[0].uvRect[0]);
    std::printf("  %zu distance fields in %.2f ms (%.3f ms each)\n", n, ms, ms / (double)n);
}

// Legible glyph shapes: the halo under the faces, with the material's strength and radius (by default a sixth of the
// em at the size drawn); the faces without it.
ESIA_TEST(FreeTypeRendering, LegibleShapesPutTheirHaloUnderTheFaces)
{
    Fixture f;
    fx::Instance m;
    std::memset(&m, 0, sizeof(m));
    m.fill0[0] = m.fill0[1] = m.fill0[2] = m.fill0[3] = 1.0f;
    m.misc[0] = 1.0f;
    m.radii[2] = 0.5f;
    m.flags[0] = fx::kFill | fx::kLegible;
    for (const float radius : {0.0f, 5.0f})
    {
        m.radii[3] = radius;
        DrawList dl = NewList();
        f.ts->DrawShapes(dl, {f.droid, 36.0f}, Vec2(0, 0), m, 0, 0.0f, "Esia", 0.0f, 0, 2.0f);
        const std::vector<fx::Instance>& in = dl.FxInstances();
        ESIA_CHECK(in.size() == 8);
        for (std::size_t i = 0; i < in.size(); ++i)
        {
            const bool halo = i < in.size() / 2;
            ESIA_CHECK(in[i].flags[0] == ((halo ? fx::kLegible : fx::kFill) | fx::kGlyph));
            ESIA_CHECK(in[i].radii[2] == (halo ? 0.5f : 0.0f) && in[i].radii[3] == (halo ? (radius > 0.0f ? 5.0f : 12.0f) : 0.0f));
        }
    }
}

// Large text draws from the fields by itself (TextRendering::distanceFieldFrom); smaller text stays coverage quads.
ESIA_TEST(FreeTypeRendering, LargeTextDrawsFromFields)
{
    text::TextRendering r;
    r.distanceFieldFrom = 32.0f;
    Fixture f(r);
    DrawList small = NewList(), large = NewList();
    f.ts->Draw(small, {f.droid, 24.0f}, Vec2(0, 0), Color::White(), "Esia");
    f.ts->Draw(large, {f.droid, 32.0f}, Vec2(0, 0), Color(1, 0.5f, 0.25f, 0.5f), "Esia");
    ESIA_CHECK(small.FxInstances().empty() && !Quads(small).empty());
    ESIA_CHECK(large.FxInstances().size() == 4 && Quads(large).empty());
    const fx::Instance& g = large.FxInstances()[0];
    ESIA_CHECK(g.flags[0] == (fx::kFill | fx::kGlyph) && g.fill0[0] == 1.0f && g.fill0[1] == 0.5f && g.fill0[3] == 0.5f && g.misc[0] == 1.0f);
    // the density counts: 24 UI units at 1.5 pixels per unit are 36 pixels
    f.ts->NewFrame({1.5f});
    DrawList dense = NewList();
    f.ts->Draw(dense, {f.droid, 24.0f}, Vec2(0, 0), Color::White(), "Esia");
    ESIA_CHECK(dense.FxInstances().size() == 4);
}

namespace
{
    int AlphaOf(const Quad& q) { return (int)std::lround(Color::FromRgba8(q.color).a * 255.0f); }

    // the ink's vertical center of a text drawn in black (Composite over white)
    float InkCenterY(Fixture& f, const DrawList& dl)
    {
        f.pages.Collect(f.textures);
        const testkit::Image img = Composite(dl, f.pages, 160, 64);
        double sum = 0.0, weighted = 0.0;
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 160; ++x)
            {
                const double ink = 255.0 - img.At(x, y)[0];
                sum += ink;
                weighted += ink * ((double)y + 0.5);
            }
        return sum > 0.0 ? (float)(weighted / sum) : 0.0f;
    }
}

// Smooth motion: a text that moves is drawn from the design's outlines; when it stops it fades into the hinted rendering
// (8 frames without a clock): both sets of quads, their alphas adding up, then the hinted ones alone.
ESIA_TEST(FreeTypeRendering, MovingTextComesToRestHinted)
{
    const text::TextRendering crisp = text::TextRendering::Crisp();
    ESIA_CHECK(crisp.smoothMotion && text::TextRendering::Light().smoothMotion && !text::TextRendering::Design().smoothMotion);
    Fixture f(crisp);
    const auto frame = [&](Vec2 pos) {
        f.ts->NewFrame({1.0f});
        DrawList dl = NewList();
        f.ts->Draw(dl, {f.droid, 13.0f}, pos, Color::Black(), "Waffle");
        return Quads(dl);
    };
    const std::vector<Quad> rest = frame(Vec2(10.0f, 20.0f));   // first seen: at rest, hinted
    ESIA_CHECK(!rest.empty());
    for (const Quad& q : rest)
        ESIA_CHECK(AlphaOf(q) == 255);
    const std::vector<Quad> moving = frame(Vec2(10.5f, 20.25f));
    ESIA_CHECK(moving.size() == rest.size());
    int otherBitmaps = 0;
    for (std::size_t i = 0; i < moving.size(); ++i)
    {
        ESIA_CHECK(AlphaOf(moving[i]) == 255);
        otherBitmaps += !(moving[i].uv0 == rest[i].uv0);
    }
    ESIA_CHECK(otherBitmaps == (int)moving.size());   // the design's outlines: glyphs of their own
    const std::vector<Quad> settling = frame(Vec2(10.5f, 20.25f));
    ESIA_CHECK(settling.size() == 2 * rest.size());
    const std::size_t n = rest.size();
    for (std::size_t i = 0; i < n && settling.size() == 2 * n; ++i)
    {
        ESIA_CHECK(AlphaOf(settling[i]) > 128 && AlphaOf(settling[n + i]) < 128);   // mostly still the moving one
        ESIA_CHECK(std::abs(AlphaOf(settling[i]) + AlphaOf(settling[n + i]) - 255) <= 2);
    }
    std::vector<Quad> settled;
    for (int i = 0; i < 8; ++i)
        settled = frame(Vec2(10.5f, 20.25f));
    ESIA_CHECK(settled.size() == n);
    for (const Quad& q : settled)
        ESIA_CHECK(AlphaOf(q) == 255);
    // the clock, when given: 0.12 s
    f.ts->NewFrame({1.0f, 10.0});
    DrawList dl = NewList();
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(30, 20), Color::Black(), "Waffle");   // moved
    f.ts->NewFrame({1.0f, 10.06});
    dl = NewList();
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(30, 20), Color::Black(), "Waffle");
    ESIA_CHECK(Quads(dl).size() == 2 * n);   // half way
    f.ts->NewFrame({1.0f, 10.13});
    dl = NewList();
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(30, 20), Color::Black(), "Waffle");
    ESIA_CHECK(Quads(dl).size() == n);
}

// In motion the ink moves by a quarter pixel when the text does, down as across (at rest the baseline stays on a whole
// pixel); without smooth motion the text keeps to whole pixels down.
ESIA_TEST(FreeTypeRendering, MovingTextMovesByQuarterPixels)
{
    for (const bool smooth : {true, false})
    {
        text::TextRendering r = text::TextRendering::Light();
        r.smoothMotion = smooth;
        Fixture f(r);
        float last = 0.0f;
        for (int i = 0; i <= 5; ++i)
        {
            f.ts->NewFrame({1.0f});
            DrawList dl = NewList();
            f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(10.0f, 20.0f + 0.25f * (float)i), Color::Black(), "Waffle");
            const float y = InkCenterY(f, dl);
            if (i >= 2)
            {
                if (smooth)
                    ESIA_CHECK(std::fabs(y - last - 0.25f) < 0.06f);
                else
                    ESIA_CHECK(std::fabs(y - last) < 0.01f || std::fabs(y - last - 1.0f) < 0.05f);   // whole pixels
            }
            last = y;
        }
    }
}

ESIA_TEST(FreeTypeRendering, RendersTheCrispGoldenImage)
{
#if !defined(ESIA_TEST_BUNDLED_FREETYPE)
    std::printf("  skipped: FreeType's own pixels, compared only with the bundled FreeType the golden was made with\n");
#else
    Fixture f(text::TextRendering::Crisp());
    const text::FontId karla = f.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());
    DrawList dl = NewList();
    const Color ink(0.08f, 0.08f, 0.10f, 1.0f);
    f.ts->Draw(dl, {f.droid, 11.0f}, Vec2(8, 6), ink, "Crisp: the fonts' own hints, the classic interpreter, whole pixels.");
    f.ts->Draw(dl, {f.droid, 13.0f}, Vec2(8, 24), ink, "The quick brown fox jumps over the lazy dog 0123456789");
    f.ts->Draw(dl, {karla, 12.0f}, Vec2(8, 44), Color(0.0f, 0.35f, 0.8f, 1.0f), "Karla 12: Waffle Toolkit, liquid glass, AV To");
    f.ts->Draw(dl, {f.droid, 16.0f}, Vec2(8, 62), ink, "Esia \xC2\xB7 AV To Wa fi \xC3\x80\xC3\x89\xC3\x8E\xC3\xB5\xC3\xBC");
    f.ts->Draw(dl, {f.droid, 9.0f}, Vec2(8, 86), ink, "Nine pixels per em: the smallest labels stay legible with full hinting.");
    f.pages.Collect(f.textures);
    CheckGolden(Composite(dl, f.pages, 480, 104), "ft_crisp");
#endif
}
