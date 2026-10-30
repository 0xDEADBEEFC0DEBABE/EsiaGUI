// Esia - SVG documents (svg.hpp): the XML into a drawing - the CSS cascade and inheritance, lengths, colors and paints,
// gradients, the basic shapes as paths, use and symbol, clip paths, group opacity - and Document's interface.
//
// A group with opacity or a clip path becomes a layer only where it has to: a group holding one shape with one paint
// passes its opacity (and clip) to that shape instead.
#include "svg_internal.hpp"
#include "svg_xml.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <unordered_map>

namespace esia::svg::detail
{
    namespace
    {
        // ------------------------------------------------------------------ text
        bool Ws(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }

        std::string_view Trim(std::string_view s)
        {
            while (!s.empty() && Ws(s.front()))
                s.remove_prefix(1);
            while (!s.empty() && Ws(s.back()))
                s.remove_suffix(1);
            return s;
        }

        char Lower(char c) { return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c; }

        bool IEq(std::string_view a, std::string_view b)
        {
            if (a.size() != b.size())
                return false;
            for (std::size_t i = 0; i < a.size(); ++i)
                if (Lower(a[i]) != Lower(b[i]))
                    return false;
            return true;
        }

        bool IStarts(std::string_view s, std::string_view prefix) { return s.size() >= prefix.size() && IEq(s.substr(0, prefix.size()), prefix); }

        void SkipWs(std::string_view s, std::size_t& i)
        {
            while (i < s.size() && Ws(s[i]))
                ++i;
        }

        void SkipSep(std::string_view s, std::size_t& i)
        {
            SkipWs(s, i);
            if (i < s.size() && s[i] == ',')
            {
                ++i;
                SkipWs(s, i);
            }
        }

        bool Alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

        // ------------------------------------------------------------------ lengths and numbers
        struct Viewport
        {
            float w = 100.0f, h = 100.0f;
        };
        enum class Axis : std::uint8_t { X, Y, Diagonal };

        float PercentOf(Axis a, const Viewport& vp)
        {
            switch (a)
            {
            case Axis::X:
                return vp.w * 0.01f;
            case Axis::Y:
                return vp.h * 0.01f;
            case Axis::Diagonal:
                break;
            }
            return std::sqrt(0.5f * (vp.w * vp.w + vp.h * vp.h)) * 0.01f;
        }

        bool ParseLength(std::string_view s, std::size_t& i, Axis axis, const Viewport& vp, float& out)
        {
            float v;
            if (!ParseNumber(s, i, v))
                return false;
            const std::size_t n = s.size();
            if (i < n && s[i] == '%')
            {
                ++i;
                out = v * PercentOf(axis, vp);
                return true;
            }
            if (i + 1 < n && Alpha(s[i]) && Alpha(s[i + 1]))
            {
                const char a = Lower(s[i]), b = Lower(s[i + 1]);
                float k = 0.0f;
                if (a == 'p' && b == 'x')
                    k = 1.0f;
                else if (a == 'p' && b == 't')
                    k = 4.0f / 3.0f;
                else if (a == 'p' && b == 'c')
                    k = 16.0f;
                else if (a == 'm' && b == 'm')
                    k = 96.0f / 25.4f;
                else if (a == 'c' && b == 'm')
                    k = 96.0f / 2.54f;
                else if (a == 'i' && b == 'n')
                    k = 96.0f;
                else if (a == 'e' && b == 'm')
                    k = 16.0f;
                else if (a == 'e' && b == 'x')
                    k = 8.0f;
                if (k != 0.0f)
                {
                    i += 2;
                    out = v * k;
                    return true;
                }
            }
            out = v;
            return true;
        }

        bool Length(std::string_view s, Axis axis, const Viewport& vp, float& out)
        {
            s = Trim(s);
            std::size_t i = 0;
            return !s.empty() && ParseLength(s, i, axis, vp, out) && i == s.size();
        }

        float LengthOr(std::string_view s, Axis axis, const Viewport& vp, float fallback)
        {
            float v;
            return Length(s, axis, vp, v) ? v : fallback;
        }

        // a number, or a percentage as a fraction ("50%" = 0.5)
        bool Fraction(std::string_view s, float& out)
        {
            s = Trim(s);
            std::size_t i = 0;
            float v;
            if (!ParseNumber(s, i, v))
                return false;
            if (i < s.size() && s[i] == '%')
            {
                v *= 0.01f;
                ++i;
            }
            if (i != s.size())
                return false;
            out = v;
            return true;
        }

        // numbers separated by white space and commas; on an error, the numbers before it
        bool Numbers(std::string_view s, std::vector<float>& out)
        {
            out.clear();
            std::size_t i = 0;
            SkipWs(s, i);
            while (i < s.size())
            {
                float v;
                if (!ParseNumber(s, i, v))
                    return false;
                out.push_back(v);
                SkipSep(s, i);
            }
            return true;
        }

        // ------------------------------------------------------------------ colors
        struct Named
        {
            const char* name;
            std::uint32_t rgb;
        };
        // CSS's named colors
        constexpr Named kNamed[] = {
            {"aliceblue", 0xF0F8FF}, {"antiquewhite", 0xFAEBD7}, {"aqua", 0x00FFFF}, {"aquamarine", 0x7FFFD4}, {"azure", 0xF0FFFF},
            {"beige", 0xF5F5DC}, {"bisque", 0xFFE4C4}, {"black", 0x000000}, {"blanchedalmond", 0xFFEBCD}, {"blue", 0x0000FF},
            {"blueviolet", 0x8A2BE2}, {"brown", 0xA52A2A}, {"burlywood", 0xDEB887}, {"cadetblue", 0x5F9EA0}, {"chartreuse", 0x7FFF00},
            {"chocolate", 0xD2691E}, {"coral", 0xFF7F50}, {"cornflowerblue", 0x6495ED}, {"cornsilk", 0xFFF8DC}, {"crimson", 0xDC143C},
            {"cyan", 0x00FFFF}, {"darkblue", 0x00008B}, {"darkcyan", 0x008B8B}, {"darkgoldenrod", 0xB8860B}, {"darkgray", 0xA9A9A9},
            {"darkgreen", 0x006400}, {"darkgrey", 0xA9A9A9}, {"darkkhaki", 0xBDB76B}, {"darkmagenta", 0x8B008B},
            {"darkolivegreen", 0x556B2F}, {"darkorange", 0xFF8C00}, {"darkorchid", 0x9932CC}, {"darkred", 0x8B0000},
            {"darksalmon", 0xE9967A}, {"darkseagreen", 0x8FBC8F}, {"darkslateblue", 0x483D8B}, {"darkslategray", 0x2F4F4F},
            {"darkslategrey", 0x2F4F4F}, {"darkturquoise", 0x00CED1}, {"darkviolet", 0x9400D3}, {"deeppink", 0xFF1493},
            {"deepskyblue", 0x00BFFF}, {"dimgray", 0x696969}, {"dimgrey", 0x696969}, {"dodgerblue", 0x1E90FF}, {"firebrick", 0xB22222},
            {"floralwhite", 0xFFFAF0}, {"forestgreen", 0x228B22}, {"fuchsia", 0xFF00FF}, {"gainsboro", 0xDCDCDC},
            {"ghostwhite", 0xF8F8FF}, {"gold", 0xFFD700}, {"goldenrod", 0xDAA520}, {"gray", 0x808080}, {"green", 0x008000},
            {"greenyellow", 0xADFF2F}, {"grey", 0x808080}, {"honeydew", 0xF0FFF0}, {"hotpink", 0xFF69B4}, {"indianred", 0xCD5C5C},
            {"indigo", 0x4B0082}, {"ivory", 0xFFFFF0}, {"khaki", 0xF0E68C}, {"lavender", 0xE6E6FA}, {"lavenderblush", 0xFFF0F5},
            {"lawngreen", 0x7CFC00}, {"lemonchiffon", 0xFFFACD}, {"lightblue", 0xADD8E6}, {"lightcoral", 0xF08080},
            {"lightcyan", 0xE0FFFF}, {"lightgoldenrodyellow", 0xFAFAD2}, {"lightgray", 0xD3D3D3}, {"lightgreen", 0x90EE90},
            {"lightgrey", 0xD3D3D3}, {"lightpink", 0xFFB6C1}, {"lightsalmon", 0xFFA07A}, {"lightseagreen", 0x20B2AA},
            {"lightskyblue", 0x87CEFA}, {"lightslategray", 0x778899}, {"lightslategrey", 0x778899}, {"lightsteelblue", 0xB0C4DE},
            {"lightyellow", 0xFFFFE0}, {"lime", 0x00FF00}, {"limegreen", 0x32CD32}, {"linen", 0xFAF0E6}, {"magenta", 0xFF00FF},
            {"maroon", 0x800000}, {"mediumaquamarine", 0x66CDAA}, {"mediumblue", 0x0000CD}, {"mediumorchid", 0xBA55D3},
            {"mediumpurple", 0x9370DB}, {"mediumseagreen", 0x3CB371}, {"mediumslateblue", 0x7B68EE}, {"mediumspringgreen", 0x00FA9A},
            {"mediumturquoise", 0x48D1CC}, {"mediumvioletred", 0xC71585}, {"midnightblue", 0x191970}, {"mintcream", 0xF5FFFA},
            {"mistyrose", 0xFFE4E1}, {"moccasin", 0xFFE4B5}, {"navajowhite", 0xFFDEAD}, {"navy", 0x000080}, {"oldlace", 0xFDF5E6},
            {"olive", 0x808000}, {"olivedrab", 0x6B8E23}, {"orange", 0xFFA500}, {"orangered", 0xFF4500}, {"orchid", 0xDA70D6},
            {"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98}, {"paleturquoise", 0xAFEEEE}, {"palevioletred", 0xDB7093},
            {"papayawhip", 0xFFEFD5}, {"peachpuff", 0xFFDAB9}, {"peru", 0xCD853F}, {"pink", 0xFFC0CB}, {"plum", 0xDDA0DD},
            {"powderblue", 0xB0E0E6}, {"purple", 0x800080}, {"rebeccapurple", 0x663399}, {"red", 0xFF0000}, {"rosybrown", 0xBC8F8F},
            {"royalblue", 0x4169E1}, {"saddlebrown", 0x8B4513}, {"salmon", 0xFA8072}, {"sandybrown", 0xF4A460}, {"seagreen", 0x2E8B57},
            {"seashell", 0xFFF5EE}, {"sienna", 0xA0522D}, {"silver", 0xC0C0C0}, {"skyblue", 0x87CEEB}, {"slateblue", 0x6A5ACD},
            {"slategray", 0x708090}, {"slategrey", 0x708090}, {"snow", 0xFFFAFA}, {"springgreen", 0x00FF7F}, {"steelblue", 0x4682B4},
            {"tan", 0xD2B48C}, {"teal", 0x008080}, {"thistle", 0xD8BFD8}, {"tomato", 0xFF6347}, {"turquoise", 0x40E0D0},
            {"violet", 0xEE82EE}, {"wheat", 0xF5DEB3}, {"white", 0xFFFFFF}, {"whitesmoke", 0xF5F5F5}, {"yellow", 0xFFFF00},
            {"yellowgreen", 0x9ACD32},
        };

        int HexDigit(char c)
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            return -1;
        }

