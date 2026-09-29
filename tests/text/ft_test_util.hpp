// Helpers of the FreeType text tests: the glyph quads a draw list holds, and the CPU composite of those quads from
// the pages the texture registry received, compared with a golden image in tests/text/golden.
#pragma once
#include "esia/core/draw_list.hpp"
#include "esia/core/texture.hpp"
#include "esia_test.hpp"
#include "image.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::texttest
{
    struct Quad
    {
        Rect r;
        Vec2 uv0, uv1;
        std::uint32_t color = 0;
        TextureId texture = 0;
    };

    // the glyph quads of a draw list (AddRectFilledUV: corners 0 and 2 of every 6 indices are min and max)
    inline std::vector<Quad> Quads(const DrawList& dl)
    {
        std::vector<Quad> out;
        for (const DrawCmd& c : dl.Commands())
        {
            if (c.kind != DrawCmdKind::Geometry)
                continue;
            for (std::uint32_t i = c.first; i + 6 <= c.first + c.count; i += 6)
            {
                const Vertex& a = dl.Vertices()[dl.Indices()[i]];
                const Vertex& b = dl.Vertices()[dl.Indices()[i + 2]];
                out.push_back({Rect(a.pos, b.pos), a.uv, b.uv, a.color, c.texture});
            }
        }
        return out;
    }

    inline DrawList NewList()
    {
        DrawList dl;
        dl.Reset(Rect(0, 0, 4096, 4096));
        return dl;
    }

    inline bool IsWhole(float v) { return std::fabs(v - std::round(v)) < 1e-4f; }

    // glyph quads per line of a layout drawn at y = 0 (line = where the quad's center falls)
    inline std::vector<int> QuadsPerLine(const std::vector<Quad>& quads, float lineHeight)
    {
        std::vector<int> lines;
        for (const Quad& q : quads)
        {
            const std::size_t line = (std::size_t)std::max(0.0f, std::floor(q.r.Center().y / lineHeight));
            if (lines.size() <= line)
                lines.resize(line + 1, 0);
            ++lines[line];
        }
        return lines;
    }

    // ------------------------------------------------------------------ CPU composite of what Draw emitted
    struct PageStore
    {
        struct Page
        {
            TextureInfo info;
            std::vector<std::uint8_t> pixels;
        };
        std::unordered_map<TextureId, Page> pages;

        void Collect(TextureRegistry& registry)
        {
            std::vector<TextureChange> changes;
            registry.TakeChanges(changes);
            for (TextureChange& c : changes)
            {
                if (c.kind == TextureChange::Kind::Create)
                    pages[c.id] = {c.info, std::move(c.pixels)};
                else if (c.kind == TextureChange::Kind::Update)
                {
                    Page& p = pages.at(c.id);
                    const std::size_t bpp = (std::size_t)BytesPerPixel(p.info.format), row = (std::size_t)c.width * bpp;
                    for (int y = 0; y < c.height; ++y)
                        std::copy_n(c.pixels.data() + row * y, row, p.pixels.data() + ((std::size_t)(c.y + y) * p.info.width + c.x) * bpp);
                }
                else
                    pages.erase(c.id);
            }
        }
    };

    // Glyph coverage over white (what the text pipeline does, without its gamma / contrast composition). Quads are
    // pixel aligned: texels map 1:1.
    inline testkit::Image Composite(const DrawList& dl, const PageStore& store, int w, int h)
    {
        testkit::Image img(w, h);
        std::fill(img.rgba.begin(), img.rgba.end(), (std::uint8_t)255);
        for (const Quad& q : Quads(dl))
        {
            const PageStore::Page& page = store.pages.at(q.texture);
            const int x0 = (int)std::lround(q.r.min.x), y0 = (int)std::lround(q.r.min.y);
            const int x1 = (int)std::lround(q.r.max.x), y1 = (int)std::lround(q.r.max.y);
            const int tx = (int)std::lround(q.uv0.x * (float)page.info.width), ty = (int)std::lround(q.uv0.y * (float)page.info.height);
            const Color col = Color::FromRgba8(q.color);
            const float ink[3] = {col.r * 255.0f, col.g * 255.0f, col.b * 255.0f};
            for (int y = std::max(y0, 0); y < std::min(y1, h); ++y)
                for (int x = std::max(x0, 0); x < std::min(x1, w); ++x)
                {
                    const float cov = (float)page.pixels[(std::size_t)(ty + y - y0) * page.info.width + (tx + x - x0)] / 255.0f * col.a;
                    std::uint8_t* d = img.At(x, y);
                    for (int c = 0; c < 3; ++c)
                        d[c] = (std::uint8_t)std::lround((float)d[c] * (1.0f - cov) + ink[c] * cov);
                }
        }
        return img;
    }

    // Writes the image next to the build and compares it with tests/text/golden/<name>.png (to create or update a
    // golden: review the written image, then copy it there).
    inline void CheckGolden(const testkit::Image& img, const std::string& name)
    {
        std::filesystem::create_directories(ESIA_TEXT_OUT_DIR);
        const std::string out = std::string(ESIA_TEXT_OUT_DIR) + "/" + name + ".png";
        ESIA_CHECK(testkit::WritePng(out, img));
        const std::string path = std::string(ESIA_TEXT_GOLDEN_DIR) + "/" + name + ".png";
        testkit::Image golden;
        const bool found = testkit::ReadPng(path, golden);
        if (!found)
            std::fprintf(stderr, "  no golden %s: review %s and copy it there\n", path.c_str(), out.c_str());
        ESIA_CHECK(found);
        if (!found)
            return;
        testkit::Tolerance tolerance;   // same fonts, same rasterizer: only float rounding may differ
        tolerance.channel = 2;
        tolerance.fraction = 0.002;
        tolerance.maxDelta = 64;
        testkit::Image diff;
        const testkit::CompareResult r = testkit::Compare(img, golden, tolerance, &diff);
        if (!r.pass)
        {
            testkit::WritePng(std::string(ESIA_TEXT_OUT_DIR) + "/" + name + ".diff.png", diff);
            std::fprintf(stderr, "  %s: max delta %d, %zu pixels differ (%.3f%%)\n", name.c_str(), r.maxDelta, r.differing, r.fraction * 100.0);
        }
        ESIA_CHECK(r.pass);
    }
}
