// SVG parsing: the XML (comments, declarations, CDATA, entities, errors), numbers and path data in their compact forms,
// the basic shapes, transforms, colors, units, CSS (sheets, selectors, specificity, !important, the style attribute),
// currentColor, use / symbol, nested viewports and the document's size.
#include "esia/svg/svg.hpp"
#include "esia_test.hpp"
#include "svg_internal.hpp"
#include "svg_xml.hpp"
#include <cmath>
#include <cstdio>
#include <string>

using namespace esia;
using namespace esia::svg;
using namespace esia::svg::detail;

namespace
{
    const std::uint8_t* Px(const Image& img, int x, int y) { return img.pixels.data() + ((std::size_t)y * (std::size_t)img.width + (std::size_t)x) * 4; }

    // the color of one pixel of `body` drawn into a 4 x 4 viewBox at 4 x 4 pixels
    const std::uint8_t* Color4(const std::string& body, Image& img, int x = 1, int y = 1, const RasterOptions& o = {})
    {
        std::string error;
        const Document doc = Document::Parse("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 4 4'>" + body + "</svg>", &error);
        ESIA_CHECK(!doc.Empty());
        ESIA_CHECK(Rasterize(doc, 4, 4, o, img));
        return Px(img, x, y);
    }

    bool Rgb(const std::uint8_t* p, int r, int g, int b, int a = 255)
    {
        return std::abs(p[0] - r) <= 1 && std::abs(p[1] - g) <= 1 && std::abs(p[2] - b) <= 1 && std::abs(p[3] - a) <= 1;
    }
}

ESIA_TEST(SvgXml, WellFormedAndErrors)
{
    xml::Document x;
    std::string error;
    ESIA_CHECK(xml::Parse("\xEF\xBB\xBF<?xml version='1.0'?><!DOCTYPE svg [ <!ENTITY a 'x>y'> ]><!-- c > -->"
                          "<svg a=\"1 &amp; 2\" b='&#x41;&#66;'><style><![CDATA[ .k > p {} ]]></style><g/></svg>",
                          x, &error));
    ESIA_CHECK(x.elements.size() == 3 && x.elements[0].name == "svg");
    ESIA_CHECK(x.Attr(x.elements[0], "a") == "1 & 2" && x.Attr(x.elements[0], "b") == "AB");
    ESIA_CHECK(x.elements[1].text == " .k > p {} ");
    ESIA_CHECK(x.elements[2].parent == 0 && x.elements[1].next == 2);
    ESIA_CHECK(xml::Parse("<svg:svg xmlns:svg='http://www.w3.org/2000/svg'><svg:rect/></svg:svg>", x, &error) && x.elements[1].name == "rect");
    ESIA_CHECK(!xml::Parse("<svg><g></svg>", x, &error) && error.find("line 1") == 0);
    ESIA_CHECK(!xml::Parse("<svg/>text", x, &error));
    ESIA_CHECK(!xml::Parse("<svg a=1/>", x, &error));
    ESIA_CHECK(!xml::Parse("<svg/><svg/>", x, &error));
    ESIA_CHECK(Document::Parse("<html/>", &error).Empty() && error.find("not an SVG") == 0);
}

