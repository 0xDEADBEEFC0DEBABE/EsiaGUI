// Esia - what text with new glyphs costs when it is drawn from distance fields (TextSystem::DrawShapes: text with a
// Style, large text): the draw that first meets a paragraph of glyphs the text system has no field of, against the same
// paragraph drawn again - the hitch of a frame in which such a text appears.
//
//   esia_field_bench [--font file] [--face n] [--from hex] [--glyphs 200] [--rounds 15] [--size 48] [--style]
//                    [--threads n] [--cache dir] [--backend d3d11] [--parts] [--digest file]
//
// Every round makes a text system and loads the font, then draws paragraph A (`--glyphs` characters of the font from
// code point `--from` on, each its own glyph: the text system's first fields), paragraph B (as many other ones) and A
// again (every field made), each in a frame of its own. With --cache the text system keeps its fields in that
// directory: the first round makes them, the later ones find them there (a second run). --backend renders the frames
// on a headless device too. --parts times one glyph's outline from FreeType and its field made alone (GenerateMtsdf,
// on this thread), as the text system does it. Printed: the median and the spread (min .. max) over the rounds.
// --digest writes a line per glyph of the face instead - its field's place, size and a hash of its texels - to compare
// the generator of two builds bit for bit.
#include "esia/core/draw_list.hpp"
#include "esia/core/texture.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/text/distance_field.hpp"
#include "esia/text/freetype.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace esia;

namespace
{
    using Clock = std::chrono::steady_clock;

