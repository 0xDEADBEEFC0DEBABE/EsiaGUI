// Esia - multi-channel signed distance fields of glyph outlines (include/esia/text/distance_field.hpp).
#include "esia/text/distance_field.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <unordered_map>

namespace esia::text
{
    // ------------------------------------------------------------------ GlyphShape
    void GlyphShape::MoveTo(Vec2 p)
    {
        starts_.push_back(edges_.size());
        pen_ = p;
    }

    void GlyphShape::LineTo(Vec2 p)
    {
        if (starts_.empty())
            MoveTo(pen_);
        if (!(p == pen_))
            edges_.push_back({{pen_, p, Vec2(), Vec2()}, 1});
        pen_ = p;
    }

    void GlyphShape::QuadTo(Vec2 control, Vec2 p)
    {
        if (starts_.empty())
            MoveTo(pen_);
        edges_.push_back({{pen_, control, p, Vec2()}, 2});
        pen_ = p;
    }

    void GlyphShape::CubicTo(Vec2 control1, Vec2 control2, Vec2 p)
    {
        if (starts_.empty())
            MoveTo(pen_);
        edges_.push_back({{pen_, control1, control2, p}, 3});
        pen_ = p;
    }

    void GlyphShape::Clear()
    {
        edges_.clear();
        starts_.clear();
        pen_ = Vec2();
    }

    std::span<const GlyphShape::Edge> GlyphShape::Contour(std::size_t i) const
    {
        const std::size_t end = i + 1 < starts_.size() ? starts_[i + 1] : edges_.size();
        return std::span<const Edge>(edges_.data() + starts_[i], end - starts_[i]);
    }

    Rect GlyphShape::Bounds() const
    {
        if (edges_.empty())
            return Rect();
        Rect r(edges_[0].p[0], edges_[0].p[0]);
        for (const Edge& e : edges_)
            for (int k = 0; k <= e.degree; ++k)
            {
                r.min.x = std::min(r.min.x, e.p[k].x);
                r.min.y = std::min(r.min.y, e.p[k].y);
                r.max.x = std::max(r.max.x, e.p[k].x);
                r.max.y = std::max(r.max.y, e.p[k].y);
            }
        return r;
    }

    // ------------------------------------------------------------------ the field
    namespace
    {
        constexpr int kMaxField = 4096;
        constexpr float kFlatness = 1.0f / 32.0f;          // curve flattening tolerance, field pixels
        constexpr float kCornerCross = 0.14112000806f;     // sin(3 rad): a turn of more than about 8 degrees is a corner
        constexpr int kCell = 4;                           // grid cell, texels

        enum : std::uint8_t { kBlack = 0, kRed = 1, kGreen = 2, kYellow = 3, kBlue = 4, kMagenta = 5, kCyan = 6, kWhite = 7 };

        float Cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }

        Vec2 Unit(Vec2 v)
        {
            const float l = Length(v);
            return l > 0.0f ? v / l : Vec2();
        }

        // A straight piece of the outline: a flattened bit of one edge, maybe cut where contours cross.
        struct Piece
        {
            Vec2 a, b;
            Vec2 ta, tb;           // tangent at a / b: the edge's own at its ends, else the piece's direction
            int edge = 0;          // the edge it comes from (pieces of one edge are consecutive)
            bool edgeStart = false, edgeEnd = false;   // a / b is that edge's end point
            bool cutA = false, cutB = false;           // a / b is where the outline crosses itself
            std::uint8_t color = kWhite;
            bool runStart = false, runEnd = false;     // the color changes at a / b: pseudo-distances extend the tangent
        };

        Vec2 StartTangent(const GlyphShape::Edge& e)
        {
            for (int k = 1; k <= e.degree; ++k)
                if (!(e.p[k] == e.p[0]))
                    return e.p[k] - e.p[0];
            return Vec2();
        }

        Vec2 EndTangent(const GlyphShape::Edge& e)
        {
            for (int k = e.degree - 1; k >= 0; --k)
                if (!(e.p[k] == e.p[e.degree]))
                    return e.p[e.degree] - e.p[k];
            return Vec2();
        }