        // the arguments of rgb() / hsl(): numbers, each maybe a percentage or an angle, separated by commas, white space
        // or the slash before the alpha
        int Components(std::string_view s, float* v, bool* pct, int max)
        {
            std::size_t i = 0;
            int n = 0;
            for (;;)
            {
                SkipWs(s, i);
                if (i < s.size() && (s[i] == ',' || s[i] == '/'))
                {
                    ++i;
                    continue;
                }
                if (i >= s.size())
                    return n;
                if (n == max || !ParseNumber(s, i, v[n]))
                    return -1;
                pct[n] = false;
                if (i < s.size() && s[i] == '%')
                {
                    pct[n] = true;
                    ++i;
                }
                else if (i < s.size() && Alpha(s[i]))
                {
                    // an angle's unit (hue)
                    std::size_t u = i;
                    while (u < s.size() && Alpha(s[u]))
                        ++u;
                    const std::string_view unit = s.substr(i, u - i);
                    if (IEq(unit, "rad"))
                        v[n] *= 180.0f / kPi;
                    else if (IEq(unit, "grad"))
                        v[n] *= 0.9f;
                    else if (IEq(unit, "turn"))
                        v[n] *= 360.0f;
                    else if (!IEq(unit, "deg"))
                        return -1;
                    i = u;
                }
                ++n;
            }
        }

        float HueChannel(float p, float q, float t)
        {
            if (t < 0.0f)
                t += 1.0f;
            if (t > 1.0f)
                t -= 1.0f;
            if (t < 1.0f / 6.0f)
                return p + (q - p) * 6.0f * t;
            if (t < 0.5f)
                return q;
            if (t < 2.0f / 3.0f)
                return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        }

        bool ParseColor(std::string_view s, Color& out)
        {
            s = Trim(s);
            if (s.empty())
                return false;
            if (s[0] == '#')
            {
                std::string_view h = s.substr(1);
                const std::size_t end = h.find_first_of(" \t\r\n");
                if (end != std::string_view::npos)
                    h = h.substr(0, end);   // "#fff icc-color(...)"
                int d[8];
                if (h.size() > 8)
                    return false;
                for (std::size_t i = 0; i < h.size(); ++i)
                    if ((d[i] = HexDigit(h[i])) < 0)
                        return false;
                if (h.size() == 3 || h.size() == 4)
                {
                    out = Color::Rgba8(d[0] * 17, d[1] * 17, d[2] * 17, h.size() == 4 ? d[3] * 17 : 255);
                    return true;
                }
                if (h.size() == 6 || h.size() == 8)
                {
                    out = Color::Rgba8(d[0] * 16 + d[1], d[2] * 16 + d[3], d[4] * 16 + d[5], h.size() == 8 ? d[6] * 16 + d[7] : 255);
                    return true;
                }
                return false;
            }
            const std::size_t paren = s.find('(');
            if (paren != std::string_view::npos)
            {
                const std::size_t close = s.find(')', paren);
                if (close == std::string_view::npos)
                    return false;
                const std::string_view fn = Trim(s.substr(0, paren));
                const std::string_view args = s.substr(paren + 1, close - paren - 1);
                float v[4];
                bool pct[4];
                const int n = Components(args, v, pct, 4);
                if (n != 3 && n != 4)
                    return false;
                const float alpha = n == 4 ? Saturate(pct[3] ? v[3] * 0.01f : v[3]) : 1.0f;
                if (IEq(fn, "rgb") || IEq(fn, "rgba"))
                {
                    float c[3];
                    for (int k = 0; k < 3; ++k)
                        c[k] = Saturate(pct[k] ? v[k] * 0.01f : v[k] / 255.0f);
                    out = Color(c[0], c[1], c[2], alpha);
                    return true;
                }
                if (IEq(fn, "hsl") || IEq(fn, "hsla"))
                {
                    const float hue = std::fmod(std::fmod(v[0], 360.0f) + 360.0f, 360.0f) / 360.0f;
                    const float sat = Saturate(v[1] * 0.01f), light = Saturate(v[2] * 0.01f);
                    const float q = light < 0.5f ? light * (1.0f + sat) : light + sat - light * sat;
                    const float p = 2.0f * light - q;
                    out = Color(HueChannel(p, q, hue + 1.0f / 3.0f), HueChannel(p, q, hue), HueChannel(p, q, hue - 1.0f / 3.0f), alpha);
                    return true;
                }
                return false;
            }
            if (IEq(s, "transparent"))
            {
                out = Color(0.0f, 0.0f, 0.0f, 0.0f);
                return true;
            }
            const std::size_t end = s.find_first_of(" \t\r\n");
            const std::string_view name = end == std::string_view::npos ? s : s.substr(0, end);
            for (const Named& c : kNamed)
                if (IEq(name, c.name))
                {
                    out = Color::Hex(c.rgb);
                    return true;
                }
            return false;
        }

        // ------------------------------------------------------------------ paints
        struct PaintSpec
        {
            enum class Kind : std::uint8_t { None, Color, Current, Url };
            Kind kind = Kind::None;
            Color color;
            std::string_view url;               // the id a url() names
            Kind fallback = Kind::None;         // what paints when it names no paint server
            Color fallbackColor;
        };

        // url(#id) -> "id" (empty: none, or outside the file)
        bool UrlId(std::string_view s, std::string_view& id, std::string_view& rest)
        {
            s = Trim(s);
            if (!IStarts(s, "url("))
                return false;
            const std::size_t close = s.find(')');
            if (close == std::string_view::npos)
                return false;
            std::string_view ref = Trim(s.substr(4, close - 4));
            if (ref.size() >= 2 && (ref.front() == '"' || ref.front() == '\'') && ref.back() == ref.front())
                ref = Trim(ref.substr(1, ref.size() - 2));
            id = !ref.empty() && ref[0] == '#' ? ref.substr(1) : std::string_view();
            rest = Trim(s.substr(close + 1));
            return true;
        }

