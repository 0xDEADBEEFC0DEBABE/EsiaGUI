// Esia - SVG coverage (svg_internal.hpp): rows of cells, and the sweep that leaves in them the boundary of the area an
// outline fills.
//
// An outline is cut into monotone chains (its runs downward or upward). In a row, the chains across it are put in order
// left to right; where they keep apart - the usual case - the winding count across them tells which chains bound the
// filled area, and only those leave their area in the cells, +1 where the area starts, -1 where it ends. Where they come
// near each other in the row (a turning point, an overlap, a crossing), the row is cut into strips at every vertex and
// crossing and each strip is walked the same way. So the cells sum to the area of the filled region in every pixel
// under either fill rule, also where contours overlap - two shapes' edges in a pixel, a stroke's pieces - which the
// usual |sum| clamped to 1 over-covers.
#include "svg_internal.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <initializer_list>
#include <limits>

namespace esia::svg::detail
{
    // ------------------------------------------------------------------ cells
    void Cells::Reset(int width)
    {
        width_ = width;
        acc_.assign((std::size_t)width + 3, 0.0f);
        bits_.assign(((std::size_t)width + 3 + 63) / 64, 0);
        lo_ = (int)bits_.size();
        hi_ = -1;
    }

    // cells x0 .. x1 hold something
    void Cells::Mark(int x0, int x1)
    {
        const int w0 = x0 >> 6, w1 = x1 >> 6;
        if (w0 == w1)
            bits_[(std::size_t)w0] |= (~0ull >> (63 - (x1 - x0))) << (x0 & 63);
        else
        {
            bits_[(std::size_t)w0] |= ~0ull << (x0 & 63);
            for (int w = w0 + 1; w < w1; ++w)
                bits_[(std::size_t)w] = ~0ull;
            bits_[(std::size_t)w1] |= ~0ull >> (63 - (x1 & 63));
        }
        lo_ = std::min(lo_, w0);
        hi_ = std::max(hi_, w1);
    }

    // A piece across cells x in [min(xa, xb), max(xa, xb)] (0 .. width), `d` its signed height: the area right of it in
    // every cell it crosses, as differences whose running sum is the coverage (font-rs's accumulation).
    void Cells::Piece(float xa, float xb, float d)
    {
        const float x0 = std::min(xa, xb), x1 = std::max(xa, xb);
        const float x0floor = std::floor(x0);
        const int x0i = (int)x0floor;
        const float x1ceil = std::ceil(x1);
        const int x1i = (int)x1ceil;
        float* row = acc_.data();
        if (x1i <= x0i + 1)
        {
            const float xmf = 0.5f * (xa + xb) - x0floor;
            row[x0i] += d - d * xmf;
            row[x0i + 1] += d * xmf;
            Mark(x0i, x0i + 1);
            return;
        }
        const float s = 1.0f / (x1 - x0);
        const float x0f = x0 - x0floor;
        const float a0 = 0.5f * s * (1.0f - x0f) * (1.0f - x0f);
        const float x1f = x1 - x1ceil + 1.0f;
        const float am = 0.5f * s * x1f * x1f;
        row[x0i] += d * a0;
        if (x1i == x0i + 2)
            row[x0i + 1] += d * (1.0f - a0 - am);
        else
        {
            const float a1 = s * (1.5f - x0f);
            row[x0i + 1] += d * (a1 - a0);
            const float ds = d * s;
            for (int xi = x0i + 2; xi < x1i - 1; ++xi)
                row[xi] += ds;
            const float a2 = a1 + (float)(x1i - x0i - 3) * s;
            row[x1i - 1] += d * (1.0f - a2 - am);
        }
        row[x1i] += d * am;
        Mark(x0i, x1i);
    }

