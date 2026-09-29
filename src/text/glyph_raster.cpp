// WGT UI - analytic glyph rasterizer (see glyph_raster.hpp).
//
// Outline: IDWriteFontFace::GetGlyphRunOutline -> contours of lines and cubic Béziers (y down, baseline
// at 0). Béziers are flattened to within 1/12 px. Coverage: signed-area accumulation per scanline (each
// edge deposits the exact area it covers in the cells it crosses, a running sum across the row gives the
// covered fraction of every pixel), non-zero winding by clamping |sum| to 1.
#include "text/glyph_raster.hpp"
#include <d2d1.h>
#include <algorithm>
#include <cmath>

namespace wgt
{
    namespace
    {
        struct Pt
        {
            float x, y;
        };

        class OutlineSink final : public IDWriteGeometrySink
        {
        public:
            std::vector<std::vector<Pt>> contours;

            // IUnknown (stack object)
            HRESULT __stdcall QueryInterface(REFIID riid, void** obj) override
            {
                if (riid == __uuidof(IDWriteGeometrySink) || riid == __uuidof(IUnknown))
                {
                    *obj = this;
                    return S_OK;
                }
                *obj = nullptr;
                return E_NOINTERFACE;
            }
            ULONG __stdcall AddRef() override { return 1; }
            ULONG __stdcall Release() override { return 1; }

            void __stdcall SetFillMode(D2D1_FILL_MODE) override {}
            void __stdcall SetSegmentFlags(D2D1_PATH_SEGMENT) override {}
            void __stdcall BeginFigure(D2D1_POINT_2F p, D2D1_FIGURE_BEGIN) override
            {
                contours.emplace_back();
                contours.back().push_back({p.x, p.y});
            }
            void __stdcall AddLines(const D2D1_POINT_2F* pts, UINT32 count) override
            {
                if (contours.empty())
                    return;
                for (UINT32 i = 0; i < count; ++i)
                    contours.back().push_back({pts[i].x, pts[i].y});
            }
            void __stdcall AddBeziers(const D2D1_BEZIER_SEGMENT* b, UINT32 count) override
            {
                if (contours.empty())
                    return;
                std::vector<Pt>& c = contours.back();
                for (UINT32 i = 0; i < count; ++i)
                {
                    const Pt p0 = c.back();
                    const Pt p1{b[i].point1.x, b[i].point1.y}, p2{b[i].point2.x, b[i].point2.y}, p3{b[i].point3.x, b[i].point3.y};
                    // subdivisions so the flattening error stays below `tol` (second-difference bound)
                    const float ax = p0.x - 2.0f * p1.x + p2.x, ay = p0.y - 2.0f * p1.y + p2.y;
                    const float bx = p1.x - 2.0f * p2.x + p3.x, by = p1.y - 2.0f * p2.y + p3.y;
                    const float dd = std::sqrt(std::max(ax * ax + ay * ay, bx * bx + by * by));
                    constexpr float tol = 1.0f / 12.0f;
                    const int n = std::clamp((int)std::ceil(std::sqrt(0.75f * dd / tol)), 1, 64);
                    for (int k = 1; k <= n; ++k)
                    {
                        const float t = (float)k / (float)n, u = 1.0f - t;
                        const float w0 = u * u * u, w1 = 3.0f * u * u * t, w2 = 3.0f * u * t * t, w3 = t * t * t;
                        c.push_back({w0 * p0.x + w1 * p1.x + w2 * p2.x + w3 * p3.x, w0 * p0.y + w1 * p1.y + w2 * p2.y + w3 * p3.y});
                    }
                }
            }
            void __stdcall EndFigure(D2D1_FIGURE_END) override
            {
                if (!contours.empty() && contours.back().size() > 1)
                {
                    const Pt a = contours.back().front(), b = contours.back().back();
                    if (a.x != b.x || a.y != b.y)
                        contours.back().push_back(a);
                }
            }
            HRESULT __stdcall Close() override { return S_OK; }
        };

