// Esia - SVG geometry (svg_internal.hpp): numbers and path data, the basic shapes as paths, exact bounds, flattening
// within a tolerance in pixels, and the stroker.
//
// Strokes are made in the path's user units and mapped to pixels after (a stroke scaled unevenly stays the shape it is
// in its units), as pieces whose union is the stroke: a band along every run of segments whose inner side does not
// fold over, a wedge at a joint between runs, caps at open ends. All of them wind the same way, so the non-zero rule
// fills their union - and the rasterizer resolves that union exactly where pieces overlap.
#include "svg_internal.hpp"
#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace esia::svg::detail
{
    // ------------------------------------------------------------------ transforms
    bool Affine::Invert(Affine& out) const
    {
        const double det = (double)a * d - (double)b * c;
        if (!std::isfinite(det) || det == 0.0)
            return false;
        const double inv = 1.0 / det;
        out.a = (float)(d * inv);
        out.b = (float)(-b * inv);
        out.c = (float)(-c * inv);
        out.d = (float)(a * inv);
        out.e = (float)(((double)c * f - (double)d * e) * inv);
        out.f = (float)(((double)b * e - (double)a * f) * inv);
        return std::isfinite(out.a) && std::isfinite(out.b) && std::isfinite(out.c) && std::isfinite(out.d) && std::isfinite(out.e) &&
               std::isfinite(out.f);
    }

    float Affine::MaxScale() const
    {
        // s1^2 + s2^2 = |M|^2 (Frobenius), s1 s2 = |det|: s1^2 = (p + sqrt(p^2 - 4 det^2)) / 2
        const double p = (double)a * a + (double)b * b + (double)c * c + (double)d * d;
        const double det = (double)a * d - (double)b * c;
        const double s2 = 0.5 * (p + std::sqrt(std::max(0.0, p * p - 4.0 * det * det)));
        return (float)std::sqrt(std::max(0.0, s2));
    }

    Affine Affine::Rotate(float rad)
    {
        const float cs = std::cos(rad), sn = std::sin(rad);
        return {cs, sn, -sn, cs, 0.0f, 0.0f};
    }

    // ------------------------------------------------------------------ numbers
    bool ParseNumber(std::string_view s, std::size_t& i, float& out)
    {
        const std::size_t n = s.size();
        std::size_t k = i;
        bool neg = false;
        if (k < n && (s[k] == '+' || s[k] == '-'))
        {
            neg = s[k] == '-';
            ++k;
        }
        std::uint64_t mant = 0;
        int exp10 = 0;
        bool any = false;
        constexpr std::uint64_t kRoom = 100000000000000000ull;   // 1e17: room for one more digit
        while (k < n && s[k] >= '0' && s[k] <= '9')
        {
            any = true;
            if (mant < kRoom)
                mant = mant * 10 + (std::uint64_t)(s[k] - '0');
            else
                ++exp10;
            ++k;
        }
        if (k < n && s[k] == '.')
        {
            std::size_t m = k + 1;
            bool frac = false;
            while (m < n && s[m] >= '0' && s[m] <= '9')
            {
                frac = true;
                if (mant < kRoom)
                {
                    mant = mant * 10 + (std::uint64_t)(s[m] - '0');
                    --exp10;
                }
                ++m;
            }
            if (any || frac)
            {
                any = true;
                k = m;
            }
        }
        if (!any)
            return false;
        if (k < n && (s[k] == 'e' || s[k] == 'E'))
        {
            // an exponent only when digits follow ("1em" is a length in ems)
            std::size_t m = k + 1;
            bool eneg = false;
            if (m < n && (s[m] == '+' || s[m] == '-'))
            {
                eneg = s[m] == '-';
                ++m;
            }
            if (m < n && s[m] >= '0' && s[m] <= '9')
            {
                int e = 0;
                while (m < n && s[m] >= '0' && s[m] <= '9')
                {
                    if (e < 100000)
                        e = e * 10 + (s[m] - '0');
                    ++m;
                }
                exp10 += eneg ? -e : e;
                k = m;
            }
        }
        static constexpr double kPow10[23] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
                                              1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
        double v = (double)mant;
        if (v != 0.0 && exp10 != 0)
        {
            const int ae = exp10 < 0 ? -exp10 : exp10;
            const double scale = ae <= 22 ? kPow10[ae] : std::pow(10.0, (double)ae);
            v = exp10 > 0 ? v * scale : v / scale;
        }
        v = std::min(v, 3.0e38);
        out = (float)(neg ? -v : v);
        i = k;
        return true;
    }

    // ------------------------------------------------------------------ path data
    namespace
    {
        bool Wsp(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }

        void Push(PathData& p, Verb v, std::initializer_list<float> xs)
        {
            p.verbs.push_back(v);
            p.pts.insert(p.pts.end(), xs.begin(), xs.end());
        }

        double Angle(double ux, double uy, double vx, double vy) { return std::atan2(ux * vy - uy * vx, ux * vx + uy * vy); }

        // SVG's arc from (x1, y1) to (x2, y2) (implementation notes F.6.5): its center parameters, or a line where it
        // has none
        void AddArc(PathData& p, Vec2 from, float rxIn, float ryIn, float angleDeg, bool large, bool sweep, Vec2 to)
        {
            if (from.x == to.x && from.y == to.y)
                return;   // the same point: the arc is left out
            double rx = std::fabs((double)rxIn), ry = std::fabs((double)ryIn);
            if (rx == 0.0 || ry == 0.0)
            {
                Push(p, Verb::Line, {to.x, to.y});
                return;
            }
            const double phi = std::fmod((double)angleDeg, 360.0) * (3.14159265358979323846 / 180.0);
            const double cs = std::cos(phi), sn = std::sin(phi);
            const double dx2 = 0.5 * ((double)from.x - to.x), dy2 = 0.5 * ((double)from.y - to.y);
            const double x1p = cs * dx2 + sn * dy2, y1p = -sn * dx2 + cs * dy2;
            const double lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
            if (lambda > 1.0)
            {
                const double k = std::sqrt(lambda);
                rx *= k;
                ry *= k;
            }
            const double rx2 = rx * rx, ry2 = ry * ry;
            const double num = rx2 * ry2 - rx2 * y1p * y1p - ry2 * x1p * x1p;
            const double den = rx2 * y1p * y1p + ry2 * x1p * x1p;
            double coef = den > 0.0 ? std::sqrt(std::max(0.0, num / den)) : 0.0;
            if (large == sweep)
                coef = -coef;
            const double cxp = coef * rx * y1p / ry, cyp = -coef * ry * x1p / rx;
            const double cx = cs * cxp - sn * cyp + 0.5 * ((double)from.x + to.x);
            const double cy = sn * cxp + cs * cyp + 0.5 * ((double)from.y + to.y);
            const double ux = (x1p - cxp) / rx, uy = (y1p - cyp) / ry;
            const double vx = (-x1p - cxp) / rx, vy = (-y1p - cyp) / ry;
            const double theta = Angle(1.0, 0.0, ux, uy);
            double delta = Angle(ux, uy, vx, vy);
            const double tau = 2.0 * 3.14159265358979323846;
            if (!sweep && delta > 0.0)
                delta -= tau;
            else if (sweep && delta < 0.0)
                delta += tau;
            Push(p, Verb::Arc, {(float)cx, (float)cy, (float)rx, (float)ry, (float)phi, (float)theta, (float)delta, to.x, to.y});
        }
    }

    bool ParsePathData(std::string_view d, PathData& out)
    {
        const std::size_t n = d.size();
        std::size_t i = 0;
        Vec2 cur(0.0f, 0.0f), start(0.0f, 0.0f), ctrl(0.0f, 0.0f);
        char cmd = 0, prev = 0;
        bool begun = false, pendingMove = false;
        const auto skipWsp = [&] {
            while (i < n && Wsp(d[i]))
                ++i;
        };
        const auto sep = [&] {
            skipWsp();
            if (i < n && d[i] == ',')
            {
                ++i;
                skipWsp();
            }
        };
        const auto num = [&](float& v) {
            sep();
            return ParseNumber(d, i, v);
        };
        const auto flag = [&](bool& f) {
            sep();
            if (i < n && (d[i] == '0' || d[i] == '1'))
            {
                f = d[i++] == '1';
                return true;
            }
            return false;
        };
        // a drawing command after a close starts its subpath where the closed one started
        const auto moveIfPending = [&] {
            if (pendingMove)
            {
                Push(out, Verb::Move, {cur.x, cur.y});
                pendingMove = false;
            }
        };

        for (;;)
        {
            skipWsp();
            if (i >= n)
                return true;
            const char c = d[i];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            {
                cmd = c;
                ++i;
            }
            else if (c == ',' && cmd != 0 && cmd != 'Z' && cmd != 'z')
            {
                // a comma between parameter sets ("L 1 2, 3 4")
                ++i;
                continue;
            }
            else if (cmd == 0 || cmd == 'Z' || cmd == 'z')
                return false;   // numbers before any command, or after a close
            else if (cmd == 'M')
                cmd = 'L';   // pairs after a moveto are linetos
            else if (cmd == 'm')
                cmd = 'l';
            if (!begun && cmd != 'M' && cmd != 'm')
                return false;   // path data starts with a moveto
            const bool rel = cmd >= 'a' && cmd <= 'z';
            const Vec2 base = rel ? cur : Vec2(0.0f, 0.0f);
            switch (cmd)
            {
            case 'M':
            case 'm':
            {
                float x, y;
                if (!num(x) || !num(y))
                    return false;
                cur = base + Vec2(x, y);
                start = cur;
                Push(out, Verb::Move, {cur.x, cur.y});
                begun = true;
                pendingMove = false;
                break;
            }
            case 'L':
            case 'l':
            {
                float x, y;
                if (!num(x) || !num(y))
                    return false;
                moveIfPending();
                cur = base + Vec2(x, y);
                Push(out, Verb::Line, {cur.x, cur.y});
                break;
            }
            case 'H':
            case 'h':
            {
                float x;
                if (!num(x))
                    return false;
                moveIfPending();
                cur.x = (rel ? cur.x : 0.0f) + x;
                Push(out, Verb::Line, {cur.x, cur.y});
                break;
            }
            case 'V':
            case 'v':
            {
                float y;
                if (!num(y))
                    return false;
                moveIfPending();
                cur.y = (rel ? cur.y : 0.0f) + y;
                Push(out, Verb::Line, {cur.x, cur.y});
                break;
            }
            case 'C':
            case 'c':
            {
                float x1, y1, x2, y2, x, y;
                if (!num(x1) || !num(y1) || !num(x2) || !num(y2) || !num(x) || !num(y))
                    return false;
                moveIfPending();
                const Vec2 c1 = base + Vec2(x1, y1), c2 = base + Vec2(x2, y2);
                cur = base + Vec2(x, y);
                Push(out, Verb::Cubic, {c1.x, c1.y, c2.x, c2.y, cur.x, cur.y});
                ctrl = c2;
                break;
            }
            case 'S':
            case 's':
            {
                float x2, y2, x, y;
                if (!num(x2) || !num(y2) || !num(x) || !num(y))
                    return false;
                moveIfPending();
                const bool smooth = prev == 'C' || prev == 'c' || prev == 'S' || prev == 's';
                const Vec2 c1 = smooth ? cur * 2.0f - ctrl : cur;
                const Vec2 c2 = base + Vec2(x2, y2);
                cur = base + Vec2(x, y);
                Push(out, Verb::Cubic, {c1.x, c1.y, c2.x, c2.y, cur.x, cur.y});
                ctrl = c2;
                break;
            }
            case 'Q':
            case 'q':
            {
                float x1, y1, x, y;
                if (!num(x1) || !num(y1) || !num(x) || !num(y))
                    return false;
                moveIfPending();
                const Vec2 c1 = base + Vec2(x1, y1);
                cur = base + Vec2(x, y);
                Push(out, Verb::Quad, {c1.x, c1.y, cur.x, cur.y});
                ctrl = c1;
                break;
            }
            case 'T':
            case 't':
            {
                float x, y;
                if (!num(x) || !num(y))
                    return false;
                moveIfPending();
                const bool smooth = prev == 'Q' || prev == 'q' || prev == 'T' || prev == 't';
                const Vec2 c1 = smooth ? cur * 2.0f - ctrl : cur;
                cur = base + Vec2(x, y);
                Push(out, Verb::Quad, {c1.x, c1.y, cur.x, cur.y});
                ctrl = c1;
                break;
            }
            case 'A':
            case 'a':
            {
                float rx, ry, rot, x, y;
                bool large, sweep;
                if (!num(rx) || !num(ry) || !num(rot) || !flag(large) || !flag(sweep) || !num(x) || !num(y))
                    return false;
                moveIfPending();
                const Vec2 to = base + Vec2(x, y);
                AddArc(out, cur, rx, ry, rot, large, sweep, to);
                cur = to;
                break;
            }
            case 'Z':
            case 'z':
                moveIfPending();
                Push(out, Verb::Close, {});
                cur = start;
                pendingMove = true;
                break;
            default:
                return false;   // not a path command
            }
            prev = cmd;
        }
    }

    // ------------------------------------------------------------------ shapes
    void AddRect(PathData& p, float x, float y, float w, float h, float rx, float ry)
    {
        rx = std::min(std::max(rx, 0.0f), 0.5f * w);
        ry = std::min(std::max(ry, 0.0f), 0.5f * h);
        if (rx <= 0.0f || ry <= 0.0f)
        {
            Push(p, Verb::Move, {x, y});
            Push(p, Verb::Line, {x + w, y});
            Push(p, Verb::Line, {x + w, y + h});
            Push(p, Verb::Line, {x, y + h});
            Push(p, Verb::Close, {});
            return;
        }
        // SVG's outline: from (x + rx, y) clockwise, the corners as quarter arcs
        const float h2 = 1.5707963267948966f, pi = 3.14159265358979323846f;
        Push(p, Verb::Move, {x + rx, y});
        Push(p, Verb::Line, {x + w - rx, y});
        Push(p, Verb::Arc, {x + w - rx, y + ry, rx, ry, 0.0f, -h2, h2, x + w, y + ry});
        Push(p, Verb::Line, {x + w, y + h - ry});
        Push(p, Verb::Arc, {x + w - rx, y + h - ry, rx, ry, 0.0f, 0.0f, h2, x + w - rx, y + h});
        Push(p, Verb::Line, {x + rx, y + h});
        Push(p, Verb::Arc, {x + rx, y + h - ry, rx, ry, 0.0f, h2, h2, x, y + h - ry});
        Push(p, Verb::Line, {x, y + ry});
        Push(p, Verb::Arc, {x + rx, y + ry, rx, ry, 0.0f, pi, h2, x + rx, y});
        Push(p, Verb::Close, {});
    }

    void AddEllipse(PathData& p, float cx, float cy, float rx, float ry)
    {
        // from (cx + rx, cy) clockwise, as SVG 2 defines it
        Push(p, Verb::Move, {cx + rx, cy});
        Push(p, Verb::Arc, {cx, cy, rx, ry, 0.0f, 0.0f, 6.2831853071795865f, cx + rx, cy});
        Push(p, Verb::Close, {});
    }

    void AddPoints(PathData& p, const std::vector<float>& xy, bool close)
    {
        const std::size_t pairs = xy.size() / 2;
        if (pairs < 1)
            return;
        Push(p, Verb::Move, {xy[0], xy[1]});
        for (std::size_t k = 1; k < pairs; ++k)
            Push(p, Verb::Line, {xy[2 * k], xy[2 * k + 1]});
        if (close)
            Push(p, Verb::Close, {});
    }

    // ------------------------------------------------------------------ bounds
    namespace
    {
        void Include(Rect& r, bool& any, Vec2 q)
        {
            if (!any)
            {
                r = Rect(q, q);
                any = true;
                return;
            }
            r.min.x = std::min(r.min.x, q.x);
            r.min.y = std::min(r.min.y, q.y);
            r.max.x = std::max(r.max.x, q.x);
            r.max.y = std::max(r.max.y, q.y);
        }

        // roots in (0, 1) of a t^2 + b t + c
        int Roots(double a, double b, double c, double* t)
        {
            int n = 0;
            const auto keep = [&](double v) {
                if (v > 0.0 && v < 1.0)
                    t[n++] = v;
            };
            if (std::fabs(a) < 1e-12)
            {
                if (std::fabs(b) > 1e-12)
                    keep(-c / b);
                return n;
            }
            const double disc = b * b - 4.0 * a * c;
            if (disc < 0.0)
                return 0;
            const double sq = std::sqrt(disc);
            keep((-b + sq) / (2.0 * a));
            keep((-b - sq) / (2.0 * a));
            return n;
        }

        Vec2 CubicAt(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t)
        {
            const float u = 1.0f - t;
            const float w0 = u * u * u, w1 = 3.0f * u * u * t, w2 = 3.0f * u * t * t, w3 = t * t * t;
            return {w0 * p0.x + w1 * p1.x + w2 * p2.x + w3 * p3.x, w0 * p0.y + w1 * p1.y + w2 * p2.y + w3 * p3.y};
        }

        Vec2 QuadAt(Vec2 p0, Vec2 p1, Vec2 p2, float t)
        {
            const float u = 1.0f - t;
            return {u * u * p0.x + 2.0f * u * t * p1.x + t * t * p2.x, u * u * p0.y + 2.0f * u * t * p1.y + t * t * p2.y};
        }

        Vec2 ArcAt(const float* a, double theta)
        {
            const double cs = std::cos((double)a[4]), sn = std::sin((double)a[4]);
            const double ex = a[2] * std::cos(theta), ey = a[3] * std::sin(theta);
            return {(float)(a[0] + cs * ex - sn * ey), (float)(a[1] + sn * ex + cs * ey)};
        }

        // whether `theta` is on the arc from `start` sweeping `sweep`
        bool OnArc(double theta, double start, double sweep)
        {
            const double tau = 2.0 * 3.14159265358979323846;
            double dlt = std::fmod(theta - start, tau);
            if (sweep >= 0.0)
            {
                if (dlt < 0.0)
                    dlt += tau;
                return dlt <= sweep;
            }
            if (dlt > 0.0)
                dlt -= tau;
            return dlt >= sweep;
        }
    }

    void ComputeBounds(PathData& path)
    {
        Rect r;
        bool any = false;
        Vec2 cur(0.0f, 0.0f);
        std::size_t k = 0;
        for (const Verb v : path.verbs)
        {
            const float* a = path.pts.data() + k;
            switch (v)
            {
            case Verb::Move:
            case Verb::Line:
                cur = Vec2(a[0], a[1]);
                Include(r, any, cur);
                break;
            case Verb::Quad:
            {
                const Vec2 p1(a[0], a[1]), p2(a[2], a[3]);
                Include(r, any, cur);
                Include(r, any, p2);
                for (int axis = 0; axis < 2; ++axis)
                {
                    const double c0 = axis ? cur.y : cur.x, c1 = axis ? p1.y : p1.x, c2 = axis ? p2.y : p2.x;
                    const double den = c0 - 2.0 * c1 + c2;
                    if (std::fabs(den) > 1e-12)
                    {
                        const double t = (c0 - c1) / den;
                        if (t > 0.0 && t < 1.0)
                            Include(r, any, QuadAt(cur, p1, p2, (float)t));
                    }
                }
                cur = p2;
                break;
            }
            case Verb::Cubic:
            {
                const Vec2 p1(a[0], a[1]), p2(a[2], a[3]), p3(a[4], a[5]);
                Include(r, any, cur);
                Include(r, any, p3);
                for (int axis = 0; axis < 2; ++axis)
                {
                    const double c0 = axis ? cur.y : cur.x, c1 = axis ? p1.y : p1.x, c2 = axis ? p2.y : p2.x, c3 = axis ? p3.y : p3.x;
                    double t[2];
                    const int nr = Roots(-c0 + 3.0 * c1 - 3.0 * c2 + c3, 2.0 * (c0 - 2.0 * c1 + c2), c1 - c0, t);
                    for (int q = 0; q < nr; ++q)
                        Include(r, any, CubicAt(cur, p1, p2, p3, (float)t[q]));
                }
                cur = p3;
                break;
            }
            case Verb::Arc:
            {
                Include(r, any, cur);
                const Vec2 end(a[7], a[8]);
                Include(r, any, end);
                const double cs = std::cos((double)a[4]), sn = std::sin((double)a[4]);
                const double tx = std::atan2(-(double)a[3] * sn, (double)a[2] * cs);   // where dx/dtheta = 0
                const double ty = std::atan2((double)a[3] * cs, (double)a[2] * sn);    // where dy/dtheta = 0
                const double cand[4] = {tx, tx + 3.14159265358979323846, ty, ty + 3.14159265358979323846};
                for (const double th : cand)
                    if (OnArc(th, a[5], a[6]))
                        Include(r, any, ArcAt(a, th));
                cur = end;
                break;
            }
            case Verb::Close:
                break;
            }
            k += (std::size_t)kVerbFloats[(int)v];
        }
        path.bounds = any ? r : Rect();
    }

    // ------------------------------------------------------------------ flattening
    namespace
    {
        constexpr int kMaxSteps = 1 << 14;

        int Steps(float ddLength, float factor, float tol)
        {
            const float n = std::ceil(std::sqrt(factor * ddLength / tol));
            return n <= 1.0f ? 1 : (n >= (float)kMaxSteps ? kMaxSteps : (int)n);
        }

        // A chord cuts inside its curve by its sagitta s, by 2/3 s on average: a flattened curve would be thinner than the
        // curve (a little less coverage all along it). So every point between the ends moves out by 2/3 of its chords'
        // sagitta, -dt^2 / 12 times the second derivative across the curve: the chords keep the curve's area, within
        // 2/3 of `tol` of it, and the ends stay where they are.
        Vec2 AreaKeeping(Vec2 p, Vec2 d1, Vec2 d2, float dt)
        {
            const float l2 = Dot(d1, d1);
            if (!(l2 > 0.0f))
                return p;
            const Vec2 across = d2 - d1 * (Dot(d2, d1) / l2);
            return p - across * (dt * dt / 12.0f);
        }

        // Wang's bounds: steps so that the chords stay within `tol` of the curve
        void FlattenQuad(Vec2 p0, Vec2 p1, Vec2 p2, float tol, std::vector<Vec2>& out)
        {
            const Vec2 dd = p0 - p1 * 2.0f + p2;
            const int n = Steps(Length(dd), 0.25f, tol);
            const float dt = 1.0f / (float)n;
            for (int k = 1; k < n; ++k)
            {
                const float t = (float)k * dt;
                const Vec2 d1 = ((p1 - p0) * (1.0f - t) + (p2 - p1) * t) * 2.0f;
                out.push_back(AreaKeeping(QuadAt(p0, p1, p2, t), d1, dd * 2.0f, dt));
            }
            out.push_back(p2);
        }

        void FlattenCubic(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float tol, std::vector<Vec2>& out)
        {
            const Vec2 a = p0 - p1 * 2.0f + p2, b = p1 - p2 * 2.0f + p3;
            const int n = Steps(std::max(Length(a), Length(b)), 0.75f, tol);
            const float dt = 1.0f / (float)n;
            for (int k = 1; k < n; ++k)
            {
                const float t = (float)k * dt, u = 1.0f - t;
                const Vec2 d1 = ((p1 - p0) * (u * u) + (p2 - p1) * (2.0f * u * t) + (p3 - p2) * (t * t)) * 3.0f;
                const Vec2 d2 = (a * u + b * t) * 6.0f;
                out.push_back(AreaKeeping(CubicAt(p0, p1, p2, p3, t), d1, d2, dt));
            }
            out.push_back(p3);
        }

        // steps for `sweep` radians of a circle of `radius` so the chords stay within `tol` of it
        int ArcSteps(float radius, double sweep, float tol)
        {
            const double a = std::fabs(sweep);
            if (radius <= tol)
                return std::max(1, (int)std::ceil(a / 1.5707963267948966));
            const double step = 2.0 * std::acos(1.0 - (double)tol / (double)radius);
            const double n = std::ceil(a / step);
            return n <= 1.0 ? 1 : (n >= (double)kMaxSteps ? kMaxSteps : (int)n);
        }

        // the arc's points after its start through `m` (its last point is its end, exactly), keeping its area as the
        // curves do (AreaKeeping: p'' = -(p - center))
        void FlattenArc(const float* a, const Affine& m, float tol, std::vector<Vec2>& out)
        {
            // the ellipse's axes in the target space, and its largest radius there
            const float cs = std::cos(a[4]), sn = std::sin(a[4]);
            const Affine axes = Affine{m.a, m.b, m.c, m.d, 0.0f, 0.0f} * Affine{cs * a[2], sn * a[2], -sn * a[3], cs * a[3], 0.0f, 0.0f};
            const int n = ArcSteps(axes.MaxScale(), a[6], tol);
            const Vec2 center = m.Apply(Vec2(a[0], a[1]));
            const float dt = a[6] / (float)n;
            for (int k = 1; k < n; ++k)
            {
                const double th = (double)a[5] + (double)a[6] * (double)k / (double)n;
                const Vec2 p = m.Apply(ArcAt(a, th));
                const Vec2 d1 = axes.ApplyVector(Vec2((float)-std::sin(th), (float)std::cos(th)));
                out.push_back(AreaKeeping(p, d1, center - p, dt));
            }
            out.push_back(m.Apply(Vec2(a[7], a[8])));
        }

        void EndContour(Contours& out, bool& open)
        {
            if (open && out.pts.size() - out.starts.back() < 3)
            {
                // fewer than three points cover nothing
                out.pts.resize(out.starts.back());
                out.starts.pop_back();
                out.groups.pop_back();
            }
            open = false;
        }
    }

    void FlattenFill(const PathData& path, const Affine& m, float tol, std::uint16_t group, Contours& out)
    {
        Vec2 cur(0.0f, 0.0f);
        bool open = false;
        std::size_t k = 0;
        for (const Verb v : path.verbs)
        {
            const float* a = path.pts.data() + k;
            k += (std::size_t)kVerbFloats[(int)v];
            if (v == Verb::Close)
            {
                EndContour(out, open);
                continue;
            }
            if (v == Verb::Move)
            {
                EndContour(out, open);
                cur = Vec2(a[0], a[1]);
                out.Begin(group);
                out.pts.push_back(m.Apply(cur));
                open = true;
                continue;
            }
            if (!open)
            {
                out.Begin(group);
                out.pts.push_back(m.Apply(cur));
                open = true;
            }
            switch (v)
            {
            case Verb::Line:
                cur = Vec2(a[0], a[1]);
                out.pts.push_back(m.Apply(cur));
                break;
            case Verb::Quad:
                FlattenQuad(m.Apply(cur), m.Apply(Vec2(a[0], a[1])), m.Apply(Vec2(a[2], a[3])), tol, out.pts);
                cur = Vec2(a[2], a[3]);
                break;
            case Verb::Cubic:
                FlattenCubic(m.Apply(cur), m.Apply(Vec2(a[0], a[1])), m.Apply(Vec2(a[2], a[3])), m.Apply(Vec2(a[4], a[5])), tol, out.pts);
                cur = Vec2(a[4], a[5]);
                break;
            case Verb::Arc:
                FlattenArc(a, m, tol, out.pts);
                cur = Vec2(a[7], a[8]);
                break;
            default:
                break;
            }
        }
        EndContour(out, open);
    }

    // ------------------------------------------------------------------ strokes
    namespace
    {
        // a subpath flattened for stroking, in user units
        struct Poly
        {
            std::vector<Vec2> p;
            std::vector<std::uint8_t> corner;   // 1: a vertex of the path (the stroke's join); 0: inside a curve (round)
            bool closed = false;
            bool drawn = false;                 // a segment command: a zero-length subpath still gets its caps
            Vec2 dir = Vec2(1.0f, 0.0f);        // a zero-length subpath's direction (square caps)
        };

        void FlattenStroke(const PathData& path, float tol, std::vector<Poly>& out)
        {
            Poly cur;
            bool open = false;
            Vec2 at(0.0f, 0.0f);
            std::vector<Vec2> pts;
            const auto finish = [&] {
                if (open && !cur.p.empty())
                    out.push_back(std::move(cur));
                cur = Poly();
                open = false;
            };
            const auto begin = [&](Vec2 q) {
                finish();
                cur.p.push_back(q);
                cur.corner.push_back(1);
                open = true;
            };
            const Affine id;
            std::size_t k = 0;
            for (const Verb v : path.verbs)
            {
                const float* a = path.pts.data() + k;
                k += (std::size_t)kVerbFloats[(int)v];
                if (v == Verb::Move)
                {
                    at = Vec2(a[0], a[1]);
                    begin(at);
                    continue;
                }
                if (!open)
                    begin(at);
                if (v == Verb::Close)
                {
                    cur.closed = true;
                    cur.drawn = true;
                    finish();
                    continue;
                }
                pts.clear();
                switch (v)
                {
                case Verb::Line:
                    pts.push_back(Vec2(a[0], a[1]));
                    break;
                case Verb::Quad:
                    FlattenQuad(at, Vec2(a[0], a[1]), Vec2(a[2], a[3]), tol, pts);
                    break;
                case Verb::Cubic:
                    FlattenCubic(at, Vec2(a[0], a[1]), Vec2(a[2], a[3]), Vec2(a[4], a[5]), tol, pts);
                    break;
                case Verb::Arc:
                    FlattenArc(a, id, tol, pts);
                    break;
                default:
                    break;
                }
                for (std::size_t q = 0; q < pts.size(); ++q)
                {
                    cur.p.push_back(pts[q]);
                    cur.corner.push_back(q + 1 == pts.size() ? 1 : 0);
                }
                at = pts.back();
                cur.drawn = true;
            }
            finish();
        }

        // the dashes of `in`: open subpaths (a dash that runs over a closed subpath's start goes on into its first)
        void Dash(const Poly& in, const std::vector<float>& pattern, float offset, std::vector<Poly>& out)
        {
            const int count = (int)pattern.size();
            float total = 0.0f;
            for (const float v : pattern)
                total += v;
            const std::size_t n = in.p.size();
            const std::size_t segs = in.closed ? n : n - 1;
            if (n < 2 && !in.closed)
                return;
            float phase = std::fmod(offset, total);
            if (phase < 0.0f)
                phase += total;
            int idx = 0;
            while (phase > 0.0f && phase >= pattern[(std::size_t)idx])
            {
                phase -= pattern[(std::size_t)idx];
                idx = (idx + 1) % count;
            }
            float rem = pattern[(std::size_t)idx] - phase;
            bool on = (idx & 1) == 0;
            const bool startsOn = on;
            const std::size_t first = out.size();
            Poly cur;
            bool open = false;
            const auto startDash = [&](Vec2 q, Vec2 dir) {
                cur = Poly();
                cur.p.push_back(q);
                cur.corner.push_back(0);
                cur.drawn = true;
                cur.dir = dir;
                open = true;
            };
            const auto endDash = [&] {
                if (open)
                    out.push_back(std::move(cur));
                cur = Poly();
                open = false;
            };
            bool started = false;
            for (std::size_t s = 0; s < segs; ++s)
            {
                const Vec2 a = in.p[s], b = in.p[(s + 1) % n];
                const std::uint8_t cb = in.corner[(s + 1) % n];
                const float len = Length(b - a);
                const Vec2 u = len > 0.0f ? (b - a) * (1.0f / len) : Vec2(1.0f, 0.0f);
                if (!started)
                {
                    started = true;
                    if (on)
                        startDash(a, u);
                }
                float pos = 0.0f;
                for (int guard = 0; guard < 1 << 20; ++guard)
                {
                    const float avail = len - pos;
                    if (rem > avail)
                    {
                        rem -= avail;
                        if (open)
                        {
                            cur.p.push_back(b);
                            cur.corner.push_back(cb);
                        }
                        break;
                    }
                    pos += rem;
                    const Vec2 q = a + u * pos;
                    if (open)
                    {
                        cur.p.push_back(q);
                        cur.corner.push_back(0);
                        endDash();
                    }
                    idx = (idx + 1) % count;
                    rem = pattern[(std::size_t)idx];
                    on = !on;
                    // dashes start before the subpath's end, never at it (SVG 2's dashing: while position < length)
                    if (on && !(s + 1 == segs && pos >= len))
                        startDash(q, u);
                }
            }
            const bool endsOn = open;
            endDash();
            // a closed subpath dashed on at both ends: the last dash goes on into the first
            if (in.closed && startsOn && endsOn && out.size() - first >= 2)
            {
                Poly& last = out.back();
                Poly& head = out[first];
                for (std::size_t q = 1; q < head.p.size(); ++q)
                {
                    last.p.push_back(head.p[q]);
                    last.corner.push_back(head.corner[q]);
                }
                head = std::move(last);
                out.pop_back();
            }
        }

        class Stroker
        {
        public:
            Stroker(const StrokeStyle& s, const Affine& m, float tolUser, std::uint16_t group, Contours& out)
                : s_(s), m_(m), hw_(0.5f * s.width), tol_(tolUser), group_(group), out_(out)
            {
            }

            void Stroke(const Poly& in);

        private:
            Vec2 L(std::size_t k) const { return Vec2(u_[k].y, -u_[k].x) * hw_; }   // the left side's offset of segment k
            void Arc(Vec2 c, float a0, float sweep, std::vector<Vec2>& pts) const;   // points strictly inside the arc
            void JoinPoints(std::size_t v, std::size_t prevSeg, std::size_t nextSeg, float side, bool forward, std::vector<Vec2>& pts) const;
            void Cap(Vec2 p, Vec2 u, std::vector<Vec2>& pts) const;   // from p + left to p - left around the front of u
            void Emit(std::vector<Vec2>& pts, bool orient);
            void Dot(Vec2 p, Vec2 dir);
            bool ConvexRing();

            struct Joint
            {
                float cross = 0.0f, dot = 1.0f, retreat = 0.0f;
                bool straight = true, outerLeft = true, broken = false, corner = true;
            };

            const StrokeStyle& s_;
            Affine m_;
            float hw_, tol_;
            std::uint16_t group_;
            Contours& out_;
            std::vector<Vec2> q_;
            std::vector<std::uint8_t> cf_;
            std::vector<Vec2> u_;
            std::vector<float> len_;
            std::vector<Joint> j_;
            std::vector<Vec2> pts_, back_;
            std::vector<std::uint32_t> prev_, next_, work_;
            std::vector<std::uint8_t> alive_;
        };

        void Stroker::Arc(Vec2 c, float a0, float sweep, std::vector<Vec2>& pts) const
        {
            const int n = ArcSteps(hw_, sweep, tol_);
            const float step = sweep / (float)n;
            const float r = hw_ * (1.0f + step * step / 12.0f);   // the circle's area kept, as FlattenArc keeps it
            for (int k = 1; k < n; ++k)
            {
                const float a = a0 + step * (float)k;
                pts.push_back(c + Vec2(std::cos(a), std::sin(a)) * r);
            }
        }

        // the outer side of joint v (between segments prevSeg and nextSeg) on `side` (+1 left, -1 right), from the end of
        // the previous segment's offset to the start of the next (forward) or back
        void Stroker::JoinPoints(std::size_t v, std::size_t prevSeg, std::size_t nextSeg, float side, bool forward, std::vector<Vec2>& pts) const
        {
            const Joint& jt = j_[v];
            const Vec2 c = q_[v];
            const Vec2 a = c + L(prevSeg) * side, b = c + L(nextSeg) * side;
            const Join join = jt.corner ? s_.join : Join::Round;
            pts.push_back(forward ? a : b);
            if (join == Join::Miter && jt.dot > -0.9999f && std::sqrt(2.0f / (1.0f + jt.dot)) <= s_.miterLimit)
                pts.push_back(c + (L(prevSeg) + L(nextSeg)) * (side / (1.0f + jt.dot)));
            else if (join == Join::Round)
            {
                const Vec2 da = a - c, db = b - c;
                const float a0 = std::atan2(da.y, da.x);
                float sweep = std::atan2(da.x * db.y - da.y * db.x, da.x * db.x + da.y * db.y);
                if (jt.dot <= -0.9999f)
                {
                    // a reversal: around the front of the segment that ends here
                    const Vec2 u = u_[prevSeg];
                    sweep = (da.x * u.y - da.y * u.x) >= 0.0f ? kPi : -kPi;
                }
                if (forward)
                    Arc(c, a0, sweep, pts);
                else
                {
                    const std::size_t at = pts.size();
                    Arc(c, a0, sweep, pts);
                    std::reverse(pts.begin() + (std::ptrdiff_t)at, pts.end());
                }
            }
            pts.push_back(forward ? b : a);
        }

        void Stroker::Cap(Vec2 p, Vec2 u, std::vector<Vec2>& pts) const
        {
            const Vec2 l = Vec2(u.y, -u.x) * hw_;
            if (s_.cap == Cap::Square)
            {
                pts.push_back(p + l + u * hw_);
                pts.push_back(p - l + u * hw_);
            }
            else if (s_.cap == Cap::Round)
                Arc(p, std::atan2(l.y, l.x), kPi, pts);
        }

        void Stroker::Emit(std::vector<Vec2>& pts, bool orient)
        {
            if (pts.size() < 3)
                return;
            if (orient)
            {
                // every piece wound like a band (positive area in user units): the union is their non-zero coverage
                double area = 0.0;
                for (std::size_t i = 0, k = pts.size() - 1; i < pts.size(); k = i++)
                    area += (double)pts[k].x * pts[i].y - (double)pts[i].x * pts[k].y;
                if (std::fabs(area) < 1e-12)
                    return;
                if (area < 0.0)
                    std::reverse(pts.begin(), pts.end());
            }
            out_.Begin(group_);
            for (const Vec2 p : pts)
                out_.pts.push_back(m_.Apply(p));
        }

        // a zero-length subpath: its caps alone
        void Stroker::Dot(Vec2 p, Vec2 dir)
        {
            pts_.clear();
            if (s_.cap == Cap::Round)
            {
                pts_.push_back(p + Vec2(hw_, 0.0f));
                Arc(p, 0.0f, 2.0f * kPi, pts_);
            }
            else if (s_.cap == Cap::Square)
            {
                const float dl = Length(dir);
                const Vec2 u = dl > 0.0f ? dir * (hw_ / dl) : Vec2(hw_, 0.0f);
                const Vec2 l(u.y, -u.x);
                pts_ = {p - u + l, p + u + l, p + u - l, p - u - l};
            }
            Emit(pts_, true);
        }

        // A closed convex outline whose inner side folds somewhere (a thick stroke on a small circle, a rounded corner
        // tighter than the width): the stroke is its outer side's loop less what lies farther than half the width inside
        // every edge - the inner offset, the intersection of the edges' half-planes, empty where the stroke fills it all.
        // Two loops instead of a band and a wedge for every folding joint, overlapping all around. false: not convex.
        bool Stroker::ConvexRing()
        {
            const std::size_t n = q_.size();   // closed: n segments
            int turn = 0;
            double total = 0.0;
            for (std::size_t v = 0; v < n; ++v)
            {
                const Joint& jt = j_[v];
                if (jt.straight)
                    continue;
                if (jt.dot <= -0.9999f)
                    return false;
                const int s = jt.cross > 0.0f ? 1 : -1;
                if (turn != 0 && s != turn)
                    return false;
                turn = s;
                total += std::atan2((double)jt.cross, (double)jt.dot);
            }
            if (turn == 0 || std::fabs(std::fabs(total) - 6.283185307179586) > 0.1)
                return false;   // turning once around: a convex outline
            const float side = turn > 0 ? 1.0f : -1.0f;   // the outer side (y down: right turns have it on the left)
            pts_.clear();
            for (std::size_t v = 0; v < n; ++v)
            {
                if (j_[v].straight)
                    pts_.push_back(q_[v] + L(v) * side);
                else
                    JoinPoints(v, (v + n - 1) % n, v, side, true, pts_);
            }
            // every edge's line moved inside by half the width; an edge whose part between its neighbours' lines comes to
            // nothing (or turns negative) drops out, and its neighbours meet instead
            const auto origin = [&](std::size_t k) { return q_[k] - L(k) * side; };
            const auto meet = [&](std::size_t k, std::size_t a, float& t) {
                const Vec2 uk = u_[k], ua = u_[a];
                const float den = uk.x * ua.y - uk.y * ua.x;
                if (std::fabs(den) < 1e-9f)
                    return false;
                const Vec2 d = origin(a) - origin(k);
                t = (d.x * ua.y - d.y * ua.x) / den;
                return true;
            };
            prev_.resize(n);
            next_.resize(n);
            alive_.assign(n, 1);
            work_.clear();
            for (std::size_t k = 0; k < n; ++k)
            {
                prev_[k] = (std::uint32_t)((k + n - 1) % n);
                next_[k] = (std::uint32_t)((k + 1) % n);
                work_.push_back((std::uint32_t)k);
            }
            std::size_t count = n;
            while (!work_.empty() && count >= 3)
            {
                const std::uint32_t k = work_.back();
                work_.pop_back();
                if (!alive_[k])
                    continue;
                float t0 = 0.0f, t1 = 0.0f;
                if (meet(k, prev_[k], t0) && meet(k, next_[k], t1) && t1 > t0)
                    continue;
                alive_[k] = 0;
                --count;
                next_[prev_[k]] = next_[k];
                prev_[next_[k]] = prev_[k];
                work_.push_back(prev_[k]);
                work_.push_back(next_[k]);
            }
            back_.clear();
            if (count >= 3)
            {
                std::size_t k = 0;
                while (!alive_[k])
                    ++k;
                for (std::size_t i = 0; i < count; ++i, k = next_[k])
                {
                    float t = 0.0f;
                    meet(k, next_[k], t);
                    back_.push_back(origin(k) + u_[k] * t);
                }
            }
            const auto area = [](const std::vector<Vec2>& loop) {
                double a = 0.0;
                for (std::size_t i = 0, k = loop.size() - 1; i < loop.size(); k = i++)
                    a += (double)loop[k].x * loop[i].y - (double)loop[i].x * loop[k].y;
                return a;
            };
            const double outer = area(pts_), hole = back_.size() >= 3 ? area(back_) : 0.0;
            if (!(hole * outer > 0.0) || std::fabs(hole) < 1e-12)
                back_.clear();   // inverted or nothing left: no hole
            std::reverse(back_.begin(), back_.end());   // the hole winds against the outer loop
            if (outer < 0.0)
            {
                std::reverse(pts_.begin(), pts_.end());
                std::reverse(back_.begin(), back_.end());
            }
            Emit(pts_, false);
            Emit(back_, false);
            return true;
        }

        void Stroker::Stroke(const Poly& in)
        {
            // distinct points (a corner where any of the merged ones was)
            q_.clear();
            cf_.clear();
            const float eps = std::max(hw_ * 1e-4f, 1e-7f);
            for (std::size_t i = 0; i < in.p.size(); ++i)
            {
                if (!q_.empty() && Length(in.p[i] - q_.back()) <= eps)
                {
                    cf_.back() = (std::uint8_t)(cf_.back() | in.corner[i]);
                    continue;
                }
                q_.push_back(in.p[i]);
                cf_.push_back(in.corner[i]);
            }
            if (in.closed && q_.size() > 1 && Length(q_.back() - q_.front()) <= eps)
            {
                cf_.front() = (std::uint8_t)(cf_.front() | cf_.back());
                q_.pop_back();
                cf_.pop_back();
            }
            const std::size_t n = q_.size();
            if (n == 0)
                return;
            if (n == 1)
            {
                if (in.drawn)
                    Dot(q_[0], in.dir);
                return;
            }
            const bool closed = in.closed;
            const std::size_t segs = closed ? n : n - 1;
            u_.resize(segs);
            len_.resize(segs);
            for (std::size_t k = 0; k < segs; ++k)
            {
                const Vec2 d = q_[(k + 1) % n] - q_[k];
                len_[k] = Length(d);
                u_[k] = d * (1.0f / len_[k]);
            }
            // joints: vertex v between segments v - 1 and v (an open subpath's ends have none)
            j_.assign(n, Joint());
            const auto isJoint = [&](std::size_t v) { return closed || (v > 0 && v + 1 < n); };
            for (std::size_t v = 0; v < n; ++v)
            {
                if (!isJoint(v))
                    continue;
                Joint& jt = j_[v];
                const Vec2 a = u_[(v + segs - 1) % segs], b = u_[v % segs];
                jt.cross = a.x * b.y - a.y * b.x;
                jt.dot = Clamp(a.x * b.x + a.y * b.y, -1.0f, 1.0f);
                jt.corner = cf_[v] != 0;
                jt.straight = std::fabs(jt.cross) < 1e-6f && jt.dot > 0.0f;
                jt.outerLeft = jt.cross > 0.0f || (jt.cross == 0.0f && jt.dot < 0.0f);   // y down: a right turn's outer side is left
                jt.broken = jt.dot <= -0.9999f;
                jt.retreat = jt.broken || jt.straight ? 0.0f : hw_ * std::sqrt((1.0f - jt.dot) / (1.0f + jt.dot));
            }
            // a segment too short for the inner offsets of the joints at its ends (they would fold the band over):
            // those joints break the band and get pieces of their own
            for (std::size_t k = 0; k < segs; ++k)
            {
                const std::size_t v0 = k, v1 = (k + 1) % n;
                for (int side = 0; side < 2; ++side)
                {
                    const bool left = side == 0;
                    float sum = 0.0f;
                    const bool c0 = isJoint(v0) && !j_[v0].broken && !j_[v0].straight && j_[v0].outerLeft != left;
                    const bool c1 = isJoint(v1) && !j_[v1].broken && !j_[v1].straight && j_[v1].outerLeft != left;
                    if (c0)
                        sum += j_[v0].retreat;
                    if (c1)
                        sum += j_[v1].retreat;
                    if (sum > len_[k])
                    {
                        if (c0)
                            j_[v0].broken = true;
                        if (c1)
                            j_[v1].broken = true;
                    }
                }
            }

            bool anyBroken = false;
            for (std::size_t v = 0; v < n; ++v)
                anyBroken = anyBroken || (isJoint(v) && j_[v].broken);

            // the inner side's point at joint v (side +1 left, -1 right)
            const auto inner = [&](std::size_t v, float side) {
                const std::size_t a = (v + segs - 1) % segs, b = v % segs;
                return q_[v] + (L(a) + L(b)) * (side / (1.0f + j_[v].dot));
            };

            if (closed && anyBroken && ConvexRing())
                return;
            if (closed && !anyBroken)
            {
                // a closed band: the left side's loop forward, the right side's back
                pts_.clear();
                for (std::size_t v = 0; v < n; ++v)
                {
                    const Joint& jt = j_[v];
                    if (jt.straight)
                        pts_.push_back(q_[v] + L(v));
                    else if (jt.outerLeft)
                        JoinPoints(v, (v + segs - 1) % segs, v, 1.0f, true, pts_);
                    else
                        pts_.push_back(inner(v, 1.0f));
                }
                back_.clear();
                for (std::size_t i = n; i-- > 0;)
                {
                    const Joint& jt = j_[i];
                    if (jt.straight)
                        back_.push_back(q_[i] - L(i));
                    else if (!jt.outerLeft)
                        JoinPoints(i, (i + segs - 1) % segs, i, -1.0f, false, back_);
                    else
                        back_.push_back(inner(i, -1.0f));
                }
                double area = 0.0;
                for (const std::vector<Vec2>* loop : {&pts_, &back_})
                    for (std::size_t i = 0, k = loop->size() - 1; i < loop->size(); k = i++)
                        area += (double)(*loop)[k].x * (*loop)[i].y - (double)(*loop)[i].x * (*loop)[k].y;
                if (area < 0.0)
                {
                    std::reverse(pts_.begin(), pts_.end());
                    std::reverse(back_.begin(), back_.end());
                }
                Emit(pts_, false);
                Emit(back_, false);
                return;
            }

            // runs of segments between broken joints (and an open subpath's ends)
            std::size_t startV = 0;
            if (closed)
                while (!j_[startV].broken)
                    ++startV;
            std::size_t v = startV;
            std::size_t done = 0;
            while (done < segs)
            {
                // the run from vertex v to the next broken joint or the end
                std::size_t e = v, count = 0;
                do
                {
                    e = (e + 1) % n;
                    ++count;
                } while (count < segs - done && !(isJoint(e) && j_[e].broken) && !(!closed && e == n - 1));
                const std::size_t s0 = v, s1 = (e + n - 1) % n;   // its first and last segments
                pts_.clear();
                pts_.push_back(q_[v] + L(s0));
                for (std::size_t w = (v + 1) % n, c = 1; c < count; w = (w + 1) % n, ++c)
                {
                    const Joint& jt = j_[w];
                    if (jt.straight)
                        continue;
                    if (jt.outerLeft)
                        JoinPoints(w, (w + segs - 1) % segs, w, 1.0f, true, pts_);
                    else
                        pts_.push_back(inner(w, 1.0f));
                }
                pts_.push_back(q_[e] + L(s1));
                if (!closed && e == n - 1)
                    Cap(q_[e], u_[s1], pts_);
                pts_.push_back(q_[e] - L(s1));
                for (std::size_t c = count - 1; c >= 1; --c)
                {
                    const std::size_t w = (v + c) % n;
                    const Joint& jt = j_[w];
                    if (jt.straight)
                        continue;
                    if (!jt.outerLeft)
                        JoinPoints(w, (w + segs - 1) % segs, w, -1.0f, false, pts_);
                    else
                        pts_.push_back(inner(w, -1.0f));
                }
                pts_.push_back(q_[v] - L(s0));
                if (!closed && v == 0)
                    Cap(q_[v], u_[s0] * -1.0f, pts_);
                Emit(pts_, true);

                // the joint the run ends at: a wedge on its outer side
                if (isJoint(e) && j_[e].broken)
                {
                    const Joint& jt = j_[e];
                    if (!jt.straight)
                    {
                        pts_.clear();
                        pts_.push_back(q_[e]);
                        JoinPoints(e, (e + segs - 1) % segs, e % segs, jt.outerLeft ? 1.0f : -1.0f, true, pts_);
                        Emit(pts_, true);
                    }
                }
                done += count;
                v = e;
            }
        }
    }

    void StrokeOutline(const PathData& path, const StrokeStyle& style, const Affine& m, float tol, std::uint16_t group, Contours& out)
    {
        const float scale = m.MaxScale();
        if (!(style.width > 0.0f) || !(scale > 0.0f))
            return;
        const float tolUser = tol / scale;
        std::vector<Poly> polys;
        FlattenStroke(path, tolUser, polys);
        if (!style.dashes.empty())
        {
            std::vector<Poly> dashes;
            for (const Poly& p : polys)
                Dash(p, style.dashes, style.dashOffset, dashes);
            polys.swap(dashes);
        }
        Stroker stroker(style, m, tolUser, group, out);
        for (const Poly& p : polys)
            stroker.Stroke(p);
    }
}
