// Esia - FreeType + HarfBuzz text system (include/esia/text/freetype.hpp).
//
// Measure / Draw -> GetLayout(), cached per text, font, size, wrap width and flags. Per paragraph: Itemize() picks a
// font and a script for every code point and cuts runs, Shape() runs HarfBuzz per run, BuildClusters() groups the
// glyphs and marks where a line may break, BreakLines() fills lines greedily, PlaceLine() positions a line's glyphs
// in visual order. Draw() snaps every glyph to the physical pixel grid and takes its bitmap from the atlas,
// rasterizing it from the FreeType outline the first time.
#include "esia/text/freetype.hpp"
#include "esia/base/hash.hpp"
#include "esia/base/utf8.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/text/glyph_atlas.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include <hb.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::text
{
    namespace
    {
        constexpr std::uint16_t kNoGlyph = 0xFFFF;          // control characters: an advance, nothing drawn (glyph ids end at 0xFFFE)
        constexpr std::size_t kLayoutCacheLimit = 1024;     // layouts kept before unused ones are dropped ...
        constexpr std::uint64_t kLayoutCacheFrames = 120;   // ... after this many frames without use

        bool IsBreakingSpace(char32_t c)
        {
            return c == U' ' || c == U'\t' || c == 0x1680 || (c >= 0x2000 && c <= 0x200A && c != 0x2007) || c == 0x205F || c == 0x3000;
        }

        bool IsControl(char32_t c) { return c < 0x20 || c == 0x7F; }

        bool IsJoinerOrSelector(char32_t c)
        {
            return c == 0x200C || c == 0x200D || (c >= 0xFE00 && c <= 0xFE0F) || (c >= 0xE0100 && c <= 0xE01EF);
        }

        bool IsHyphen(char32_t c) { return c == U'-' || c == 0x2010 || c == 0x2013; }

        // Scripts that break between characters: Han, kana, Hangul, Bopomofo, CJK symbols and full-width forms.
        bool IsCjk(char32_t c)
        {
            return (c >= 0x1100 && c <= 0x11FF) || (c >= 0x2E80 && c <= 0x9FFF) || (c >= 0xA960 && c <= 0xA97F) || (c >= 0xAC00 && c <= 0xD7AF) ||
                   (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFF00 && c <= 0xFFEF) || (c >= 0x20000 && c <= 0x3FFFF);
        }

        bool IsKana(char32_t c) { return (c >= 0x3040 && c <= 0x30FF) || (c >= 0x31F0 && c <= 0x31FF) || (c >= 0xFF65 && c <= 0xFF9F); }

        bool IsHangul(char32_t c)
        {
            return (c >= 0x1100 && c <= 0x11FF) || (c >= 0x3130 && c <= 0x318F) || (c >= 0xA960 && c <= 0xA97F) || (c >= 0xAC00 && c <= 0xD7FF) ||
                   (c >= 0xFFA0 && c <= 0xFFDC);
        }

        // OS/2 code pages (ulCodePageRange1) naming the CJK convention a font is drawn for: the CJK fonts of a system's
        // chain hold each other's kana and often Hangul (GB 18030 has both; Microsoft YaHei draws kana full width), so
        // these go to a font made for them (Yu Gothic UI's proportional kana, Malgun Gothic) when the chain has one.
        constexpr std::uint32_t kPageJapanese = 1u << 17;                   // JIS
        constexpr std::uint32_t kPageKorean = (1u << 19) | (1u << 21);   // Wansung, Johab

        // Line-breaking rules of CJK typography (kinsoku, simplified): no line starts with closing punctuation, small
        // kana or the prolonged sound mark, and none ends with opening punctuation.
        bool NoBreakBefore(char32_t c)
        {
            static constexpr std::u32string_view kSet = U")]}»›,.:;!?、。，．：；？！）］｝〕〉》」』】〙〗〟’”｠ー々〻ぁぃぅぇぉっゃゅょゎゕゖァィゥェォッャュョヮヵヶ・‥…";
            return kSet.find(c) != std::u32string_view::npos;
        }

        bool NoBreakAfter(char32_t c)
        {
            static constexpr std::u32string_view kSet = U"([{«‹（［｛〔〈《「『【〘〖〝‘“｟";
            return kSet.find(c) != std::u32string_view::npos;
        }

        struct Face
        {
            // AddFontFile: the file, mapped into memory (HarfBuzz's blob); FreeType and HarfBuzz read it in place, and
            // the faces of a collection share it. Mapped, not read: a color emoji font is 200 MB, of which a frame
            // touches a few pages.
            std::shared_ptr<hb_blob_t> file;
            FT_Face ft = nullptr;
            bool colorStrikes = false;   // color bitmap strikes (sbix, CBDT: emoji): drawn from those, not outlines
            hb_font_t* hb = nullptr;
            float upem = 1000.0f;
            float ascent = 0.0f, descent = 0.0f, gap = 0.0f;   // design units, descent positive
            std::uint32_t codePages = 0;                        // OS/2 ulCodePageRange1

            Face() = default;
            Face(const Face&) = delete;
            Face& operator=(const Face&) = delete;
            ~Face()
            {
                if (hb)
                    hb_font_destroy(hb);
                if (ft)
                    FT_Done_Face(ft);
            }

            bool Has(char32_t c) const
            {
                hb_codepoint_t glyph = 0;
                return hb_font_get_nominal_glyph(hb, c, &glyph) != 0;
            }

            float SpaceAdvance() const   // design units
            {
                hb_codepoint_t space = 0;
                return hb_font_get_nominal_glyph(hb, U' ', &space) ? (float)hb_font_get_glyph_h_advance(hb, space) : upem * 0.25f;
            }
        };

        struct CodePoint
        {
            char32_t c = 0;
            std::uint32_t offset = 0;   // byte offset in the paragraph
            std::uint16_t face = 0;
            hb_script_t script = HB_SCRIPT_COMMON;
            bool rtl = false;           // resolved direction
        };

        struct Run
        {
            std::uint32_t begin = 0, end = 0;   // byte range in the paragraph
            std::uint16_t face = 0;
            hb_script_t script = HB_SCRIPT_COMMON;
            bool rtl = false;
        };

        struct RunGlyph
        {
            std::uint16_t face = 0;
            std::uint16_t glyph = 0;
            float advance = 0.0f;       // UI units
            Vec2 offset;                // UI units, y down
            std::uint32_t cluster = 0;  // byte offset in the paragraph
            std::uint32_t run = 0;
        };

        struct Cluster
        {
            std::uint32_t begin = 0, end = 0;   // bytes
            std::uint32_t g0 = 0, g1 = 0;       // glyphs, logical order
            float advance = 0.0f;
            bool space = false;                 // breaking whitespace only: hangs at the end of a line
            bool breakBefore = false;           // a line may start with this cluster
        };

        struct Line
        {
            std::uint32_t c0 = 0, c1 = 0;   // clusters
            bool ellipsis = false;
        };

        struct LineInfo
        {
            std::size_t g0 = 0, g1 = 0;   // placed glyphs
            float width = 0.0f;           // without trailing spaces (alignment)
            float full = 0.0f;            // with them (the box)
        };

        struct PlacedGlyph
        {
            std::uint16_t face = 0;
            std::uint16_t glyph = 0;
            Vec2 pos;   // pen position on the baseline, UI units from the box's top-left
        };

        struct Layout
        {
            std::string text;
            std::uint32_t params[4] = {};   // font id, size, wrap width, flags (the cache key's inputs)
            std::vector<PlacedGlyph> glyphs;
            TextMetrics metrics;
            std::uint64_t lastFrame = 0;
        };

        // FreeType outline (design units, y up) -> Outline (pixels, y down)
        struct OutlineSink
        {
            Outline* outline = nullptr;
            float scale = 1.0f;
            Vec2 Map(const FT_Vector* v) const { return Vec2((float)v->x * scale, -(float)v->y * scale); }
        };

        int SinkMoveTo(const FT_Vector* to, void* user)
        {
            const OutlineSink& s = *static_cast<const OutlineSink*>(user);
            s.outline->MoveTo(s.Map(to));
            return 0;
        }

        int SinkLineTo(const FT_Vector* to, void* user)
        {
            const OutlineSink& s = *static_cast<const OutlineSink*>(user);
            s.outline->LineTo(s.Map(to));
            return 0;
        }

        int SinkConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
        {
            const OutlineSink& s = *static_cast<const OutlineSink*>(user);
            s.outline->QuadTo(s.Map(control), s.Map(to));
            return 0;
        }

        int SinkCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
        {
            const OutlineSink& s = *static_cast<const OutlineSink*>(user);
            s.outline->CubicTo(s.Map(control1), s.Map(control2), s.Map(to));
            return 0;
        }

        class FreeTypeTextSystem final : public TextSystem
        {
        public:
            FreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc)
                : atlas_(textures, GlyphAtlasDesc{desc.atlasPageSize, desc.atlasMaxPages, 1}),
                  colorAtlas_(textures, GlyphAtlasDesc{std::min(desc.atlasPageSize, 1024), 2, 1, TextureFormat::RGBA8})
            {
            }

            ~FreeTypeTextSystem() override
            {
                faces_.clear();   // before the library that owns them
                if (buffer_)
                    hb_buffer_destroy(buffer_);
                if (ft_)
                    FT_Done_FreeType(ft_);
            }

            bool Init()
            {
                if (FT_Init_FreeType(&ft_) != 0)
                {
                    ft_ = nullptr;
                    return false;
                }
                buffer_ = hb_buffer_create();
                unicode_ = hb_unicode_funcs_get_default();
                return hb_buffer_allocation_successful(buffer_) != 0;
            }

            // ------------------------------------------------------------------ fonts
            FontId AddFontFile(const char* path, int faceIndex) override
            {
                if (!path)
                    return 0;
                // the faces of a collection (the CJK fonts of a system's fallback chain: 20 MB and more) share one copy
                std::weak_ptr<hb_blob_t>& cached = files_[path];
                std::shared_ptr<hb_blob_t> blob = cached.lock();
                if (!blob)
                {
                    blob = MapFile(path);
                    if (!blob)
                        return 0;
                    cached = blob;
                }
                return AddFace(std::move(blob), nullptr, 0, faceIndex);
            }

            FontId AddFontMemory(const void* data, std::size_t size, int faceIndex) override { return AddFace(nullptr, data, size, faceIndex); }

            void AddFallback(FontId font) override
            {
                if (font == 0 || font > faces_.size())
                    return;
                const std::uint16_t face = (std::uint16_t)(font - 1);
                if (std::find(fallbacks_.begin(), fallbacks_.end(), face) != fallbacks_.end())
                    return;
                fallbacks_.push_back(face);
                layouts_.clear();   // texts laid out before may pick other fonts now
            }

            // ------------------------------------------------------------------ frame
            void NewFrame(const RasterParams& params) override
            {
                params_ = params;
                if (!(params_.pixelsPerUnit > 0.0f))
                    params_.pixelsPerUnit = 1.0f;
                ++frame_;
                atlas_.BeginFrame();
                colorAtlas_.BeginFrame();
                if (layouts_.size() > kLayoutCacheLimit)
                    std::erase_if(layouts_, [this](const auto& entry) { return entry.second.lastFrame + kLayoutCacheFrames < frame_; });
            }

            // ------------------------------------------------------------------ editing
            void CaretStops(FontRef font, std::string_view text, std::vector<CaretStop>& out) override
            {
                out.clear();
                if (font.id == 0 || font.id > faces_.size() || !(font.size > 0.0f))
                {
                    out.push_back({0, 0.0f});
                    return;
                }
                Itemize((std::uint16_t)(font.id - 1), text);
                Shape(text, font.size);
                BuildClusters(text);
                // every glyph's left edge as PlaceLine puts it: the runs left to right, a right-to-left run reversed
                glyphX_.assign(glyphs_.size(), 0.0f);
                float pen = 0.0f;
                for (std::uint32_t g = 0; g < (std::uint32_t)glyphs_.size();)
                {
                    std::uint32_t e = g + 1;
                    while (e < glyphs_.size() && glyphs_[e].run == glyphs_[g].run)
                        ++e;
                    const bool rtl = runs_[glyphs_[g].run].rtl;
                    for (std::uint32_t j = 0; j < e - g; ++j)
                    {
                        const std::uint32_t k = rtl ? e - 1 - j : g + j;
                        glyphX_[k] = pen;
                        pen += glyphs_[k].advance;
                    }
                    g = e;
                }
                // a caret stands before a cluster at its leading edge (left in a left-to-right run, right in a
                // right-to-left one); inside a cluster of several graphemes (a ligature) at even steps across it
                std::size_t ci = 0;   // code points
                float endX = 0.0f;
                for (const Cluster& c : clusters_)
                {
                    float x0 = std::numeric_limits<float>::infinity(), x1 = -x0;
                    for (std::uint32_t g = c.g0; g < c.g1; ++g)
                    {
                        x0 = std::min(x0, glyphX_[g]);
                        x1 = std::max(x1, glyphX_[g] + glyphs_[g].advance);
                    }
                    const bool rtl = runs_[glyphs_[c.g0].run].rtl;
                    while (ci < cps_.size() && cps_[ci].offset < c.begin)
                        ++ci;
                    graphemes_.clear();
                    for (std::size_t k = ci; k < cps_.size() && cps_[k].offset < c.end; ++k)
                        if (k == ci || GraphemeBreakBefore(k))
                            graphemes_.push_back(cps_[k].offset);
                    const float n = (float)std::max<std::size_t>(graphemes_.size(), 1);
                    // a cluster that continues a grapheme (a flag or a ZWJ sequence the font has no glyph for) is no stop
                    for (std::size_t j = GraphemeBreakBefore(ci) ? 0 : 1; j < graphemes_.size(); ++j)
                    {
                        const float t = (float)j / n;
                        out.push_back({graphemes_[j], rtl ? x1 - (x1 - x0) * t : x0 + (x1 - x0) * t});
                    }
                    endX = rtl ? x0 : x1;   // after the last cluster: its trailing edge
                }
                out.push_back({(std::uint32_t)text.size(), endX});
            }

            // Grapheme cluster boundaries (UAX #29) inside a shaped cluster, the rules a caret needs: CR LF stays whole;
            // marks, ZWJ / ZWNJ, emoji modifiers and tags extend the character before; a pictograph after ZWJ joins
            // it; regional indicators pair; Hangul jamo form syllables; a consonant after a virama joins the conjunct.
            // Clusters themselves are whole graphemes (HarfBuzz merges them), so this only splits ligatures.
            bool GraphemeBreakBefore(std::size_t i) const
            {
                if (i == 0 || i >= cps_.size())
                    return true;
                const char32_t prev = cps_[i - 1].c, cur = cps_[i].c;
                if (prev == U'\r' && cur == U'\n')
                    return false;
                if (IsControl32(prev) || IsControl32(cur))
                    return true;
                const hb_unicode_general_category_t gc = hb_unicode_general_category(unicode_, cur);
                if (gc == HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK || gc == HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK ||
                    gc == HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK || cur == 0x200C || cur == 0x200D || (cur >= 0x1F3FB && cur <= 0x1F3FF) ||
                    (cur >= 0xE0020 && cur <= 0xE007F))
                    return false;
                if (prev == 0x200D && (gc == HB_UNICODE_GENERAL_CATEGORY_OTHER_SYMBOL || (cur >= 0x1F000 && cur <= 0x1FAFF)))
                    return false;
                if (IsRegionalIndicator(cur) && IsRegionalIndicator(prev))
                {
                    std::size_t run = 0;
                    for (std::size_t k = i; k > 0 && IsRegionalIndicator(cps_[k - 1].c); --k)
                        ++run;
                    return run % 2 == 0;
                }
                const int hp = HangulKind(prev), hc = HangulKind(cur);
                if ((hp == kHangulL && hc != 0 && hc != kHangulT) || ((hp == kHangulV || hp == kHangulLV) && (hc == kHangulV || hc == kHangulT)) ||
                    ((hp == kHangulT || hp == kHangulLVT) && hc == kHangulT))
                    return false;
                if (hb_unicode_combining_class(unicode_, prev) == HB_UNICODE_COMBINING_CLASS_VIRAMA && gc == HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER)
                    return false;
                return true;
            }

            static bool IsControl32(char32_t c) { return c < 0x20 || (c >= 0x7F && c < 0xA0) || c == 0x2028 || c == 0x2029; }
            static bool IsRegionalIndicator(char32_t c) { return c >= 0x1F1E6 && c <= 0x1F1FF; }
            static constexpr int kHangulL = 1, kHangulV = 2, kHangulT = 3, kHangulLV = 4, kHangulLVT = 5;
            static int HangulKind(char32_t c)
            {
                if ((c >= 0x1100 && c <= 0x115F) || (c >= 0xA960 && c <= 0xA97C))
                    return kHangulL;
                if ((c >= 0x1160 && c <= 0x11A7) || (c >= 0xD7B0 && c <= 0xD7C6))
                    return kHangulV;
                if ((c >= 0x11A8 && c <= 0x11FF) || (c >= 0xD7CB && c <= 0xD7FB))
                    return kHangulT;
                if (c >= 0xAC00 && c <= 0xD7A3)
                    return (c - 0xAC00) % 28 == 0 ? kHangulLV : kHangulLVT;
                return 0;
            }

            // ------------------------------------------------------------------ layout and drawing
            TextMetrics Measure(FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags) override
            {
                const Layout* layout = GetLayout(font, text, wrapWidth, flags);
                return layout ? layout->metrics : TextMetrics{};
            }

            Vec2 Draw(DrawList& dl, FontRef font, Vec2 pos, Color color, std::string_view text, float wrapWidth, std::uint32_t flags,
                      float scale) override
            {
                const Layout* layout = GetLayout(font, text, wrapWidth, flags);
                if (!layout)
                    return Vec2(0, 0);
                if (color.a <= 0.0f || !(scale > 0.0f))
                    return layout->metrics.size;
                const float rs = params_.pixelsPerUnit, inv = 1.0f / rs;
                // animated scales are quantized to 1/4 px of em, so an animation does not flood the atlas
                const float em = scale == 1.0f ? font.size * rs : std::floor(font.size * scale * rs * 4.0f + 0.5f) * 0.25f;
                const std::uint32_t rgba = color.ToRgba8(), colorGlyphRgba = Color::White(color.a).ToRgba8();
                const Rect clip = dl.ClipRect();
                TextureId bound = 0;
                for (const PlacedGlyph& g : layout->glyphs)
                {
                    // physical-pixel placement: the baseline on a whole pixel, the pen at a quarter-pixel phase
                    const float px = (pos.x + g.pos.x * scale) * rs, py = (pos.y + g.pos.y * scale) * rs;
                    float xi = std::floor(px);
                    int phase = (int)std::floor((px - xi) * 4.0f + 0.5f);
                    if (phase == 4)
                    {
                        phase = 0;
                        xi += 1.0f;
                    }
                    const float yi = std::floor(py + 0.5f);
                    const GlyphSlot* s = Glyph(g.face, g.glyph, em, phase);
                    if (!s || !s->page)
                        continue;
                    const Rect r((xi + (float)s->left) * inv, (yi + (float)s->top) * inv, (xi + (float)(s->left + s->width)) * inv,
                                 (yi + (float)(s->top + s->height)) * inv);
                    if (!r.Overlaps(clip))
                        continue;
                    if (s->page != bound)
                    {
                        if (bound)
                            dl.PopTexture();
                        dl.PushTexture(s->page);
                        bound = s->page;
                    }
                    dl.AddRectFilledUV(r, s->uv0, s->uv1, s->color ? colorGlyphRgba : rgba);
                }
                if (bound)
                    dl.PopTexture();
                return layout->metrics.size;
            }

            void DrawGlyph(DrawList& dl, FontRef font, char32_t codepoint, Vec2 center, Color color) override
            {
                if (font.id == 0 || font.id > faces_.size() || !(font.size > 0.0f) || color.a <= 0.0f)
                    return;
                const std::uint16_t face = PickFace((std::uint16_t)(font.id - 1), codepoint, -1);
                hb_codepoint_t glyph = 0;
                if (!hb_font_get_nominal_glyph(faces_[face]->hb, codepoint, &glyph))
                    return;   // no font has it: an icon's .notdef box would help nobody
                const float rs = params_.pixelsPerUnit, inv = 1.0f / rs;
                // icon sizes animate: quantized to 1/4 px so the animation reuses atlas entries
                const GlyphSlot* s = Glyph(face, (std::uint16_t)glyph, std::floor(font.size * rs * 4.0f + 0.5f) * 0.25f, 0);
                if (!s || !s->page)
                    return;
                // optical centering: the ink box's center on `center`, the pen on a whole pixel
                const float ox = std::floor(center.x * rs - (s->ink.min.x + s->ink.max.x) * 0.5f + 0.5f);
                const float oy = std::floor(center.y * rs - (s->ink.min.y + s->ink.max.y) * 0.5f + 0.5f);
                dl.AddImage(s->page,
                            Rect((ox + (float)s->left) * inv, (oy + (float)s->top) * inv, (ox + (float)(s->left + s->width)) * inv,
                                 (oy + (float)(s->top + s->height)) * inv),
                            s->uv0, s->uv1, s->color ? Color::White(color.a).ToRgba8() : color.ToRgba8());
            }

        private:
            // The path is UTF-8 on every platform (HarfBuzz converts it for Windows); mapped where the platform can.
            static std::shared_ptr<hb_blob_t> MapFile(const char* path)
            {
                hb_blob_t* blob = hb_blob_create_from_file_or_fail(path);
                if (!blob)
                    return nullptr;
                // not make_shared: with mingw-w64's libstdc++ it duplicates std::type_info::operator== at link time
                std::shared_ptr<hb_blob_t> shared(blob, &hb_blob_destroy);
                if (hb_blob_get_length(blob) == 0)
                    return nullptr;
                return shared;
            }

            FontId AddFace(std::shared_ptr<hb_blob_t> file, const void* data, std::size_t size, int faceIndex)
            {
                if (faces_.size() >= kNoGlyph || faceIndex < 0)
                    return 0;
                auto face = std::make_unique<Face>();
                face->file = std::move(file);
                if (face->file)
                {
                    unsigned length = 0;
                    data = hb_blob_get_data(face->file.get(), &length);
                    size = length;
                }
                if (!data || size == 0 || FT_New_Memory_Face(ft_, static_cast<const FT_Byte*>(data), (FT_Long)size, faceIndex, &face->ft) != 0)
                {
                    face->ft = nullptr;
                    return 0;
                }
                face->colorStrikes = FT_HAS_COLOR(face->ft) && FT_HAS_FIXED_SIZES(face->ft);
                if (!FT_IS_SCALABLE(face->ft) && !face->colorStrikes)
                    return 0;   // monochrome bitmap fonts: nothing to rasterize
                hb_blob_t* blob = face->file ? hb_blob_reference(face->file.get())
                                             : hb_blob_create(static_cast<const char*>(data), (unsigned)size, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
                hb_face_t* hbFace = hb_face_create(blob, (unsigned)faceIndex);
                hb_blob_destroy(blob);
                face->hb = hb_font_create(hbFace);
                hb_face_destroy(hbFace);
                face->upem = (float)std::max<FT_UShort>(face->ft->units_per_EM, 1);
                hb_font_set_scale(face->hb, (int)face->upem, (int)face->upem);   // shaping in design units
                hb_font_extents_t extents{};
                hb_font_get_h_extents(face->hb, &extents);
                face->ascent = (float)extents.ascender;
                face->descent = (float)-extents.descender;
                face->gap = (float)extents.line_gap;
                if (face->ascent + face->descent <= 0.0f)
                {
                    // no usable hhea / OS/2 metrics: FreeType's (derived from the bounding box)
                    face->ascent = (float)face->ft->ascender;
                    face->descent = (float)-face->ft->descender;
                    face->gap = std::max(0.0f, (float)face->ft->height - face->ascent - face->descent);
                }
                if (const auto* os2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(face->ft, FT_SFNT_OS2)); os2 && os2->version >= 1 && os2->version != 0xFFFF)
                    face->codePages = (std::uint32_t)os2->ulCodePageRange1;
                faces_.push_back(std::move(face));
                return (FontId)faces_.size();
            }

            bool IsMark(char32_t c) const
            {
                const hb_unicode_general_category_t g = hb_unicode_general_category(unicode_, c);
                return g == HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK || g == HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK ||
                       g == HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK;
            }

            bool IsAlnum(char32_t c) const
            {
                switch (hb_unicode_general_category(unicode_, c))
                {
                case HB_UNICODE_GENERAL_CATEGORY_UPPERCASE_LETTER:
                case HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER:
                case HB_UNICODE_GENERAL_CATEGORY_TITLECASE_LETTER:
                case HB_UNICODE_GENERAL_CATEGORY_MODIFIER_LETTER:
                case HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER:
                case HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER: return true;
                default: return false;
                }
            }

            // The requested font if it has `c`, else the first fallback that has it, else the requested font (its
            // .notdef). Marks, joiners, variation selectors and controls stay with the character before them, so they
            // neither split a run nor separate a mark from its base; so do spaces the requested font lacks (a space it
            // has is its own, as with CSS and DirectWrite: Malgun Gothic's is 0.35 em, Segoe UI's 0.27). Kana and
            // Hangul take the first fallback made for them (kPageJapanese / kPageKorean) that has them; Han and CJK
            // punctuation the fallback of the paragraph's last CJK character (`cjk`) when it has them, so a Japanese
            // sentence keeps Japanese forms.
            std::uint16_t PickFace(std::uint16_t primary, char32_t c, int previous, int cjk = -1) const
            {
                const bool inPrimary = faces_[primary]->Has(c);
                if (previous >= 0)
                {
                    const Face& prev = *faces_[(std::size_t)previous];
                    if (IsJoinerOrSelector(c) || IsControl(c) || (IsMark(c) && prev.Has(c)) || (IsBreakingSpace(c) && !inPrimary && prev.Has(c)))
                        return (std::uint16_t)previous;
                }
                if (inPrimary)
                    return primary;
                if (const std::uint32_t page = IsKana(c) ? kPageJapanese : IsHangul(c) ? kPageKorean : 0)
                    for (const std::uint16_t f : fallbacks_)
                        if ((faces_[f]->codePages & page) != 0 && faces_[f]->Has(c))
                            return f;
                if (cjk >= 0 && IsCjk(c) && faces_[(std::size_t)cjk]->Has(c))
                    return (std::uint16_t)cjk;
                for (const std::uint16_t f : fallbacks_)
                    if (faces_[f]->Has(c))
                        return f;
                return primary;
            }

            // May a line start with the code point cps_[i]? (i = the first code point of a cluster)
            bool CanBreakBefore(std::size_t i) const
            {
                if (i == 0 || i >= cps_.size())
                    return false;
                const char32_t cur = cps_[i].c, prev = cps_[i - 1].c;
                if (IsBreakingSpace(cur))
                    return false;   // spaces stay at the end of the line before
                if (NoBreakBefore(cur))
                    return false;   // not even after a space: "word !" keeps its "!"
                if (IsBreakingSpace(prev))
                    return true;
                if (NoBreakAfter(prev))
                    return false;
                if (IsHyphen(prev) && i >= 2 && IsAlnum(cps_[i - 2].c) && IsAlnum(cur))
                    return true;   // "well-" | "known"
                return IsCjk(prev) || IsCjk(cur);
            }

            const Layout* GetLayout(FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags)
            {
                if (font.id == 0 || font.id > faces_.size() || !(font.size > 0.0f))
                    return nullptr;
                const std::uint32_t params[4] = {font.id, std::bit_cast<std::uint32_t>(font.size), std::bit_cast<std::uint32_t>(wrapWidth), flags};
                const std::uint64_t key = ((std::uint64_t)HashBytes(params, sizeof(params), 0) << 32) | HashString(text, params[1]);
                Layout& layout = layouts_[key];
                if (layout.text != text || std::memcmp(layout.params, params, sizeof(params)) != 0)
                {
                    // new, or a hash collision: (re)build in place
                    layout.text.assign(text);
                    std::memcpy(layout.params, params, sizeof(params));
                    layout.glyphs.clear();
                    BuildLayout(font, text, wrapWidth, flags, layout);
                }
                layout.lastFrame = frame_;
                return &layout;
            }

            void BuildLayout(FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags, Layout& layout)
            {
                const std::uint16_t primary = (std::uint16_t)(font.id - 1);
                const Face& pf = *faces_[primary];
                const float k = font.size / pf.upem;
                // one line box for every line, from the requested font (WGT's uniform line spacing)
                const float lineHeight = (pf.ascent + pf.descent + pf.gap) * k;
                const float baseline = (pf.ascent + pf.gap * 0.5f) * k;
                const bool ellipsis = (flags & TextFlags_Ellipsis) != 0 && wrapWidth > 0.0f;
                const float maxWidth = wrapWidth > 0.0f && !ellipsis ? wrapWidth : std::numeric_limits<float>::infinity();

                lineInfo_.clear();
                for (std::size_t p = 0;;)
                {
                    // a paragraph ends at a hard break: LF, CR, CR LF, U+2028 LINE / U+2029 PARAGRAPH SEPARATOR
                    std::size_t q = p, next = std::string_view::npos;
                    while (q < text.size())
                    {
                        const unsigned char c = (unsigned char)text[q];
                        if (c == '\n' || c == '\r')
                        {
                            next = q + ((c == '\r' && q + 1 < text.size() && text[q + 1] == '\n') ? 2 : 1);
                            break;
                        }
                        if (c == 0xE2 && q + 2 < text.size() && (unsigned char)text[q + 1] == 0x80 && ((unsigned char)text[q + 2] | 1u) == 0xA9)
                        {
                            next = q + 3;
                            break;
                        }
                        ++q;
                    }
                    LayoutParagraph(primary, font.size, text.substr(p, q - p), maxWidth, ellipsis ? wrapWidth : 0.0f, lineHeight, baseline, layout);
                    if (next == std::string_view::npos)
                        break;
                    p = next;
                }

                float full = 0.0f;
                for (const LineInfo& li : lineInfo_)
                    full = std::max(full, li.full);
                float width = wrapWidth > 0.0f ? std::min(full, wrapWidth) : full;
                const bool center = (flags & TextFlags_AlignCenter) != 0, right = (flags & TextFlags_AlignRight) != 0;
                if (center || right)
                {
                    // aligned lines sit in the wrap column when there is one, else in the widest line
                    if (wrapWidth > 0.0f)
                        width = wrapWidth;
                    for (const LineInfo& li : lineInfo_)
                    {
                        const float dx = (width - li.width) * (center ? 0.5f : 1.0f);
                        for (std::size_t g = li.g0; g < li.g1; ++g)
                            layout.glyphs[g].pos.x += dx;
                    }
                }
                layout.metrics.size = Vec2(width, lineHeight * (float)lineInfo_.size());
                layout.metrics.baseline = baseline;
                layout.metrics.lines = (int)lineInfo_.size();
            }

            void LayoutParagraph(std::uint16_t primary, float size, std::string_view para, float maxWidth, float trimWidth, float lineHeight,
                                 float baseline, Layout& layout)
            {
                Itemize(primary, para);
                Shape(para, size);
                BuildClusters(para);
                BreakLines(maxWidth);
                for (Line& line : lines_)
                {
                    if (trimWidth > 0.0f)
                        Trim(line, trimWidth, primary, size);
                    PlaceLine(line, baseline + lineHeight * (float)lineInfo_.size(), layout);
                }
            }

            // Code points with a font and a script each, then runs of one font and one script.
            void Itemize(std::uint16_t primary, std::string_view para)
            {
                cps_.clear();
                runs_.clear();
                int previous = -1, cjk = -1;
                for (std::size_t i = 0; i < para.size();)
                {
                    CodePoint cp;
                    cp.offset = (std::uint32_t)i;
                    cp.c = DecodeUtf8(para, i);
                    cp.face = PickFace(primary, cp.c, previous, cjk);
                    previous = cp.face;
                    if (cp.face != primary && IsCjk(cp.c))
                        cjk = cp.face;
                    cps_.push_back(cp);
                }
                // common / inherited characters (spaces, digits, punctuation, marks) join the script before them; at the
                // start of the paragraph, the first one after them
                hb_script_t last = HB_SCRIPT_INVALID;
                for (CodePoint& cp : cps_)
                {
                    const hb_script_t s = hb_unicode_script(unicode_, cp.c);
                    if (s != HB_SCRIPT_COMMON && s != HB_SCRIPT_INHERITED && s != HB_SCRIPT_UNKNOWN)
                        last = s;
                    cp.script = last;
                }
                const auto firstReal = std::find_if(cps_.begin(), cps_.end(), [](const CodePoint& cp) { return cp.script != HB_SCRIPT_INVALID; });
                const hb_script_t first = firstReal != cps_.end() ? firstReal->script : HB_SCRIPT_COMMON;
                for (auto it = cps_.begin(); it != firstReal; ++it)
                    it->script = first;
                ResolveDirections();

                for (std::size_t i = 0; i < cps_.size(); ++i)
                {
                    if (i == 0 || cps_[i].face != cps_[i - 1].face || cps_[i].script != cps_[i - 1].script || cps_[i].rtl != cps_[i - 1].rtl)
                    {
                        Run r;
                        r.begin = cps_[i].offset;
                        r.face = cps_[i].face;
                        r.script = cps_[i].script;
                        r.rtl = cps_[i].rtl;
                        runs_.push_back(r);
                    }
                    runs_.back().end = i + 1 < cps_.size() ? cps_[i + 1].offset : (std::uint32_t)para.size();
                }
            }

            // The direction of every code point, a small part of UAX #9 for left-to-right paragraphs with right-to-left
            // words in them (paragraphs are not reordered): letters of right-to-left scripts are right to left; digits
            // and the letters of other scripts left to right; marks follow their base; white space, punctuation and
            // symbols between two right-to-left characters stay right to left and take their script, elsewhere they are
            // left to right ("a <Arabic> b": the spaces around the Arabic word stay where they are typed).
            void ResolveDirections()
            {
                enum : std::uint8_t { kNeutral, kL, kR };
                std::vector<std::uint8_t>& dir = directions_;
                dir.assign(cps_.size(), kNeutral);
                for (std::size_t i = 0; i < cps_.size(); ++i)
                {
                    const char32_t c = cps_[i].c;
                    switch (hb_unicode_general_category(unicode_, c))
                    {
                    case HB_UNICODE_GENERAL_CATEGORY_UPPERCASE_LETTER:
                    case HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER:
                    case HB_UNICODE_GENERAL_CATEGORY_TITLECASE_LETTER:
                    case HB_UNICODE_GENERAL_CATEGORY_MODIFIER_LETTER:
                    case HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER:
                        dir[i] = hb_script_get_horizontal_direction(hb_unicode_script(unicode_, c)) == HB_DIRECTION_RTL ? kR : kL;
                        break;
                    case HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER:
                        dir[i] = kL;
                        break;
                    case HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK:
                    case HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK:
                    case HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK:
                        dir[i] = i > 0 ? dir[i - 1] : kL;
                        break;
                    default:
                        break;
                    }
                }
                for (std::size_t i = 0; i < cps_.size();)
                {
                    if (dir[i] != kNeutral)
                    {
                        cps_[i].rtl = dir[i] == kR;
                        ++i;
                        continue;
                    }
                    std::size_t j = i;
                    while (j < cps_.size() && dir[j] == kNeutral)
                        ++j;
                    const bool rtl = i > 0 && j < cps_.size() && dir[i - 1] == kR && dir[j] == kR;
                    for (std::size_t k = i; k < j; ++k)
                    {
                        cps_[k].rtl = rtl;
                        // a left-to-right neutral after a right-to-left word takes the script of what follows it
                        if (!rtl && hb_script_get_horizontal_direction(cps_[k].script) == HB_DIRECTION_RTL)
                            cps_[k].script = j < cps_.size() && dir[j] == kL ? cps_[j].script : HB_SCRIPT_COMMON;
                    }
                    i = j;
                }
            }

            // HarfBuzz per run, in design units; glyphs_ in logical order (right-to-left runs reversed back).
            void Shape(std::string_view para, float size)
            {
                glyphs_.clear();
                for (std::uint32_t r = 0; r < (std::uint32_t)runs_.size(); ++r)
                {
                    const Run& run = runs_[r];
                    const Face& face = *faces_[run.face];
                    hb_buffer_clear_contents(buffer_);
                    // the whole paragraph goes in as context: shaping across run boundaries (Arabic joining) sees it
                    hb_buffer_add_utf8(buffer_, para.data(), (int)para.size(), run.begin, (int)(run.end - run.begin));
                    hb_buffer_set_direction(buffer_, run.rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
                    hb_buffer_set_script(buffer_, run.script);
                    hb_buffer_set_language(buffer_, hb_language_get_default());
                    unsigned flags = HB_BUFFER_FLAG_DEFAULT;
                    if (run.begin == 0)
                        flags |= HB_BUFFER_FLAG_BOT;
                    if (run.end == para.size())
                        flags |= HB_BUFFER_FLAG_EOT;
                    hb_buffer_set_flags(buffer_, (hb_buffer_flags_t)flags);
                    hb_shape(face.hb, buffer_, nullptr, 0);
                    unsigned n = 0;
                    const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer_, &n);
                    const hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer_, nullptr);
                    const float k = size / face.upem;
                    const std::size_t first = glyphs_.size();
                    for (unsigned i = 0; i < n; ++i)
                    {
                        RunGlyph g;
                        g.face = run.face;
                        g.glyph = (std::uint16_t)info[i].codepoint;
                        g.advance = (float)pos[i].x_advance * k;
                        g.offset = Vec2((float)pos[i].x_offset * k, -(float)pos[i].y_offset * k);
                        g.cluster = info[i].cluster;
                        g.run = r;
                        const unsigned char b = (unsigned char)para[g.cluster];
                        if (IsControl(b))
                        {
                            // control characters draw nothing; a tab advances like four spaces
                            g.glyph = kNoGlyph;
                            g.offset = Vec2();
                            g.advance = b == '\t' ? 4.0f * face.SpaceAdvance() * k : 0.0f;
                        }
                        glyphs_.push_back(g);
                    }
                    if (run.rtl)
                        std::reverse(glyphs_.begin() + (std::ptrdiff_t)first, glyphs_.end());
                }
            }

            // Glyphs with the same cluster value form a cluster: the unit of line breaking and trimming.
            void BuildClusters(std::string_view para)
            {
                clusters_.clear();
                std::size_t ci = 0;   // first code point of the cluster
                for (std::uint32_t g = 0; g < (std::uint32_t)glyphs_.size();)
                {
                    Cluster c;
                    c.begin = glyphs_[g].cluster;
                    c.g0 = g;
                    while (g < glyphs_.size() && glyphs_[g].cluster == c.begin)
                    {
                        c.advance += glyphs_[g].advance;
                        ++g;
                    }
                    c.g1 = g;
                    c.end = g < glyphs_.size() ? glyphs_[g].cluster : (std::uint32_t)para.size();
                    while (ci < cps_.size() && cps_[ci].offset < c.begin)
                        ++ci;
                    c.breakBefore = CanBreakBefore(ci);
                    c.space = true;
                    for (std::size_t k = ci; k < cps_.size() && cps_[k].offset < c.end; ++k)
                        c.space = c.space && IsBreakingSpace(cps_[k].c);
                    clusters_.push_back(c);
                }
            }

            // Greedy filling, word by word (a word runs from one break opportunity to the next, its trailing spaces
            // included; they hang past the line's end). A word longer than a line on its own breaks between clusters.
            void BreakLines(float maxWidth)
            {
                lines_.clear();
                const std::uint32_t n = (std::uint32_t)clusters_.size();
                std::uint32_t lineStart = 0;
                float lineWidth = 0.0f;   // clusters [lineStart, i), trailing spaces included
                for (std::uint32_t i = 0; i < n;)
                {
                    std::uint32_t end = i + 1;
                    while (end < n && !clusters_[end].breakBefore)
                        ++end;
                    float word = 0.0f, ink = 0.0f;
                    for (std::uint32_t k = i; k < end; ++k)
                    {
                        word += clusters_[k].advance;
                        if (!clusters_[k].space)
                            ink = word;
                    }
                    if (i > lineStart && lineWidth + ink > maxWidth)
                    {
                        lines_.push_back({lineStart, i, false});
                        lineStart = i;
                        lineWidth = 0.0f;
                    }
                    if (i == lineStart && ink > maxWidth)
                    {
                        float w = 0.0f;
                        for (std::uint32_t k = i; k < end; ++k)
                        {
                            const Cluster& c = clusters_[k];
                            if (k > lineStart && !c.space && w + c.advance > maxWidth)
                            {
                                lines_.push_back({lineStart, k, false});
                                lineStart = k;
                                w = 0.0f;
                            }
                            w += c.advance;
                        }
                        lineWidth = w;   // the word's rest stays open for the words after it
                    }
                    else
                        lineWidth += word;
                    i = end;
                }
                lines_.push_back({lineStart, n, false});
            }

            // U+2026 (or "..." when no font has it) into ellipsis_; returns its advance.
            float ShapeEllipsis(std::uint16_t primary, float size)
            {
                ellipsis_.clear();
                std::uint16_t face = PickFace(primary, 0x2026, -1);
                const bool single = faces_[face]->Has(0x2026);
                if (!single)
                    face = primary;
                const Face& f = *faces_[face];
                hb_buffer_clear_contents(buffer_);
                hb_buffer_add_utf8(buffer_, single ? "\xE2\x80\xA6" : "...", -1, 0, -1);
                hb_buffer_guess_segment_properties(buffer_);
                hb_shape(f.hb, buffer_, nullptr, 0);
                unsigned n = 0;
                const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer_, &n);
                const hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer_, nullptr);
                const float k = size / f.upem;
                float advance = 0.0f;
                for (unsigned i = 0; i < n; ++i)
                {
                    RunGlyph g;
                    g.face = face;
                    g.glyph = (std::uint16_t)info[i].codepoint;
                    g.advance = (float)pos[i].x_advance * k;
                    g.offset = Vec2((float)pos[i].x_offset * k, -(float)pos[i].y_offset * k);
                    ellipsis_.push_back(g);
                    advance += g.advance;
                }
                return advance;
            }

            // Cuts a line wider than `maxWidth` so that it fits with an ellipsis after it (spaces before it go).
            void Trim(Line& line, float maxWidth, std::uint16_t primary, float size)
            {
                float w = 0.0f, ink = 0.0f;
                for (std::uint32_t c = line.c0; c < line.c1; ++c)
                {
                    w += clusters_[c].advance;
                    if (!clusters_[c].space)
                        ink = w;
                }
                if (ink <= maxWidth)
                    return;
                const float avail = maxWidth - ShapeEllipsis(primary, size);
                std::uint32_t k = line.c0;
                w = 0.0f;
                while (k < line.c1 && w + clusters_[k].advance <= avail)
                    w += clusters_[k++].advance;
                while (k > line.c0 && clusters_[k - 1].space)
                    --k;
                line.c1 = k;
                line.ellipsis = true;
            }

            // The line's glyphs in visual order (each right-to-left run reversed back), pen from 0; y = `baseline`.
            void PlaceLine(const Line& line, float baseline, Layout& layout)
            {
                LineInfo li;
                li.g0 = layout.glyphs.size();
                float pen = 0.0f;
                for (std::uint32_t c = line.c0; c < line.c1; ++c)
                {
                    pen += clusters_[c].advance;
                    if (!clusters_[c].space)
                        li.width = pen;
                }
                pen = 0.0f;
                if (line.c0 < line.c1)
                {
                    const std::uint32_t g1 = clusters_[line.c1 - 1].g1;
                    for (std::uint32_t g = clusters_[line.c0].g0; g < g1;)
                    {
                        std::uint32_t e = g + 1;
                        while (e < g1 && glyphs_[e].run == glyphs_[g].run)
                            ++e;
                        const bool rtl = runs_[glyphs_[g].run].rtl;
                        for (std::uint32_t j = 0; j < e - g; ++j)
                        {
                            const RunGlyph& rg = glyphs_[rtl ? e - 1 - j : g + j];
                            if (rg.glyph != kNoGlyph)
                                layout.glyphs.push_back({rg.face, rg.glyph, Vec2(pen + rg.offset.x, baseline + rg.offset.y)});
                            pen += rg.advance;
                        }
                        g = e;
                    }
                }
                if (line.ellipsis)
                {
                    for (const RunGlyph& rg : ellipsis_)
                    {
                        layout.glyphs.push_back({rg.face, rg.glyph, Vec2(pen + rg.offset.x, baseline + rg.offset.y)});
                        pen += rg.advance;
                    }
                    li.width = pen;
                }
                li.g1 = layout.glyphs.size();
                li.full = pen;
                lineInfo_.push_back(li);
            }

            // The atlas slot of a glyph at `emPixels` (physical pixels per em) and pen phase `phase` (quarter pixels),
            // rasterized from the unhinted outline on first use.
            const GlyphSlot* Glyph(std::uint16_t face, std::uint16_t glyph, float emPixels, int phase)
            {
                const std::uint64_t q = (std::uint64_t)std::min(emPixels * 16.0f + 0.5f, 134217727.0f);   // 1/16 px, 27 bits
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) | (q << 2) | (std::uint64_t)(phase & 3);
                if (faces_[face]->colorStrikes)
                    return ColorGlyph(face, glyph, emPixels);
                if (const GlyphSlot* s = atlas_.Find(key))
                    return s;
                Face& f = *faces_[face];
                outline_.Clear();
                // design units, no hinting, no embedded bitmaps (color strikes: ColorGlyph)
                if (FT_Load_Glyph(f.ft, glyph, FT_LOAD_NO_SCALE) == 0 && f.ft->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    FT_Outline_Funcs funcs{};
                    funcs.move_to = &SinkMoveTo;
                    funcs.line_to = &SinkLineTo;
                    funcs.conic_to = &SinkConicTo;
                    funcs.cubic_to = &SinkCubicTo;
                    OutlineSink sink{&outline_, ((float)q / 16.0f) / f.upem};
                    FT_Outline_Decompose(&f.ft->glyph->outline, &funcs, &sink);
                }
                const float offsetX = 0.25f * (float)phase;
                if (!RasterizeGray(outline_, offsetX, bitmap_))
                    bitmap_ = GlyphBitmap{};   // larger than the rasterizer takes: cached as empty, not retried every frame
                const GlyphSlot* s = atlas_.Add(key, bitmap_, outline_.Bounds());
                return s ? s : atlas_.Add(key, GlyphBitmap{}, Rect());   // larger than a page: empty too
            }

            // A color glyph (emoji) at `emPixels`: the bitmap of the nearest strike at or above that size (the largest
            // when none is), decoded by FreeType (sbix / CBDT: PNG), scaled by area averaging in premultiplied alpha,
            // stored straight-alpha in the RGBA atlas. Whole pixels: no pen phases.
            const GlyphSlot* ColorGlyph(std::uint16_t face, std::uint16_t glyph, float emPixels)
            {
                const std::uint64_t q = (std::uint64_t)std::min(emPixels * 4.0f + 0.5f, 134217727.0f);   // 1/4 px
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) | q;
                if (const GlyphSlot* s = colorAtlas_.Find(key))
                    return s;
                FT_Face ft = faces_[face]->ft;
                int strike = -1;
                float strikePx = 0.0f;
                for (int i = 0; i < ft->num_fixed_sizes; ++i)
                {
                    const float ppem = (float)ft->available_sizes[i].y_ppem / 64.0f;
                    const bool better = strike < 0 || (strikePx < emPixels ? ppem > strikePx : (ppem >= emPixels && ppem < strikePx));
                    if (ppem > 0.0f && better)
                    {
                        strike = i;
                        strikePx = ppem;
                    }
                }
                bitmap_ = GlyphBitmap{};
                bitmap_.channels = 4;
                Rect ink;
                if (strike >= 0 && FT_Select_Size(ft, strike) == 0 && FT_Load_Glyph(ft, glyph, FT_LOAD_COLOR) == 0 &&
                    (ft->glyph->format == FT_GLYPH_FORMAT_BITMAP || FT_Render_Glyph(ft->glyph, FT_RENDER_MODE_NORMAL) == 0) &&
                    ft->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && ft->glyph->bitmap.width > 0 && ft->glyph->bitmap.rows > 0)
                {
                    const FT_Bitmap& b = ft->glyph->bitmap;
                    const float k = emPixels / strikePx;
                    ScaleBgra(b.buffer, (int)b.width, (int)b.rows, b.pitch, k, bitmap_);
                    bitmap_.left = (int)std::floor((float)ft->glyph->bitmap_left * k + 0.5f);
                    bitmap_.top = -(int)std::floor((float)ft->glyph->bitmap_top * k + 0.5f);
                    ink = Rect((float)bitmap_.left, (float)bitmap_.top, (float)(bitmap_.left + bitmap_.width), (float)(bitmap_.top + bitmap_.height));
                }
                const GlyphSlot* s = colorAtlas_.Add(key, bitmap_, ink);
                return s ? s : colorAtlas_.Add(key, GlyphBitmap{0, 0, 0, 0, {}, 4}, Rect());   // larger than a page: empty
            }

            // Premultiplied BGRA (FreeType's color bitmaps) -> straight RGBA8, scaled by `k`: each target pixel the area
            // average of the source pixels under it (a box filter both ways; only downscaling keeps every source pixel)
            static void ScaleBgra(const unsigned char* src, int w, int h, int pitch, float k, GlyphBitmap& out)
            {
                const int dw = std::max(1, (int)std::lround((float)w * k)), dh = std::max(1, (int)std::lround((float)h * k));
                const float sx = (float)w / (float)dw, sy = (float)h / (float)dh;
                std::vector<float> rows((std::size_t)dw * h * 4, 0.0f);   // horizontal pass: dw x h
                for (int y = 0; y < h; ++y)
                {
                    const unsigned char* line = src + (std::ptrdiff_t)y * pitch;
                    for (int x = 0; x < dw; ++x)
                    {
                        const float x0 = (float)x * sx, x1 = x0 + sx;
                        float acc[4] = {0, 0, 0, 0};
                        for (int i = (int)x0; i < w && (float)i < x1; ++i)
                        {
                            const float wgt = std::min((float)i + 1.0f, x1) - std::max((float)i, x0);
                            for (int c = 0; c < 4; ++c)
                                acc[c] += (float)line[i * 4 + c] * wgt;
                        }
                        for (int c = 0; c < 4; ++c)
                            rows[((std::size_t)y * dw + x) * 4 + c] = acc[c] / sx;
                    }
                }
                out.width = dw;
                out.height = dh;
                out.pixels.assign((std::size_t)dw * dh * 4, 0);
                for (int y = 0; y < dh; ++y)
                {
                    const float y0 = (float)y * sy, y1 = y0 + sy;
                    for (int x = 0; x < dw; ++x)
                    {
                        float acc[4] = {0, 0, 0, 0};   // b g r a, premultiplied
                        for (int j = (int)y0; j < h && (float)j < y1; ++j)
                        {
                            const float wgt = std::min((float)j + 1.0f, y1) - std::max((float)j, y0);
                            for (int c = 0; c < 4; ++c)
                                acc[c] += rows[((std::size_t)j * dw + x) * 4 + c] * wgt;
                        }
                        for (float& v : acc)
                            v /= sy;
                        std::uint8_t* px = out.pixels.data() + ((std::size_t)y * dw + x) * 4;
                        const float a = acc[3];
                        if (a <= 0.0f)
                            continue;
                        const auto straight = [a](float v) { return (std::uint8_t)std::clamp(v * 255.0f / a + 0.5f, 0.0f, 255.0f); };
                        px[0] = straight(acc[2]);
                        px[1] = straight(acc[1]);
                        px[2] = straight(acc[0]);
                        px[3] = (std::uint8_t)std::clamp(a + 0.5f, 0.0f, 255.0f);
                    }
                }
            }

            FT_Library ft_ = nullptr;
            hb_buffer_t* buffer_ = nullptr;
            hb_unicode_funcs_t* unicode_ = nullptr;
            std::vector<std::unique_ptr<Face>> faces_;   // FontId - 1
            std::unordered_map<std::string, std::weak_ptr<hb_blob_t>> files_;   // AddFontFile's, by path
            std::vector<std::uint16_t> fallbacks_;
            GlyphAtlas atlas_;
            GlyphAtlas colorAtlas_;   // RGBA8: color glyphs
            RasterParams params_;
            std::uint64_t frame_ = 0;
            std::unordered_map<std::uint64_t, Layout> layouts_;

            // scratch, reused by every layout
            std::vector<CodePoint> cps_;
            std::vector<Run> runs_;
            std::vector<RunGlyph> glyphs_;
            std::vector<Cluster> clusters_;
            std::vector<float> glyphX_;               // CaretStops: each glyph's left edge
            std::vector<std::uint8_t> directions_;    // ResolveDirections
            std::vector<std::uint32_t> graphemes_;    // CaretStops: the graphemes of a cluster
            std::vector<Line> lines_;
            std::vector<LineInfo> lineInfo_;
            std::vector<RunGlyph> ellipsis_;
            Outline outline_;
            GlyphBitmap bitmap_;
        };
    }

    std::unique_ptr<TextSystem> CreateFreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc)
    {
        auto system = std::make_unique<FreeTypeTextSystem>(textures, desc);
        if (!system->Init())
            return nullptr;
        return system;
    }
}