        // Signed-area accumulation of one edge (bitmap coordinates, y down).
        class Accumulator
        {
        public:
            Accumulator(int w, int h) : w_(w), h_(h), a_((size_t)w * h + 4, 0.0f) {}

            void Line(Pt p0, Pt p1)
            {
                if (std::fabs(p0.y - p1.y) <= 1e-6f)
                    return;
                float dir = 1.0f;
                if (p0.y > p1.y)
                {
                    std::swap(p0, p1);
                    dir = -1.0f;
                }
                const float dxdy = (p1.x - p0.x) / (p1.y - p0.y);
                float x = p0.x;
                if (p0.y < 0.0f)
                    x -= p0.y * dxdy;
                const int yEnd = std::min(h_, (int)std::ceil(p1.y));
                for (int y = std::max(0, (int)p0.y); y < yEnd; ++y)
                {
                    float* row = a_.data() + (size_t)y * w_;
                    const float dy = std::min((float)(y + 1), p1.y) - std::max((float)y, p0.y);
                    const float xnext = x + dxdy * dy;
                    const float d = dy * dir;
                    const float x0 = std::min(x, xnext), x1 = std::max(x, xnext);
                    const float x0floor = std::floor(x0);
                    const int x0i = (int)x0floor;
                    const float x1ceil = std::ceil(x1);
                    const int x1i = (int)x1ceil;
                    if (x0i < 0 || x1i + 1 >= w_)
                    {
                        x = xnext;   // outside the padded box: cannot happen for a correct bounding box
                        continue;
                    }
                    if (x1i <= x0i + 1)
                    {
                        const float xmf = 0.5f * (x + xnext) - x0floor;
                        row[x0i] += d - d * xmf;
                        row[x0i + 1] += d * xmf;
                    }
                    else
                    {
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
                            for (int xi = x0i + 2; xi < x1i - 1; ++xi)
                                row[xi] += d * s;
                            const float a2 = a1 + (float)(x1i - x0i - 3) * s;
                            row[x1i - 1] += d * (1.0f - a2 - am);
                        }
                        row[x1i] += d * am;
                    }
                    x = xnext;
                }
            }

            // coverage in [0, 1] per cell (sub-pixel pipeline filters it before quantizing)
            void ResolveFloat(std::vector<float>& out) const
            {
                out.resize((size_t)w_ * h_);
                for (int y = 0; y < h_; ++y)
                {
                    float acc = 0.0f;
                    const float* row = a_.data() + (size_t)y * w_;
                    float* dst = out.data() + (size_t)y * w_;
                    for (int x = 0; x < w_; ++x)
                    {
                        acc += row[x];
                        dst[x] = std::min(std::fabs(acc), 1.0f);
                    }
                }
            }

            void Resolve(std::vector<unsigned char>& out) const
            {
                out.resize((size_t)w_ * h_);
                for (int y = 0; y < h_; ++y)
                {
                    float acc = 0.0f;
                    const float* row = a_.data() + (size_t)y * w_;
                    unsigned char* dst = out.data() + (size_t)y * w_;
                    for (int x = 0; x < w_; ++x)
                    {
                        acc += row[x];
                        const float c = std::min(std::fabs(acc), 1.0f);
                        dst[x] = (unsigned char)(c * 255.0f + 0.5f);
                    }
                }
            }