ESIA_TEST(SvgPathData, CompactForms)
{
    std::size_t i = 0;
    float v = 0.0f;
    ESIA_CHECK(ParseNumber("-.5e-1x", i, v) && i == 6 && std::fabs(v + 0.05f) < 1e-7f);
    i = 0;
    ESIA_CHECK(ParseNumber("3em", i, v) && i == 1 && v == 3.0f);   // "e" without digits is a unit
    i = 0;
    ESIA_CHECK(ParseNumber("1.5.5", i, v) && i == 3 && v == 1.5f);
    i = 0;
    ESIA_CHECK(!ParseNumber(".", i, v));

    PathData p;
    ESIA_CHECK(ParsePathData("M1.5.5L-1-2 3e1,4", p));   // implicit linetos, numbers without separators
    ESIA_CHECK(p.verbs.size() == 3 && p.verbs[1] == Verb::Line && p.verbs[2] == Verb::Line);
    ESIA_CHECK(p.pts[0] == 1.5f && p.pts[1] == 0.5f && p.pts[2] == -1.0f && p.pts[3] == -2.0f && p.pts[4] == 30.0f && p.pts[5] == 4.0f);

    PathData r;
    ESIA_CHECK(ParsePathData("m10 10 h5 v5 h-5 z l5 5", r));   // relative; a line after a close starts at the subpath's start
    ESIA_CHECK(r.verbs.size() == 7 && r.verbs[4] == Verb::Close && r.verbs[5] == Verb::Move && r.pts[8] == 10.0f && r.pts[9] == 10.0f);
    ESIA_CHECK(r.pts[r.pts.size() - 2] == 15.0f && r.pts.back() == 15.0f);

    PathData a;
    ESIA_CHECK(ParsePathData("M0 0a5 5 0 1010 0", a));   // flags without separators
    ESIA_CHECK(a.verbs.size() == 2 && a.verbs[1] == Verb::Arc);
    ComputeBounds(a);
    ESIA_CHECK(std::fabs(a.bounds.min.y + 5.0f) < 1e-4f || std::fabs(a.bounds.max.y - 5.0f) < 1e-4f);   // the half circle's bulge

    PathData s;
    ESIA_CHECK(ParsePathData("M0 0 C 0 10 10 10 10 0 S 20 -10 20 0", s));
    ESIA_CHECK(s.verbs.size() == 3 && s.pts[8] == 10.0f && s.pts[9] == -10.0f);   // the reflected control point

    PathData e;
    ESIA_CHECK(!ParsePathData("M0 0 L10 10 L20", e));   // an error: what came before it stays
    ESIA_CHECK(e.verbs.size() == 2);
    ESIA_CHECK(!ParsePathData("L10 10", e));   // path data starts with a moveto
}

ESIA_TEST(SvgPathData, BoundsOfCurves)
{
    PathData p;
    ParsePathData("M0 0 C 0 -10 10 -10 10 0", p);   // the cubic's top at y = -7.5
    ComputeBounds(p);
    ESIA_CHECK(std::fabs(p.bounds.min.y + 7.5f) < 1e-4f && p.bounds.max.y == 0.0f && p.bounds.max.x == 10.0f);
    PathData c;
    AddEllipse(c, 5.0f, 6.0f, 3.0f, 2.0f);
    ComputeBounds(c);
    ESIA_CHECK(std::fabs(c.bounds.min.x - 2.0f) < 1e-4f && std::fabs(c.bounds.max.y - 8.0f) < 1e-4f && std::fabs(c.bounds.min.y - 4.0f) < 1e-4f);
}

ESIA_TEST(SvgDocument, LoadFromAFile)
{
    std::string error;
    ESIA_CHECK(Document::Load("no-such-file.svg", &error).Empty() && error.find("cannot open") == 0);
    const char* path = "esia_svg_load_test.svg";
    std::FILE* f = std::fopen(path, "wb");
    ESIA_CHECK(f != nullptr);
    if (!f)
        return;
    const std::string text = "<svg viewBox='0 0 24 12'><rect width='24' height='12'/></svg>";
    std::fwrite(text.data(), 1, text.size(), f);
    std::fclose(f);
    const Document d = Document::Load(path, &error);
    std::remove(path);
    ESIA_CHECK(!d.Empty() && d.Size() == Vec2(24.0f, 12.0f));
}

ESIA_TEST(SvgDocument, SizeAndUnits)
{
    ESIA_CHECK(Document::Parse("<svg viewBox='0 0 24 12'/>").Size() == Vec2(24.0f, 12.0f));
    ESIA_CHECK(Document::Parse("<svg width='1in' height='72pt'/>").Size() == Vec2(96.0f, 96.0f));
    ESIA_CHECK(Document::Parse("<svg width='48' viewBox='0 0 24 12'/>").Size() == Vec2(48.0f, 24.0f));
    ESIA_CHECK(Document::Parse("<svg width='100%' viewBox='0 0 24 12'/>").Size() == Vec2(24.0f, 12.0f));
    // preserveAspectRatio: xMidYMid meet centers the 4 x 4 viewBox in 8 x 4 pixels
    const Document d = Document::Parse("<svg viewBox='0 0 4 4'><rect width='4' height='4'/></svg>");
    Image img;
    ESIA_CHECK(Rasterize(d, 8, 4, {}, img));
    ESIA_CHECK(Px(img, 1, 1)[3] == 0 && Px(img, 2, 1)[3] == 255 && Px(img, 5, 1)[3] == 255 && Px(img, 6, 1)[3] == 0);
    const Document none = Document::Parse("<svg viewBox='0 0 4 4' preserveAspectRatio='none'><rect width='4' height='4'/></svg>");
    ESIA_CHECK(Rasterize(none, 8, 4, {}, img) && Px(img, 0, 1)[3] == 255 && Px(img, 7, 1)[3] == 255);
}