        bool ParsePaint(std::string_view s, PaintSpec& out)
        {
            s = Trim(s);
            using K = PaintSpec::Kind;
            if (IEq(s, "none") || IEq(s, "context-fill") || IEq(s, "context-stroke"))
            {
                out = PaintSpec();
                return true;
            }
            if (IEq(s, "currentColor"))
            {
                out = PaintSpec();
                out.kind = K::Current;
                return true;
            }
            std::string_view id, rest;
            if (UrlId(s, id, rest))
            {
                PaintSpec p;
                p.kind = K::Url;
                p.url = id;
                Color c;
                if (rest.empty() || IEq(rest, "none"))
                    p.fallback = K::None;
                else if (IEq(rest, "currentColor"))
                    p.fallback = K::Current;
                else if (ParseColor(rest, c))
                {
                    p.fallback = K::Color;
                    p.fallbackColor = c;
                }
                out = p;
                return true;
            }
            Color c;
            if (!ParseColor(s, c))
                return false;
            out = PaintSpec();
            out.kind = K::Color;
            out.color = c;
            return true;
        }

        // ------------------------------------------------------------------ transforms
        bool ParseTransform(std::string_view s, Affine& out)
        {
            out = Affine();
            std::size_t i = 0;
            const std::size_t n = s.size();
            for (;;)
            {
                SkipSep(s, i);
                if (i >= n)
                    return true;
                const std::size_t name0 = i;
                while (i < n && Alpha(s[i]))
                    ++i;
                const std::string_view name = s.substr(name0, i - name0);
                SkipWs(s, i);
                if (i >= n || s[i] != '(')
                    return false;
                ++i;
                float v[6];
                int count = 0;
                for (;;)
                {
                    SkipSep(s, i);
                    if (i < n && s[i] == ')')
                    {
                        ++i;
                        break;
                    }
                    if (count == 6 || !ParseNumber(s, i, v[count]))
                        return false;
                    ++count;
                }
                Affine m;
                if (name == "matrix" && count == 6)
                    m = Affine{v[0], v[1], v[2], v[3], v[4], v[5]};
                else if (name == "translate" && (count == 1 || count == 2))
                    m = Affine::Translate(v[0], count == 2 ? v[1] : 0.0f);
                else if (name == "scale" && (count == 1 || count == 2))
                    m = Affine::Scale(v[0], count == 2 ? v[1] : v[0]);
                else if (name == "rotate" && (count == 1 || count == 3))
                {
                    m = Affine::Rotate(Radians(v[0]));
                    if (count == 3)
                        m = Affine::Translate(v[1], v[2]) * m * Affine::Translate(-v[1], -v[2]);
                }
                else if (name == "skewX" && count == 1)
                    m = Affine{1.0f, 0.0f, std::tan(Radians(v[0])), 1.0f, 0.0f, 0.0f};
                else if (name == "skewY" && count == 1)
                    m = Affine{1.0f, std::tan(Radians(v[0])), 0.0f, 1.0f, 0.0f, 0.0f};
                else
                    return false;
                out = out * m;
            }
        }

        Affine TransformOf(std::string_view s)
        {
            Affine m;
            if (!ParseTransform(s, m))
                m = Affine();   // an invalid list is no transform
            return m;
        }

        // preserveAspectRatio
        struct Aspect
        {
            bool none = false, slice = false;
            std::uint8_t x = 1, y = 1;
        };

        Aspect ParseAspect(std::string_view s)
        {
            Aspect a;
            std::size_t i = 0;
            for (;;)
            {
                SkipWs(s, i);
                if (i >= s.size())
                    break;
                const std::size_t t0 = i;
                while (i < s.size() && !Ws(s[i]))
                    ++i;
                const std::string_view tok = s.substr(t0, i - t0);
                if (tok == "none")
                    a.none = true;
                else if (tok == "slice")
                    a.slice = true;
                else if (tok.size() == 8 && tok[0] == 'x' && tok[4] == 'Y')
                {
                    const auto axis = [](std::string_view p) -> int { return p == "Min" ? 0 : (p == "Mid" ? 1 : (p == "Max" ? 2 : -1)); };
                    const int ax = axis(tok.substr(1, 3)), ay = axis(tok.substr(5, 3));
                    if (ax >= 0 && ay >= 0)
                    {
                        a.x = (std::uint8_t)ax;
                        a.y = (std::uint8_t)ay;
                    }
                }
            }
            return a;
        }

        bool ParseViewBox(std::string_view s, Rect& out)
        {
            std::vector<float> v;
            if (!Numbers(s, v) || v.size() != 4 || !(v[2] > 0.0f) || !(v[3] > 0.0f))
                return false;
            out = Rect(v[0], v[1], v[0] + v[2], v[1] + v[3]);
            return true;
        }

        Affine ViewBoxTransform(const Rect& vb, const Aspect& a, float w, float h)
        {
            if (!(vb.Width() > 0.0f) || !(vb.Height() > 0.0f))
                return Affine::Scale(0.0f, 0.0f);
            const float sx = w / vb.Width(), sy = h / vb.Height();
            if (a.none)
                return Affine{sx, 0.0f, 0.0f, sy, -vb.min.x * sx, -vb.min.y * sy};
            const float s = a.slice ? std::max(sx, sy) : std::min(sx, sy);
            const float k[3] = {0.0f, 0.5f, 1.0f};
            return Affine{s, 0.0f, 0.0f, s, (w - vb.Width() * s) * k[a.x] - vb.min.x * s, (h - vb.Height() * s) * k[a.y] - vb.min.y * s};
        }

        // ------------------------------------------------------------------ CSS
        struct Decl
        {
            std::string_view prop, value;
            bool important = false;
        };

        void ParseDecls(std::string_view s, std::vector<Decl>& out)
        {
            std::size_t i = 0;
            while (i < s.size())
            {
                std::size_t j = i;
                int depth = 0;
                char quote = 0;
                for (; j < s.size(); ++j)
                {
                    const char c = s[j];
                    if (quote)
                    {
                        if (c == quote)
                            quote = 0;
                    }
                    else if (c == '"' || c == '\'')
                        quote = c;
                    else if (c == '(')
                        ++depth;
                    else if (c == ')')
                        --depth;
                    else if (c == ';' && depth <= 0)
                        break;
                }
                const std::string_view d = s.substr(i, j - i);
                i = j + 1;
                const std::size_t colon = d.find(':');
                if (colon == std::string_view::npos)
                    continue;
                Decl decl;
                decl.prop = Trim(d.substr(0, colon));
                std::string_view v = Trim(d.substr(colon + 1));
                const std::size_t bang = v.rfind('!');
                if (bang != std::string_view::npos && IEq(Trim(v.substr(bang + 1)), "important"))
                {
                    decl.important = true;
                    v = Trim(v.substr(0, bang));
                }
                decl.value = v;
                if (!decl.prop.empty())
                    out.push_back(decl);
            }
        }

        std::string StripComments(std::string_view s)
        {
            std::string out;
            out.reserve(s.size());
            for (std::size_t i = 0; i < s.size();)
            {
                if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '*')
                {
                    const std::size_t end = s.find("*/", i + 2);
                    i = end == std::string_view::npos ? s.size() : end + 2;
                    out += ' ';
                    continue;
                }
                out += s[i++];
            }
            return out;
        }

        bool IdentChar(char c) { return Alpha(c) || (c >= '0' && c <= '9') || c == '-' || c == '_' || (unsigned char)c >= 0x80; }

        struct Compound
        {
            std::string_view tag;   // empty: any
            std::string_view id;
            std::vector<std::string_view> classes;
        };

        struct Selector
        {
            std::vector<Compound> parts;
            std::vector<char> comb;   // between parts: ' ' descendant, '>' child
            int specificity = 0;
        };

        struct Rule
        {
            Selector sel;
            std::vector<Decl> decls;
            int order = 0;
        };

        // type, class, id, universal, descendant and child selectors; anything else drops the rule
        bool ParseSelector(std::string_view s, Selector& out)
        {
            s = Trim(s);
            std::size_t i = 0;
            const std::size_t n = s.size();
            const auto ident = [&](std::string_view& id) {
                const std::size_t a = i;
                while (i < n && IdentChar(s[i]))
                    ++i;
                id = s.substr(a, i - a);
                return !id.empty();
            };
            while (i < n)
            {
                Compound c;
                bool any = false;
                if (s[i] == '*')
                {
                    ++i;
                    any = true;
                }
                else if (IdentChar(s[i]))
                {
                    if (!ident(c.tag))
                        return false;
                    any = true;
                    out.specificity += 1;
                }
                for (;;)
                {
                    if (i < n && s[i] == '.')
                    {
                        ++i;
                        std::string_view cls;
                        if (!ident(cls))
                            return false;
                        c.classes.push_back(cls);
                        out.specificity += 100;
                        any = true;
                    }
                    else if (i < n && s[i] == '#')
                    {
                        ++i;
                        if (!ident(c.id))
                            return false;
                        out.specificity += 10000;
                        any = true;
                    }
                    else
                        break;
                }
                if (!any)
                    return false;
                out.parts.push_back(std::move(c));
                const std::size_t before = i;
                SkipWs(s, i);
                if (i >= n)
                    break;
                if (s[i] == '>')
                {
                    ++i;
                    SkipWs(s, i);
                    out.comb.push_back('>');
                }
                else if (i > before && (IdentChar(s[i]) || s[i] == '.' || s[i] == '#' || s[i] == '*'))
                    out.comb.push_back(' ');
                else
                    return false;   // +, ~, [attr], :pseudo
            }
            return !out.parts.empty();
        }