        Vec2 Evaluate(const GlyphShape::Edge& e, float t)
        {
            const float u = 1.0f - t;
            if (e.degree == 2)
                return e.p[0] * (u * u) + e.p[1] * (2.0f * u * t) + e.p[2] * (t * t);
            if (e.degree == 3)
                return e.p[0] * (u * u * u) + e.p[1] * (3.0f * u * u * t) + e.p[2] * (3.0f * u * t * t) + e.p[3] * (t * t * t);
            return Lerp(e.p[0], e.p[1], t);
        }

        // The contour's edges as pieces (the closing line included), each curve in as many as its flatness asks.
        void Flatten(std::span<const GlyphShape::Edge> contour, int& edgeIndex, std::vector<Piece>& out)
        {
            std::vector<GlyphShape::Edge> edges(contour.begin(), contour.end());
            if (!(edges.back().p[edges.back().degree] == edges.front().p[0]))
                edges.push_back({{edges.back().p[edges.back().degree], edges.front().p[0], Vec2(), Vec2()}, 1});
            for (const GlyphShape::Edge& e : edges)
            {
                int n = 1;
                if (e.degree == 2)
                    n = (int)std::ceil(std::sqrt(Length(e.p[0] - e.p[1] * 2.0f + e.p[2]) / (8.0f * kFlatness)));
                else if (e.degree == 3)
                    n = (int)std::ceil(std::sqrt(0.75f * std::max(Length(e.p[0] - e.p[1] * 2.0f + e.p[2]), Length(e.p[1] - e.p[2] * 2.0f + e.p[3])) / kFlatness));
                n = std::clamp(n, 1, 256);
                const Vec2 t0 = StartTangent(e), t1 = EndTangent(e);
                if (LengthSq(t0) == 0.0f)
                    continue;   // a point
                Vec2 prev = e.p[0];
                const std::size_t first = out.size();
                for (int i = 1; i <= n; ++i)
                {
                    const Vec2 q = i == n ? e.p[e.degree] : Evaluate(e, (float)i / (float)n);
                    if (q == prev)
                        continue;
                    Piece s;
                    s.a = prev;
                    s.b = q;
                    s.ta = s.tb = q - prev;
                    s.edge = edgeIndex;
                    out.push_back(s);
                    prev = q;
                }
                if (out.size() > first)
                {
                    out[first].edgeStart = true;
                    out[first].ta = t0;
                    out.back().edgeEnd = true;
                    out.back().tb = t1;
                }
                ++edgeIndex;
            }
        }

        void Reverse(Piece& s)
        {
            std::swap(s.a, s.b);
            const Vec2 ta = s.ta;
            s.ta = -s.tb;
            s.tb = -ta;
            std::swap(s.edgeStart, s.edgeEnd);
            std::swap(s.cutA, s.cutB);
        }

        // Non-zero winding number of `segments` at p (a ray towards +x).
        int Winding(const std::vector<Piece>& segments, Vec2 p)
        {
            int w = 0;
            for (const Piece& s : segments)
                if ((s.a.y <= p.y) != (s.b.y <= p.y))
                {
                    const float x = s.a.x + (p.y - s.a.y) * (s.b.x - s.a.x) / (s.b.y - s.a.y);
                    if (x > p.x)
                        w += s.b.y > s.a.y ? 1 : -1;
                }
            return w;
        }

        struct Cut
        {
            float t;   // along the piece
            Vec2 at;   // the crossing, the same point for both pieces (loops link by exact end points)
        };