    void Cells::Add(float xa, float ya, float xb, float yb, float sign)
    {
        const float dy = yb - ya;
        if (!(dy > 0.0f))
            return;
        const float w = (float)width_;
        const float lo = std::min(xa, xb), hi = std::max(xa, xb);
        if (lo >= 0.0f && hi <= w)
        {
            Piece(xa, xb, dy * sign);
            return;
        }
        if (hi <= 0.0f)
        {
            Piece(0.0f, 0.0f, dy * sign);   // left of the row: an edge at its left end
            return;
        }
        if (lo >= w)
            return;   // right of the row: nothing in its pixels
        // the parts left of 0, inside, right of the width
        float t[4] = {0.0f, 1.0f, 1.0f, 1.0f};
        int n = 1;
        const float dx = xb - xa;
        for (const float edge : {0.0f, w})
            if (lo < edge && edge < hi)
                t[n++] = (edge - xa) / dx;
        t[n++] = 1.0f;
        std::sort(t, t + n);
        for (int k = 0; k + 1 < n; ++k)
        {
            const float ta = t[k], tb = t[k + 1];
            if (!(tb > ta))
                continue;
            const float xm = xa + dx * 0.5f * (ta + tb);
            const float d = dy * (tb - ta) * sign;
            if (xm <= 0.0f)
                Piece(0.0f, 0.0f, d);
            else if (xm < w)
                Piece(Clamp(xa + dx * ta, 0.0f, w), Clamp(xa + dx * tb, 0.0f, w), d);
        }
    }

    void Cells::Resolve(std::vector<Span>& spans, float* cov, bool crisp)
    {
        spans.clear();
        if (hi_ < lo_)
            return;
        constexpr float kEps = 1e-5f;
        const auto value = [crisp](float s) {
            const float c = std::min(std::fabs(s), 1.0f);
            return crisp ? (c >= 0.5f ? 1.0f : 0.0f) : c;
        };
        const auto run = [&](int x0, int x1, float c) {
            if (c <= kEps)
                return;
            spans.push_back({x0, x1, c >= 1.0f - kEps ? 1.0f : c});
        };
        float sum = 0.0f;
        int x = 0;   // the pixels before it are done
        for (int w = lo_; w <= hi_; ++w)
        {
            std::uint64_t bits = bits_[(std::size_t)w];
            if (!bits)
                continue;
            bits_[(std::size_t)w] = 0;
            while (bits)
            {
                const int cx = (w << 6) + std::countr_zero(bits);
                bits &= bits - 1;
                if (cx >= width_)
                {
                    acc_[(std::size_t)cx] = 0.0f;   // past the row's last pixel
                    continue;
                }
                if (cx > x)
                    run(x, cx, value(sum));
                sum += acc_[(std::size_t)cx];
                acc_[(std::size_t)cx] = 0.0f;
                cov[cx] = value(sum);
                if (!spans.empty() && spans.back().value < 0.0f && spans.back().x1 == cx)
                    spans.back().x1 = cx + 1;
                else
                    spans.push_back({cx, cx + 1, -1.0f});
                x = cx + 1;
            }
        }
        if (x < width_)
            run(x, width_, value(sum));
        lo_ = (int)bits_.size();
        hi_ = -1;
    }

    // ------------------------------------------------------------------ sweep
    namespace
    {
        int Sign(float v) { return v > 0.0f ? 1 : (v < 0.0f ? -1 : 0); }
        // Crossings a row cuts its strips at before the rest of it goes in strips of 1/32 pixel, each walked in the order
        // of its middle: an outline crossing itself thousands of times still costs a bounded time a row, its coverage
        // off by at most about 1/64 where pieces cross inside those thin strips.
        constexpr int kRowSplits = 256;
        constexpr float kThinStrips = 32.0f;
    }