        private:
            int w_, h_;
            std::vector<float> a_;
        };
    }

    bool RasterizeGlyphOutline(IDWriteFontFace* face, std::uint16_t glyph, float emPixels, float offsetX, GlyphBitmap& out)
    {
        out = {};
        if (!face || emPixels <= 0.0f)
            return false;
        OutlineSink sink;
        if (FAILED(face->GetGlyphRunOutline(emPixels, &glyph, nullptr, nullptr, 1, FALSE, FALSE, &sink)))
            return false;
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        size_t points = 0;
        for (const auto& c : sink.contours)
            for (const Pt& p : c)
            {
                minX = std::min(minX, p.x + offsetX);
                maxX = std::max(maxX, p.x + offsetX);
                minY = std::min(minY, p.y);
                maxY = std::max(maxY, p.y);
                ++points;
            }
        if (points < 2)
            return true;   // empty glyph (space): valid, nothing to draw
        const int left = (int)std::floor(minX) - 1, top = (int)std::floor(minY) - 1;
        const int right = (int)std::ceil(maxX) + 2, bottom = (int)std::ceil(maxY) + 1;
        const int w = right - left, h = bottom - top;
        if (w <= 0 || h <= 0 || w > 4096 || h > 4096)
            return false;

        Accumulator acc(w, h);
        for (const auto& c : sink.contours)
            for (size_t i = 1; i < c.size(); ++i)
                acc.Line({c[i - 1].x + offsetX - left, c[i - 1].y - top}, {c[i].x + offsetX - left, c[i].y - top});
        acc.Resolve(out.pixels);
        out.left = left;
        out.top = top;
        out.width = w;
        out.height = h;
        return true;
    }

    bool RasterizeGlyphOutlineLcd(IDWriteFontFace* face, std::uint16_t glyph, float emPixels, float offsetX, bool bgr, GlyphBitmap& out)
    {
        out = {};
        if (!face || emPixels <= 0.0f)
            return false;
        OutlineSink sink;
        if (FAILED(face->GetGlyphRunOutline(emPixels, &glyph, nullptr, nullptr, 1, FALSE, FALSE, &sink)))
            return false;
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        size_t points = 0;
        for (const auto& c : sink.contours)
            for (const Pt& p : c)
            {
                minX = std::min(minX, p.x + offsetX);
                maxX = std::max(maxX, p.x + offsetX);
                minY = std::min(minY, p.y);
                maxY = std::max(maxY, p.y);
                ++points;
            }
        if (points < 2)
            return true;
        // the filter spreads 2 stripes to each side: one extra pixel of margin left and right
        const int left = (int)std::floor(minX) - 2, top = (int)std::floor(minY) - 1;
        const int right = (int)std::ceil(maxX) + 3, bottom = (int)std::ceil(maxY) + 1;
        const int w = right - left, h = bottom - top, w3 = w * 3;
        if (w <= 0 || h <= 0 || w > 4096 || h > 4096)
            return false;

        Accumulator acc(w3, h);
        for (const auto& c : sink.contours)
            for (size_t i = 1; i < c.size(); ++i)
                acc.Line({(c[i - 1].x + offsetX - left) * 3.0f, c[i - 1].y - top}, {(c[i].x + offsetX - left) * 3.0f, c[i].y - top});
        std::vector<float> cov;
        acc.ResolveFloat(cov);

        static const float kFir[5] = {8.0f / 256.0f, 77.0f / 256.0f, 86.0f / 256.0f, 77.0f / 256.0f, 8.0f / 256.0f};
        out.pixels.resize((size_t)w * h * 4);
        for (int y = 0; y < h; ++y)
        {
            const float* row = cov.data() + (size_t)y * w3;
            unsigned char* dst = out.pixels.data() + (size_t)y * w * 4;
            for (int x = 0; x < w; ++x)
            {
                float ch[3];
                for (int s = 0; s < 3; ++s)
                {
                    const int c = x * 3 + s;
                    float v = 0.0f;
                    for (int t = -2; t <= 2; ++t)
                    {
                        const int k = c + t;
                        if (k >= 0 && k < w3)
                            v += row[k] * kFir[t + 2];
                    }
                    ch[s] = std::min(v, 1.0f);
                }
                if (bgr)
                    std::swap(ch[0], ch[2]);
                dst[x * 4 + 0] = (unsigned char)(ch[0] * 255.0f + 0.5f);
                dst[x * 4 + 1] = (unsigned char)(ch[1] * 255.0f + 0.5f);
                dst[x * 4 + 2] = (unsigned char)(ch[2] * 255.0f + 0.5f);
                dst[x * 4 + 3] = (unsigned char)((ch[0] + ch[1] + ch[2]) * (255.0f / 3.0f) + 0.5f);
            }
        }
        out.left = left;
        out.top = top;
        out.width = w;
        out.height = h;
        return true;
    }
}