ESIA_TEST(SvgDocument, Colors)
{
    Image img;
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='#f80'/>", img), 255, 136, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='#11223380'/>", img), 0x11, 0x22, 0x33, 0x80));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='rgb(100%, 0%, 50%)'/>", img), 255, 0, 128));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='rgba(0,255,0,0.5)'/>", img), 0, 255, 0, 128));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='rgb(0 0 255 / 25%)'/>", img), 0, 0, 255, 64));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='hsl(120, 100%, 25%)'/>", img), 0, 128, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='RebeccaPurple'/>", img), 0x66, 0x33, 0x99));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='transparent'/>", img), 0, 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4'/>", img), 0, 0, 0));                         // black by default
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='bogus'/>", img), 0, 0, 0));            // an invalid value is ignored
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='url(#none) red'/>", img), 255, 0, 0));  // the fallback
    ESIA_CHECK(Rgb(Color4("<rect width='4' height='4' fill='url(#none)'/>", img), 0, 0, 0, 0));
}

ESIA_TEST(SvgDocument, CascadeAndSelectors)
{
    Image img;
    // presentation attribute < sheet (by specificity) < style attribute < !important
    ESIA_CHECK(Rgb(Color4("<style>rect { fill: red } .a { fill: lime } #r { fill: blue }</style>"
                          "<rect id='r' class='a b' width='4' height='4' fill='black'/>", img),
                   0, 0, 255));
    ESIA_CHECK(Rgb(Color4("<style>#r { fill: blue }</style><rect id='r' width='4' height='4' style='fill: #0f0'/>", img), 0, 255, 0));
    ESIA_CHECK(Rgb(Color4("<style>rect { fill: blue !important }</style><rect width='4' height='4' style='fill: #0f0'/>", img), 0, 0, 255));
    ESIA_CHECK(Rgb(Color4("<style>/* a comment */ g > .x { fill: #f00 } svg .y { fill: #0f0 } p + .z { fill: #00f }</style>"
                          "<g><rect class='x y z' width='4' height='4'/></g>", img),
                   0, 255, 0));   // the later rule of equal specificity; the unsupported one is dropped
    // inheritance and inherit
    ESIA_CHECK(Rgb(Color4("<g fill='#0000ff'><g><rect width='4' height='4'/></g></g>", img), 0, 0, 255));
    ESIA_CHECK(Rgb(Color4("<g stroke='#00ff00' stroke-width='9'><rect width='1' height='1' fill='none'/></g>", img), 0, 255, 0));
    ESIA_CHECK(Rgb(Color4("<g opacity='0.5'><rect width='4' height='4' opacity='inherit'/></g>", img), 0, 0, 0, 64));
    // display: none skips a subtree, visibility: hidden only what is hidden
    ESIA_CHECK(Rgb(Color4("<g display='none'><rect width='4' height='4'/></g>", img), 0, 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<g visibility='hidden'><rect width='4' height='4' fill='red' visibility='visible'/></g>", img), 255, 0, 0));
}

ESIA_TEST(SvgDocument, CurrentColor)
{
    const Document mono = Document::Parse("<svg viewBox='0 0 4 4'><rect width='4' height='4' fill='currentColor'/></svg>");
    ESIA_CHECK(mono.Monochrome() && mono.UsesCurrentColor());
    Image img;
    RasterOptions o;
    o.currentColor = Color(0.0f, 0.5f, 1.0f, 1.0f);
    ESIA_CHECK(Rasterize(mono, 4, 4, o, img) && Rgb(Px(img, 1, 1), 0, 128, 255));
    // a color property above it: currentColor is that color, not the host's
    const Document set = Document::Parse("<svg viewBox='0 0 4 4' color='#f00'><rect width='4' height='4' fill='currentColor'/></svg>");
    ESIA_CHECK(!set.Monochrome() && !set.UsesCurrentColor());
    ESIA_CHECK(Rasterize(set, 4, 4, o, img) && Rgb(Px(img, 1, 1), 255, 0, 0));
    ESIA_CHECK(!Document::Parse("<svg viewBox='0 0 4 4'><rect width='4' height='4' fill='currentColor' stroke='red'/></svg>").Monochrome());
}

ESIA_TEST(SvgDocument, TransformsUseAndSymbols)
{
    Image img;
    // rotate about a point: a bar along x turned upright through (2, 2)
    ESIA_CHECK(Rgb(Color4("<rect x='0' y='1' width='4' height='2' transform='rotate(90 2 2)'/>", img, 2, 0), 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<rect x='0' y='1' width='4' height='2' transform='rotate(90 2 2)'/>", img, 0, 2), 0, 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='1' height='1' transform='translate(2 1) scale(2 1)'/>", img, 3, 1), 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='1' height='1' transform='translate(2 1) scale(2 1)'/>", img, 1, 1), 0, 0, 0, 0));
    ESIA_CHECK(Rgb(Color4("<rect width='1' height='1' transform='bogus(3)'/>", img, 0, 0), 0, 0, 0));   // an invalid list: none
    // use: x / y, its style inherited by what it uses
    ESIA_CHECK(Rgb(Color4("<defs><rect id='r' width='1' height='1'/></defs><use href='#r' x='3' y='3' fill='#0f0'/>", img, 3, 3), 0, 255, 0));
    ESIA_CHECK(Rgb(Color4("<defs><rect id='r' width='1' height='1'/></defs><use xlink:href='#r' x='3' y='3'/>", img, 0, 0), 0, 0, 0, 0));
    // a symbol: its viewBox fitted into the use's width and height
    ESIA_CHECK(Rgb(Color4("<symbol id='s' viewBox='0 0 1 1'><rect width='1' height='1' fill='#00f'/></symbol>"
                          "<use href='#s' x='2' y='0' width='2' height='2'/>",
                          img, 3, 1),
                   0, 0, 255));
    // a use of itself draws nothing (and ends)
    ESIA_CHECK(Rgb(Color4("<g id='g'><use href='#g'/></g>", img), 0, 0, 0, 0));
    // a nested svg: its own viewport and viewBox
    ESIA_CHECK(Rgb(Color4("<svg x='2' y='2' width='2' height='2' viewBox='0 0 10 10'><rect width='10' height='10' fill='#f0f'/></svg>", img, 3, 3),
                   255, 0, 255));
}

ESIA_TEST(SvgDocument, LayersOnlyWhereNeeded)
{
    // one shape with one paint in a translucent group: no layer, the opacity on the shape
    const Document one = Document::Parse("<svg viewBox='0 0 4 4'><g opacity='0.5'><rect width='4' height='4'/></g></svg>");
    ESIA_CHECK(one.Data()->ops.size() == 1 && std::fabs(one.Data()->shapes[0].fillOpacity - 0.5f) < 1e-6f);
    // two shapes, or a fill and a stroke: a layer
    const Document two = Document::Parse("<svg viewBox='0 0 4 4'><g opacity='0.5'><rect width='4' height='4'/><rect width='2' height='2'/></g></svg>");
    ESIA_CHECK(two.Data()->ops.size() == 4 && two.Data()->ops[0].kind == OpKind::Begin && two.Data()->ops[0].match == 3);
    const Document both = Document::Parse("<svg viewBox='0 0 4 4'><rect width='4' height='4' stroke='red' opacity='0.5'/></svg>");
    ESIA_CHECK(both.Data()->ops.size() == 3);
    // nothing to draw in it: no layer either
    ESIA_CHECK(Document::Parse("<svg viewBox='0 0 4 4'><g opacity='0.5'><rect width='0' height='4'/></g></svg>").Data()->ops.empty());
}