    void Sweep::Build(const Contours& contours, const std::uint8_t* evenOdd, int groups, int width, int height)
    {
        pts_.clear();
        chains_.clear();
        active_.clear();
        next_ = 0;
        width_ = width;
        evenOdd_.assign(evenOdd, evenOdd + groups);
        winding_.assign((std::size_t)groups, 0);
        float minX = std::numeric_limits<float>::max(), minY = minX, maxX = -minX, maxY = -minX;
        const std::size_t count = contours.starts.size();
        for (std::size_t k = 0; k < count; ++k)
        {
            const std::size_t s = contours.starts[k], e = k + 1 < count ? contours.starts[k + 1] : contours.pts.size();
            const std::size_t n = e - s;
            if (n < 3)
                continue;
            const Vec2* p = contours.pts.data() + s;
            bool finite = true;
            for (std::size_t i = 0; i < n && finite; ++i)
                finite = std::isfinite(p[i].x) && std::isfinite(p[i].y);
            if (!finite)
                continue;
            // start where the contour turns from down to up or back, so no chain wraps around its start
            const auto dir = [&](std::size_t i) { return Sign(p[(i + 1) % n].y - p[i].y); };
            std::size_t first = n;
            int last = 0;
            for (std::size_t i = 0; i < n; ++i)
                if (dir(i) != 0)
                {
                    first = i;
                    last = dir(i);
                    break;
                }
            if (first == n)
                continue;   // flat: no area
            std::size_t start = n;
            for (std::size_t q = 1; q <= n; ++q)
            {
                const std::size_t i = (first + q) % n;
                const int d = dir(i);
                if (d != 0 && d != last)
                {
                    start = i;
                    break;
                }
                if (d != 0)
                    last = d;
            }
            if (start == n)
                continue;
            const std::uint16_t group = contours.groups[k];
            int cdir = dir(start);
            std::uint32_t begin = (std::uint32_t)pts_.size();
            const auto finish = [&] {
                const std::uint32_t end = (std::uint32_t)pts_.size();
                if (cdir < 0)
                    std::reverse(pts_.begin() + begin, pts_.end());
                chains_.push_back({begin, end - 1, (float)cdir, group});
            };
            pts_.push_back(p[start]);
            for (std::size_t q = 0; q < n; ++q)
            {
                const std::size_t i = (start + q) % n;
                const int d = dir(i);
                if (d != 0 && d != cdir)
                {
                    finish();
                    begin = (std::uint32_t)pts_.size();
                    pts_.push_back(p[i]);
                    cdir = d;
                }
                pts_.push_back(p[(i + 1) % n]);
            }
            finish();
            for (std::size_t i = 0; i < n; ++i)
            {
                minX = std::min(minX, p[i].x);
                maxX = std::max(maxX, p[i].x);
                minY = std::min(minY, p[i].y);
                maxY = std::max(maxY, p[i].y);
            }
        }
        if (chains_.empty())
        {
            top_ = bottom_ = left_ = right_ = 0;
            return;
        }
        std::stable_sort(chains_.begin(), chains_.end(), [&](const Chain& a, const Chain& b) { return pts_[a.first].y < pts_[b.first].y; });
        const auto clampi = [](float v, int lo, int hi) { return v <= (float)lo ? lo : (v >= (float)hi ? hi : (int)v); };
        top_ = clampi(std::floor(minY), 0, height);
        bottom_ = clampi(std::ceil(maxY), 0, height);
        left_ = clampi(std::floor(minX), 0, width);
        right_ = clampi(std::ceil(maxX) + 1.0f, 0, width);
    }

    void Sweep::Deposit(const Active& a, float y0, Cells& cells, float sign) const
    {
        const Chain& c = chains_[a.chain];
        const float y1 = y0 + 1.0f;
        for (std::uint32_t k = a.seg; k < c.last; ++k)
        {
            const Vec2 p = pts_[k], q = pts_[k + 1];
            if (p.y >= y1)
                break;
            if (q.y <= y0 || q.y == p.y)
                continue;
            const float ya = std::max(p.y, y0), yb = std::min(q.y, y1);
            const float kx = (q.x - p.x) / (q.y - p.y);
            cells.Add(p.x + (ya - p.y) * kx, ya - y0, p.x + (yb - p.y) * kx, yb - y0, sign);
        }
    }

    void Sweep::SimpleRow(float y0, Cells& cells)
    {
        int in = 0;   // groups the walk is inside of
        bool inside = false;
        for (const Active& a : active_)
        {
            const Chain& c = chains_[a.chain];
            int& w = winding_[c.group];
            const bool eo = evenOdd_[c.group] != 0;
            const bool was = eo ? (w & 1) != 0 : w != 0;
            w += (int)c.dir;
            const bool now = eo ? (w & 1) != 0 : w != 0;
            if (was != now)
                in += now ? 1 : -1;
            if ((in > 0) != inside)
            {
                inside = in > 0;
                Deposit(a, y0, cells, inside ? 1.0f : -1.0f);
            }
        }
        std::fill(winding_.begin(), winding_.end(), 0);
    }

