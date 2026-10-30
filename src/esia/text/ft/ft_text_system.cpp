// Esia - FreeType + HarfBuzz text system (include/esia/text/freetype.hpp).
//
// Measure / Draw -> GetLayout(), cached per text, font, size, wrap width and flags. Per paragraph: Itemize() picks a
// font and a script for every code point and cuts runs, Shape() runs HarfBuzz per run, BuildClusters() groups the
// glyphs and marks where a line may break, BreakLines() fills lines greedily, PlaceLine() positions a line's glyphs
// in visual order. Draw() snaps every glyph to the physical pixel grid and takes its bitmap from the atlas,
// rasterizing it from the FreeType outline the first time. A system font FreeType cannot draw (Apple: PingFang, through
// Core Text) is a face of the platform's: HarfBuzz on its tables, the platform's outlines (platform_face.hpp). Color
// glyphs: bitmap strikes (sbix, CBDT) scaled, COLR layers and paints (Segoe UI Emoji, Android's Noto Color Emoji)
// painted from their outlines (colr.hpp).
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include "colr.hpp"
#include "platform_face.hpp"
#include "png.hpp"
#include "esia/base/hash.hpp"
#include "esia/base/utf8.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/text/distance_field.hpp"
#include "esia/text/glyph_atlas.hpp"