        void ParseSheet(std::string_view css, std::vector<Rule>& rules)
        {
            std::size_t i = 0;
            const std::size_t n = css.size();
            while (i < n)
            {
                SkipWs(css, i);
                if (i >= n)
                    break;
                if (css[i] == '@')
                {
                    // an at-rule: to its ';' or past its block
                    const std::size_t semi = css.find(';', i), brace = css.find('{', i);
                    if (brace == std::string_view::npos || (semi != std::string_view::npos && semi < brace))
                    {
                        i = semi == std::string_view::npos ? n : semi + 1;
                        continue;
                    }
                    int depth = 0;
                    std::size_t j = brace;
                    for (; j < n; ++j)
                    {
                        if (css[j] == '{')
                            ++depth;
                        else if (css[j] == '}' && --depth == 0)
                            break;
                    }
                    i = j + 1;
                    continue;
                }
                const std::size_t brace = css.find('{', i);
                if (brace == std::string_view::npos)
                    break;
                const std::size_t close = css.find('}', brace);
                const std::string_view sels = css.substr(i, brace - i);
                const std::string_view block = css.substr(brace + 1, (close == std::string_view::npos ? n : close) - brace - 1);
                i = close == std::string_view::npos ? n : close + 1;
                std::vector<Decl> decls;
                ParseDecls(block, decls);
                std::size_t a = 0;
                while (a <= sels.size())
                {
                    std::size_t comma = sels.find(',', a);
                    if (comma == std::string_view::npos)
                        comma = sels.size();
                    Rule r;
                    if (ParseSelector(sels.substr(a, comma - a), r.sel))
                    {
                        r.decls = decls;
                        r.order = (int)rules.size();
                        rules.push_back(std::move(r));
                    }
                    a = comma + 1;
                }
            }
        }

        // ------------------------------------------------------------------ computed style
        struct Style
        {
            // inherited
            PaintSpec fill, stroke;
            float fillOpacity = 1.0f, strokeOpacity = 1.0f;
            bool fillEvenOdd = false, clipEvenOdd = false;
            float strokeWidth = 1.0f;
            Cap cap = Cap::Butt;
            Join join = Join::Miter;
            float miterLimit = 4.0f;
            std::vector<float> dashes;
            float dashOffset = 0.0f;
            Color color = Color::Black();
            bool colorSet = false;   // a color property somewhere above: currentColor is it, not the host's
            bool visible = true;
            bool crisp = false;
            bool strokeFirst = false;
            // not inherited
            float opacity = 1.0f;
            bool display = true;
            std::string_view clipPath;
            Color stopColor = Color::Black();
            float stopOpacity = 1.0f;
            bool stopCurrent = false;
        };

        Style DefaultStyle()
        {
            Style s;
            s.fill.kind = PaintSpec::Kind::Color;
            s.fill.color = Color::Black();
            return s;
        }

        Style Inherit(const Style& parent)
        {
            Style s = parent;
            s.opacity = 1.0f;
            s.display = true;
            s.clipPath = {};
            s.stopColor = Color::Black();
            s.stopOpacity = 1.0f;
            s.stopCurrent = false;
            return s;
        }

        bool IsProperty(std::string_view n)
        {
            static constexpr std::string_view kProps[] = {"fill", "fill-opacity", "fill-rule", "stroke", "stroke-width", "stroke-opacity",
                                                          "stroke-linecap", "stroke-linejoin", "stroke-miterlimit", "stroke-dasharray",
                                                          "stroke-dashoffset", "opacity", "color", "display", "visibility", "clip-path",
                                                          "clip-rule", "stop-color", "stop-opacity", "shape-rendering", "paint-order"};
            for (const std::string_view p : kProps)
                if (n == p)
                    return true;
            return false;
        }

        bool Opacity(std::string_view v, float& out)
        {
            float f;
            if (!Fraction(v, f))
                return false;
            out = Saturate(f);
            return true;
        }

        void Apply(Style& s, const Style& parent, std::string_view prop, std::string_view value, const Viewport& vp)
        {
            value = Trim(value);
            const bool inherit = IEq(value, "inherit");
            float f;
            if (prop == "fill")
            {
                PaintSpec p;
                if (inherit)
                    s.fill = parent.fill;
                else if (ParsePaint(value, p))
                    s.fill = p;
            }
            else if (prop == "stroke")
            {
                PaintSpec p;
                if (inherit)
                    s.stroke = parent.stroke;
                else if (ParsePaint(value, p))
                    s.stroke = p;
            }
            else if (prop == "fill-opacity")
            {
                if (inherit)
                    s.fillOpacity = parent.fillOpacity;
                else
                    Opacity(value, s.fillOpacity);
            }
            else if (prop == "stroke-opacity")
            {
                if (inherit)
                    s.strokeOpacity = parent.strokeOpacity;
                else
                    Opacity(value, s.strokeOpacity);
            }
            else if (prop == "opacity")
            {
                if (inherit)
                    s.opacity = parent.opacity;
                else
                    Opacity(value, s.opacity);
            }
            else if (prop == "stop-opacity")
            {
                if (inherit)
                    s.stopOpacity = parent.stopOpacity;
                else
                    Opacity(value, s.stopOpacity);
            }
            else if (prop == "fill-rule" || prop == "clip-rule")
            {
                bool& rule = prop == "fill-rule" ? s.fillEvenOdd : s.clipEvenOdd;
                if (inherit)
                    rule = prop == "fill-rule" ? parent.fillEvenOdd : parent.clipEvenOdd;
                else if (value == "evenodd")
                    rule = true;
                else if (value == "nonzero")
                    rule = false;
            }
            else if (prop == "stroke-width")
            {
                if (inherit)
                    s.strokeWidth = parent.strokeWidth;
                else if (Length(value, Axis::Diagonal, vp, f) && f >= 0.0f)
                    s.strokeWidth = f;
            }
            else if (prop == "stroke-linecap")
            {
                if (inherit)
                    s.cap = parent.cap;
                else if (value == "butt")
                    s.cap = Cap::Butt;
                else if (value == "round")
                    s.cap = Cap::Round;
                else if (value == "square")
                    s.cap = Cap::Square;
            }
            else if (prop == "stroke-linejoin")
            {
                if (inherit)
                    s.join = parent.join;
                else if (value == "miter" || value == "miter-clip" || value == "arcs")
                    s.join = Join::Miter;
                else if (value == "round")
                    s.join = Join::Round;
                else if (value == "bevel")
                    s.join = Join::Bevel;
            }
            else if (prop == "stroke-miterlimit")
            {
                std::size_t i = 0;
                if (inherit)
                    s.miterLimit = parent.miterLimit;
                else if (ParseNumber(value, i, f) && i == value.size() && f >= 1.0f)
                    s.miterLimit = f;
            }
            else if (prop == "stroke-dasharray")
            {
                if (inherit)
                    s.dashes = parent.dashes;
                else if (value == "none")
                    s.dashes.clear();
                else
                {
                    std::vector<float> d;
                    std::size_t i = 0;
                    SkipWs(value, i);
                    bool ok = true;
                    while (ok && i < value.size())
                    {
                        float v;
                        ok = ParseLength(value, i, Axis::Diagonal, vp, v) && v >= 0.0f;
                        d.push_back(v);
                        SkipSep(value, i);
                    }
                    if (ok)
                        s.dashes = std::move(d);
                }
            }
            else if (prop == "stroke-dashoffset")
            {
                if (inherit)
                    s.dashOffset = parent.dashOffset;
                else if (Length(value, Axis::Diagonal, vp, f))
                    s.dashOffset = f;
            }
            else if (prop == "color")
            {
                Color c;
                if (inherit)
                {
                    s.color = parent.color;
                    s.colorSet = parent.colorSet;
                }
                else if (!IEq(value, "currentColor") && ParseColor(value, c))
                {
                    s.color = c;
                    s.colorSet = true;
                }
            }
            else if (prop == "display")
                s.display = inherit ? parent.display : value != "none";
            else if (prop == "visibility")
            {
                if (inherit)
                    s.visible = parent.visible;
                else if (value == "hidden" || value == "collapse")
                    s.visible = false;
                else if (value == "visible")
                    s.visible = true;
            }
            else if (prop == "clip-path")
                s.clipPath = inherit ? parent.clipPath : (value == "none" ? std::string_view() : value);
            else if (prop == "stop-color")
            {
                Color c;
                if (inherit)
                {
                    s.stopColor = parent.stopColor;
                    s.stopCurrent = parent.stopCurrent;
                }
                else if (IEq(value, "currentColor"))
                    s.stopCurrent = true;
                else if (ParseColor(value, c))
                {
                    s.stopColor = c;
                    s.stopCurrent = false;
                }
            }
            else if (prop == "shape-rendering")
                s.crisp = inherit ? parent.crisp : IEq(value, "crispEdges");
            else if (prop == "paint-order")
            {
                if (inherit)
                    s.strokeFirst = parent.strokeFirst;
                else
                {
                    const std::size_t st = value.find("stroke"), fl = value.find("fill");
                    s.strokeFirst = st != std::string_view::npos && (fl == std::string_view::npos || st < fl);
                }
            }
        }