    // a strip of thin strips, each walked in the order of its middle (a row past its crossings budget)
    void Sweep::Thin(std::vector<Piece>& pieces, float ya, float yb, float y0, Cells& cells)
    {
        const int parts = std::max(1, (int)std::ceil((yb - ya) * kThinStrips));
        thin_.resize(pieces.size());
        for (int k = 0; k < parts; ++k)
        {
            const float t0 = (float)k / (float)parts, t1 = (float)(k + 1) / (float)parts;
            for (std::size_t i = 0; i < pieces.size(); ++i)
            {
                const Piece& p = pieces[i];
                thin_[i] = {p.chain, p.xa + (p.xb - p.xa) * t0, p.xa + (p.xb - p.xa) * t1};
            }
            std::sort(thin_.begin(), thin_.end(), [](const Piece& a, const Piece& b) { return a.xa + a.xb < b.xa + b.xb; });
            const float sa = ya + (yb - ya) * t0, sb = ya + (yb - ya) * t1;
            int in = 0;
            bool inside = false;
            for (const Piece& p : thin_)
            {
                const Chain& c = chains_[p.chain];
                int& w = winding_[c.group];
                const bool eo = evenOdd_[c.group] != 0;
                const bool was = eo ? (w & 1) != 0 : w != 0;
                w += (int)c.dir;
                const bool now = eo ? (w & 1) != 0 : w != 0;
                if (was != now)
                    in += now ? 1 : -1;
                if ((in > 0) != inside)
                {
                    inside = in > 0;
                    cells.Add(p.xa, sa - y0, p.xb, sb - y0, inside ? 1.0f : -1.0f);
                }
            }
            std::fill(winding_.begin(), winding_.end(), 0);
        }
    }

    // a strip [ya, yb] of the row at y0, its pieces in any order: cut at the crossings, each part ordered and walked
    void Sweep::Walk(std::vector<Piece>& pieces, float ya, float yb, float y0, Cells& cells)
    {
        for (;;)
        {
            if (budget_ <= 0)
            {
                Thin(pieces, ya, yb, y0, cells);
                return;
            }
            --budget_;
            // in order across the strip's top (pieces from one point: in the order they part)
            for (std::size_t i = 1; i < pieces.size(); ++i)
            {
                const Piece p = pieces[i];
                std::size_t j = i;
                while (j > 0 && (pieces[j - 1].xa > p.xa || (pieces[j - 1].xa == p.xa && pieces[j - 1].xb > p.xb)))
                {
                    pieces[j] = pieces[j - 1];
                    --j;
                }
                pieces[j] = p;
            }
            // the first crossing is of pieces next to each other at the top
            float t = 1.0f;
            std::size_t pair = pieces.size();
            for (std::size_t i = 0; i + 1 < pieces.size(); ++i)
                {
                    const float da = pieces[i + 1].xa - pieces[i].xa, db = pieces[i + 1].xb - pieces[i].xb;
                    if (db < 0.0f)
                    {
                        const float tc = da / (da - db);
                        if (tc < t)
                        {
                            t = tc;
                            pair = i;
                        }
                    }
                }
            if (pair < pieces.size() && t <= 1e-6f)
            {
                // crossing at the top: one point for both, and they sort in the order they part
                const float x = 0.5f * (pieces[pair].xa + pieces[pair + 1].xa);
                pieces[pair].xa = pieces[pair + 1].xa = x;
                continue;
            }
            const bool cut = pair < pieces.size() && t < 1.0f - 1e-6f;
            const float ym = cut ? ya + (yb - ya) * t : yb;
            int in = 0;
            bool inside = false;
            for (Piece& p : pieces)
            {
                const float xm = cut ? p.xa + (p.xb - p.xa) * t : p.xb;
                const Chain& c = chains_[p.chain];
                int& w = winding_[c.group];
                const bool eo = evenOdd_[c.group] != 0;
                const bool was = eo ? (w & 1) != 0 : w != 0;
                w += (int)c.dir;
                const bool now = eo ? (w & 1) != 0 : w != 0;
                if (was != now)
                    in += now ? 1 : -1;
                if ((in > 0) != inside)
                {
                    inside = in > 0;
                    cells.Add(p.xa, ya - y0, xm, ym - y0, inside ? 1.0f : -1.0f);
                }
                p.xa = xm;
            }
            std::fill(winding_.begin(), winding_.end(), 0);
            if (!cut)
                return;
            // the crossing pair meet at one point, so the next part orders them as they go on
            const float x = 0.5f * (pieces[pair].xa + pieces[pair + 1].xa);
            pieces[pair].xa = pieces[pair + 1].xa = x;
            ya = ym;
        }
    }