        // Where the pieces of different places of the outline cross: each piece cut there. A sweep along x: pieces
        // sorted by their left end, each compared with those that start before it ends.
        std::vector<Piece> CutAtCrossings(const std::vector<Piece>& in, const std::vector<int>& contourOf, bool& crossed)
        {
            const std::size_t n = in.size();
            std::vector<std::vector<Cut>> cuts(n);
            crossed = false;
            std::vector<std::size_t> order(n);
            std::iota(order.begin(), order.end(), std::size_t{0});
            std::sort(order.begin(), order.end(), [&](std::size_t x, std::size_t y) { return std::min(in[x].a.x, in[x].b.x) < std::min(in[y].a.x, in[y].b.x); });
            for (std::size_t oi = 0; oi < n; ++oi)
            {
                const std::size_t i = order[oi];
                const Piece& p = in[i];
                const float pmaxx = std::max(p.a.x, p.b.x);
                const float pminy = std::min(p.a.y, p.b.y), pmaxy = std::max(p.a.y, p.b.y);
                for (std::size_t oj = oi + 1; oj < n; ++oj)
                {
                    const std::size_t j = order[oj];
                    const Piece& q = in[j];
                    if (std::min(q.a.x, q.b.x) > pmaxx)
                        break;   // this one and every later one start right of p's end
                    if (std::max(q.a.y, q.b.y) < pminy || std::min(q.a.y, q.b.y) > pmaxy)
                        continue;
                    // neighbours in a contour share an end point: not a crossing
                    if (contourOf[i] == contourOf[j] && (q.a == p.b || q.b == p.a))
                        continue;
                    const double rx = (double)p.b.x - p.a.x, ry = (double)p.b.y - p.a.y;
                    const double sx = (double)q.b.x - q.a.x, sy = (double)q.b.y - q.a.y;
                    const double den = rx * sy - ry * sx;
                    if (std::fabs(den) < 1e-12)
                        continue;   // parallel (collinear overlaps are left as they are)
                    const double qx = (double)q.a.x - p.a.x, qy = (double)q.a.y - p.a.y;
                    const double t = (qx * sy - qy * sx) / den, u = (qx * ry - qy * rx) / den;
                    if (t < -1e-9 || t > 1.0 + 1e-9 || u < -1e-9 || u > 1.0 + 1e-9)
                        continue;
                    constexpr double kEnd = 1e-5;
                    const bool cutP = t > kEnd && t < 1.0 - kEnd, cutQ = u > kEnd && u < 1.0 - kEnd;
                    if (!cutP && !cutQ)
                        continue;   // they touch at an end point
                    // one point for both; an end point of a piece that is not cut stays that end point
                    const Vec2 at = !cutP ? (t < 0.5 ? p.a : p.b) : !cutQ ? (u < 0.5 ? q.a : q.b) : Vec2((float)(p.a.x + rx * t), (float)(p.a.y + ry * t));
                    if (cutP)
                        cuts[i].push_back({(float)t, at});
                    if (cutQ)
                        cuts[j].push_back({(float)u, at});
                    crossed = true;
                }
            }
            std::vector<Piece> out;
            out.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                std::vector<Cut>& c = cuts[i];
                if (c.empty())
                {
                    out.push_back(in[i]);
                    continue;
                }
                std::sort(c.begin(), c.end(), [](const Cut& x, const Cut& y) { return x.t < y.t; });
                c.erase(std::unique(c.begin(), c.end(), [](const Cut& x, const Cut& y) { return y.t - x.t < 1e-6f; }), c.end());
                const Piece& p = in[i];
                Vec2 prev = p.a;
                for (std::size_t k = 0; k <= c.size(); ++k)
                {
                    const Vec2 q = k < c.size() ? c[k].at : p.b;
                    if (q == prev)
                        continue;
                    Piece s = p;
                    s.a = prev;
                    s.b = q;
                    s.ta = k == 0 ? p.ta : q - prev;
                    s.tb = k == c.size() ? p.tb : q - prev;
                    s.edgeStart = k == 0 && p.edgeStart;
                    s.edgeEnd = k == c.size() && p.edgeEnd;
                    s.cutA = k > 0;
                    s.cutB = k < c.size();
                    out.push_back(s);
                    prev = q;
                }
            }
            return out;
        }

        // The boundary of the filled area (non-zero) as loops, inside on the left of every piece (Cross(dir, p - a) > 0).
        // False when the pieces do not close into loops (then the caller takes the contours as they are).
        bool BoundaryLoops(const std::vector<Piece>& pieces, const std::vector<Piece>& all, std::vector<std::vector<Piece>>& loops)
        {
            std::vector<Piece> kept;
            for (Piece s : pieces)
            {
                const Vec2 dir = s.b - s.a;
                const float len = Length(dir);
                if (len <= 0.0f)
                    continue;
                const Vec2 mid = Lerp(s.a, s.b, 0.5f), n = Vec2(-dir.y, dir.x) * (1e-3f / len);
                const bool left = Winding(all, mid + n) != 0, right = Winding(all, mid - n) != 0;
                if (left == right)
                    continue;   // inside (or outside) on both sides: not the area's boundary
                if (!left)
                    Reverse(s);   // inside on the right: turn it round
                kept.push_back(s);
            }
            // link the pieces end to start (a crossing is the same point in both pieces cut there)
            const auto key = [](Vec2 p) {
                return (std::uint64_t)std::bit_cast<std::uint32_t>(p.x) << 32 | std::bit_cast<std::uint32_t>(p.y);
            };
            std::unordered_multimap<std::uint64_t, std::size_t> starts;
            for (std::size_t i = 0; i < kept.size(); ++i)
                starts.emplace(key(kept[i].a), i);
            std::vector<bool> used(kept.size(), false);
            for (std::size_t first = 0; first < kept.size(); ++first)
            {
                if (used[first])
                    continue;
                std::vector<Piece> loop;
                std::size_t cur = first;
                for (;;)
                {
                    used[cur] = true;
                    loop.push_back(kept[cur]);
                    if (kept[cur].b == kept[first].a)
                        break;   // closed
                    std::size_t next = kept.size();
                    const auto range = starts.equal_range(key(kept[cur].b));
                    for (auto it = range.first; it != range.second; ++it)
                        if (!used[it->second] && kept[it->second].a == kept[cur].b)
                        {
                            next = it->second;
                            break;
                        }
                    if (next == kept.size())
                        return false;
                    cur = next;
                }
                loops.push_back(std::move(loop));
            }
            return true;
        }

        // Each contour a loop of its own, turned so that the filled side is on the left (no crossings, or linking failed).
        void ContourLoops(const std::vector<std::vector<Piece>>& contours, const std::vector<Piece>& all, std::vector<std::vector<Piece>>& loops)
        {
            for (const std::vector<Piece>& c : contours)
            {
                if (c.empty())
                    continue;
                // which side is filled: test the longest piece (the farthest from anything else's end points)
                std::size_t best = 0;
                for (std::size_t i = 1; i < c.size(); ++i)
                    if (LengthSq(c[i].b - c[i].a) > LengthSq(c[best].b - c[best].a))
                        best = i;
                const Vec2 dir = c[best].b - c[best].a;
                const Vec2 mid = Lerp(c[best].a, c[best].b, 0.5f), n = Vec2(-dir.y, dir.x) * (1e-3f / Length(dir));
                const bool left = Winding(all, mid + n) != 0, right = Winding(all, mid - n) != 0;
                if (left == right)
                    continue;   // inside a larger area entirely: no boundary of the shape
                std::vector<Piece> loop = c;
                if (!left)
                {
                    std::reverse(loop.begin(), loop.end());
                    for (Piece& s : loop)
                        Reverse(s);
                }
                loops.push_back(std::move(loop));
            }
        }

        // A corner between the piece ending at a vertex and the piece starting there.
        bool IsCorner(const Piece& in, const Piece& out)
        {
            if (in.edge == out.edge && !in.cutB && !out.cutA && !in.edgeEnd && !out.edgeStart)
                return false;   // inside one flattened curve
            const Vec2 a = Unit(in.tb), b = Unit(out.ta);
            return Dot(a, b) <= 0.0f || std::fabs(Cross(a, b)) > kCornerCross;
        }

        // Chlumsky's color switching: two channels per run, the next run sharing exactly one of them.
        void SwitchColor(std::uint8_t& color, std::uint64_t& seed, std::uint8_t banned = kBlack)
        {
            const std::uint8_t combined = color & banned;
            if (combined == kRed || combined == kGreen || combined == kBlue)
            {
                color = combined ^ kWhite;
                return;
            }
            if (color == kBlack || color == kWhite)
            {
                static const std::uint8_t start[3] = {kCyan, kMagenta, kYellow};
                color = start[seed % 3];
                seed /= 3;
                return;
            }
            const int shifted = color << (1 + (seed & 1));
            color = (std::uint8_t)((shifted | shifted >> 3) & kWhite);
            seed >>= 1;
        }

        int SymmetricalTrichotomy(int position, int n) { return (int)(3 + 2.875f * (float)position / (float)(n - 1) - 1.4375f + 0.5f) - 3; }

        // Every piece of a loop in three, keeping the vertices between them smooth (a teardrop needs three runs).
        void Thirds(std::vector<Piece>& loop)
        {
            std::vector<Piece> out;
            for (const Piece& s : loop)
                for (int k = 0; k < 3; ++k)
                {
                    Piece p = s;
                    p.a = Lerp(s.a, s.b, (float)k / 3.0f);
                    p.b = Lerp(s.a, s.b, (float)(k + 1) / 3.0f);
                    p.ta = k == 0 ? s.ta : s.b - s.a;
                    p.tb = k == 2 ? s.tb : s.b - s.a;
                    p.edgeStart = k == 0 && s.edgeStart;
                    p.edgeEnd = k == 2 && s.edgeEnd;
                    p.cutA = k == 0 && s.cutA;
                    p.cutB = k == 2 && s.cutB;
                    out.push_back(p);
                }
            loop = std::move(out);
        }

        void ColorLoops(std::vector<std::vector<Piece>>& loops)
        {
            std::uint64_t seed = 0;
            std::uint8_t color = kWhite;
            for (std::vector<Piece>& loop : loops)
            {
                std::vector<int> corners;
                for (int i = 0; i < (int)loop.size(); ++i)
                    if (IsCorner(loop[(i + (int)loop.size() - 1) % (int)loop.size()], loop[(std::size_t)i]))
                        corners.push_back(i);
                if (corners.empty())
                {
                    SwitchColor(color, seed);
                    for (Piece& s : loop)
                        s.color = color;
                }
                else if (corners.size() == 1)
                {
                    // a teardrop: three colors along the one run, white in the middle
                    std::uint8_t colors[3] = {kWhite, kWhite, kWhite};
                    SwitchColor(colors[0], seed);
                    colors[2] = colors[0];
                    SwitchColor(colors[2], seed);
                    if (loop.size() < 3)
                    {
                        Thirds(loop);
                        corners[0] *= 3;
                    }
                    const int m = (int)loop.size(), start = corners[0];
                    for (int i = 0; i < m; ++i)
                        loop[(std::size_t)((start + i) % m)].color = colors[1 + SymmetricalTrichotomy(i, m)];
                }
                else
                {
                    const int m = (int)loop.size(), start = corners[0];
                    std::size_t spline = 0;
                    SwitchColor(color, seed);
                    const std::uint8_t initial = color;
                    for (int i = 0; i < m; ++i)
                    {
                        const int index = (start + i) % m;
                        if (spline + 1 < corners.size() && corners[spline + 1] == index)
                        {
                            ++spline;
                            SwitchColor(color, seed, spline == corners.size() - 1 ? initial : kBlack);
                        }
                        loop[(std::size_t)index].color = color;
                    }
                }
                // runs: the color changes at a piece's start / end
                const std::size_t m = loop.size();
                for (std::size_t i = 0; i < m; ++i)
                {
                    loop[i].runStart = loop[i].color != loop[(i + m - 1) % m].color;
                    loop[i].runEnd = loop[i].color != loop[(i + 1) % m].color;
                }
            }
        }

        float Median(float a, float b, float c) { return std::max(std::min(a, b), std::min(std::max(a, b), c)); }

        // Bilinear filtering between two texels would cross the edge in one channel where the others do not
        // (Chlumsky's clash test, on values in 0 .. 1): flag the texel farther from the edge.
        bool Clash(const float* a, const float* b, float threshold)
        {
            float a0 = a[0], a1 = a[1], a2 = a[2], b0 = b[0], b1 = b[1], b2 = b[2];
            // order the pairs by their difference, largest first
            if (std::fabs(b0 - a0) < std::fabs(b1 - a1))
            {
                std::swap(a0, a1);
                std::swap(b0, b1);
            }
            if (std::fabs(b1 - a1) < std::fabs(b2 - a2))
            {
                std::swap(a1, a2);
                std::swap(b1, b2);
                if (std::fabs(b0 - a0) < std::fabs(b1 - a1))
                {
                    std::swap(a0, a1);
                    std::swap(b0, b1);
                }
            }
            return std::fabs(b1 - a1) >= threshold && !(b0 == b1 && b0 == b2) && std::fabs(a2 - 0.5f) >= std::fabs(b2 - 0.5f);
        }
    }

    bool GenerateMtsdf(const GlyphShape& shape, const DistanceFieldDesc& desc, GlyphBitmap& out)
    {
        out = {};
        out.channels = 4;
        out.sdf = true;
        const float range = std::max(desc.range, 1e-3f), alphaRange = std::max(desc.alphaRange, range);
        if (shape.Empty())
            return true;

        // ---- the outline as pieces, per contour
        std::vector<std::vector<Piece>> contours;
        std::vector<Piece> all;
        std::vector<int> contourOf;
        int edgeIndex = 0;
        for (std::size_t c = 0; c < shape.ContourCount(); ++c)
        {
            const std::span<const GlyphShape::Edge> contour = shape.Contour(c);
            if (contour.empty())
                continue;
            std::vector<Piece> pieces;
            Flatten(contour, edgeIndex, pieces);
            if (pieces.size() < 2)
                continue;
            for (const Piece& s : pieces)
            {
                all.push_back(s);
                contourOf.push_back((int)contours.size());
            }
            contours.push_back(std::move(pieces));
        }
        if (all.empty())
            return true;

        // ---- the filled area's boundary, run into loops, colored
        bool crossed = false;
        const std::vector<Piece> cut = CutAtCrossings(all, contourOf, crossed);
        std::vector<std::vector<Piece>> loops;
        if (!crossed || !BoundaryLoops(cut, all, loops))
        {
            loops.clear();
            ContourLoops(contours, all, loops);
        }
        ColorLoops(loops);
        std::vector<Piece> pieces;
        for (const std::vector<Piece>& loop : loops)
            pieces.insert(pieces.end(), loop.begin(), loop.end());

        // ---- the bitmap: the shape and alphaRange / 2 + 1 pixels around it
        const Rect b = shape.Bounds();
        const float margin = alphaRange * 0.5f + 1.0f;
        const int left = (int)std::floor(b.min.x - margin), top = (int)std::floor(b.min.y - margin);
        const int w = (int)std::ceil(b.max.x + margin) - left, h = (int)std::ceil(b.max.y + margin) - top;
        if (w > kMaxField || h > kMaxField)
            return false;
        const std::size_t texels = (std::size_t)w * (std::size_t)h;

        // ---- inside or outside of every texel center (non-zero), row by row
        std::vector<std::uint8_t> inside(texels, 0);
        {
            std::vector<std::pair<float, int>> crossings;
            for (int y = 0; y < h; ++y)
            {
                const float py = (float)(top + y) + 0.5f;
                crossings.clear();
                int total = 0;
                for (const Piece& s : all)
                    if ((s.a.y <= py) != (s.b.y <= py))
                    {
                        const int d = s.b.y > s.a.y ? 1 : -1;
                        crossings.emplace_back(s.a.x + (py - s.a.y) * (s.b.x - s.a.x) / (s.b.y - s.a.y), d);
                        total += d;
                    }
                std::sort(crossings.begin(), crossings.end());
                std::size_t k = 0;
                int winding = total;   // the crossings right of the texel (Winding's ray towards +x)
                for (int x = 0; x < w; ++x)
                {
                    const float px = (float)(left + x) + 0.5f;
                    while (k < crossings.size() && crossings[k].first <= px)
                        winding -= crossings[k++].second;
                    inside[(std::size_t)y * w + x] = winding != 0;
                }
            }
        }

        // ---- the pieces near each cell of texels (within the multi-channel range)
        const float reach = range * 0.5f + 1.5f;
        const int gw = (w + kCell - 1) / kCell, gh = (h + kCell - 1) / kCell;
        std::vector<std::vector<std::uint32_t>> grid((std::size_t)gw * (std::size_t)gh);
        for (std::uint32_t i = 0; i < (std::uint32_t)pieces.size(); ++i)
        {
            const Piece& s = pieces[i];
            const int x0 = std::max(0, (int)std::floor((std::min(s.a.x, s.b.x) - reach - (float)left) / kCell));
            const int x1 = std::min(gw - 1, (int)std::floor((std::max(s.a.x, s.b.x) + reach - (float)left) / kCell));
            const int y0 = std::max(0, (int)std::floor((std::min(s.a.y, s.b.y) - reach - (float)top) / kCell));
            const int y1 = std::min(gh - 1, (int)std::floor((std::max(s.a.y, s.b.y) + reach - (float)top) / kCell));
            for (int gy = y0; gy <= y1; ++gy)
                for (int gx = x0; gx <= x1; ++gx)
                    grid[(std::size_t)gy * gw + gx].push_back(i);
        }

        // ---- per texel: each channel's pseudo-distance to its nearest run, the true distance to the boundary
        constexpr float kFar = std::numeric_limits<float>::max();
        std::vector<float> rgb(texels * 3);
        std::vector<Vec2> nearest(texels);
        std::vector<std::uint8_t> hasNearest(texels, 0);
        const float limit = range * 0.5f;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const std::size_t i = (std::size_t)y * w + x;
                const Vec2 p((float)(left + x) + 0.5f, (float)(top + y) + 0.5f);
                struct Best
                {
                    float d2 = kFar, ortho = 0.0f, t = 0.0f;
                    std::uint32_t piece = 0;
                    bool found = false;
                } best[3];
                float trueD2 = kFar;
                Vec2 trueQ;
                for (std::uint32_t pi : grid[(std::size_t)(y / kCell) * gw + (x / kCell)])
                {
                    const Piece& s = pieces[pi];
                    const Vec2 ab = s.b - s.a, ap = p - s.a;
                    const float len2 = LengthSq(ab);
                    const float t = len2 > 0.0f ? Clamp(Dot(ap, ab) / len2, 0.0f, 1.0f) : 0.0f;
                    const Vec2 q = s.a + ab * t, pq = p - q;
                    const float d2 = LengthSq(pq);
                    if (d2 < trueD2)
                    {
                        trueD2 = d2;
                        trueQ = q;
                    }
                    // at an end point the more perpendicular piece wins a tie (its side is the right one)
                    const float ortho = (t > 0.0f && t < 1.0f) || d2 == 0.0f ? 0.0f : std::fabs(Dot(Unit(ab), Unit(pq)));
                    for (int c = 0; c < 3; ++c)
                        if (s.color & (1 << c))
                        {
                            Best& bc = best[c];
                            if (!bc.found || d2 < bc.d2 * (1.0f - 1e-5f) || (d2 <= bc.d2 * (1.0f + 1e-5f) && ortho < bc.ortho))
                                bc = {d2, ortho, t, pi, true};
                        }
                }
                if (trueD2 < kFar)
                {
                    nearest[i] = trueQ;
                    hasNearest[i] = 1;
                }
                for (int c = 0; c < 3; ++c)
                {
                    float d = inside[i] ? limit : -limit;
                    if (best[c].found)
                    {
                        const Piece& s = pieces[best[c].piece];
                        const Vec2 ab = s.b - s.a, q = s.a + ab * best[c].t;
                        float sd = std::sqrt(best[c].d2);
                        if (Cross(ab, p - q) < 0.0f)
                            sd = -sd;
                        // past a run's end: the distance to its tangent line (the corner's other side stays sharp)
                        if (best[c].t <= 0.0f && s.runStart)
                        {
                            const Vec2 dir = Unit(s.ta);
                            if (Dot(dir, p - s.a) < 0.0f)
                            {
                                const float ps = Cross(dir, p - s.a);
                                if (std::fabs(ps) <= std::fabs(sd))
                                    sd = ps;
                            }
                        }
                        if (best[c].t >= 1.0f && s.runEnd)
                        {
                            const Vec2 dir = Unit(s.tb);
                            if (Dot(dir, p - s.b) > 0.0f)
                            {
                                const float ps = Cross(dir, p - s.b);
                                if (std::fabs(ps) <= std::fabs(sd))
                                    sd = ps;
                            }
                        }
                        d = Clamp(sd, -limit, limit);
                    }
                    rgb[i * 3 + (std::size_t)c] = d;
                }
                // the median must say what the winding says (a texel past several runs' ends may disagree)
                float* v = &rgb[i * 3];
                if ((Median(v[0], v[1], v[2]) > 0.0f) != (inside[i] != 0))
                    for (int c = 0; c < 3; ++c)
                        v[c] = -v[c];
            }

        // ---- the true distance farther out: nearest boundary points handed on from texel to texel
        const auto relax = [&](int x, int y, int nx, int ny) {
            if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                return;
            const std::size_t i = (std::size_t)y * w + x, n = (std::size_t)ny * w + nx;
            if (!hasNearest[n])
                return;
            const Vec2 p((float)(left + x) + 0.5f, (float)(top + y) + 0.5f);
            if (!hasNearest[i] || LengthSq(p - nearest[n]) < LengthSq(p - nearest[i]))
            {
                nearest[i] = nearest[n];
                hasNearest[i] = 1;
            }
        };
        for (int pass = 0; pass < 2; ++pass)
        {
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                {
                    relax(x, y, x - 1, y);
                    relax(x, y, x - 1, y - 1);
                    relax(x, y, x, y - 1);
                    relax(x, y, x + 1, y - 1);
                }
            for (int y = h - 1; y >= 0; --y)
                for (int x = w - 1; x >= 0; --x)
                {
                    relax(x, y, x + 1, y);
                    relax(x, y, x + 1, y + 1);
                    relax(x, y, x, y + 1);
                    relax(x, y, x - 1, y + 1);
                }
        }

        // ---- 0 .. 1, clashes flattened to the median, RGBA8
        std::vector<float> norm(texels * 3);
        for (std::size_t i = 0; i < texels * 3; ++i)
            norm[i] = Saturate(0.5f + rgb[i] / range);
        const float threshold = 1.001f / range;
        std::vector<std::size_t> clashes;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const float* a = &norm[((std::size_t)y * w + x) * 3];
                if ((x > 0 && Clash(a, a - 3, threshold)) || (x < w - 1 && Clash(a, a + 3, threshold)) ||
                    (y > 0 && Clash(a, a - (std::size_t)w * 3, threshold)) || (y < h - 1 && Clash(a, a + (std::size_t)w * 3, threshold)))
                    clashes.push_back((std::size_t)y * w + x);
            }
        for (std::size_t i : clashes)
        {
            float* v = &norm[i * 3];
            v[0] = v[1] = v[2] = Median(v[0], v[1], v[2]);
        }
        out.left = left;
        out.top = top;
        out.width = w;
        out.height = h;
        out.pixels.resize(texels * 4);
        const auto q8 = [](float v) { return (std::uint8_t)(Saturate(v) * 255.0f + 0.5f); };
        for (std::size_t i = 0; i < texels; ++i)
        {
            const Vec2 p((float)(left + (int)(i % (std::size_t)w)) + 0.5f, (float)(top + (int)(i / (std::size_t)w)) + 0.5f);
            float a = alphaRange * 0.5f;
            if (hasNearest[i])
                a = std::min(Length(p - nearest[i]), a);
            out.pixels[i * 4 + 0] = q8(norm[i * 3 + 0]);
            out.pixels[i * 4 + 1] = q8(norm[i * 3 + 1]);
            out.pixels[i * 4 + 2] = q8(norm[i * 3 + 2]);
            out.pixels[i * 4 + 3] = q8(0.5f + (inside[i] ? a : -a) / alphaRange);
        }
        return true;
    }
}