        bool IsShapeName(std::string_view n)
        {
            return n == "path" || n == "rect" || n == "circle" || n == "ellipse" || n == "line" || n == "polyline" || n == "polygon";
        }

        // ------------------------------------------------------------------ the builder
        class Builder
        {
        public:
            Builder(const xml::Document& x, Tree& t) : x_(x), t_(t) {}
            bool Build(std::string* error);

        private:
            struct Group
            {
                std::size_t begin = 0;
                std::uint32_t layer = 0;
                bool layered = false;
                std::int32_t clipElement = -1;
                Affine ctm;
                float opacity = 1.0f;
            };

            const xml::Element& El(std::int32_t e) const { return x_.elements[(std::size_t)e]; }
            std::string_view Attr(std::int32_t e, std::string_view name, bool* found = nullptr) const { return x_.Attr(El(e), name, found); }
            std::int32_t Ref(std::string_view id) const
            {
                const auto it = ids_.find(id);
                return it == ids_.end() ? -1 : it->second;
            }
            std::int32_t Href(std::int32_t e) const
            {
                bool found;
                std::string_view v = Attr(e, "href", &found);
                if (!found)
                    v = Attr(e, "xlink:href");
                v = Trim(v);
                return !v.empty() && v[0] == '#' ? Ref(v.substr(1)) : -1;
            }
            std::int32_t ClipElement(std::string_view value) const
            {
                std::string_view id, rest;
                if (!UrlId(value, id, rest) || id.empty())
                    return -1;
                const std::int32_t e = Ref(id);
                return e >= 0 && El(e).name == "clipPath" ? e : -1;
            }

            void Sheets();
            bool Matches(const Selector& sel, std::size_t part, std::int32_t e) const;
            bool MatchesCompound(const Compound& c, std::int32_t e) const;
            void Cascade(std::int32_t e, const Style& parent, const Viewport& vp, Style& s);
            const Style& TreeStyle(std::int32_t e);

            bool OpenGroup(const Style& s, const Affine& ctm, Group& g);
            void CloseGroup(Group& g);
            Rect OpsBounds(std::size_t from, const Affine& inv) const;
            bool Visible(const Shape& sh, bool stroke) const;

            void Node(std::int32_t e, const Style& parent, const Affine& ctm, const Viewport& vp);
            void Children(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp);
            void Use(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp);
            void ShapeNode(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp);
            std::int32_t PathOf(std::int32_t e, const Viewport& vp);
            Paint ResolvePaint(const PaintSpec& p, const Style& s);
            std::int32_t GradientOf(std::int32_t e);
            std::int32_t ClipOf(std::int32_t clipElement, const Affine& ctm, const Rect& bbox);

            const xml::Document& x_;
            Tree& t_;
            std::vector<Rule> rules_;
            std::vector<std::vector<std::uint32_t>> matched_;
            std::vector<std::string> sheets_;
            std::unordered_map<std::string_view, std::int32_t> ids_;
            std::unordered_map<std::int32_t, std::int32_t> gradients_, paths_;
            std::unordered_map<std::int32_t, Style> treeStyles_;
            std::vector<std::int32_t> useStack_, clipStack_;
            struct Weighted
            {
                std::string_view prop, value;
                std::uint64_t key;
            };
            std::vector<Weighted> decls_;
            std::vector<Decl> styleDecls_;
            Viewport rootVp_;
        };

        void Builder::Sheets()
        {
            for (std::size_t e = 0; e < x_.elements.size(); ++e)
            {
                const xml::Element& el = x_.elements[e];
                if (el.name != "style" || el.text.empty())
                    continue;
                const std::string_view type = Trim(x_.Attr(el, "type"));
                if (!type.empty() && type != "text/css")
                    continue;
                sheets_.push_back(StripComments(el.text));
            }
            // the views into sheets_ stay valid: it does not grow after this
            for (const std::string& css : sheets_)
                ParseSheet(css, rules_);
            matched_.assign(x_.elements.size(), {});
            if (rules_.empty())
                return;
            for (std::size_t e = 0; e < x_.elements.size(); ++e)
                for (std::size_t r = 0; r < rules_.size(); ++r)
                    if (Matches(rules_[r].sel, rules_[r].sel.parts.size() - 1, (std::int32_t)e))
                        matched_[e].push_back((std::uint32_t)r);
        }

        bool Builder::MatchesCompound(const Compound& c, std::int32_t e) const
        {
            if (!c.tag.empty() && El(e).name != c.tag)
                return false;
            if (!c.id.empty() && Trim(Attr(e, "id")) != c.id)
                return false;
            if (c.classes.empty())
                return true;
            const std::string_view list = Attr(e, "class");
            for (const std::string_view cls : c.classes)
            {
                bool found = false;
                std::size_t i = 0;
                while (!found && i < list.size())
                {
                    SkipWs(list, i);
                    const std::size_t a = i;
                    while (i < list.size() && !Ws(list[i]))
                        ++i;
                    found = list.substr(a, i - a) == cls;
                }
                if (!found)
                    return false;
            }
            return true;
        }

        bool Builder::Matches(const Selector& sel, std::size_t part, std::int32_t e) const
        {
            if (!MatchesCompound(sel.parts[part], e))
                return false;
            if (part == 0)
                return true;
            std::int32_t p = El(e).parent;
            if (sel.comb[part - 1] == '>')
                return p >= 0 && Matches(sel, part - 1, p);
            for (; p >= 0; p = El(p).parent)
                if (Matches(sel, part - 1, p))
                    return true;
            return false;
        }

        // presentation attributes, then style sheet rules by specificity and order, then the style attribute; !important
        // declarations above all of them
        void Builder::Cascade(std::int32_t e, const Style& parent, const Viewport& vp, Style& s)
        {
            s = Inherit(parent);
            decls_.clear();
            const xml::Element& el = El(e);
            for (std::uint32_t a = el.attrBegin; a < el.attrEnd; ++a)
            {
                const xml::Attribute& at = x_.attributes[a];
                if (IsProperty(at.name))
                    decls_.push_back({at.name, at.value, 0});
            }
            const auto key = [](std::uint64_t level, std::uint64_t spec, std::uint64_t order) { return (level << 56) | (spec << 24) | order; };
            if ((std::size_t)e < matched_.size())
                for (const std::uint32_t r : matched_[(std::size_t)e])
                    for (const Decl& d : rules_[r].decls)
                        decls_.push_back({d.prop, d.value, key(d.important ? 3 : 1, (std::uint64_t)rules_[r].sel.specificity, (std::uint64_t)rules_[r].order)});
            bool found;
            const std::string_view style = Attr(e, "style", &found);
            if (found)
            {
                styleDecls_.clear();
                ParseDecls(style, styleDecls_);
                for (const Decl& d : styleDecls_)
                    decls_.push_back({d.prop, d.value, key(d.important ? 4 : 2, 0, 0)});
            }
            std::stable_sort(decls_.begin(), decls_.end(), [](const Weighted& a, const Weighted& b) { return a.key < b.key; });
            for (const Weighted& d : decls_)
                Apply(s, parent, d.prop, d.value, vp);
        }

        const Style& Builder::TreeStyle(std::int32_t e)
        {
            const auto it = treeStyles_.find(e);
            if (it != treeStyles_.end())
                return it->second;
            const std::int32_t p = El(e).parent;
            Style parent = p >= 0 ? TreeStyle(p) : DefaultStyle();
            Style s;
            Cascade(e, parent, rootVp_, s);
            return treeStyles_.emplace(e, std::move(s)).first->second;
        }

        bool Builder::OpenGroup(const Style& s, const Affine& ctm, Group& g)
        {
            if (!(s.opacity > 0.0f))
                return false;
            g.clipElement = s.clipPath.empty() ? -1 : ClipElement(s.clipPath);
            g.opacity = s.opacity;
            g.ctm = ctm;
            g.begin = t_.ops.size();
            g.layered = s.opacity < 1.0f || g.clipElement >= 0;
            if (g.layered)
            {
                g.layer = (std::uint32_t)t_.layers.size();
                t_.layers.push_back({s.opacity, -1});
                t_.ops.push_back({OpKind::Begin, g.layer, 0});
            }
            return true;
        }

        bool Builder::Visible(const Shape& sh, bool stroke) const
        {
            const Paint& p = stroke ? sh.stroke : sh.fill;
            const float o = stroke ? sh.strokeOpacity : sh.fillOpacity;
            if (stroke && sh.strokeStyle < 0)
                return false;
            return p.kind != PaintKind::None && o > 0.0f && (p.kind != PaintKind::Solid || p.color.a > 0.0f);
        }