    void Sweep::ComplexRow(float y0, Cells& cells)
    {
        const float y1 = y0 + 1.0f;
        events_.clear();
        events_.push_back(y0);
        events_.push_back(y1);
        for (const Active& a : active_)
        {
            const Chain& c = chains_[a.chain];
            for (std::uint32_t k = a.seg; k <= c.last && pts_[k].y < y1; ++k)
                if (pts_[k].y > y0)
                    events_.push_back(pts_[k].y);
        }
        std::sort(events_.begin(), events_.end());
        events_.erase(std::unique(events_.begin(), events_.end()), events_.end());
        budget_ = kRowSplits;
        segs_.resize(active_.size());
        for (std::size_t i = 0; i < active_.size(); ++i)
            segs_[i] = active_[i].seg;
        for (std::size_t e = 0; e + 1 < events_.size(); ++e)
        {
            const float ya = events_[e], yb = events_[e + 1];
            pieces_.clear();
            for (std::size_t i = 0; i < active_.size(); ++i)
            {
                const Chain& c = chains_[active_[i].chain];
                if (pts_[c.first].y > ya || pts_[c.last].y < yb)
                    continue;   // not across this strip
                std::uint32_t& s = segs_[i];
                while (s + 1 < c.last && pts_[s + 1].y <= ya)
                    ++s;
                const Vec2 p = pts_[s], q = pts_[s + 1];
                if (!(q.y > p.y))
                    continue;
                const float kx = (q.x - p.x) / (q.y - p.y);
                pieces_.push_back({active_[i].chain, p.x + (ya - p.y) * kx, p.x + (yb - p.y) * kx});
            }
            if (pieces_.size() >= 2)
                Walk(pieces_, ya, yb, y0, cells);
        }
    }

    void Sweep::Pass(int y)
    {
        const float y0 = (float)y, y1 = y0 + 1.0f;
        while (next_ < chains_.size() && pts_[chains_[next_].first].y < y1)
        {
            const Chain& c = chains_[next_];
            if (pts_[c.last].y > y0)
                active_.push_back({next_, c.first, 0.0f, 0.0f});
            ++next_;
        }
        // the chains still across the row, and their extents across it
        std::size_t live = 0;
        for (std::size_t i = 0; i < active_.size(); ++i)
        {
            Active a = active_[i];
            const Chain& c = chains_[a.chain];
            if (pts_[c.last].y <= y0)
                continue;
            while (a.seg + 1 < c.last && pts_[a.seg + 1].y <= y0)
                ++a.seg;
            float lo = std::numeric_limits<float>::max(), hi = -lo;
            for (std::uint32_t k = a.seg; k < c.last; ++k)
            {
                const Vec2 p = pts_[k], q = pts_[k + 1];
                if (p.y >= y1)
                    break;
                if (q.y <= y0)
                    continue;
                if (q.y == p.y)
                {
                    lo = std::min(lo, std::min(p.x, q.x));
                    hi = std::max(hi, std::max(p.x, q.x));
                    continue;
                }
                const float ya = std::max(p.y, y0), yb = std::min(q.y, y1);
                const float kx = (q.x - p.x) / (q.y - p.y);
                const float xa = p.x + (ya - p.y) * kx, xb = p.x + (yb - p.y) * kx;
                lo = std::min(lo, std::min(xa, xb));
                hi = std::max(hi, std::max(xa, xb));
            }
            a.xmin = lo;
            a.xmax = hi;
            active_[live++] = a;
        }
        active_.resize(live);
        for (std::size_t i = 1; i < active_.size(); ++i)
        {
            const Active a = active_[i];
            std::size_t j = i;
            while (j > 0 && active_[j - 1].xmin > a.xmin)
            {
                active_[j] = active_[j - 1];
                --j;
            }
            active_[j] = a;
        }
    }

    void Sweep::Row(int y, Cells& cells)
    {
        Pass(y);
        if (active_.empty())
            return;
        const float y0 = (float)y;
        // chains apart from each other across the row: one walk for the whole row
        float reach = -std::numeric_limits<float>::max();
        bool apart = true;
        for (const Active& a : active_)
        {
            if (a.xmin < reach)
            {
                apart = false;
                break;
            }
            reach = std::max(reach, a.xmax);
        }
        if (apart)
            SimpleRow(y0, cells);
        else
            ComplexRow(y0, cells);
    }
}