#include <array>
#include <atomic>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_BITMAP_H
#include FT_DRIVER_H
#include FT_LCD_FILTER_H
#include FT_MODULE_H
#include FT_MULTIPLE_MASTERS_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include <hb-ot.h>
#include <hb.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace esia::text
{
    namespace
    {
        // std::floor for the glyph loop: a call where the build targets SSE2 (no roundss), three per glyph. Exact for
        // |v| < 2^31 (pixel positions); the sign of a zero may differ, which no position notices.
        inline float FloorPx(float v)
        {
            if (!(std::fabs(v) < 2.0e9f))
                return std::floor(v);
            const float t = (float)(int)v;
            return t > v ? t - 1.0f : t;
        }

        constexpr std::uint16_t kNoGlyph = 0xFFFF;          // control characters: an advance, nothing drawn (glyph ids end at 0xFFFE)
        constexpr std::size_t kLayoutCacheLimit = 1024;     // layouts kept before unused ones are dropped ...
        constexpr std::uint64_t kLayoutCacheFrames = 120;   // ... after this many frames without use
        constexpr std::size_t kLayoutSweepBuckets = 256;    // ... looked for in this many buckets a frame

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

        // A font file mapped into memory (HarfBuzz's blob), shared by the faces of a collection. Its own type rather
        // than a shared_ptr with hb_blob_destroy as deleter: a custom deleter instantiates std::type_info::operator==,
        // which clang with mingw-w64's libstdc++ 13 defines twice (a duplicate symbol at link time).
        struct MappedFile
        {
            hb_blob_t* blob = nullptr;
            explicit MappedFile(hb_blob_t* b) : blob(b) {}
            MappedFile(const MappedFile&) = delete;
            MappedFile& operator=(const MappedFile&) = delete;
            ~MappedFile() { hb_blob_destroy(blob); }
        };

        struct Face
        {
            // AddFontFile: the file, mapped into memory (HarfBuzz's blob); FreeType and HarfBuzz read it in place, and
            // the faces of a collection share it. Mapped, not read: a color emoji font is 200 MB, of which a frame
            // touches a few pages.
            std::shared_ptr<const MappedFile> file;
            FT_Face ft = nullptr;
            std::unique_ptr<detail::PlatformFace> platform;   // instead of `ft`: outlines from the platform
            // a color font's color glyphs FreeType cannot decode: drawn by the platform (opened on first need)
            std::unique_ptr<detail::PlatformFace> colorPlatform;
            bool colorPlatformTried = false;
            bool colorStrikes = false;   // color bitmap strikes (sbix, CBDT: emoji): drawn from those, not outlines
            bool colorLayers = false;    // COLR color glyphs (vector emoji): painted from their layers (colr.hpp)
            // A bitmap font (BDF, PCF, Windows FNT / FON, a bitmap-only sfnt): drawn from its strikes, pixel for pixel.
            // The strike in use (SelectStrike) sets upem (its pixels), the metrics and what its HarfBuzz font reads.
            // A Windows .fon holds a face per size: its faces of one family and style are this face's strikes.
            struct Strike
            {
                FT_Face ft = nullptr;   // `ft` or one of `moreFaces`
                int index = 0;          // its FT_Bitmap_Size
                int px = 0;             // its em in pixels
            };
            bool bitmap = false;
            std::vector<Strike> strikes;
            std::vector<FT_Face> moreFaces;   // a .fon's other sizes (owned)
            int strike = -1;                  // in use
            FT_Face active = nullptr;         // its face
            hb_font_t* hb = nullptr;
            float upem = 1000.0f;
            float ascent = 0.0f, descent = 0.0f, gap = 0.0f;   // design units, descent positive
            FT_F26Dot6 sizeSet = 0;   // the em FT_Set_Char_Size last gave `ft` (hinted loads), 26.6 pixels; 0 = none
            std::uint32_t codePages = 0;                        // OS/2 ulCodePageRange1

            Face() = default;
            Face(const Face&) = delete;
            Face& operator=(const Face&) = delete;
            ~Face()
            {
                if (hb)
                    hb_font_destroy(hb);
                for (FT_Face f : moreFaces)
                    FT_Done_Face(f);
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
            // font id, size, wrap width, flags, and the density when the advances are hinted (the cache key's inputs)
            std::uint32_t params[5] = {};
            std::vector<PlacedGlyph> glyphs;
            TextMetrics metrics;   // (metrics.layout: its serial, given whenever it is laid out; 0 out of the cache)
            bool bitmapGlyphs = false;   // a glyph of a bitmap font among them (PlaceQuads puts those on whole pixels)
            std::uint64_t lastFrame = 0;
            // Draw's glyph quads, from its last call: positions in physical pixels from the whole pixel at the text's
            // origin, so they hold wherever the origin has the same sub-pixel offsets - the next frame, another row,
            // another window. Valid while the atlases did not start over and the em size, the scale and those offsets
            // are the same: the text is then copied in, not placed glyph by glyph (that was most of building a frame of
            // labels). The colors go in as the quads are copied: a text that changes color (a fade, a hover tint, the
            // cross-fade of smooth motion) is not placed again.
            struct QuadRun
            {
                TextureId page = 0;
                std::uint32_t first = 0, count = 0;   // quads
                Rect bounds{1e30f, 1e30f, -1e30f, -1e30f};   // pixels, as QuadSet::bounds
            };
            struct QuadRow   // the quads of a line (lines go downwards): what a clip holds of a long text
            {
                std::uint32_t first = 0, count = 0;
                float y0 = 1e30f, y1 = -1e30f;   // pixels, as QuadSet::bounds
            };
            struct Placed
            {
                // one per drawn glyph, its rect in pixels from the origin; packed[2] 0 and packed[3] 1 for a color
                // glyph (drawn white, with the text's alpha), 0 for one in the text's color
                std::vector<QuadInstance> quads;
                std::vector<QuadRun> runs;   // one per change of atlas page
                std::vector<QuadRow> rows;
                Rect bounds;                 // pixels from the origin's whole pixel
            };
            struct QuadSet : Placed
            {
                std::uint64_t generation = 0;   // 0 = none
                float em = 0.0f, scale = 0.0f, fx = -1.0f, fy = -1.0f;
                std::uint32_t outline = 0;   // an outline's set: its width in 1/8 px
                std::uint32_t misses = 0;    // draws placed again since the last one that found its quads here
                // every glyph's atlas slot at each pen phase (null: not looked up yet) for slotGeneration, slotEm and
                // slotOutline: a text drawn at other sub-pixel offsets (labels over a scene) is placed again without the
                // atlas's map. The moving sets' at each pen phase and vertical phase (-1 .. 3): movingSlots.
                std::vector<std::array<const GlyphSlot*, 4>> slots;
                std::vector<std::array<const GlyphSlot*, 20>> movingSlots;
                std::uint64_t slotGeneration = 0;   // 0 = none
                float slotEm = 0.0f;
                std::uint32_t slotOutline = 0;
                // Placements by rounding class - a text drawn at many sub-pixel offsets (labels over a scene, the same
                // name on many of them): a glyph's quad depends only on the quarter pixel its pen rounds to across and
                // the whole pixel its line rounds to down, so between the offsets where one of them changes (xBreaks,
                // yBreaks: 0 .. 1, sorted) the whole placement is the same, and is copied in as a text placed before.
                // Valid for classGeneration, classEm, classScale, classRs, classOutline.
                std::vector<float> xBreaks, yBreaks;
                std::vector<std::int32_t> classIndex;   // x class * (yBreaks + 1) + y class -> classes; -1: not placed
                std::vector<Placed> classes;
                std::uint32_t classCount = 0;        // classes in use (the others keep their storage)
                std::int32_t classScore = 0;         // -64 .. 64: whether this text's offsets come back (DrawQuads)
                std::uint64_t classGeneration = 0;   // 0 = none
                float classEm = 0.0f, classScale = 0.0f, classRs = 0.0f;
                std::uint32_t classOutline = 0;
            };
            QuadSet fill;      // Draw's
            QuadSet outline;   // DrawOutline's (TextOutline): the glyphs grown, in the outline's color
            QuadSet moving, movingOutline;   // the same in motion (TextRendering::smoothMotion): design outlines

            // Smooth motion: per draw of the layout in a frame (in order), where it was and since when it is still
            struct Motion
            {
                Vec2 pos;
                float scale = 0.0f;
                std::uint32_t still = 0;   // frames
                double since = 0.0;        // RasterParams::time it stopped at
                bool seen = false;
            };
            struct Draws
            {
                std::vector<Motion> motion;
                std::uint64_t frame = 0;   // of `count`
                std::uint32_t count = 0;   // draws so far in that frame
            };
            Draws fillDraws, outlineDraws;
        };

        // FreeType outline (design units, y up) -> GlyphShape (field pixels, y down), curves kept
        struct ShapeSink
        {
            GlyphShape* shape = nullptr;
            float scale = 1.0f;
            Vec2 Map(const FT_Vector* v) const { return Vec2((float)v->x * scale, -(float)v->y * scale); }
        };

        int ShapeMoveTo(const FT_Vector* to, void* user)
        {
            const ShapeSink& s = *static_cast<const ShapeSink*>(user);
            s.shape->MoveTo(s.Map(to));
            return 0;
        }

        int ShapeLineTo(const FT_Vector* to, void* user)
        {
            const ShapeSink& s = *static_cast<const ShapeSink*>(user);
            s.shape->LineTo(s.Map(to));
            return 0;
        }

        int ShapeConicTo(const FT_Vector* control, const FT_Vector* to, void* user)
        {
            const ShapeSink& s = *static_cast<const ShapeSink*>(user);
            s.shape->QuadTo(s.Map(control), s.Map(to));
            return 0;
        }

        int ShapeCubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
        {
            const ShapeSink& s = *static_cast<const ShapeSink*>(user);
            s.shape->CubicTo(s.Map(control1), s.Map(control2), s.Map(to));
            return 0;
        }

        // Smooth motion: a text still for this long is drawn fully hinted (RasterParams::time, else frames)
        constexpr double kRestSeconds = 0.12;
        constexpr std::uint32_t kRestFrames = 8;

        // Distance fields: one per glyph for every size, made at this em with these ranges (field pixels)
        constexpr float kFieldEm = 48.0f;
        constexpr DistanceFieldDesc kFieldDesc{8.0f, 24.0f};

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

        std::uint64_t NextSerial()
        {
            static std::atomic<std::uint64_t> next{0};
            return ++next;
        }

        class FreeTypeTextSystem final : public TextSystem
        {
        public:
            FreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc)
                : atlas_(textures, GlyphAtlasDesc{desc.atlasPageSize, desc.atlasMaxPages, 1}),
                  colorAtlas_(textures, GlyphAtlasDesc{std::min(desc.atlasPageSize, 1024), 2, 1, TextureFormat::RGBA8}),
                  fieldAtlas_(textures, GlyphAtlasDesc{desc.atlasPageSize, 2, 1, TextureFormat::RGBA8, TextureFlags_DistanceField}),
                  fields_(FieldMakerDesc{desc.fieldThreads, desc.fieldCacheDirectory}), rendering_(desc.rendering), pending_(desc.rendering)
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
                ApplyRendering();
                return hb_buffer_allocation_successful(buffer_) != 0;
            }

            // ------------------------------------------------------------------ fonts
            FontId AddFontFile(const char* path, int faceIndex) override
            {
                if (!path)
                    return 0;
                if (std::unique_ptr<detail::PlatformFace> platform = detail::OpenPlatformFace(path))
                    return AddPlatformFace(std::move(platform));
                // the faces of a collection (the CJK fonts of a system's fallback chain: 20 MB and more) share one copy
                std::weak_ptr<const MappedFile>& cached = files_[path];
                std::shared_ptr<const MappedFile> blob = cached.lock();
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
                lastLayout_ = nullptr;   // it was one of them
                layoutFront_.fill(nullptr);
                bySerial_.fill({});
                ++drawGeneration_;   // (a glyph drawn alone may come from another font too)
            }

            // ------------------------------------------------------------------ frame
            // tiles under keys with the top bit set: a glyph's key never has it (faces stay far below 2^15)
            const GlyphSlot* FindTile(std::uint64_t key) override { return atlas_.Find(key | (1ull << 63)); }
            const GlyphSlot* AddTile(std::uint64_t key, const GlyphBitmap& bitmap) override
            {
                return atlas_.Add(key | (1ull << 63), bitmap, Rect());
            }
            std::uint64_t TileGeneration() override { return serial_ << 32 | ((std::uint64_t)atlas_.Resets() + 1u); }

            // ---- draws done again (DrawNote)
            void SetDrawNotes(std::vector<DrawNote>* notes) override { notes_ = notes; }

            std::uint64_t DrawGeneration() override { return serial_ << 32 | (drawGeneration_ & 0xFFFFFFFFu); }

            bool ReplayDraws(std::span<const DrawNote> notes) override
            {
                if (!CanReplayDraws(notes))
                    return false;
                for (const DrawNote& n : notes)
                {
                    Layout& l = *bySerial_[n.layout & (kSerialSlots - 1)].layout;
                    (void)RestWeight(n.outline ? l.outlineDraws : l.fillDraws, n.pos, n.scale);
                    l.lastFrame = frame_;   // in use, as drawn
                }
                return true;
            }

            void KeepLayouts(std::span<const std::uint64_t> layouts) override
            {
                for (const std::uint64_t serial : layouts)
                {
                    const SerialSlot& s = bySerial_[serial & (kSerialSlots - 1)];
                    if (serial != 0 && s.serial == serial)
                        s.layout->lastFrame = frame_;
                }
            }

            bool CanReplayDraws(std::span<const DrawNote> notes) override
            {
                // Each draw as RestWeight would take it now: the next record of its layout this frame (after the notes
                // before it of the same layout), seen, where it is, at rest.
                for (std::size_t i = 0; i < notes.size(); ++i)
                {
                    const DrawNote& n = notes[i];
                    const SerialSlot& s = bySerial_[n.layout & (kSerialSlots - 1)];
                    if (n.layout == 0 || s.serial != n.layout || !rendering_.smoothMotion || !(n.rest >= 1.0f))
                        return false;
                    const Layout::Draws& d = n.outline ? s.layout->outlineDraws : s.layout->fillDraws;
                    std::size_t k = d.frame == frame_ ? d.count : 0;
                    for (std::size_t j = 0; j < i; ++j)
                        k += notes[j].layout == n.layout && notes[j].outline == n.outline;
                    if (k >= d.motion.size())
                        return false;
                    const Layout::Motion& m = d.motion[k];
                    if (!m.seen || LengthSq(n.pos - m.pos) > 1e-8f || n.scale != m.scale)
                        return false;   // moved: drawn in motion
                    const std::uint32_t still = m.still < kRestFrames ? m.still + 1 : m.still;
                    const float t = params_.time >= 0.0 ? (float)((params_.time - m.since) / kRestSeconds) : (float)still / (float)kRestFrames;
                    if (!(SmoothStep(0.0f, 1.0f, t) >= 1.0f))
                        return false;   // still settling
                }
                return true;
            }

            // A layout leaving the cache (or laid out again): its serial finds it no more
            void Forget(Layout& layout)
            {
                SerialSlot& s = bySerial_[layout.metrics.layout & (kSerialSlots - 1)];
                if (layout.metrics.layout != 0 && s.serial == layout.metrics.layout)
                    s = SerialSlot{};
                layout.metrics.layout = 0;
            }

            void SetRendering(const TextRendering& rendering) override { pending_ = rendering; }
            TextRendering Rendering() const override { return pending_; }

            void NewFrame(const RasterParams& params) override
            {
                params_ = params;
                if (!(params_.pixelsPerUnit > 0.0f))
                    params_.pixelsPerUnit = 1.0f;
                if (!(pending_ == rendering_))
                {
                    // another rendering: every glyph is rasterized again, and the layouts are built again when the
                    // advances change with it
                    const bool advances = HintedAdvances();
                    rendering_ = pending_;
                    ApplyRendering();
                    atlas_.StartOver();
                    hintedAdvanceCache_.clear();
                    ++drawGeneration_;
                    if (advances || HintedAdvances())
                    {
                        layouts_.clear();
                        spareLayouts_.clear();
                        layoutFront_.fill(nullptr);
                        lastLayout_ = nullptr;
                        bySerial_.fill({});
                    }
                }
                if (bitmapFaces_ > 0 && params_.pixelsPerUnit != layoutPixelsPerUnit_)
                {
                    // a bitmap font's sizes snap to its strikes in pixels: its layouts change with the density
                    layouts_.clear();
                    spareLayouts_.clear();
                    layoutFront_.fill(nullptr);
                    bySerial_.fill({});
                }
                if (params_.pixelsPerUnit != layoutPixelsPerUnit_)
                    ++drawGeneration_;
                layoutPixelsPerUnit_ = params_.pixelsPerUnit;
                ++frame_;
                atlas_.BeginFrame();
                colorAtlas_.BeginFrame();
                fieldAtlas_.BeginFrame();
                const std::uint64_t resets[3] = {(std::uint64_t)atlas_.Resets(), (std::uint64_t)colorAtlas_.Resets(), (std::uint64_t)fieldAtlas_.Resets()};
                if (std::memcmp(resets, lastResets_, sizeof(resets)) != 0)
                {
                    // pages started over: drawn quads point at other glyphs now
                    std::memcpy(lastResets_, resets, sizeof(resets));
                    ++drawGeneration_;
                }
                lastLayout_ = nullptr;   // (it may leave the cache now)
                // layouts unused for a while leave the cache, and wait (with their storage) to hold the next new texts:
                // text that changes every frame (a frame rate, a timer) does not allocate once the cache is warm
                // A few buckets a frame are looked through (sweepBucket_ goes round the map), not the whole map: a cache
                // of many texts in use costs a frame nothing; a layout that leaves takes its front slot with it.
                if (layouts_.size() > kLayoutCacheLimit)
                {
                    const std::size_t buckets = layouts_.bucket_count();
                    for (std::size_t n = 0; n < kLayoutSweepBuckets && n < buckets; ++n)
                    {
                        const std::size_t b = sweepBucket_++ % buckets;
                        sweepKeys_.clear();
                        for (auto it = layouts_.begin(b); it != layouts_.end(b); ++it)
                            if (it->second.lastFrame + kLayoutCacheFrames < frame_)
                                sweepKeys_.push_back(it->first);
                        for (const std::uint64_t key : sweepKeys_)
                        {
                            const auto it = layouts_.find(key);
                            Layout*& front = layoutFront_[(std::size_t)(key ^ (key >> 32)) & (kLayoutFront - 1)];
                            if (front == &it->second)
                                front = nullptr;
                            Forget(it->second);
                            if (spareLayouts_.size() < kLayoutCacheLimit / 4)
                                spareLayouts_.push_back(layouts_.extract(it));
                            else
                                layouts_.erase(it);
                        }
                    }
                }
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
                Layout* layout = GetLayout(font, text, wrapWidth, flags);
                if (!layout)
                    return Vec2(0, 0);
                if (color.a <= 0.0f || !(scale > 0.0f))
                    return layout->metrics.size;
                const bool snap = (flags & TextFlags_PixelSnap) != 0;
                if (rendering_.distanceFieldFrom > 0.0f && font.size * scale * params_.pixelsPerUnit >= rendering_.distanceFieldFrom)
                {
                    if (snap)
                        pos = SnapToPixel(pos);
                    // large: the glyphs' distance fields, one per glyph for every size
                    fx::Instance m;
                    std::memset(&m, 0, sizeof(m));
                    m.fill0[0] = m.fill1[0] = color.r;
                    m.fill0[1] = m.fill1[1] = color.g;
                    m.fill0[2] = m.fill1[2] = color.b;
                    m.fill0[3] = m.fill1[3] = color.a;
                    m.misc[0] = 1.0f;
                    m.flags[0] = fx::kFill;
                    m.flags[1] = (std::uint32_t)fx::PaintKind::Solid;
                    return DrawShapes(dl, font, pos, m, 0, 0.0f, text, wrapWidth, flags, scale);
                }
                // in motion: the design's outlines at quarter pixels both ways; at rest: this rendering; between: both.
                // On whole pixels a text is placed the same everywhere: it never moves by a fraction to smooth.
                const float rest = rendering_.smoothMotion && !snap ? RestWeight(layout->fillDraws, pos, scale) : 1.0f;
                if (notes_ && rendering_.smoothMotion && !snap)
                    notes_->push_back({layout->metrics.layout, pos, scale, rest, 0});
                if (rest < 1.0f)
                    DrawQuads(dl, *layout, layout->moving, font, pos, color.WithAlpha(color.a * (1.0f - rest)), scale, 0, true, false);
                if (rest > 0.0f)
                    DrawQuads(dl, *layout, layout->fill, font, pos, color.WithAlpha(color.a * rest), scale, 0, false, snap);
                return layout->metrics.size;
            }

            // `pos` (UI units) at the nearest whole pixel
            Vec2 SnapToPixel(Vec2 pos) const
            {
                const float rs = params_.pixelsPerUnit;
                return Vec2(std::floor(pos.x * rs + 0.5f) / rs, std::floor(pos.y * rs + 0.5f) / rs);
            }

            // The quads of `layout` at `pos` in one of its sets: placed again when anything they depend on changed.
            // `snap`: from the nearest whole pixel (TextFlags_PixelSnap), without sub-pixel offsets.
            void DrawQuads(DrawList& dl, Layout& layout, Layout::QuadSet& set, FontRef font, Vec2 pos, Color color, float scale, std::uint32_t outlineQ,
                           bool moving, bool snap)
            {
                const float rs = params_.pixelsPerUnit, inv = 1.0f / rs;
                // animated scales are quantized to 1/4 px of em, so an animation does not flood the atlas
                const float em = scale == 1.0f ? font.size * rs : std::floor(font.size * scale * rs * 4.0f + 0.5f) * 0.25f;
                // color glyphs: white with the text's alpha (an outline has none of them)
                const std::uint32_t rgba = color.ToRgba8(), colorGlyphRgba = outlineQ ? 0u : (rgba | 0x00FFFFFFu);
                if ((rgba >> 24) == 0)
                    return;   // nothing shows (a color glyph has the same alpha)
                // Glyphs are placed from the whole pixel at the origin (ox, oy) by its sub-pixel offsets (fx, fy): the
                // same offsets, the same quads wherever the text is
                const float ox = FloorPx(pos.x * rs + (snap ? 0.5f : 0.0f)), oy = FloorPx(pos.y * rs + (snap ? 0.5f : 0.0f));
                const float fx = snap ? 0.0f : pos.x * rs - ox, fy = snap ? 0.0f : pos.y * rs - oy;
                const std::uint64_t generation = (((std::uint64_t)atlas_.Resets() << 32) | (std::uint32_t)colorAtlas_.Resets()) + 1u;
                if (set.generation != generation || set.em != em || set.scale != scale || set.fx != fx || set.fy != fy || set.outline != outlineQ)
                {
                    // Other sub-pixel offsets: the placement of their rounding class when it was kept. Classes are kept
                    // while the text's offsets come back (classScore: +4 a class found, -1 one placed); a text whose
                    // offsets do not (a label that moves on, another text every few frames) is placed as below and its
                    // classes neither looked up nor kept, but every 16th time, to see whether they do now.
                    std::int32_t* index = nullptr;
                    if (layout.glyphs.size() <= kClassGlyphs && (set.classScore > -32 || (set.misses & 15) == 0))
                    {
                        ClassBreaks(layout, set, em, scale, generation, outlineQ, moving);
                        const std::size_t cx = (std::size_t)(std::upper_bound(set.xBreaks.begin(), set.xBreaks.end(), fx) - set.xBreaks.begin());
                        const std::size_t cy = (std::size_t)(std::upper_bound(set.yBreaks.begin(), set.yBreaks.end(), fy) - set.yBreaks.begin());
                        index = &set.classIndex[cx * (set.yBreaks.size() + 1) + cy];
                        if (*index >= 0)
                        {
                            set.classScore = std::min(set.classScore + 4, 64);
                            EmitQuads(dl, set.classes[(std::size_t)*index], ox, oy, rs, rgba, colorGlyphRgba);
                            return;
                        }
                        set.classScore = std::max(set.classScore - 1, -64);
                        if (set.classScore < 0 && (set.misses & 15) != 0)
                            index = nullptr;
                    }
                    // Placed again. When the text lies inside the clip with an em to spare (its ink cannot reach past),
                    // the quads go into the list as they are placed: a label at a new sub-pixel offset every frame
                    // (over a moving scene) takes one pass over its glyphs, not two. A text placed again and again without
                    // ever being drawn twice at the same offsets (that label; one text at many places in a frame) is
                    // not kept either, but every 8th time, to see whether it settled - or for its class.
                    const bool keep = index || set.misses < 2 || (set.misses & 7) == 0;
                    ++set.misses;
                    const float margin = font.size * scale;
                    const Vec2 size = layout.metrics.size * scale;
                    if (dl.ClipRect().Contains(Rect(pos.x - margin, pos.y - margin, pos.x + size.x + margin, pos.y + size.y + margin)))
                    {
                        const QuadOut out{&dl, ox, oy, inv, rgba, colorGlyphRgba};
                        if (keep)
                            PlaceQuads<true>(layout, set, em, scale, fx, fy, generation, outlineQ, moving, &out);
                        else
                            PlaceQuads<false>(layout, set, em, scale, fx, fy, generation, outlineQ, moving, &out);
                    }
                    else
                    {
                        PlaceQuads<true>(layout, set, em, scale, fx, fy, generation, outlineQ, moving);
                        EmitQuads(dl, set, ox, oy, rs, rgba, colorGlyphRgba);
                    }
                    if (index)
                    {
                        // kept for its class
                        if (set.classCount == kMaxClasses)
                        {
                            // drawn at more offsets than are kept: the classes start over
                            std::fill(set.classIndex.begin(), set.classIndex.end(), -1);
                            set.classCount = 0;
                        }
                        if (set.classes.size() <= set.classCount)
                            set.classes.emplace_back();
                        static_cast<Layout::Placed&>(set.classes[set.classCount]) = static_cast<const Layout::Placed&>(set);
                        *index = (std::int32_t)set.classCount++;
                    }
                    return;
                }
                set.misses = 0;
                EmitQuads(dl, set, ox, oy, rs, rgba, colorGlyphRgba);
            }

            // Smooth motion: how much of a draw is this rendering's - 1 at rest, 0 in motion, rising while it settles.
            // The k-th draw of a layout in a frame is taken for the k-th draw of it in the last frame.
            float RestWeight(Layout::Draws& draws, Vec2 pos, float scale)
            {
                if (draws.frame != frame_)
                {
                    draws.frame = frame_;
                    draws.count = 0;
                }
                const std::uint32_t k = draws.count++;
                if (draws.motion.size() <= k)
                    draws.motion.resize(k + 1);
                Layout::Motion& m = draws.motion[k];   // (ReplayDraws foresees this as it is done here)
                const bool moved = m.seen && (LengthSq(pos - m.pos) > 1e-8f || scale != m.scale);
                if (!m.seen)
                {
                    m.still = kRestFrames;   // first seen: at rest
                    m.since = params_.time - kRestSeconds;
                }
                else if (moved)
                {
                    m.still = 0;
                    m.since = params_.time;
                }
                else if (m.still < kRestFrames)
                    ++m.still;
                m.pos = pos;
                m.scale = scale;
                m.seen = true;
                const float t = params_.time >= 0.0 ? (float)((params_.time - m.since) / kRestSeconds) : (float)m.still / (float)kRestFrames;
                return moved ? 0.0f : SmoothStep(0.0f, 1.0f, t);
            }

            Vec2 DrawShapes(DrawList& dl, FontRef font, Vec2 pos, const fx::Instance& material, EffectId effect, float weight, std::string_view text,
                            float wrapWidth, std::uint32_t flags, float scale) override
            {
                Layout* layout = GetLayout(font, text, wrapWidth, flags);
                if (!layout)
                    return Vec2(0, 0);
                if (!(scale > 0.0f) || material.misc[0] <= 0.0f)
                    return layout->metrics.size;
                // the outer layers of every glyph first - a drop shadow, an outer glow, a stroke outside -, then the faces
                const std::uint32_t feat = material.flags[0];
                std::uint32_t under = 0;
                if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow))
                    under |= fx::kShadow;
                if (feat & fx::kGlow)
                    under |= fx::kGlow | (feat & fx::kHalo);
                if ((feat & fx::kStroke) && material.strokeParams[1] >= 1.0f)
                    under |= fx::kStroke;
                if (feat & fx::kLegible)
                    under |= fx::kLegible;   // its halo, under every face (in the tone the fill's color decides)
                const std::uint32_t faces = feat & ~under;
                if (under != 0)
                    under |= feat & (fx::kMask | fx::kMotion);   // masked and blurred as the faces are
                const float k = font.size * scale / kFieldEm;   // UI units per field pixel
                // the text's box: what paints, glass and shimmer span (a gradient across the text, not each glyph)
                const Vec2 box = layout->metrics.size * scale;
                constexpr std::uint32_t kDrawn = fx::kFill | fx::kStroke | fx::kShadow | fx::kGlow | fx::kInnerGlow | fx::kGlass | fx::kCustom | fx::kLegible;
                if (((under | faces) & kDrawn) != 0)
                    FieldSlots(*layout);
                for (const std::uint32_t pass : {under, faces})
                {
                    if ((pass & kDrawn) == 0)
                        continue;
                    for (std::size_t gi = 0; gi < layout->glyphs.size(); ++gi)
                    {
                        const GlyphSlot* s = fieldSlots_[gi];
                        if (!s || !s->page)
                            continue;
                        const PlacedGlyph& g = layout->glyphs[gi];
                        const Vec2 pen = pos + g.pos * scale;
                        fx::Instance inst = material;
                        inst.rect[0] = pos.x;
                        inst.rect[1] = pos.y;
                        inst.rect[2] = pos.x + box.x;
                        inst.rect[3] = pos.y + box.y;
                        inst.shape2[0] = pen.x + (float)s->left * k;
                        inst.shape2[1] = pen.y + (float)s->top * k;
                        inst.shape2[2] = inst.shape2[0] + (float)s->width * k;
                        inst.shape2[3] = inst.shape2[1] + (float)s->height * k;
                        inst.uvRect[0] = s->uv0.x;
                        inst.uvRect[1] = s->uv0.y;
                        inst.uvRect[2] = s->uv1.x;
                        inst.uvRect[3] = s->uv1.y;
                        inst.radii[0] = kFieldDesc.range;
                        inst.radii[1] = kFieldDesc.alphaRange;
                        inst.radii[2] = (pass & fx::kLegible) ? material.radii[2] : 0.0f;
                        inst.radii[3] = (pass & fx::kLegible) ? (material.radii[3] > 0.0f ? material.radii[3] : font.size * scale / 6.0f) : 0.0f;
                        inst.shape[2] = 1.0f / k;
                        inst.shape[3] = weight * scale;
                        inst.flags[0] = pass | fx::kGlyph;
                        inst.flags[2] = (std::uint32_t)fx::ShapeKind::Glyph;
                        dl.AddFx(inst, effect, s->page);
                    }
                }
                // glyphs without outlines (color glyphs, bitmap fonts): their bitmaps at the size, on whole pixels
                const Color fill(material.fill0[0], material.fill0[1], material.fill0[2], material.fill0[3] * material.misc[0]);
                const float rs = params_.pixelsPerUnit, inv = 1.0f / rs, emPixels = font.size * scale * rs;
                for (const PlacedGlyph& g : layout->glyphs)
                {
                    if (g.glyph == kNoGlyph || Outlined(g.face))
                        continue;
                    const float xi = FloorPx((pos.x + g.pos.x * scale) * rs + 0.5f), yi = FloorPx((pos.y + g.pos.y * scale) * rs + 0.5f);
                    const GlyphSlot* s = Glyph(g.face, g.glyph, emPixels, 0);
                    if (!s || !s->page || s->width <= 0 || s->height <= 0)
                        continue;
                    const Rect r((xi + (float)s->left) * inv, (yi + (float)s->top) * inv, (xi + (float)(s->left + s->width)) * inv,
                                 (yi + (float)(s->top + s->height)) * inv);
                    dl.AddQuad(s->page, r, s->uv0, s->uv1, (s->color ? Color::White(fill.a) : fill).ToRgba8());
                }
                return layout->metrics.size;
            }

            // A face's glyphs have outlines for distance fields (not color glyphs, not bitmap fonts).
            bool Outlined(std::uint16_t face) const
            {
                const Face& f = *faces_[face];
                return !f.bitmap && !f.colorStrikes && !f.colorLayers;
            }

            // The offsets where PlaceQuads puts a glyph of `layout` elsewhere (QuadSet::xBreaks, yBreaks) for these em,
            // scale, density and atlases - made again, the placements dropped, when one of them changed. A pen goes to
            // the nearest quarter pixel (to the next one at fx + pen = n + 1/8, 3/8 ...), a whole-pixel pen and a line
            // to the nearest pixel (at n + 1/2); in motion an outline's line to the nearest quarter pixel too.
            void ClassBreaks(const Layout& layout, Layout::QuadSet& set, float em, float scale, std::uint64_t generation, std::uint32_t outlineQ, bool moving)
            {
                const float rs = params_.pixelsPerUnit;
                if (set.classGeneration == generation && set.classEm == em && set.classScale == scale && set.classRs == rs && set.classOutline == outlineQ)
                    return;
                set.classGeneration = generation;
                set.classEm = em;
                set.classScale = scale;
                set.classRs = rs;
                set.classOutline = outlineQ;
                set.xBreaks.clear();
                set.yBreaks.clear();
                // the pens' and lines' offsets as PlaceQuads computes them (fx + g.pos.x * scale * rs, in floats)
                const auto at = [](double edge, float pen) {
                    const double v = edge - (double)pen;
                    return (float)(v - std::floor(v));
                };
                for (const PlacedGlyph& g : layout.glyphs)
                {
                    const bool smooth = moving && Outlined(g.face);
                    const float px = g.pos.x * scale * rs, py = g.pos.y * scale * rs;
                    if ((layout.bitmapGlyphs && faces_[g.face]->bitmap) || (wholePixelPens_ && !smooth))
                        set.xBreaks.push_back(at(0.5, px));
                    else
                        for (int k = 1; k < 8; k += 2)
                            set.xBreaks.push_back(at(k * 0.125, px));
                    if (smooth)
                        for (int k = 1; k < 8; k += 2)
                            set.yBreaks.push_back(at(k * 0.125, py));
                    else
                        set.yBreaks.push_back(at(0.5, py));
                }
                for (std::vector<float>* b : {&set.xBreaks, &set.yBreaks})
                {
                    std::sort(b->begin(), b->end());
                    b->erase(std::unique(b->begin(), b->end()), b->end());
                }
                set.classIndex.assign((set.xBreaks.size() + 1) * (set.yBreaks.size() + 1), -1);
                set.classCount = 0;
            }

            // Every glyph's distance field (MTSDF at kFieldEm pixels per em, the design's outline) into fieldSlots_, by the
            // glyph's index in the layout (null: none, a glyph without an outline); the missing ones made first.
            void FieldSlots(const Layout& layout)
            {
                fieldSlots_.assign(layout.glyphs.size(), nullptr);
                fieldMissing_.clear();
                for (std::size_t i = 0; i < layout.glyphs.size(); ++i)
                {
                    const PlacedGlyph& g = layout.glyphs[i];
                    if (g.glyph == kNoGlyph || !Outlined(g.face))
                        continue;
                    if (!(fieldSlots_[i] = fieldAtlas_.Find(FieldKey(g.face, g.glyph))))
                        fieldMissing_.push_back(i);
                }
                if (fieldMissing_.empty())
                    return;
                // The fields of the text's new glyphs, made together (a text that appears with many: the hitch of its
                // frame): their outlines read here (FreeType's faces belong to this thread), handed to the workers as
                // they come, made by them and this thread at once; then packed in the order the passes meet them, as
                // when each was made at its first draw.
                for (const std::size_t i : fieldMissing_)
                {
                    const PlacedGlyph& g = layout.glyphs[i];
                    const std::uint64_t key = FieldKey(g.face, g.glyph);
                    if (!fieldBatchKeys_.insert(key).second)
                        continue;   // made for an earlier one
                    GlyphShape shape;
                    FieldShape(g.face, g.glyph, shape);
                    fieldBatch_.push_back({key, shape.Bounds()});
                    fields_.Add(std::move(shape), kFieldDesc);
                }
                fields_.Make();
                for (std::size_t b = 0; b < fieldBatch_.size(); ++b)
                    AddField(fieldBatch_[b].key, fields_.Field(b), fieldBatch_[b].ink);
                fieldBatch_.clear();
                fieldBatchKeys_.clear();
                for (const std::size_t i : fieldMissing_)
                    fieldSlots_[i] = fieldAtlas_.Find(FieldKey(layout.glyphs[i].face, layout.glyphs[i].glyph));
            }

            static std::uint64_t FieldKey(std::uint16_t face, std::uint16_t glyph) { return ((std::uint64_t)face << 16) | glyph; }

            // The glyph's design outline in field pixels (kFieldEm per em), y down: what its field is made from.
            void FieldShape(std::uint16_t face, std::uint16_t glyph, GlyphShape& shape)
            {
                Face& f = *faces_[face];
                shape.Clear();
                if (f.platform)
                {
                    // the platform's outline is flattened already: its points joined by lines
                    outline_.Clear();
                    f.platform->GlyphOutline(glyph, kFieldEm / f.upem, outline_);
                    for (std::size_t c = 0; c < outline_.ContourCount(); ++c)
                    {
                        const std::span<const Vec2> pts = outline_.Contour(c);
                        if (pts.empty())
                            continue;
                        shape.MoveTo(pts[0]);
                        for (std::size_t i = 1; i < pts.size(); ++i)
                            shape.LineTo(pts[i]);
                    }
                }
                else if (FT_Load_Glyph(f.ft, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) == 0 && f.ft->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    FT_Outline_Funcs funcs{};
                    funcs.move_to = &ShapeMoveTo;
                    funcs.line_to = &ShapeLineTo;
                    funcs.conic_to = &ShapeConicTo;
                    funcs.cubic_to = &ShapeCubicTo;
                    ShapeSink sink{&shape, kFieldEm / f.upem};
                    FT_Outline_Decompose(&f.ft->glyph->outline, &funcs, &sink);
                }
            }

            // A field made (null: GenerateMtsdf refused it) into the field atlas under `key`
            void AddField(std::uint64_t key, const GlyphBitmap* field, const Rect& ink)
            {
                GlyphBitmap none;   // larger than a field may be, or than a page: cached as empty
                none.channels = 4;
                none.sdf = true;
                if (!fieldAtlas_.Add(key, field ? *field : none, ink))
                    fieldAtlas_.Add(key, none, Rect());
            }

            void DrawOutline(DrawList& dl, FontRef font, Vec2 pos, const TextOutline& outline, std::string_view text, float wrapWidth, std::uint32_t flags,
                             float scale) override
            {
                if (!outline.Visible() || !(scale > 0.0f))
                    return;
                Layout* layout = GetLayout(font, text, wrapWidth, flags);
                if (!layout)
                    return;
                // the placement Draw uses (the same em, origin and sub-pixel offsets, the same motion), the glyphs grown
                const std::uint32_t width = OutlineQ(outline.width * scale * params_.pixelsPerUnit);
                const bool snap = (flags & TextFlags_PixelSnap) != 0;
                const float rest = rendering_.smoothMotion && !snap ? RestWeight(layout->outlineDraws, pos, scale) : 1.0f;
                if (notes_ && rendering_.smoothMotion && !snap)
                    notes_->push_back({layout->metrics.layout, pos, scale, rest, 1});
                if (rest < 1.0f)
                    DrawQuads(dl, *layout, layout->movingOutline, font, pos, outline.color.WithAlpha(outline.color.a * (1.0f - rest)), scale, width, true,
                              false);
                if (rest > 0.0f)
                    DrawQuads(dl, *layout, layout->outline, font, pos, outline.color.WithAlpha(outline.color.a * rest), scale, width, false, snap);
            }

            // An outline's width in physical pixels as OutlineGlyph keys it: 1/8 px, at most 255 (31.875 px), 1 for any
            // width above 0
            static std::uint32_t OutlineQ(float pixels) { return pixels > 0.0f ? (std::uint32_t)std::clamp(pixels * 8.0f + 0.5f, 1.0f, 255.0f) : 0u; }

            // A glyph's slot for a quad: its bitmap at this em size and pen phase - an outline's set (outlineQ > 0): grown
            // by that width (OutlineGlyph), none for a color glyph. kNoInk when there is nothing to draw.
            const GlyphSlot* QuadSlot(const PlacedGlyph& g, float em, int phase, int yPhase, std::uint32_t outlineQ)
            {
                static const GlyphSlot kNoInk{};
                const GlyphSlot* s = Glyph(g.face, g.glyph, em, phase, yPhase);
                if (outlineQ > 0)
                {
                    if (!s || s->color)
                        return &kNoInk;   // a color glyph (emoji) has no outline
                    s = OutlineGlyph(g.face, g.glyph, em, phase, outlineQ, yPhase);
                }
                return s ? s : &kNoInk;
            }

            // Where PlaceQuads also writes the quads as it places them: `dl` (all of them in its clip), at the whole pixel
            // (ox, oy) in UI units (inv units per pixel), in EmitQuads' colors
            struct QuadOut
            {
                DrawList* dl;
                float ox, oy, inv;
                std::uint32_t rgba, colorRgba;
            };

            // The layout's quads for these sub-pixel offsets (Draw): physical-pixel placement, the baseline on a whole
            // pixel, the pen at a quarter-pixel phase; glyphs without ink left out (QuadSlot). Colorless: EmitQuads
            // colors them - or, with `out`, they go into its list too. Keep false (with `out` only): into the list
            // alone, the set left as it was (its glyph slots aside).
            template <bool Keep>
            void PlaceQuads(Layout& layout, Layout::QuadSet& set, float em, float scale, float fx, float fy, std::uint64_t generation, std::uint32_t outlineQ,
                            bool moving = false, const QuadOut* out = nullptr)
            {
                const float rs = params_.pixelsPerUnit;
                if constexpr (Keep)
                {
                    set.quads.clear();
                    set.quads.reserve(layout.glyphs.size());
                    set.runs.clear();
                    set.rows.clear();
                    set.bounds = Rect(1e30f, 1e30f, -1e30f, -1e30f);
                }
                float ink[2] = {}, colored[2] = {};
                DrawList::QuadWriter w;
                TextureId writing = 0;   // the page of `w`, 0: none open
                if (out)
                {
                    QuadInstance::PackColor(out->rgba, ink);
                    QuadInstance::PackColor(out->colorRgba, colored);
                }
                if (set.slotGeneration != generation || set.slotEm != em || set.slotOutline != outlineQ ||
                    (moving ? set.movingSlots.size() : set.slots.size()) != layout.glyphs.size())
                {
                    if (moving)
                        set.movingSlots.assign(layout.glyphs.size(), std::array<const GlyphSlot*, 20>{});
                    else
                        set.slots.assign(layout.glyphs.size(), std::array<const GlyphSlot*, 4>{});
                    set.slotGeneration = generation;
                    set.slotEm = em;
                    set.slotOutline = outlineQ;
                }
                [[maybe_unused]] float rowBaseline = std::numeric_limits<float>::quiet_NaN();
                for (std::size_t gi = 0; gi < layout.glyphs.size(); ++gi)
                {
                    const PlacedGlyph& g = layout.glyphs[gi];
                    const float px = fx + g.pos.x * scale * rs, py = fy + g.pos.y * scale * rs;
                    float xi = FloorPx(px);
                    int phase = (int)((px - xi) * 4.0f + 0.5f);   // 0.5 .. 4.5: truncation is the floor
                    if (phase == 4)
                    {
                        phase = 0;
                        xi += 1.0f;
                    }
                    // in motion an outline's glyph moves by quarter pixels down too (from the design's outline)
                    const bool smooth = moving && Outlined(g.face);
                    if ((layout.bitmapGlyphs && faces_[g.face]->bitmap) || (wholePixelPens_ && !smooth))
                    {
                        // a bitmap font's pixels on the target's, or outlines hinted in both directions for a pen on a
                        // whole pixel: whole pixels, no phases
                        xi = FloorPx(px + 0.5f);
                        phase = 0;
                    }
                    float yi = FloorPx(py + 0.5f);
                    int yPhase = -1;
                    if (smooth)
                    {
                        yi = FloorPx(py);
                        yPhase = (int)((py - yi) * 4.0f + 0.5f);
                        if (yPhase == 4)
                        {
                            yPhase = 0;
                            yi += 1.0f;
                        }
                    }
                    const GlyphSlot*& known = moving ? set.movingSlots[gi][(std::size_t)(phase + 4 * (yPhase + 1))] : set.slots[gi][(std::size_t)phase];
                    if (!known)
                        known = QuadSlot(g, em, phase, yPhase, outlineQ);
                    const GlyphSlot* s = known;
                    if (!s->page || s->width <= 0 || s->height <= 0)
                        continue;
                    const Rect r(xi + (float)s->left, yi + (float)s->top, xi + (float)(s->left + s->width), yi + (float)(s->top + s->height));
                    if constexpr (Keep)
                    {
                        if (set.runs.empty() || set.runs.back().page != s->page)
                            set.runs.push_back({s->page, (std::uint32_t)set.quads.size(), 0});
                        ++set.runs.back().count;
                        if (set.rows.empty() || g.pos.y != rowBaseline)
                        {
                            set.rows.push_back({(std::uint32_t)set.quads.size(), 0});
                            rowBaseline = g.pos.y;
                        }
                        QuadInstance& q = set.quads.emplace_back();
                        q.rect[0] = r.min.x;
                        q.rect[1] = r.min.y;
                        q.rect[2] = r.max.x;
                        q.rect[3] = r.max.y;
                        q.packed[0] = s->quadUv[0];
                        q.packed[1] = s->quadUv[1];
                        q.packed[2] = 0.0f;
                        q.packed[3] = s->color ? 1.0f : 0.0f;
                        set.bounds = set.bounds.Union(r);
                        set.runs.back().bounds = set.runs.back().bounds.Union(r);
                        Layout::QuadRow& row = set.rows.back();
                        ++row.count;
                        row.y0 = std::min(row.y0, r.min.y);
                        row.y1 = std::max(row.y1, r.max.y);
                    }
                    if (out)
                    {
                        if (writing != s->page)
                        {
                            if (writing)
                                out->dl->EndQuads(w);
                            w = out->dl->BeginQuads(s->page, (std::uint32_t)(layout.glyphs.size() - gi));   // (at most the glyphs left)
                            writing = s->page;
                        }
                        QuadInstance& d = w.quad[w.written++];
                        const Rect u((r.min.x + out->ox) * out->inv, (r.min.y + out->oy) * out->inv, (r.max.x + out->ox) * out->inv,
                                     (r.max.y + out->oy) * out->inv);
                        d.rect[0] = u.min.x;
                        d.rect[1] = u.min.y;
                        d.rect[2] = u.max.x;
                        d.rect[3] = u.max.y;
                        const float* c = s->color ? colored : ink;
                        d.packed[0] = s->quadUv[0];
                        d.packed[1] = s->quadUv[1];
                        d.packed[2] = c[0];
                        d.packed[3] = c[1];
                        w.bounds = w.bounds.Union(u);
                    }
                }
                if (writing)
                    out->dl->EndQuads(w);
                if constexpr (Keep)
                {
                    // the rows' extents made monotonic (a deep descender may reach below the next line's): each row's
                    // bottom the lowest of it and the rows above, its top the highest of it and the rows below - a
                    // search for the rows a clip holds then never misses one
                    for (std::size_t k = 1; k < set.rows.size(); ++k)
                        set.rows[k].y1 = std::max(set.rows[k].y1, set.rows[k - 1].y1);
                    for (std::size_t k = set.rows.size(); k-- > 1;)
                        set.rows[k - 1].y0 = std::min(set.rows[k - 1].y0, set.rows[k].y0);
                    set.generation = generation;
                    set.em = em;
                    set.scale = scale;
                    set.fx = fx;
                    set.fy = fy;
                    set.outline = outlineQ;
                }
            }

            // The quads at the whole pixel (ox, oy), in UI units (rs pixels per unit), in `rgba` - a color glyph's in
            // `colorRgba`; the ones outside the clip left out (none to test when the text lies inside it).
            static void EmitQuads(DrawList& dl, const Layout::Placed& set, float ox, float oy, float rs, std::uint32_t rgba, std::uint32_t colorRgba)
            {
                const float inv = 1.0f / rs;
                const Rect clip = dl.ClipRect();
                const Rect all((set.bounds.min.x + ox) * inv, (set.bounds.min.y + oy) * inv, (set.bounds.max.x + ox) * inv, (set.bounds.max.y + oy) * inv);
                if (set.runs.empty() || !all.Overlaps(clip))
                    return;
                float ink[2], colored[2];
                QuadInstance::PackColor(rgba, ink);
                QuadInstance::PackColor(colorRgba, colored);
                if (clip.Contains(all))
                {
                    // all in view: the quads go in as a block, through local cursors (stores through the writer's
                    // members kept the compiler from keeping them in registers), with the bounds of their run
                    for (const Layout::QuadRun& run : set.runs)
                    {
                        DrawList::QuadWriter w = dl.BeginQuads(run.page, run.count);
                        const QuadInstance* q = set.quads.data() + run.first;
                        QuadInstance* d = w.quad;
                        for (std::uint32_t i = 0; i < run.count; ++i, ++q, ++d)
                        {
                            d->rect[0] = (q->rect[0] + ox) * inv;
                            d->rect[1] = (q->rect[1] + oy) * inv;
                            d->rect[2] = (q->rect[2] + ox) * inv;
                            d->rect[3] = (q->rect[3] + oy) * inv;
                            const float* c = q->packed[3] != 0.0f ? colored : ink;
                            d->packed[0] = q->packed[0];
                            d->packed[1] = q->packed[1];
                            d->packed[2] = c[0];
                            d->packed[3] = c[1];
                        }
                        w.written = run.count;
                        w.bounds = Rect((run.bounds.min.x + ox) * inv, (run.bounds.min.y + oy) * inv, (run.bounds.max.x + ox) * inv,
                                        (run.bounds.max.y + oy) * inv);
                        dl.EndQuads(w);
                    }
                    return;
                }
                // a clip cuts it: only the lines it holds are looked at (a long text in a small view), and room is
                // reserved for their quads only
                const float cy0 = clip.min.y * rs - oy, cy1 = clip.max.y * rs - oy;   // the clip's rows in the set's pixels
                const auto firstRow = std::partition_point(set.rows.begin(), set.rows.end(), [cy0](const Layout::QuadRow& row) { return row.y1 <= cy0; });
                const auto endRow = std::partition_point(firstRow, set.rows.end(), [cy1](const Layout::QuadRow& row) { return row.y0 < cy1; });
                if (firstRow == endRow)
                    return;
                const std::uint32_t qa = firstRow->first, qb = std::prev(endRow)->first + std::prev(endRow)->count;
                for (const Layout::QuadRun& run : set.runs)
                {
                    const std::uint32_t i0 = std::max(run.first, qa), i1 = std::min(run.first + run.count, qb);
                    if (i0 >= i1)
                        continue;
                    DrawList::QuadWriter w;
                    bool open = false;
                    const QuadInstance* q = set.quads.data() + i0;
                    for (std::uint32_t i = i0; i < i1; ++i, ++q)
                    {
                        const Rect r((q->rect[0] + ox) * inv, (q->rect[1] + oy) * inv, (q->rect[2] + ox) * inv, (q->rect[3] + oy) * inv);
                        if (!r.Overlaps(clip))
                            continue;
                        if (!open)   // a run opens at its first quad in view (no empty command when none is)
                        {
                            w = dl.BeginQuads(run.page, i1 - i);
                            open = true;
                        }
                        QuadInstance& d = w.quad[w.written++];
                        d.rect[0] = r.min.x;
                        d.rect[1] = r.min.y;
                        d.rect[2] = r.max.x;
                        d.rect[3] = r.max.y;
                        const float* c = q->packed[3] != 0.0f ? colored : ink;
                        d.packed[0] = q->packed[0];
                        d.packed[1] = q->packed[1];
                        d.packed[2] = c[0];
                        d.packed[3] = c[1];
                        w.bounds = w.bounds.Union(r);
                    }
                    if (open)
                        dl.EndQuads(w);
                }
            }

            void DrawGlyphOutline(DrawList& dl, FontRef font, char32_t codepoint, Vec2 center, const TextOutline& outline) override
            {
                if (font.id == 0 || font.id > faces_.size() || !(font.size > 0.0f) || !outline.Visible())
                    return;
                const std::uint16_t face = PickFace((std::uint16_t)(font.id - 1), codepoint, -1);
                hb_codepoint_t glyph = 0;
                if (!hb_font_get_nominal_glyph(faces_[face]->hb, codepoint, &glyph))
                    return;
                const float rs = params_.pixelsPerUnit, inv = 1.0f / rs;
                const float em = std::floor(font.size * rs * 4.0f + 0.5f) * 0.25f;
                const GlyphSlot* s = Glyph(face, (std::uint16_t)glyph, em, 0);
                if (!s || !s->page || s->color)
                    return;   // nothing drawn, or a color glyph: no outline
                // placed as DrawGlyph places the glyph (the glyph's ink box on `center`), grown around it
                const float ox = std::floor(center.x * rs - (s->ink.min.x + s->ink.max.x) * 0.5f + 0.5f);
                const float oy = std::floor(center.y * rs - (s->ink.min.y + s->ink.max.y) * 0.5f + 0.5f);
                const GlyphSlot* o = OutlineGlyph(face, (std::uint16_t)glyph, em, 0, OutlineQ(outline.width * rs));
                if (!o || !o->page || o->width <= 0 || o->height <= 0)
                    return;
                dl.AddQuad(o->page,
                           Rect((ox + (float)o->left) * inv, (oy + (float)o->top) * inv, (ox + (float)(o->left + o->width)) * inv,
                                (oy + (float)(o->top + o->height)) * inv),
                           o->uv0, o->uv1, outline.color.ToRgba8());
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
                dl.AddQuad(s->page,
                           Rect((ox + (float)s->left) * inv, (oy + (float)s->top) * inv, (ox + (float)(s->left + s->width)) * inv,
                                (oy + (float)(s->top + s->height)) * inv),
                           s->uv0, s->uv1, s->color ? Color::White(color.a).ToRgba8() : color.ToRgba8());
            }

        private:
            // The path is UTF-8 on every platform (HarfBuzz converts it for Windows); mapped where the platform can.
            static std::shared_ptr<const MappedFile> MapFile(const char* path)
            {
                hb_blob_t* blob = hb_blob_create_from_file_or_fail(path);
                if (!blob)
                    return nullptr;
                // not make_shared: with mingw-w64's libstdc++ it duplicates std::type_info::operator== at link time
                std::shared_ptr<const MappedFile> file(new MappedFile(blob));
                if (hb_blob_get_length(blob) == 0)
                    return nullptr;
                return file;
            }

            FontId AddFace(std::shared_ptr<const MappedFile> file, const void* data, std::size_t size, int faceIndex)
            {
                if (faces_.size() >= kNoGlyph || faceIndex < 0)
                    return 0;
                auto face = std::make_unique<Face>();
                face->file = std::move(file);
                if (face->file)
                {
                    unsigned length = 0;
                    data = hb_blob_get_data(face->file->blob, &length);
                    size = length;
                }
                if (!data || size == 0 || FT_New_Memory_Face(ft_, static_cast<const FT_Byte*>(data), (FT_Long)size, faceIndex, &face->ft) != 0)
                {
                    face->ft = nullptr;
                    return 0;
                }
                face->colorStrikes = FT_HAS_COLOR(face->ft) && FT_HAS_FIXED_SIZES(face->ft);
                if (!FT_IS_SCALABLE(face->ft) && !face->colorStrikes)
                {
                    // a bitmap font: no outlines, and no OpenType tables to shape with - HarfBuzz reads its characters,
                    // advances and metrics from FreeType at the strike in use (BitmapFontFuncs)
                    if (!FT_HAS_FIXED_SIZES(face->ft) || face->ft->num_fixed_sizes <= 0)
                        return 0;
                    face->bitmap = true;
                    AddStrikes(*face, face->ft);
                    if (faceIndex == 0)
                        for (FT_Long i = 1; i < face->ft->num_faces; ++i)
                        {
                            // a .fon's other sizes of the font
                            FT_Face more = nullptr;
                            if (FT_New_Memory_Face(ft_, static_cast<const FT_Byte*>(data), (FT_Long)size, i, &more) != 0)
                                continue;
                            const auto same = [](const char* a, const char* b) { return (!a && !b) || (a && b && std::strcmp(a, b) == 0); };
                            if (!FT_IS_SCALABLE(more) && FT_HAS_FIXED_SIZES(more) && same(more->family_name, face->ft->family_name) &&
                                same(more->style_name, face->ft->style_name))
                            {
                                face->moreFaces.push_back(more);
                                AddStrikes(*face, more);
                            }
                            else
                                FT_Done_Face(more);
                        }
                    // a face of its own without tables (HarfBuzz's empty face is inert: nothing is shaped with it)
                    hb_face_t* hbFace = hb_face_create_for_tables([](hb_face_t*, hb_tag_t, void*) -> hb_blob_t* { return nullptr; }, nullptr, nullptr);
                    hb_face_set_glyph_count(hbFace, (unsigned)std::max<FT_Long>(face->ft->num_glyphs, 1));
                    face->hb = hb_font_create(hbFace);
                    hb_face_destroy(hbFace);
                    hb_font_set_funcs(face->hb, BitmapFontFuncs(), face.get(), nullptr);
                    Face& f = *face;
                    faces_.push_back(std::move(face));
                    ++bitmapFaces_;
                    SelectStrike(f, 0);
                    return (FontId)faces_.size();
                }
                face->colorLayers = !face->colorStrikes && detail::HasColrGlyphs(face->ft);
                hb_blob_t* blob = face->file ? hb_blob_reference(face->file->blob)
                                             : hb_blob_create(static_cast<const char*>(data), (unsigned)size, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
                hb_face_t* hbFace = hb_face_create(blob, (unsigned)(faceIndex & 0xFFFF));   // the face of a collection
                hb_blob_destroy(blob);
                face->hb = hb_font_create(hbFace);
                hb_face_destroy(hbFace);
                // A variable font's named instance (faceIndex >> 16, FreeType's convention): shaped with the coordinates
                // FreeType draws with, read from FreeType - HarfBuzz reads the instance from the index only from 3.3 on,
                // and before that took the index for a face of a collection (none: an empty face)
                if ((faceIndex >> 16) != 0 && FT_HAS_MULTIPLE_MASTERS(face->ft))
                {
                    FT_MM_Var* mm = nullptr;
                    if (FT_Get_MM_Var(face->ft, &mm) == 0)
                    {
                        std::vector<FT_Fixed> fixed(mm->num_axis);
                        if (mm->num_axis > 0 && FT_Get_Var_Design_Coordinates(face->ft, mm->num_axis, fixed.data()) == 0)
                        {
                            std::vector<float> coords(mm->num_axis);
                            for (FT_UInt i = 0; i < mm->num_axis; ++i)
                                coords[i] = (float)fixed[i] / 65536.0f;
                            hb_font_set_var_coords_design(face->hb, coords.data(), mm->num_axis);
                        }
                        FT_Done_MM_Var(ft_, mm);
                    }
                }
                // FreeType leaves units_per_EM 0 for fonts without outlines (CBDT emoji): HarfBuzz reads it from head
                face->upem = (float)(face->ft->units_per_EM ? face->ft->units_per_EM : std::max(hb_face_get_upem(hb_font_get_face(face->hb)), 1u));
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

            // A font the platform draws: HarfBuzz reads its tables through the platform, metrics as for a file.
            FontId AddPlatformFace(std::unique_ptr<detail::PlatformFace> platform)
            {
                if (faces_.size() >= kNoGlyph)
                    return 0;
                auto face = std::make_unique<Face>();
                hb_face_t* hbFace = platform->CreateHbFace();
                face->upem = (float)std::max(hb_face_get_upem(hbFace), 1u);
                face->codePages = CodePages(hbFace);
                face->hb = hb_font_create(hbFace);
                hb_face_destroy(hbFace);
                if (hb_face_get_glyph_count(hb_font_get_face(face->hb)) == 0)
                    return 0;
                platform->SetVariations(face->hb);
                face->platform = std::move(platform);
                hb_font_set_scale(face->hb, (int)face->upem, (int)face->upem);
                hb_font_extents_t extents{};
                hb_font_get_h_extents(face->hb, &extents);
                face->ascent = (float)extents.ascender;
                face->descent = (float)-extents.descender;
                face->gap = (float)extents.line_gap;
                faces_.push_back(std::move(face));
                return (FontId)faces_.size();
            }

            // OS/2 ulCodePageRange1 (version 1 and later), from HarfBuzz's copy of the table
            static std::uint32_t CodePages(hb_face_t* face)
            {
                hb_blob_t* os2 = hb_face_reference_table(face, HB_TAG('O', 'S', '/', '2'));
                unsigned length = 0;
                const auto* d = reinterpret_cast<const std::uint8_t*>(hb_blob_get_data(os2, &length));
                std::uint32_t pages = 0;
                if (d && length >= 82 && ((d[0] << 8) | d[1]) >= 1)
                    pages = ((std::uint32_t)d[78] << 24) | ((std::uint32_t)d[79] << 16) | ((std::uint32_t)d[80] << 8) | (std::uint32_t)d[81];
                hb_blob_destroy(os2);
                return pages;
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

            // A text's key: its parameters and its bytes, 8 at a time (FNV-1a went byte by byte)
            // The part of a layout's key from its parameters: the same for every text of a font, size and wrap width
            // (GetLayout keeps the last one)
            static std::uint64_t ParamsKey(const std::uint32_t (&params)[5])
            {
                std::uint64_t h = 0x9E3779B97F4A7C15ull;
                const auto mix = [&h](std::uint64_t v) {
                    h ^= v * 0xBF58476D1CE4E5B9ull;
                    h = (h ^ (h >> 29)) * 0x94D049BB133111EBull;
                };
                mix(((std::uint64_t)params[0] << 32) | params[1]);
                mix(((std::uint64_t)params[2] << 32) | params[3]);
                mix(params[4]);
                return h;
            }

            // ... and the text's: a multiply per 8 bytes, the mixing at the end
            static std::uint64_t LayoutKey(std::uint64_t h, std::string_view text)
            {
                h ^= (std::uint64_t)text.size() * 0x9E3779B97F4A7C15ull;
                const char* p = text.data();
                std::size_t n = text.size();
                for (; n >= 8; p += 8, n -= 8)
                {
                    std::uint64_t v;
                    std::memcpy(&v, p, 8);
                    h = (h ^ v) * 0xBF58476D1CE4E5B9ull;
                    h ^= h >> 29;
                }
                if (n > 0)
                {
                    std::uint64_t v = 0;
                    std::memcpy(&v, p, n);
                    h = (h ^ v) * 0xBF58476D1CE4E5B9ull;
                }
                h = (h ^ (h >> 32)) * 0x94D049BB133111EBull;
                return h ^ (h >> 29);
            }

            Layout* GetLayout(FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags)
            {
                if (font.id == 0 || font.id > faces_.size() || !(font.size > 0.0f))
                    return nullptr;
                const std::uint32_t params[5] = {font.id, std::bit_cast<std::uint32_t>(font.size), std::bit_cast<std::uint32_t>(wrapWidth), flags,
                                                 HintedAdvances() ? std::bit_cast<std::uint32_t>(params_.pixelsPerUnit) : 0u};
                // a widget measures a text and then draws it: the same layout again, found without hashing
                if (lastLayout_ && lastLayout_->text == text && std::memcmp(lastLayout_->params, params, sizeof(params)) == 0)
                {
                    lastLayout_->lastFrame = frame_;
                    return lastLayout_;
                }
                if (std::memcmp(lastParams_, params, sizeof(params)) != 0)
                {
                    std::memcpy(lastParams_, params, sizeof(params));
                    lastParamsKey_ = ParamsKey(params);
                }
                const std::uint64_t key = LayoutKey(lastParamsKey_, text);
                // the front: a layout found here without the map's buckets (most texts of a frame)
                Layout*& front = layoutFront_[(std::size_t)(key ^ (key >> 32)) & (kLayoutFront - 1)];
                if (front && front->text == text && std::memcmp(front->params, params, sizeof(params)) == 0)
                {
                    front->lastFrame = frame_;
                    lastLayout_ = front;
                    return front;
                }
                auto it = layouts_.find(key);
                if (it == layouts_.end())
                {
                    if (!spareLayouts_.empty())
                    {
                        // an evicted layout takes the new text: its strings and glyphs keep their storage
                        auto node = std::move(spareLayouts_.back());
                        spareLayouts_.pop_back();
                        node.key() = key;
                        node.mapped().text.clear();
                        it = layouts_.insert(std::move(node)).position;
                    }
                    else
                        it = layouts_.try_emplace(key).first;
                }
                Layout& layout = it->second;
                if (layout.text != text || std::memcmp(layout.params, params, sizeof(params)) != 0)
                {
                    // new, or a hash collision: (re)build in place
                    layout.text.assign(text);
                    std::memcpy(layout.params, params, sizeof(params));
                    layout.glyphs.clear();
                    layout.glyphs.reserve(text.size() + 3);   // at most a glyph per byte (and an ellipsis): no regrowth
                    // other glyphs: placed again (the moving sets too - their placements were the old text's)
                    for (Layout::QuadSet* s : {&layout.fill, &layout.outline, &layout.moving, &layout.movingOutline})
                    {
                        s->generation = s->slotGeneration = s->classGeneration = 0;
                        s->classScore = 0;
                    }
                    Forget(layout);   // (before BuildLayout, which sets its metrics)
                    BuildLayout(font, text, wrapWidth, flags, layout);
                    layout.metrics.layout = ++layoutSerial_;
                    bySerial_[layout.metrics.layout & (kSerialSlots - 1)] = {layout.metrics.layout, &layout};
                }
                layout.lastFrame = frame_;
                lastLayout_ = &layout;
                front = &layout;
                return &layout;
            }

            void BuildLayout(FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags, Layout& layout)
            {
                const std::uint16_t primary = (std::uint16_t)(font.id - 1);
                const float primarySize = Snapped(primary, font.size);   // a bitmap font: its strike's
                const Face& pf = *faces_[primary];
                const float k = primarySize / pf.upem;
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
                layout.bitmapGlyphs = std::any_of(layout.glyphs.begin(), layout.glyphs.end(),
                                                  [this](const PlacedGlyph& g) { return g.glyph != kNoGlyph && faces_[g.face]->bitmap; });
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
                    const float runSize = Snapped(run.face, size);   // before shaping: a bitmap font's strike
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
                    const float k = runSize / face.upem;
                    // hinted advances: the glyph's hinted width at this size and density on whole pixels, plus what the
                    // shaping added (kerning, marks), rounded to whole pixels too
                    const bool hinted = HintedAdvances() && face.ft && !face.platform && !face.bitmap && !face.colorStrikes;
                    const float ppu = params_.pixelsPerUnit, ppem = runSize * ppu;
                    const std::size_t first = glyphs_.size();
                    for (unsigned i = 0; i < n; ++i)
                    {
                        RunGlyph g;
                        g.face = run.face;
                        g.glyph = (std::uint16_t)info[i].codepoint;
                        g.advance = (float)pos[i].x_advance * k;
                        g.offset = Vec2((float)pos[i].x_offset * k, -(float)pos[i].y_offset * k);
                        if (hinted)
                        {
                            const float linear = (float)hb_font_get_glyph_h_advance(face.hb, info[i].codepoint);
                            const float shaped = ((float)pos[i].x_advance - linear) * ppem / face.upem;
                            g.advance = ((float)HintedAdvance(run.face, g.glyph, ppem) + std::round(shaped)) / ppu;
                            g.offset = Vec2(std::round(g.offset.x * ppu) / ppu, std::round(g.offset.y * ppu) / ppu);
                        }
                        g.cluster = info[i].cluster;
                        g.run = r;
                        const unsigned char b = (unsigned char)para[g.cluster];
                        if (IsControl(b))
                        {
                            // control characters draw nothing; a tab advances like four spaces
                            g.glyph = kNoGlyph;
                            g.offset = Vec2();
                            g.advance = b == '\t' ? 4.0f * face.SpaceAdvance() * k : 0.0f;
                            if (hinted)
                                g.advance = std::round(g.advance * ppu) / ppu;
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
                const float faceSize = Snapped(face, size);
                const Face& f = *faces_[face];
                hb_buffer_clear_contents(buffer_);
                hb_buffer_add_utf8(buffer_, single ? "\xE2\x80\xA6" : "...", -1, 0, -1);
                hb_buffer_guess_segment_properties(buffer_);
                hb_shape(f.hb, buffer_, nullptr, 0);
                unsigned n = 0;
                const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer_, &n);
                const hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer_, nullptr);
                const float k = faceSize / f.upem;
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
                // a box exactly as wide as the measured text arrives through a rectangle ((x + w) - x): float rounding
                // can take a fraction of a pixel off, which must not cost the text its last letters
                constexpr float kSlack = 1.0f / 64.0f;
                if (ink <= maxWidth + kSlack)
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
            // A glyph at `emPixels`, its pen `phase` quarter pixels right; yPhase >= 0: a glyph in motion (smooth motion),
            // the design's outline `yPhase` quarter pixels down too - its key has bit 30 set and the phase in bits 27 .. 28
            // (its em capped to bits 2 .. 26).
            const GlyphSlot* Glyph(std::uint16_t face, std::uint16_t glyph, float emPixels, int phase, int yPhase = -1)
            {
                if (faces_[face]->bitmap)
                    return BitmapGlyph(face, glyph, emPixels, 0);
                const bool moving = yPhase >= 0;
                const std::uint64_t q = (std::uint64_t)std::min(emPixels * 16.0f + 0.5f, moving ? 33554431.0f : 134217727.0f);   // 1/16 px, 25 / 27 bits
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) |
                                          (moving ? (1ull << 30) | ((std::uint64_t)(yPhase & 3) << 27) : 0ull) | (q << 2) | (std::uint64_t)(phase & 3);
                if (faces_[face]->colorStrikes)
                    return ColorGlyph(face, glyph, emPixels);
                if (faces_[face]->colorLayers)
                    if (const GlyphSlot* s = ColrGlyph(face, glyph, key, phase))
                        return s;
                if (const GlyphSlot* s = atlas_.Find(key))
                    return s;
                const bool freetype = LoadOutline(face, glyph, (float)q / 16.0f, moving) && rendering_.rasterizer == GlyphRasterizer::FreeType;
                const Vec2 offset(0.25f * (float)phase, 0.25f * (float)std::max(yPhase, 0));
                const bool rasterized = atlas_.Lcd() ? (freetype && RenderLcdWithFreeType(*faces_[face], phase, std::max(yPhase, 0), bitmap_)) ||
                                                           RasterizeLcd(outline_, offset, rendering_.subpixel == SubpixelLayout::Bgr, LcdWeights(), bitmap_)
                                                     : (freetype && RenderWithFreeType(*faces_[face], phase, std::max(yPhase, 0), bitmap_)) ||
                                                           RasterizeGray(outline_, offset, bitmap_);
                if (!rasterized)
                    bitmap_ = GlyphBitmap{};   // larger than the rasterizer takes: cached as empty, not retried every frame
                const GlyphSlot* s = atlas_.Add(key, bitmap_, outline_.Bounds());
                return s ? s : atlas_.Add(key, GlyphBitmap{}, Rect());   // larger than a page: empty too
            }

            // The glyph's outline at `emPixels` into outline_ (pixels, y down, the pen at the origin): the design's, or, with
            // hinting or FreeType's rasterizer, FreeType's at that size (hinted as the rendering asks, unless `unhinted`:
            // text in motion); no embedded bitmaps (color strikes: ColorGlyph). True when FreeType's glyph slot holds that
            // sized outline after it (for RenderWithFreeType); a platform face's outline is always the design's.
            bool LoadOutline(std::uint16_t face, std::uint16_t glyph, float emPixels, bool unhinted = false)
            {
                Face& f = *faces_[face];
                outline_.Clear();
                if (f.platform)
                {
                    f.platform->GlyphOutline(glyph, emPixels / f.upem, outline_);
                    return false;
                }
                const bool sized = (rendering_.hinting != Hinting::None && !unhinted) || rendering_.rasterizer == GlyphRasterizer::FreeType;
                const FT_Int32 flags = unhinted ? FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING : LoadFlags();
                if (sized && SetSize(f, emPixels) && FT_Load_Glyph(f.ft, glyph, flags) == 0 && f.ft->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    Decompose(f.ft->glyph->outline, 1.0f / 64.0f);   // 26.6 pixels
                    return true;
                }
                if (FT_Load_Glyph(f.ft, glyph, FT_LOAD_NO_SCALE) == 0 && f.ft->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                    Decompose(f.ft->glyph->outline, emPixels / f.upem);   // design units
                return false;
            }

            void Decompose(const FT_Outline& outline, float scale)
            {
                FT_Outline_Funcs funcs{};
                funcs.move_to = &SinkMoveTo;
                funcs.line_to = &SinkLineTo;
                funcs.conic_to = &SinkConicTo;
                funcs.cubic_to = &SinkCubicTo;
                OutlineSink sink{&outline_, scale};
                FT_Outline_Decompose(const_cast<FT_Outline*>(&outline), &funcs, &sink);
            }

            // ------------------------------------------------------------------ rendering (TextRendering)
            // FreeType's settings for the rendering: the TrueType interpreter, stem darkening, the LCD filter; the glyph
            // pages' kind (grayscale or sub-pixel). Faces request their size again afterwards (a size's prepared hints
            // belong to the interpreter that ran them).
            void ApplyRendering()
            {
                FT_UInt interpreter = rendering_.interpreter == TrueTypeInterpreter::V35 ? TT_INTERPRETER_VERSION_35 : TT_INTERPRETER_VERSION_40;
                FT_Property_Set(ft_, "truetype", "interpreter-version", &interpreter);
                FT_Bool noDarkening = rendering_.stemDarkening ? 0 : 1;
                FT_Property_Set(ft_, "cff", "no-stem-darkening", &noDarkening);
                FT_Property_Set(ft_, "autofitter", "no-stem-darkening", &noDarkening);
                for (const std::unique_ptr<Face>& f : faces_)
                    f->sizeSet = 0;
                const bool lcd = rendering_.subpixel != SubpixelLayout::None;
                // a FreeType without FT_CONFIG_OPTION_SUBPIXEL_RENDERING renders LCD glyphs its own way (Harmony) and
                // has no filters: the call fails and changes nothing
                const FT_LcdFilter filters[] = {FT_LCD_FILTER_DEFAULT, FT_LCD_FILTER_LIGHT, FT_LCD_FILTER_NONE};
                FT_Library_SetLcdFilter(ft_, filters[(int)rendering_.lcdFilter]);
                atlas_.Reformat(lcd ? TextureFormat::RGBA8 : TextureFormat::Alpha8, lcd ? TextureFlags_LcdCoverage : TextureFlags_None);
                // hinting across: the classic interpreter, the autohinter (not for LCD targets: it leaves stems alone
                // across there, so color fringes stay even)
                const bool both = (rendering_.hinting == Hinting::Auto && !lcd) ||
                                  (rendering_.hinting == Hinting::Full && rendering_.interpreter == TrueTypeInterpreter::V35);
                wholePixelPens_ = both || HintedAdvances();
            }

            const std::uint16_t (&LcdWeights() const)[5]
            {
                return rendering_.lcdFilter == LcdFilter::Light ? kLcdFilterLight : rendering_.lcdFilter == LcdFilter::None ? kLcdFilterNone : kLcdFilterDefault;
            }

            // The advances are the hinted ones (TextRendering::hintedAdvances, with hinting that may change them).
            bool HintedAdvances() const
            {
                return rendering_.hintedAdvances && (rendering_.hinting == Hinting::Full || rendering_.hinting == Hinting::Auto);
            }

            FT_Int32 LoadFlags() const
            {
                // LCD targets: the TrueType interpreter tells fonts they are hinted for ClearType, the autohinter leaves
                // stems alone across
                const FT_Int32 target = rendering_.subpixel != SubpixelLayout::None ? FT_LOAD_TARGET_LCD : FT_LOAD_TARGET_NORMAL;
                FT_Int32 flags = FT_LOAD_NO_BITMAP;
                switch (rendering_.hinting)
                {
                case Hinting::None: flags |= FT_LOAD_NO_HINTING; break;
                case Hinting::Light: flags |= FT_LOAD_TARGET_LIGHT; break;
                case Hinting::Full: flags |= target; break;
                case Hinting::Auto: flags |= target | FT_LOAD_FORCE_AUTOHINT; break;
                }
                return flags;
            }

            // The face at `emPixels` per em for FreeType's sized (hinted) loads. Requesting a size runs a TrueType
            // font's preparation program: only when the size changes.
            bool SetSize(Face& f, float emPixels)
            {
                const FT_F26Dot6 size = (FT_F26Dot6)std::lround(std::clamp(emPixels, 1.0f, 16384.0f) * 64.0f);
                if (f.sizeSet == size)
                    return true;
                if (FT_Set_Char_Size(f.ft, 0, size, 72, 72) != 0)
                {
                    f.sizeSet = 0;
                    return false;
                }
                f.sizeSet = size;
                return true;
            }

            // The hinted advance width of `glyph` at `ppem` (physical pixels per em), in whole pixels.
            int HintedAdvance(std::uint16_t face, std::uint16_t glyph, float ppem)
            {
                const std::uint32_t size = (std::uint32_t)std::lround(std::clamp(ppem, 1.0f, 16384.0f) * 64.0f);
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) | size;
                if (const auto it = hintedAdvanceCache_.find(key); it != hintedAdvanceCache_.end())
                    return it->second;
                Face& f = *faces_[face];
                FT_Fixed advance = 0;   // 16.16 pixels
                int px = 0;
                if (SetSize(f, ppem) && FT_Get_Advance(f.ft, glyph, LoadFlags(), &advance) == 0)
                    px = (int)((advance + 0x8000) >> 16);
                hintedAdvanceCache_.emplace(key, px);
                return px;
            }

            // FreeType's rasterizer on the outline LoadOutline left in the face's glyph slot, moved right by the pen's
            // phase (quarter pixels): 8-bit coverage. False when it cannot (the caller rasterizes the outline itself).
            bool RenderWithFreeType(Face& f, int phase, int yPhase, GlyphBitmap& out)
            {
                FT_GlyphSlot slot = f.ft->glyph;
                if (phase != 0 || yPhase != 0)
                    FT_Outline_Translate(&slot->outline, 16 * phase, -16 * yPhase);   // 26.6: a quarter pixel is 16 (y up)
                if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) != 0 || slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
                    return false;
                const FT_Bitmap& b = slot->bitmap;
                out = GlyphBitmap{};
                out.left = slot->bitmap_left;
                out.top = -slot->bitmap_top;
                out.width = (int)b.width;
                out.height = (int)b.rows;
                out.pixels.resize((std::size_t)out.width * (std::size_t)out.height);
                for (int y = 0; y < out.height; ++y)
                {
                    // rows top first with a positive pitch; from the bottom with a negative one
                    const unsigned char* row = b.pitch >= 0 ? b.buffer + (std::ptrdiff_t)y * b.pitch : b.buffer + (std::ptrdiff_t)(out.height - 1 - y) * -b.pitch;
                    std::memcpy(out.pixels.data() + (std::size_t)y * (std::size_t)out.width, row, (std::size_t)out.width);
                }
                return true;
            }

            // FreeType's sub-pixel rendering of that outline (its LCD filter): the stripes left to right as R G B, or
            // B G R for `Bgr` panels, and their mean in A for the grayscale fallback. False when it cannot.
            bool RenderLcdWithFreeType(Face& f, int phase, int yPhase, GlyphBitmap& out)
            {
                FT_GlyphSlot slot = f.ft->glyph;
                if (phase != 0 || yPhase != 0)
                    FT_Outline_Translate(&slot->outline, 16 * phase, -16 * yPhase);
                if (FT_Render_Glyph(slot, FT_RENDER_MODE_LCD) != 0 || slot->bitmap.pixel_mode != FT_PIXEL_MODE_LCD)
                    return false;
                const FT_Bitmap& b = slot->bitmap;
                const bool bgr = rendering_.subpixel == SubpixelLayout::Bgr;
                out = GlyphBitmap{};
                out.channels = 4;
                out.lcd = true;
                out.left = slot->bitmap_left;
                out.top = -slot->bitmap_top;
                out.width = (int)b.width / 3;
                out.height = (int)b.rows;
                out.pixels.resize((std::size_t)out.width * (std::size_t)out.height * 4);
                for (int y = 0; y < out.height; ++y)
                {
                    const unsigned char* row = b.pitch >= 0 ? b.buffer + (std::ptrdiff_t)y * b.pitch : b.buffer + (std::ptrdiff_t)(out.height - 1 - y) * -b.pitch;
                    std::uint8_t* dst = out.pixels.data() + (std::size_t)y * (std::size_t)out.width * 4;
                    for (int x = 0; x < out.width; ++x)
                    {
                        const unsigned char* s = row + x * 3;
                        dst[x * 4 + 0] = s[bgr ? 2 : 0];
                        dst[x * 4 + 1] = s[1];
                        dst[x * 4 + 2] = s[bgr ? 0 : 2];
                        dst[x * 4 + 3] = (std::uint8_t)(((int)s[0] + (int)s[1] + (int)s[2] + 1) / 3);
                    }
                }
                return true;
            }

            // A glyph grown by an outline's width (`widthQ`, 1/8 px: OutlineQ), in the gray atlas beside the glyphs. Its
            // key has bit 31 set, the width in bits 20 .. 27, the em in bits 2 .. 19 (capped at 2^18 / 16 px), and in
            // motion (yPhase >= 0: the design's outline, shifted down too) bit 30 and the phase in bits 28 .. 29.
            const GlyphSlot* OutlineGlyph(std::uint16_t face, std::uint16_t glyph, float emPixels, int phase, std::uint32_t widthQ, int yPhase = -1)
            {
                if (faces_[face]->bitmap)
                    return BitmapGlyph(face, glyph, emPixels, widthQ);
                const bool moving = yPhase >= 0;
                const std::uint64_t q = (std::uint64_t)std::min(emPixels * 16.0f + 0.5f, 262143.0f);   // 1/16 px, 18 bits
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) | (1ull << 31) |
                                          (moving ? (1ull << 30) | ((std::uint64_t)(yPhase & 3) << 28) : 0ull) | ((std::uint64_t)(widthQ & 0xFF) << 20) |
                                          (q << 2) | (std::uint64_t)(phase & 3);
                if (const GlyphSlot* s = atlas_.Find(key))
                    return s;
                (void)LoadOutline(face, glyph, (float)q / 16.0f, moving);
                if (!RasterizeGrown(outline_, Vec2(0.25f * (float)phase, 0.25f * (float)std::max(yPhase, 0)), (float)widthQ / 8.0f, bitmap_))
                    bitmap_ = GlyphBitmap{};
                const GlyphSlot* s = atlas_.Add(key, bitmap_, outline_.Bounds());
                return s ? s : atlas_.Add(key, GlyphBitmap{}, Rect());
            }

            // A COLR color glyph (a vector emoji) under the gray glyph's `key` (size and phase), painted on first use; null
            // when `glyph` has no colors (the font's digits, its space): an outline like any other.
            const GlyphSlot* ColrGlyph(std::uint16_t face, std::uint16_t glyph, std::uint64_t key, int phase)
            {
                if (const GlyphSlot* s = colorAtlas_.Find(key))
                    return s;
                FT_Face ft = faces_[face]->ft;
                if (!detail::IsColrGlyph(ft, glyph))
                    return nullptr;
                Rect ink;
                const float emPixels = (float)((key >> 2) & 0x7FFFFFF) / 16.0f;
                if (!detail::DrawColrGlyph(ft, glyph, emPixels, 0.25f * (float)phase, bitmap_, ink))
                {
                    bitmap_ = GlyphBitmap{0, 0, 0, 0, {}, 4};   // larger than the painter takes: cached as empty
                    ink = Rect();
                }
                const GlyphSlot* s = colorAtlas_.Add(key, bitmap_, ink);
                return s ? s : colorAtlas_.Add(key, GlyphBitmap{0, 0, 0, 0, {}, 4}, Rect());   // larger than a page: empty
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
                faces_[face]->sizeSet = 0;   // a strike replaces the size hinted loads set
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
                else if (strike >= 0 && PngGlyph(*faces_[face], glyph, emPixels))
                    ink = Rect((float)bitmap_.left, (float)bitmap_.top, (float)(bitmap_.left + bitmap_.width), (float)(bitmap_.top + bitmap_.height));
                else if (Face& f = *faces_[face]; strike >= 0)
                {
                    // a strike FreeType cannot decode (iOS' Apple Color Emoji: emjc, not PNG): the platform draws the
                    // glyph, the same font found by its PostScript name
                    if (!f.colorPlatformTried)
                    {
                        f.colorPlatformTried = true;
                        if (const char* ps = FT_Get_Postscript_Name(ft))
                            f.colorPlatform = detail::OpenPlatformFace(std::string(kCoreTextFontScheme) + ps);
                    }
                    if (f.colorPlatform && f.colorPlatform->ColorGlyph(glyph, emPixels, bitmap_))
                        ink = Rect((float)bitmap_.left, (float)bitmap_.top, (float)(bitmap_.left + bitmap_.width), (float)(bitmap_.top + bitmap_.height));
                    else
                        bitmap_ = GlyphBitmap{0, 0, 0, 0, {}, 4};
                }
                const GlyphSlot* s = colorAtlas_.Add(key, bitmap_, ink);
                return s ? s : colorAtlas_.Add(key, GlyphBitmap{0, 0, 0, 0, {}, 4}, Rect());   // larger than a page: empty
            }

            // A color bitmap glyph whose PNG FreeType cannot decode (built without libpng, as Esia's bundled FreeType is):
            // the largest strike's image from HarfBuzz, decoded by Esia, placed by its extents (design units: the
            // HarfBuzz font's scale), scaled to `emPixels` into bitmap_.
            bool PngGlyph(const Face& f, std::uint16_t glyph, float emPixels)
            {
                hb_blob_t* blob = hb_ot_color_glyph_reference_png(f.hb, glyph);
                unsigned length = 0;
                const char* data = hb_blob_get_data(blob, &length);
                int w = 0, h = 0;
                std::vector<std::uint8_t> pixels;
                const bool decoded = length > 0 && detail::DecodePng(reinterpret_cast<const std::uint8_t*>(data), length, w, h, pixels);
                hb_blob_destroy(blob);
                hb_glyph_extents_t ext{};
                if (!decoded || !hb_font_get_glyph_extents(f.hb, glyph, &ext) || ext.width <= 0)
                    return false;
                for (std::size_t i = 0; i < pixels.size(); i += 4)   // straight RGBA -> premultiplied BGRA
                {
                    const unsigned a = pixels[i + 3], r = pixels[i];
                    pixels[i] = (std::uint8_t)((pixels[i + 2] * a + 127) / 255);
                    pixels[i + 1] = (std::uint8_t)((pixels[i + 1] * a + 127) / 255);
                    pixels[i + 2] = (std::uint8_t)((r * a + 127) / 255);
                }
                const float px = emPixels / f.upem;
                ScaleBgra(pixels.data(), w, h, w * 4, (float)ext.width * px / (float)w, bitmap_);
                bitmap_.left = (int)std::floor((float)ext.x_bearing * px + 0.5f);
                bitmap_.top = -(int)std::floor((float)ext.y_bearing * px + 0.5f);
                return true;
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

            // ---- bitmap fonts
            // A strike's height in pixels (its em)
            static int StrikePx(const Face& f, int strike) { return f.strikes[(std::size_t)strike].px; }

            // `ft`'s bitmap sizes as strikes of `f`
            static void AddStrikes(Face& f, FT_Face ft)
            {
                // a Windows FNT's one character map is not marked Unicode, so FreeType selects none: its codes are the
                // font's code page, Latin-1 for the usual ANSI fonts
                if (!ft->charmap && ft->num_charmaps > 0)
                    FT_Set_Charmap(ft, ft->charmaps[0]);
                for (int i = 0; i < ft->num_fixed_sizes; ++i)
                {
                    const FT_Bitmap_Size& s = ft->available_sizes[i];
                    f.strikes.push_back({ft, i, std::max(1, s.y_ppem > 0 ? (int)((s.y_ppem + 32) >> 6) : (int)s.height)});
                }
            }

            // The strike and whole multiple of it nearest `px` pixels (a larger strike on a tie)
            static void PickStrike(const Face& f, float px, int& strike, int& times)
            {
                strike = 0;
                times = 1;
                float best = std::numeric_limits<float>::infinity();
                for (int i = 0; i < (int)f.strikes.size(); ++i)
                {
                    const int s = StrikePx(f, i);
                    const int k = std::max(1, (int)std::floor(px / (float)s + 0.5f));
                    const float d = std::fabs((float)(k * s) - px);
                    if (d < best || (d == best && s > StrikePx(f, strike)))
                    {
                        best = d;
                        strike = i;
                        times = k;
                    }
                }
            }

            // Selects `strike` in FreeType: upem becomes its pixels, the metrics its own
            void SelectStrike(Face& f, int strike)
            {
                const Face::Strike& s = f.strikes[(std::size_t)strike];
                if (f.strike == strike || FT_Select_Size(s.ft, s.index) != 0)
                    return;
                f.strike = strike;
                f.active = s.ft;
                const FT_Size_Metrics& m = s.ft->size->metrics;
                f.upem = (float)StrikePx(f, strike);
                f.ascent = (float)((m.ascender + 32) >> 6);
                f.descent = (float)((-m.descender + 32) >> 6);
                if (f.ascent + f.descent <= 0.0f)
                {
                    f.ascent = f.upem;
                    f.descent = 0.0f;
                }
                f.gap = std::max(0.0f, (float)((m.height + 32) >> 6) - f.ascent - f.descent);
                hb_font_set_scale(f.hb, (int)f.upem, (int)f.upem);
            }

            // The size (UI units) a face draws `size` at: `size`, or for a bitmap font the strike and whole multiple of
            // it nearest in pixels - selected, so its metrics and HarfBuzz font are that strike's
            float Snapped(std::uint16_t face, float size)
            {
                Face& f = *faces_[face];
                if (!f.bitmap)
                    return size;
                int strike = 0, times = 1;
                PickStrike(f, size * params_.pixelsPerUnit, strike, times);
                SelectStrike(f, strike);
                return (float)(times * StrikePx(f, strike)) / params_.pixelsPerUnit;
            }

            // A bitmap font's glyph at the strike and multiple nearest `emPixels`: its pixels, each repeated `times` x
            // `times` (nearest neighbour), on whole pixels with no phases. `outlineQ` > 0: its outline (TextOutline) -
            // every pixel within the width (in whole pixels, a square around each: crisp as the font).
            const GlyphSlot* BitmapGlyph(std::uint16_t face, std::uint16_t glyph, float emPixels, std::uint32_t outlineQ)
            {
                Face& f = *faces_[face];
                int strike = 0, times = 1;
                PickStrike(f, emPixels, strike, times);
                const std::uint64_t q = (std::uint64_t)(times * StrikePx(f, strike)) * 16u;
                const std::uint64_t key = ((std::uint64_t)face << 48) | ((std::uint64_t)glyph << 32) |
                                          (outlineQ ? (1ull << 31) | ((std::uint64_t)(outlineQ & 0xFF) << 22) : 0u) | (std::min<std::uint64_t>(q, 1048575u) << 2);
                if (const GlyphSlot* s = atlas_.Find(key))
                    return s;
                Rect ink;
                if (!BitmapCoverage(f, glyph, strike, times, bitmap_, ink))
                    bitmap_ = GlyphBitmap{};
                if (outlineQ > 0)
                    GrowPixels(bitmap_, std::max(1, (int)(outlineQ / 8.0f + 0.5f)));
                const GlyphSlot* s = atlas_.Add(key, bitmap_, ink);
                return s ? s : atlas_.Add(key, GlyphBitmap{}, Rect());
            }

            // The glyph's bitmap at `strike` as 8-bit coverage, scaled `times` (nearest neighbour), with an empty pixel
            // around it; `ink`: its pixels (from the pen, y down). False when FreeType has none.
            bool BitmapCoverage(Face& f, std::uint16_t glyph, int strike, int times, GlyphBitmap& out, Rect& ink)
            {
                out = GlyphBitmap{};
                ink = Rect();
                SelectStrike(f, strike);
                FT_Face ft = f.active;
                if (FT_Load_Glyph(ft, glyph, FT_LOAD_DEFAULT) != 0 || ft->glyph->format != FT_GLYPH_FORMAT_BITMAP)
                    return false;
                FT_Bitmap gray;
                FT_Bitmap_Init(&gray);
                if (FT_Bitmap_Convert(ft_, &ft->glyph->bitmap, &gray, 1) != 0)
                {
                    FT_Bitmap_Done(ft_, &gray);
                    return false;
                }
                const int w = (int)gray.width, h = (int)gray.rows;
                const bool fits = w > 0 && h > 0 && w * times + 2 <= 4096 && h * times + 2 <= 4096;
                if (fits)
                {
                    // 0 .. num_grays - 1 (a monochrome font: 0 or 1) to 0 .. 255
                    const int levels = std::max(1, (int)gray.num_grays - 1);
                    out.width = w * times + 2;
                    out.height = h * times + 2;
                    out.left = ft->glyph->bitmap_left * times - 1;
                    out.top = -ft->glyph->bitmap_top * times - 1;
                    out.pixels.assign((std::size_t)out.width * (std::size_t)out.height, 0);
                    for (int y = 0; y < h; ++y)
                        for (int x = 0; x < w; ++x)
                        {
                            const int v = std::min(255, gray.buffer[(std::size_t)y * (std::size_t)gray.pitch + (std::size_t)x] * 255 / levels);
                            for (int yy = 0; yy < times; ++yy)
                                std::memset(&out.pixels[(std::size_t)(1 + y * times + yy) * (std::size_t)out.width + (std::size_t)(1 + x * times)], v, (std::size_t)times);
                        }
                    ink = Rect((float)(out.left + 1), (float)(out.top + 1), (float)(out.left + 1 + w * times), (float)(out.top + 1 + h * times));
                }
                FT_Bitmap_Done(ft_, &gray);
                return fits || w == 0 || h == 0;
            }

            // Every pixel takes the most coverage within `r` pixels across and down (a square around it): the bitmap
            // grows by `r` on every side, its edges stay empty.
            static void GrowPixels(GlyphBitmap& b, int r)
            {
                if (b.width <= 0 || b.height <= 0)
                    return;
                const int w = b.width + 2 * r, h = b.height + 2 * r;
                std::vector<std::uint8_t> rows((std::size_t)w * (std::size_t)b.height, 0), out((std::size_t)w * (std::size_t)h, 0);
                for (int y = 0; y < b.height; ++y)
                    for (int x = 0; x < w; ++x)
                    {
                        std::uint8_t m = 0;
                        for (int sx = std::max(0, x - 2 * r); sx <= std::min(b.width - 1, x); ++sx)
                            m = std::max(m, b.pixels[(std::size_t)y * (std::size_t)b.width + (std::size_t)sx]);
                        rows[(std::size_t)y * (std::size_t)w + (std::size_t)x] = m;
                    }
                for (int y = 0; y < h; ++y)
                    for (int x = 0; x < w; ++x)
                    {
                        std::uint8_t m = 0;
                        for (int sy = std::max(0, y - 2 * r); sy <= std::min(b.height - 1, y); ++sy)
                            m = std::max(m, rows[(std::size_t)sy * (std::size_t)w + (std::size_t)x]);
                        out[(std::size_t)y * (std::size_t)w + (std::size_t)x] = m;
                    }
                b.pixels.swap(out);
                b.left -= r;
                b.top -= r;
                b.width = w;
                b.height = h;
            }

            // HarfBuzz's view of a bitmap font: its characters, advances (in pixels of the strike in use) and metrics
            static hb_bool_t BitmapNominalGlyph(hb_font_t*, void* data, hb_codepoint_t unicode, hb_codepoint_t* glyph, void*)
            {
                const FT_UInt g = FT_Get_Char_Index(static_cast<Face*>(data)->active, unicode);
                *glyph = g;
                return g != 0;
            }
            static hb_position_t BitmapAdvance(hb_font_t*, void* data, hb_codepoint_t glyph, void*)
            {
                FT_Face ft = static_cast<Face*>(data)->active;
                if (FT_Load_Glyph(ft, glyph, FT_LOAD_DEFAULT) != 0)
                    return 0;
                return (hb_position_t)((ft->glyph->advance.x + 32) >> 6);
            }
            static hb_bool_t BitmapExtents(hb_font_t*, void* data, hb_font_extents_t* extents, void*)
            {
                const Face& f = *static_cast<Face*>(data);
                extents->ascender = (hb_position_t)f.ascent;
                extents->descender = -(hb_position_t)f.descent;
                extents->line_gap = (hb_position_t)f.gap;
                return true;
            }
            static hb_font_funcs_t* BitmapFontFuncs()
            {
                static hb_font_funcs_t* const funcs = [] {
                    hb_font_funcs_t* f = hb_font_funcs_create();
                    hb_font_funcs_set_nominal_glyph_func(f, &BitmapNominalGlyph, nullptr, nullptr);
                    hb_font_funcs_set_glyph_h_advance_func(f, &BitmapAdvance, nullptr, nullptr);
                    hb_font_funcs_set_font_h_extents_func(f, &BitmapExtents, nullptr, nullptr);
                    hb_font_funcs_make_immutable(f);
                    return f;
                }();
                return funcs;
            }

            int bitmapFaces_ = 0;                  // bitmap fonts loaded
            float layoutPixelsPerUnit_ = 0.0f;     // the density the layouts were built at (bitmap fonts snap to it)
            FT_Library ft_ = nullptr;
            hb_buffer_t* buffer_ = nullptr;
            hb_unicode_funcs_t* unicode_ = nullptr;
            std::vector<std::unique_ptr<Face>> faces_;   // FontId - 1
            std::unordered_map<std::string, std::weak_ptr<const MappedFile>> files_;   // AddFontFile's, by path
            std::vector<std::uint16_t> fallbacks_;
            GlyphAtlas atlas_;
            GlyphAtlas colorAtlas_;   // RGBA8: color glyphs
            GlyphAtlas fieldAtlas_;   // RGBA8: distance fields (one per glyph, every size)
            FieldMaker fields_;       // the fields of new glyphs: worker threads, the cache on disk
            struct FieldNeed
            {
                std::uint64_t key;
                Rect ink;
            };
            std::vector<FieldNeed> fieldBatch_;                            // FieldSlots: the fields being made, in order
            std::unordered_set<std::uint64_t, IntHash> fieldBatchKeys_;   // ... their keys
            std::vector<const GlyphSlot*> fieldSlots_;                     // DrawShapes: each glyph's field
            std::vector<std::size_t> fieldMissing_;                        // ... the glyphs without one yet
            const std::uint64_t serial_ = NextSerial();   // TileGeneration: this text system's
            RasterParams params_;
            std::uint64_t frame_ = 0;
            std::unordered_map<std::uint64_t, Layout, IntHash> layouts_;
            Layout* lastLayout_ = nullptr;   // the last GetLayout's (a measure is followed by the draw)
            std::uint32_t lastParams_[5] = {};   // ... its parameters' key (ParamsKey): most texts of a frame share them
            std::uint64_t lastParamsKey_ = ParamsKey(lastParams_);
            static constexpr std::size_t kLayoutFront = 256;
            std::array<Layout*, kLayoutFront> layoutFront_{};   // layouts_ by key, direct-mapped (a layout leaves with its slot)
            // QuadSet's rounding classes: for texts of up to kClassGlyphs glyphs (longer ones are placed again), at most
            // kMaxClasses kept per set
            static constexpr std::size_t kClassGlyphs = 64;
            static constexpr std::uint32_t kMaxClasses = 256;
            std::vector<std::unordered_map<std::uint64_t, Layout, IntHash>::node_type> spareLayouts_;   // evicted, to be reused
            std::size_t sweepBucket_ = 0;           // NewFrame's next bucket of layouts_ to look through for unused layouts
            std::vector<std::uint64_t> sweepKeys_;   // ... its scratch

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

            // TextRendering: in use, and asked for (SetRendering; it takes over at the next NewFrame)
            TextRendering rendering_, pending_;
            // draws done again (DrawNote): the layouts by serial (direct: a serial whose slot another took is not found,
            // and its draw is done again rather than replayed), the notes being kept, what changes drawn text
            static constexpr std::size_t kSerialSlots = 2048;
            struct SerialSlot
            {
                std::uint64_t serial = 0;
                Layout* layout = nullptr;
            };
            std::array<SerialSlot, kSerialSlots> bySerial_{};
            std::uint64_t layoutSerial_ = serial_ << 32;   // (unique among the text systems too)
            std::vector<DrawNote>* notes_ = nullptr;
            std::uint64_t drawGeneration_ = 1;
            std::uint64_t lastResets_[3] = {};
            bool wholePixelPens_ = false;   // the outlines are hinted across too (vertical stems), or the advances: pens on whole pixels
            std::unordered_map<std::uint64_t, int, IntHash> hintedAdvanceCache_;   // face, glyph, 26.6 ppem -> pixels
        };
    }

    std::unique_ptr<TextSystem> CreateFreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc)
    {
        FreeTypeDesc d = desc;
        if (const char* spec = std::getenv("ESIA_TEXT_RENDERING"); spec && *spec)
        {
            std::string error;
            if (!ParseTextRendering(spec, d.rendering, &error))
                std::fprintf(stderr, "esia: ESIA_TEXT_RENDERING: %s (ignored)\n", error.c_str());
        }
        auto system = std::make_unique<FreeTypeTextSystem>(textures, d);
        if (!system->Init())
            return nullptr;
        return system;
    }
}