        Rect Builder::OpsBounds(std::size_t from, const Affine& inv) const
        {
            Rect r;
            bool any = false;
            for (std::size_t i = from; i < t_.ops.size(); ++i)
            {
                if (t_.ops[i].kind != OpKind::Shape)
                    continue;
                const Shape& sh = t_.shapes[t_.ops[i].index];
                const Rect b = t_.paths[sh.path].bounds;
                const Affine m = inv * sh.transform;
                for (const Vec2 c : {b.min, Vec2(b.max.x, b.min.y), b.max, Vec2(b.min.x, b.max.y)})
                {
                    const Vec2 p = m.Apply(c);
                    if (!any)
                    {
                        r = Rect(p, p);
                        any = true;
                        continue;
                    }
                    r.min.x = std::min(r.min.x, p.x);
                    r.min.y = std::min(r.min.y, p.y);
                    r.max.x = std::max(r.max.x, p.x);
                    r.max.y = std::max(r.max.y, p.y);
                }
            }
            return r;
        }

        void Builder::CloseGroup(Group& g)
        {
            if (!g.layered)
                return;
            if (t_.ops.size() == g.begin + 1)
            {
                t_.ops.resize(g.begin);   // nothing inside
                return;
            }
            std::int32_t clip = -1;
            if (g.clipElement >= 0)
            {
                Affine inv;
                const Rect bbox = g.ctm.Invert(inv) ? OpsBounds(g.begin + 1, inv) : Rect();
                clip = ClipOf(g.clipElement, g.ctm, bbox);
                if (clip == -2)
                {
                    t_.ops.resize(g.begin);   // clipped away
                    return;
                }
                t_.layers[g.layer].clip = clip;
            }
            // one shape with one paint: its opacity and clip instead of a layer
            if (t_.ops.size() == g.begin + 2 && t_.ops[g.begin + 1].kind == OpKind::Shape)
            {
                Shape& sh = t_.shapes[t_.ops[g.begin + 1].index];
                if (Visible(sh, false) != Visible(sh, true) && (sh.clip < 0 || clip < 0))
                {
                    sh.fillOpacity *= g.opacity;
                    sh.strokeOpacity *= g.opacity;
                    if (clip >= 0)
                        sh.clip = clip;
                    const Op op = t_.ops[g.begin + 1];
                    t_.ops.resize(g.begin);
                    t_.ops.push_back(op);
                    return;
                }
            }
            t_.ops[g.begin].match = (std::uint32_t)t_.ops.size();
            t_.ops.push_back({OpKind::End, g.layer, (std::uint32_t)g.begin});
        }

        void Builder::Children(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp)
        {
            for (std::int32_t c = El(e).firstChild; c >= 0; c = El(c).next)
                Node(c, s, ctm, vp);
        }

        void Builder::Node(std::int32_t e, const Style& parent, const Affine& ctm, const Viewport& vp)
        {
            const std::string_view name = El(e).name;
            const bool shape = IsShapeName(name);
            if (!shape && name != "g" && name != "a" && name != "svg" && name != "use" && name != "switch")
                return;   // not drawn (defs, symbol, gradients, clipPath, style, text ...)
            Style s;
            Cascade(e, parent, vp, s);
            if (!s.display)
                return;
            const Affine m = ctm * TransformOf(Attr(e, "transform"));
            if (shape)
            {
                ShapeNode(e, s, m, vp);
                return;
            }
            if (name == "use")
            {
                Use(e, s, m, vp);
                return;
            }
            Group g;
            if (!OpenGroup(s, m, g))
                return;
            if (name == "svg")
            {
                // a nested viewport
                const float x = LengthOr(Attr(e, "x"), Axis::X, vp, 0.0f), y = LengthOr(Attr(e, "y"), Axis::Y, vp, 0.0f);
                const float w = LengthOr(Attr(e, "width"), Axis::X, vp, vp.w), h = LengthOr(Attr(e, "height"), Axis::Y, vp, vp.h);
                Rect vb;
                Viewport inner{w, h};
                Affine local = Affine::Translate(x, y);
                if (ParseViewBox(Attr(e, "viewBox"), vb))
                {
                    local = local * ViewBoxTransform(vb, ParseAspect(Attr(e, "preserveAspectRatio")), w, h);
                    inner = {vb.Width(), vb.Height()};
                }
                if (w > 0.0f && h > 0.0f)
                    Children(e, s, m * local, inner);
            }
            else if (name == "switch")
            {
                // the first child it can draw
                for (std::int32_t c = El(e).firstChild; c >= 0; c = El(c).next)
                {
                    const std::string_view cn = El(c).name;
                    bool ext = false;
                    Attr(c, "requiredExtensions", &ext);
                    if (ext || !(IsShapeName(cn) || cn == "g" || cn == "a" || cn == "svg" || cn == "use" || cn == "switch"))
                        continue;
                    Node(c, s, m, vp);
                    break;
                }
            }
            else
                Children(e, s, m, vp);
            CloseGroup(g);
        }

        void Builder::Use(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp)
        {
            const std::int32_t target = Href(e);
            if (target < 0 || useStack_.size() >= 32 || std::find(useStack_.begin(), useStack_.end(), target) != useStack_.end())
                return;
            for (std::int32_t a = El(e).parent; a >= 0; a = El(a).parent)
                if (a == target)
                    return;   // a use inside what it uses
            const float x = LengthOr(Attr(e, "x"), Axis::X, vp, 0.0f), y = LengthOr(Attr(e, "y"), Axis::Y, vp, 0.0f);
            const Affine m = ctm * Affine::Translate(x, y);
            Group g;
            if (!OpenGroup(s, m, g))
                return;
            useStack_.push_back(target);
            const std::string_view tn = El(target).name;
            if (tn == "symbol" || tn == "svg")
            {
                Style ts;
                Cascade(target, s, vp, ts);
                if (ts.display)
                {
                    bool fw, fh;
                    const std::string_view uw = Attr(e, "width", &fw), uh = Attr(e, "height", &fh);
                    const float w = fw ? LengthOr(uw, Axis::X, vp, vp.w) : LengthOr(Attr(target, "width"), Axis::X, vp, vp.w);
                    const float h = fh ? LengthOr(uh, Axis::Y, vp, vp.h) : LengthOr(Attr(target, "height"), Axis::Y, vp, vp.h);
                    Affine local = TransformOf(Attr(target, "transform"));
                    if (tn == "svg")
                        local = local * Affine::Translate(LengthOr(Attr(target, "x"), Axis::X, vp, 0.0f), LengthOr(Attr(target, "y"), Axis::Y, vp, 0.0f));
                    Rect vb;
                    Viewport inner{w, h};
                    if (ParseViewBox(Attr(target, "viewBox"), vb))
                    {
                        local = local * ViewBoxTransform(vb, ParseAspect(Attr(target, "preserveAspectRatio")), w, h);
                        inner = {vb.Width(), vb.Height()};
                    }
                    Group inside;
                    if (w > 0.0f && h > 0.0f && OpenGroup(ts, m * local, inside))
                    {
                        Children(target, ts, m * local, inner);
                        CloseGroup(inside);
                    }
                }
            }
            else
                Node(target, s, m, vp);
            useStack_.pop_back();
            CloseGroup(g);
        }

        std::int32_t Builder::PathOf(std::int32_t e, const Viewport& vp)
        {
            const auto it = paths_.find(e);
            if (it != paths_.end())
                return it->second;
            PathData pd;
            const std::string_view n = El(e).name;
            const auto len = [&](const char* attr, Axis axis, float def) { return LengthOr(Attr(e, attr), axis, vp, def); };
            if (n == "path")
                ParsePathData(Attr(e, "d"), pd);   // drawn up to an error
            else if (n == "rect")
            {
                const float w = len("width", Axis::X, 0.0f), h = len("height", Axis::Y, 0.0f);
                if (w > 0.0f && h > 0.0f)
                {
                    float rx = -1.0f, ry = -1.0f;
                    Length(Attr(e, "rx"), Axis::X, vp, rx);
                    Length(Attr(e, "ry"), Axis::Y, vp, ry);
                    if (rx < 0.0f)
                        rx = ry;   // auto: the other one
                    if (ry < 0.0f)
                        ry = rx;
                    AddRect(pd, len("x", Axis::X, 0.0f), len("y", Axis::Y, 0.0f), w, h, std::max(rx, 0.0f), std::max(ry, 0.0f));
                }
            }
            else if (n == "circle")
            {
                const float r = len("r", Axis::Diagonal, 0.0f);
                if (r > 0.0f)
                    AddEllipse(pd, len("cx", Axis::X, 0.0f), len("cy", Axis::Y, 0.0f), r, r);
            }
            else if (n == "ellipse")
            {
                float rx = -1.0f, ry = -1.0f;
                Length(Attr(e, "rx"), Axis::X, vp, rx);
                Length(Attr(e, "ry"), Axis::Y, vp, ry);
                if (rx < 0.0f)
                    rx = ry;
                if (ry < 0.0f)
                    ry = rx;
                if (rx > 0.0f && ry > 0.0f)
                    AddEllipse(pd, len("cx", Axis::X, 0.0f), len("cy", Axis::Y, 0.0f), rx, ry);
            }
            else if (n == "line")
                AddPoints(pd, {len("x1", Axis::X, 0.0f), len("y1", Axis::Y, 0.0f), len("x2", Axis::X, 0.0f), len("y2", Axis::Y, 0.0f)}, false);
            else if (n == "polyline" || n == "polygon")
            {
                std::vector<float> xy;
                Numbers(Attr(e, "points"), xy);   // the points before an error
                if (xy.size() >= 4)
                    AddPoints(pd, xy, n == "polygon");
            }
            std::int32_t index = -1;
            if (!pd.verbs.empty())
            {
                ComputeBounds(pd);
                index = (std::int32_t)t_.paths.size();
                t_.paths.push_back(std::move(pd));
            }
            paths_.emplace(e, index);
            return index;
        }