    double Ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b - a).count(); }

    struct Series
    {
        std::vector<double> v;
        double At(double q)
        {
            if (v.empty())
                return 0.0;
            std::sort(v.begin(), v.end());
            const std::size_t i = std::min(v.size() - 1, (std::size_t)(q * (double)(v.size() - 1) + 0.5));
            return v[i];
        }
        std::string Text(double scale = 1.0)
        {
            char s[96];
            std::snprintf(s, sizeof(s), "%8.3f  (%.3f .. %.3f)", At(0.5) * scale, At(0.0) * scale, At(1.0) * scale);
            return s;
        }
    };

    void AppendUtf8(std::string& out, char32_t c)
    {
        if (c < 0x80)
            out += (char)c;
        else if (c < 0x800)
        {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        }
        else if (c < 0x10000)
        {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
        else
        {
            out += (char)(0xF0 | (c >> 18));
            out += (char)(0x80 | ((c >> 12) & 0x3F));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }

    // Letters and signs a paragraph is made of (no marks, which join the letter before, no spaces or controls)
    bool Letter(char32_t c)
    {
        return (c >= 0x21 && c <= 0x7E) || (c >= 0xA1 && c <= 0x24F && c != 0xAD) || (c >= 0x386 && c <= 0x3CE) || (c >= 0x400 && c <= 0x482) ||
               (c >= 0x48A && c <= 0x4FF) || (c >= 0x3041 && c <= 0x3096) || (c >= 0x30A1 && c <= 0x30FA) || (c >= 0x4E00 && c <= 0x9FFF) ||
               (c >= 0xAC00 && c <= 0xD7A3);
    }

    // FreeType outline (design units, y up) -> GlyphShape (field pixels, y down), as the text system makes it
    struct Sink
    {
        text::GlyphShape* shape;
        float scale;
        Vec2 Map(const FT_Vector* v) const { return Vec2((float)v->x * scale, -(float)v->y * scale); }
    };

    int MoveTo(const FT_Vector* to, void* user)
    {
        const Sink& s = *static_cast<const Sink*>(user);
        s.shape->MoveTo(s.Map(to));
        return 0;
    }

    int LineTo(const FT_Vector* to, void* user)
    {
        const Sink& s = *static_cast<const Sink*>(user);
        s.shape->LineTo(s.Map(to));
        return 0;
    }

    int ConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
    {
        const Sink& s = *static_cast<const Sink*>(user);
        s.shape->QuadTo(s.Map(control), s.Map(to));
        return 0;
    }

    int CubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
    {
        const Sink& s = *static_cast<const Sink*>(user);
        s.shape->CubicTo(s.Map(control1), s.Map(control2), s.Map(to));
        return 0;
    }
}

int main(int argc, char** argv)
{
    std::string font = std::string(ESIA_BENCH_FONT_DIR) + "/DroidSans.ttf", cache, digest, backend;
    int face = 0, glyphs = 200, rounds = 15, threads = -1;
    unsigned long from = 0;
    float size = 48.0f;
    bool style = false, parts = false;
    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        const bool more = i + 1 < argc;
        if (a == "--font" && more)
            font = argv[++i];
        else if (a == "--face" && more)
            face = std::atoi(argv[++i]);
        else if (a == "--from" && more)
            from = std::strtoul(argv[++i], nullptr, 16);
        else if (a == "--glyphs" && more)
            glyphs = std::max(1, std::atoi(argv[++i]));
        else if (a == "--rounds" && more)
            rounds = std::max(1, std::atoi(argv[++i]));
        else if (a == "--size" && more)
            size = (float)std::atof(argv[++i]);
        else if (a == "--style")
            style = true;
        else if (a == "--threads" && more)
            threads = std::atoi(argv[++i]);
        else if (a == "--cache" && more)
            cache = argv[++i];
        else if (a == "--parts")
            parts = true;
        else if (a == "--digest" && more)
            digest = argv[++i];
        else if (a == "--backend" && more)
            backend = argv[++i];
        else
        {
            std::fprintf(stderr, "usage: esia_field_bench [--font file] [--face n] [--from hex] [--glyphs n] [--rounds n] [--size px] [--style] [--threads n] "
                                 "[--cache dir] [--backend name] [--parts] [--digest file]\n");
            return 2;
        }
    }

    FT_Library ft = nullptr;
    FT_Face ftFace = nullptr;
    if (FT_Init_FreeType(&ft) != 0 || FT_New_Face(ft, font.c_str(), face, &ftFace) != 0)
    {
        std::fprintf(stderr, "esia_field_bench: cannot open %s\n", font.c_str());
        return 1;
    }
    if (!digest.empty())
    {
        // every glyph of the face: its outline as the text system reads it, its field made on every processor
        const float scale = 48.0f / (float)ftFace->units_per_EM;
        const std::size_t n = (std::size_t)ftFace->num_glyphs;
        std::vector<text::GlyphShape> shapes(n);
        for (std::size_t g = 0; g < n; ++g)
            if (FT_Load_Glyph(ftFace, (FT_UInt)g, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) == 0 && ftFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                FT_Outline_Funcs funcs{};
                funcs.move_to = &MoveTo;
                funcs.line_to = &LineTo;
                funcs.conic_to = &ConicTo;
                funcs.cubic_to = &CubicTo;
                Sink sink{&shapes[g], scale};
                FT_Outline_Decompose(&ftFace->glyph->outline, &funcs, &sink);
            }
        std::vector<text::GlyphBitmap> fields(n);
        std::vector<char> made(n, 0);
        std::atomic<std::size_t> next{0};
        std::vector<std::thread> pool;
        const Clock::time_point t0 = Clock::now();
        for (unsigned t = 0; t < std::max(1u, std::thread::hardware_concurrency()); ++t)
            pool.emplace_back([&] {
                for (std::size_t g; (g = next.fetch_add(1)) < n;)
                    made[g] = text::GenerateMtsdf(shapes[g], text::DistanceFieldDesc{8.0f, 24.0f}, fields[g]) ? 1 : 0;
            });
        for (std::thread& t : pool)
            t.join();
        const double ms = Ms(t0, Clock::now());
        std::FILE* out = std::fopen(digest.c_str(), "w");
        if (!out)
            return 1;
        std::uint64_t all = 14695981039346656037ull;
        for (std::size_t g = 0; g < n; ++g)
        {
            const text::GlyphBitmap& b = fields[g];
            std::uint64_t h = 14695981039346656037ull;
            for (const std::uint8_t v : b.pixels)
                h = (h ^ v) * 1099511628211ull;
            std::fprintf(out, "%zu %d %d %d %d %d %016llx\n", g, (int)made[g], b.left, b.top, b.width, b.height, (unsigned long long)h);
            all = (all ^ h) * 1099511628211ull;
        }
        std::fclose(out);
        std::printf("esia_field_bench: %zu fields of %s in %.0f ms on %zu threads, digest %016llx\n", n, font.c_str(), ms, pool.size(), (unsigned long long)all);
        return 0;
    }

    // the font's characters with a glyph each, in code point order: paragraph A the first `glyphs`, B the next
    std::vector<char32_t> chars;
    std::vector<FT_UInt> ids;
    std::unordered_set<FT_UInt> seen;
    FT_UInt gid = 0;
    for (FT_ULong c = FT_Get_First_Char(ftFace, &gid); gid != 0 && chars.size() < (std::size_t)glyphs * 2; c = FT_Get_Next_Char(ftFace, c, &gid))
        if (c >= from && Letter((char32_t)c) && seen.insert(gid).second)
        {
            chars.push_back((char32_t)c);
            ids.push_back(gid);
        }
    if (chars.size() < (std::size_t)glyphs * 2)
    {
        std::fprintf(stderr, "esia_field_bench: %s has %zu letters, %d asked for\n", font.c_str(), chars.size(), glyphs * 2);
        return 1;
    }
    std::string paragraph[2];
    for (int p = 0; p < 2; ++p)
        for (int i = 0; i < glyphs; ++i)
        {
            AppendUtf8(paragraph[p], chars[(std::size_t)(p * glyphs + i)]);
            if (i % 12 == 11)
                paragraph[p] += ' ';   // words: spaces draw nothing
        }

    std::printf("esia_field_bench: %s, %d glyphs a paragraph at %.0f px, %s, %d rounds%s%s\n", font.c_str(), glyphs, size,
                style ? "fill + stroke outside + shadow" : "fill", rounds, cache.empty() ? "" : ", fields kept in ", cache.c_str());

    if (parts)
    {
        // one glyph at a time on this thread: FreeType's outline, then its field
        const float scale = 48.0f / (float)ftFace->units_per_EM;
        Series outline, field;
        text::GlyphShape shape;
        text::GlyphBitmap bitmap;
        const text::DistanceFieldDesc desc{8.0f, 24.0f};
        for (int r = 0; r < 3; ++r)
            for (int i = 0; i < glyphs; ++i)
            {
                const Clock::time_point t0 = Clock::now();
                shape.Clear();
                if (FT_Load_Glyph(ftFace, ids[(std::size_t)i], FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) == 0 && ftFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    FT_Outline_Funcs funcs{};
                    funcs.move_to = &MoveTo;
                    funcs.line_to = &LineTo;
                    funcs.conic_to = &ConicTo;
                    funcs.cubic_to = &CubicTo;
                    Sink sink{&shape, scale};
                    FT_Outline_Decompose(&ftFace->glyph->outline, &funcs, &sink);
                }
                const Clock::time_point t1 = Clock::now();
                text::GenerateMtsdf(shape, desc, bitmap);
                const Clock::time_point t2 = Clock::now();
                outline.v.push_back(Ms(t0, t1) * 1000.0);
                field.v.push_back(Ms(t1, t2));
            }
        std::printf("  per glyph, alone:   outline us %s   field ms %s\n", outline.Text().c_str(), field.Text().c_str());
    }
    FT_Done_Face(ftFace);
    FT_Done_FreeType(ft);

    fx::Instance m;
    std::memset(&m, 0, sizeof(m));
    m.fill0[0] = m.fill0[1] = m.fill0[2] = m.fill0[3] = 1.0f;
    m.fill1[0] = m.fill1[1] = m.fill1[2] = m.fill1[3] = 1.0f;
    m.misc[0] = 1.0f;
    m.flags[0] = fx::kFill;
    if (style)
    {
        m.stroke[0] = 0.04f;
        m.stroke[1] = 0.52f;
        m.stroke[2] = 1.0f;
        m.stroke[3] = 1.0f;
        m.strokeParams[0] = 2.0f;
        m.strokeParams[1] = 1.0f;   // outside
        m.shadow[3] = 0.5f;
        m.shadowParams[0] = 6.0f;
        m.shadowParams[3] = 3.0f;
        m.flags[0] |= fx::kStroke | fx::kShadow;
    }

    // With --backend every paragraph is a frame of its own, rendered on a headless device: what the frame's fields
    // cost the renderer to take up (the first: a new page) is its submit time, the GPU waited for after it. The rounds
    // before the measured ones compile the device's shaders.
    rhi::HeadlessDevice hdev;
    if (!backend.empty())
    {
        if (!std::getenv("ESIA_VULKAN_VALIDATION"))   // the driver's time, not the validation layer's (as esia_bench)
        {
#if defined(_WIN32)
            _putenv("ESIA_VULKAN_VALIDATION=0");
#else
            setenv("ESIA_VULKAN_VALIDATION", "0", 1);
#endif
        }
        rhi::RegisterBuiltinBackends();
        const rhi::BackendInfo* info = rhi::FindBackend(backend);
        std::string error;
        rhi::HeadlessDesc hd;
        hd.width = 1600;
        hd.height = 1000;
        if (info)
            hdev = info->createHeadless(hd, error);
        if (!hdev.device)
        {
            std::fprintf(stderr, "esia_field_bench: no %s device: %s\n", backend.c_str(), info ? error.c_str() : "not in this build");
            return 77;
        }
        std::printf("  frames on %s (%s)\n", backend.c_str(), hdev.adapter.c_str());
    }
    Series draw[3], submit[3], perGlyph;
    const char* const names[3] = {"A, the first fields  ", "B, new glyphs        ", "A again, all made    "};
    std::size_t instances = 0;
    for (int r = hdev.device ? -2 : 0; r < rounds; ++r)
    {
        TextureRegistry textures;
        text::FreeTypeDesc desc;
        desc.fieldThreads = threads;
        desc.fieldCacheDirectory = cache;
        const std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures, desc);
        const text::FontId id = ts ? ts->AddFontFile(font.c_str(), face) : 0;
        if (!id)
        {
            std::fprintf(stderr, "esia_field_bench: the text system cannot load %s\n", font.c_str());
            return 1;
        }
        const std::unique_ptr<render::Renderer> renderer = hdev.device ? std::make_unique<render::Renderer>(*hdev.device) : nullptr;
        double ms[3];
        for (int k = 0; k < 3; ++k)
        {
            ts->NewFrame({1.0f});
            DrawList dl;
            dl.Reset(esia::Rect(0, 0, 1600, 1000));
            const Clock::time_point t0 = Clock::now();
            ts->DrawShapes(dl, {id, size}, Vec2(0.0f, 0.0f), m, 0, 0.0f, paragraph[k == 1 ? 1 : 0]);
            const Clock::time_point t1 = Clock::now();
            ms[k] = Ms(t0, t1);
            if (k == 0)
                instances = dl.FxInstances().size();
            if (renderer)
            {
                DrawData dd;
                dd.lists.push_back(&dl);
                dd.displaySize = Vec2(1600, 1000);
                std::vector<std::uint8_t> pixel;
                if (!renderer->Render(dd, &textures, hdev.target, render::RenderParams()))
                    return 1;
                hdev.device->ReadPixels(hdev.target, rhi::IRect{0, 0, 1, 1}, pixel);   // the GPU finishes the frame
            }
            else
            {
                std::vector<TextureChange> changes;   // what a renderer would take up: taken, as a frame does
                textures.TakeChanges(changes);
            }
            if (r >= 0)
            {
                draw[k].v.push_back(ms[k]);
                submit[k].v.push_back(Ms(t1, Clock::now()));
            }
        }
        if (r >= 0)
            perGlyph.v.push_back((ms[1] - ms[2]) / (double)glyphs);
    }
    std::printf("  glyph instances of a paragraph: %zu\n", instances);
    for (int k = 0; k < 3; ++k)
        if (hdev.device)
            std::printf("  %s draw ms %s   submit ms %s\n", names[k], draw[k].Text().c_str(), submit[k].Text().c_str());
        else
            std::printf("  %s draw ms %s\n", names[k], draw[k].Text().c_str());
    std::printf("  (B - A again) / glyph   ms %s\n", perGlyph.Text().c_str());
    return 0;
}