        std::int32_t Builder::GradientOf(std::int32_t e)
        {
            const auto it = gradients_.find(e);
            if (it != gradients_.end())
                return it->second;
            gradients_[e] = -1;   // while it is built: a reference cycle ends here
            // the gradients it refers to: attributes from the first one that has them, stops likewise
            std::vector<std::int32_t> chain{e};
            for (std::int32_t r = Href(e); r >= 0 && chain.size() < 16; r = Href(r))
            {
                const std::string_view rn = El(r).name;
                if ((rn != "linearGradient" && rn != "radialGradient") || std::find(chain.begin(), chain.end(), r) != chain.end())
                    break;
                chain.push_back(r);
            }
            const auto attr = [&](std::string_view name, std::string_view& out) {
                for (const std::int32_t c : chain)
                {
                    bool found;
                    const std::string_view v = Attr(c, name, &found);
                    if (found)
                    {
                        out = v;
                        return true;
                    }
                }
                return false;
            };
            Gradient g;
            g.radial = El(e).name == "radialGradient";
            std::string_view v;
            if (attr("gradientUnits", v))
                g.userSpace = Trim(v) == "userSpaceOnUse";
            if (attr("gradientTransform", v))
                g.transform = TransformOf(v);
            if (attr("spreadMethod", v))
            {
                v = Trim(v);
                g.spread = v == "reflect" ? Spread::Reflect : (v == "repeat" ? Spread::Repeat : Spread::Pad);
            }
            const Viewport& vp = rootVp_;
            // fractions of the bounding box, or lengths in user units
            const auto coord = [&](const char* name, Axis axis, float boxDefault, float userDefault) {
                std::string_view a;
                float out = g.userSpace ? userDefault : boxDefault;
                if (attr(name, a))
                {
                    if (g.userSpace)
                        out = LengthOr(a, axis, vp, out);
                    else
                        Fraction(a, out);
                }
                return out;
            };
            if (!g.radial)
            {
                g.x1 = coord("x1", Axis::X, 0.0f, 0.0f);
                g.y1 = coord("y1", Axis::Y, 0.0f, 0.0f);
                g.x2 = coord("x2", Axis::X, 1.0f, vp.w);
                g.y2 = coord("y2", Axis::Y, 0.0f, 0.0f);
            }
            else
            {
                g.cx = coord("cx", Axis::X, 0.5f, 0.5f * vp.w);
                g.cy = coord("cy", Axis::Y, 0.5f, 0.5f * vp.h);
                g.r = coord("r", Axis::Diagonal, 0.5f, 50.0f * PercentOf(Axis::Diagonal, vp));
                g.fx = coord("fx", Axis::X, g.cx, g.cx);
                g.fy = coord("fy", Axis::Y, g.cy, g.cy);
                g.fr = coord("fr", Axis::Diagonal, 0.0f, 0.0f);
            }
            for (const std::int32_t c : chain)
            {
                bool any = false;
                for (std::int32_t k = El(c).firstChild; k >= 0; k = El(k).next)
                {
                    if (El(k).name != "stop")
                        continue;
                    any = true;
                    const Style& ss = TreeStyle(k);
                    Stop st;
                    float off = 0.0f;
                    Fraction(Attr(k, "offset"), off);
                    st.offset = std::max(Saturate(off), g.stops.empty() ? 0.0f : g.stops.back().offset);
                    if (ss.stopCurrent && !ss.colorSet)
                    {
                        st.current = true;
                        st.color = Color(1.0f, 1.0f, 1.0f, ss.stopOpacity);
                        t_.usesCurrent = true;
                    }
                    else
                    {
                        st.color = ss.stopCurrent ? ss.color : ss.stopColor;
                        st.color.a *= ss.stopOpacity;
                    }
                    g.stops.push_back(st);
                }
                if (any)
                    break;
            }
            const std::int32_t index = (std::int32_t)t_.gradients.size();
            t_.gradients.push_back(std::move(g));
            gradients_[e] = index;
            return index;
        }

        Paint Builder::ResolvePaint(const PaintSpec& p, const Style& s)
        {
            using K = PaintSpec::Kind;
            Paint out;
            K kind = p.kind;
            Color color = p.color;
            if (kind == K::Url)
            {
                const std::int32_t el = p.url.empty() ? -1 : Ref(p.url);
                const std::string_view n = el >= 0 ? El(el).name : std::string_view();
                if (n == "linearGradient" || n == "radialGradient")
                {
                    const std::int32_t g = GradientOf(el);
                    if (g < 0 || t_.gradients[(std::size_t)g].stops.empty())
                        return out;   // a gradient without stops paints nothing
                    out.kind = PaintKind::Gradient;
                    out.gradient = g;
                    return out;
                }
                kind = p.fallback;   // not a paint server it can draw: the fallback
                color = p.fallbackColor;
            }
            switch (kind)
            {
            case K::Color:
                out.kind = PaintKind::Solid;
                out.color = color;
                break;
            case K::Current:
                if (s.colorSet)
                {
                    out.kind = PaintKind::Solid;
                    out.color = s.color;
                }
                else
                {
                    out.kind = PaintKind::Current;
                    out.color = Color::White();
                    t_.usesCurrent = true;
                }
                break;
            default:
                break;
            }
            return out;
        }

        // -1: no clip; -2: it clips everything away
        std::int32_t Builder::ClipOf(std::int32_t el, const Affine& ctm, const Rect& bbox)
        {
            if (clipStack_.size() >= 8 || std::find(clipStack_.begin(), clipStack_.end(), el) != clipStack_.end())
                return -1;
            Affine units;
            if (Trim(Attr(el, "clipPathUnits")) == "objectBoundingBox")
            {
                if (!(bbox.Width() > 0.0f) || !(bbox.Height() > 0.0f))
                    return -2;
                units = Affine{bbox.Width(), 0.0f, 0.0f, bbox.Height(), bbox.min.x, bbox.min.y};
            }
            clipStack_.push_back(el);
            const Affine base = ctm * TransformOf(Attr(el, "transform")) * units;
            Clip clip;
            for (std::int32_t c = El(el).firstChild; c >= 0; c = El(c).next)
            {
                const std::string_view cn = El(c).name;
                if (!IsShapeName(cn) && cn != "use")
                    continue;
                const Style& cs = TreeStyle(c);
                if (!cs.display || !cs.visible)
                    continue;
                const Affine m = base * TransformOf(Attr(c, "transform"));
                if (cn == "use")
                {
                    const std::int32_t target = Href(c);
                    if (target < 0 || !IsShapeName(El(target).name))
                        continue;
                    const Style& ts = TreeStyle(target);
                    const std::int32_t path = ts.display ? PathOf(target, rootVp_) : -1;
                    if (path < 0)
                        continue;
                    const Affine at = Affine::Translate(LengthOr(Attr(c, "x"), Axis::X, rootVp_, 0.0f), LengthOr(Attr(c, "y"), Axis::Y, rootVp_, 0.0f));
                    clip.items.push_back({(std::uint32_t)path, m * at * TransformOf(Attr(target, "transform")), ts.clipEvenOdd});
                    continue;
                }
                const std::int32_t path = PathOf(c, rootVp_);
                if (path >= 0)
                    clip.items.push_back({(std::uint32_t)path, m, cs.clipEvenOdd});
            }
            // the clip path's own clip path: the intersection
            const Style& own = TreeStyle(el);
            const std::int32_t nestedEl = own.clipPath.empty() ? -1 : ClipElement(own.clipPath);
            if (nestedEl >= 0)
            {
                const std::int32_t nested = ClipOf(nestedEl, ctm, bbox);
                if (nested == -2)
                {
                    clipStack_.pop_back();
                    return -2;
                }
                clip.clip = nested;
            }
            clipStack_.pop_back();
            if (clip.items.empty())
                return -2;
            t_.clips.push_back(std::move(clip));
            return (std::int32_t)t_.clips.size() - 1;
        }

        void Builder::ShapeNode(std::int32_t e, const Style& s, const Affine& ctm, const Viewport& vp)
        {
            if (!s.visible || !(s.opacity > 0.0f))
                return;
            const std::int32_t path = PathOf(e, vp);
            if (path < 0)
                return;
            Shape sh;
            sh.path = (std::uint32_t)path;
            sh.transform = ctm;
            sh.evenOdd = s.fillEvenOdd;
            sh.crisp = s.crisp;
            sh.strokeFirst = s.strokeFirst;
            sh.fill = ResolvePaint(s.fill, s);
            sh.fillOpacity = s.fillOpacity;
            if (s.stroke.kind != PaintSpec::Kind::None && s.strokeWidth > 0.0f)
            {
                sh.stroke = ResolvePaint(s.stroke, s);
                sh.strokeOpacity = s.strokeOpacity;
                StrokeStyle st;
                st.width = s.strokeWidth;
                st.cap = s.cap;
                st.join = s.join;
                st.miterLimit = s.miterLimit;
                float total = 0.0f;
                for (const float d : s.dashes)
                    total += d;
                if (total > 0.0f)
                {
                    st.dashes = s.dashes;
                    if (st.dashes.size() & 1)
                        st.dashes.insert(st.dashes.end(), s.dashes.begin(), s.dashes.end());   // an odd list repeats
                    st.dashOffset = s.dashOffset;
                }
                sh.strokeStyle = (std::int32_t)t_.strokes.size();
                t_.strokes.push_back(std::move(st));
            }
            const bool fill = Visible(sh, false), stroke = Visible(sh, true);
            if (!fill && !stroke)
                return;
            std::int32_t clip = -1;
            if (!s.clipPath.empty())
            {
                const std::int32_t ce = ClipElement(s.clipPath);
                if (ce >= 0)
                {
                    clip = ClipOf(ce, ctm, t_.paths[(std::size_t)path].bounds);
                    if (clip == -2)
                        return;
                }
            }
            const std::uint32_t index = (std::uint32_t)t_.shapes.size();
            if (fill && stroke && (s.opacity < 1.0f || clip >= 0))
            {
                // a fill and a stroke under one opacity or clip: a layer, so the stroke does not show the fill through it
                const std::uint32_t layer = (std::uint32_t)t_.layers.size();
                t_.layers.push_back({s.opacity, clip});
                const std::size_t begin = t_.ops.size();
                t_.ops.push_back({OpKind::Begin, layer, 0});
                t_.shapes.push_back(sh);
                t_.ops.push_back({OpKind::Shape, index, 0});
                t_.ops[begin].match = (std::uint32_t)t_.ops.size();
                t_.ops.push_back({OpKind::End, layer, (std::uint32_t)begin});
                return;
            }
            sh.fillOpacity *= s.opacity;
            sh.strokeOpacity *= s.opacity;
            sh.clip = clip;
            t_.shapes.push_back(sh);
            t_.ops.push_back({OpKind::Shape, index, 0});
        }

        bool Builder::Build(std::string* error)
        {
            const xml::Element& root = El(0);
            if (root.name != "svg")
            {
                if (error)
                    *error = "not an SVG document (the root element is <" + std::string(root.name) + ">)";
                return false;
            }
            for (std::size_t e = 0; e < x_.elements.size(); ++e)
            {
                bool found;
                const std::string_view id = Trim(x_.Attr(x_.elements[e], "id", &found));
                if (found && !id.empty())
                    ids_.emplace(id, (std::int32_t)e);   // the first element with an id keeps it
            }
            Sheets();

            // the root's size: width and height in absolute units, else the viewBox's
            Rect vb;
            const bool hasVb = ParseViewBox(Attr(0, "viewBox"), vb);
            const Viewport none{0.0f, 0.0f};
            const auto absolute = [&](std::string_view v, float& out) {
                v = Trim(v);
                return !v.empty() && v.back() != '%' && Length(v, Axis::X, none, out) && out > 0.0f;
            };
            float w = 0.0f, h = 0.0f;
            const bool hw = absolute(Attr(0, "width"), w), hh = absolute(Attr(0, "height"), h);
            if (hasVb)
            {
                if (!hw && !hh)
                {
                    w = vb.Width();
                    h = vb.Height();
                }
                else if (!hw)
                    w = h * vb.Width() / vb.Height();
                else if (!hh)
                    h = w * vb.Height() / vb.Width();
            }
            else
            {
                if (!hw)
                    w = 300.0f;   // a browser's size for an image that gives none
                if (!hh)
                    h = 150.0f;
            }
            t_.width = w;
            t_.height = h;
            if (hasVb)
                t_.viewBox = vb;
            const Aspect a = ParseAspect(Attr(0, "preserveAspectRatio"));
            t_.alignNone = a.none;
            t_.slice = a.slice;
            t_.alignX = a.x;
            t_.alignY = a.y;
            rootVp_ = hasVb ? Viewport{vb.Width(), vb.Height()} : Viewport{w, h};

            Style s;
            Cascade(0, DefaultStyle(), rootVp_, s);
            if (s.display)
            {
                const Affine m = TransformOf(Attr(0, "transform"));
                Group g;
                if (OpenGroup(s, m, g))
                {
                    Children(0, s, m, rootVp_);
                    CloseGroup(g);
                }
            }

            // monochrome: every paint that draws is currentColor
            bool any = false, other = false;
            for (const Op& op : t_.ops)
            {
                if (op.kind != OpKind::Shape)
                    continue;
                const Shape& sh = t_.shapes[op.index];
                for (int k = 0; k < 2; ++k)
                    if (Visible(sh, k == 1))
                    {
                        any = true;
                        other = other || (k == 1 ? sh.stroke : sh.fill).kind != PaintKind::Current;
                    }
            }
            t_.onlyCurrent = any && !other;
            return true;
        }
    }

    Affine ViewTransform(const Tree& tree, float width, float height)
    {
        const Rect vb = tree.viewBox.Empty() ? Rect(0.0f, 0.0f, tree.width, tree.height) : tree.viewBox;
        Aspect a;
        a.none = tree.alignNone;
        a.slice = tree.slice;
        a.x = tree.alignX;
        a.y = tree.alignY;
        return ViewBoxTransform(vb, a, width, height);
    }

    bool BuildTree(std::string_view text, Tree& tree, std::string* error)
    {
        xml::Document x;
        if (!xml::Parse(text, x, error))
            return false;
        Builder b(x, tree);
        return b.Build(error);
    }
}

namespace esia::svg
{
    Document Document::Parse(std::string_view text, std::string* error)
    {
        // not make_shared: its control block compares type_info, and MinGW's libstdc++ then defines that comparison
        // twice (inline here and in its own tinfo.o: a duplicate symbol for lld)
        std::shared_ptr<detail::Tree> tree(new detail::Tree());
        if (!detail::BuildTree(text, *tree, error))
            return {};
        static std::atomic<std::uint64_t> next{1};
        tree->id = next.fetch_add(1);
        Document d;
        d.tree_ = std::move(tree);
        return d;
    }

    Document Document::Load(const std::string& path, std::string* error)
    {
        std::FILE* f = std::fopen(path.c_str(), "rb");
        if (!f)
        {
            if (error)
                *error = "cannot open " + path;
            return {};
        }
        std::string text;
        char buf[1 << 16];
        for (std::size_t n; (n = std::fread(buf, 1, sizeof(buf), f)) > 0;)
            text.append(buf, n);
        std::fclose(f);
        return Parse(text, error);
    }

    bool Document::Empty() const { return !tree_; }
    Vec2 Document::Size() const { return tree_ ? Vec2(tree_->width, tree_->height) : Vec2(); }
    bool Document::Monochrome() const { return tree_ && tree_->onlyCurrent; }
    bool Document::UsesCurrentColor() const { return tree_ && tree_->usesCurrent; }
    std::uint64_t Document::Id() const { return tree_ ? tree_->id : 0; }

    bool Rasterize(const Document& doc, int width, int height, const RasterOptions& options, Image& out)
    {
        out.width = out.height = 0;
        out.pixels.clear();   // its memory is used again
        const detail::Tree* t = doc.Data();
        if (!t || width <= 0 || height <= 0 || width > 16384 || height > 16384 || options.border < 0 || options.border > 256)
            return false;
        const Rect& vp = options.viewport;
        if (!std::isfinite(vp.min.x) || !std::isfinite(vp.min.y) || !std::isfinite(vp.max.x) || !std::isfinite(vp.max.y))
            return false;
        return detail::Render(*t, width, height, options, out);
    }
}
