// Esia - system fonts drawn through the platform (platform_face.hpp). Apple: Core Text - the tables it copies out of
// the font for HarfBuzz, its glyph paths for the outlines. A font is created at its units per em as point size, so
// the paths are in design units.
#include "platform_face.hpp"
#include "esia/text/system_fonts.hpp"
#if defined(__APPLE__)
#include <CoreText/CoreText.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#endif

namespace esia::text::detail
{
#if defined(__APPLE__)
    namespace
    {
        struct PathSink
        {
            Outline* out = nullptr;
            float scale = 1.0f;
            Vec2 Map(CGPoint p) const { return Vec2((float)p.x * scale, -(float)p.y * scale); }
        };

        void AddElement(void* info, const CGPathElement* e)
        {
            const PathSink& s = *static_cast<const PathSink*>(info);
            switch (e->type)
            {
            case kCGPathElementMoveToPoint: s.out->MoveTo(s.Map(e->points[0])); break;
            case kCGPathElementAddLineToPoint: s.out->LineTo(s.Map(e->points[0])); break;
            case kCGPathElementAddQuadCurveToPoint: s.out->QuadTo(s.Map(e->points[0]), s.Map(e->points[1])); break;
            case kCGPathElementAddCurveToPoint: s.out->CubicTo(s.Map(e->points[0]), s.Map(e->points[1]), s.Map(e->points[2])); break;
            default: break;   // close: contours close implicitly
            }
        }

        // A table for HarfBuzz: Core Text's copy, released with the blob.
        hb_blob_t* CopyTable(hb_face_t*, hb_tag_t tag, void* font)
        {
            if (tag == 0)
                return nullptr;   // the whole font: there is no file
            CFDataRef data = CTFontCopyTable(static_cast<CTFontRef>(font), (CTFontTableTag)tag, kCTFontTableOptionNoOptions);
            if (!data)
                return nullptr;
            return hb_blob_create(reinterpret_cast<const char*>(CFDataGetBytePtr(data)), (unsigned)CFDataGetLength(data), HB_MEMORY_MODE_READONLY,
                                  const_cast<void*>(static_cast<const void*>(data)), [](void* d) { CFRelease(static_cast<CFDataRef>(d)); });
        }

        class CoreTextFace final : public PlatformFace
        {
        public:
            explicit CoreTextFace(CTFontRef font) : font_(font) {}
            ~CoreTextFace() override { CFRelease(font_); }
            CoreTextFace(const CoreTextFace&) = delete;
            CoreTextFace& operator=(const CoreTextFace&) = delete;

            hb_face_t* CreateHbFace() override
            {
                CFRetain(font_);   // the face's, until HarfBuzz destroys it
                hb_face_t* face = hb_face_create_for_tables(&CopyTable, const_cast<void*>(static_cast<const void*>(font_)),
                                                            [](void* f) { CFRelease(static_cast<CTFontRef>(f)); });
                hb_face_set_upem(face, CTFontGetUnitsPerEm(font_));
                return face;
            }

            void SetVariations(hb_font_t* font) override
            {
                CFDictionaryRef axes = CTFontCopyVariation(font_);
                if (!axes)
                    return;
                const CFIndex n = CFDictionaryGetCount(axes);
                std::vector<const void*> keys((std::size_t)n), values((std::size_t)n);
                CFDictionaryGetKeysAndValues(axes, keys.data(), values.data());
                std::vector<hb_variation_t> variations;
                for (CFIndex i = 0; i < n; ++i)
                {
                    long long tag = 0;   // the axis' identifier: its tag as a number ('wght')
                    double value = 0.0;
                    if (CFNumberGetValue(static_cast<CFNumberRef>(keys[(std::size_t)i]), kCFNumberLongLongType, &tag) &&
                        CFNumberGetValue(static_cast<CFNumberRef>(values[(std::size_t)i]), kCFNumberDoubleType, &value))
                        variations.push_back({(hb_tag_t)tag, (float)value});
                }
                CFRelease(axes);
                if (!variations.empty())
                    hb_font_set_variations(font, variations.data(), (unsigned)variations.size());
            }

            bool GlyphOutline(std::uint16_t glyph, float scale, Outline& out) override
            {
                CGPathRef path = CTFontCreatePathForGlyph(font_, (CGGlyph)glyph, nullptr);
                if (!path)
                    return false;
                PathSink sink{&out, scale};
                CGPathApply(path, &sink, &AddElement);
                CGPathRelease(path);
                return true;
            }

            bool ColorGlyph(std::uint16_t glyph, float emPixels, GlyphBitmap& out) override
            {
                CTFontRef sized = CTFontCreateCopyWithAttributes(font_, (CGFloat)emPixels, nullptr, nullptr);
                if (!sized)
                    return false;
                const CGGlyph g = glyph;
                const CGRect box = CTFontGetBoundingRectsForGlyphs(sized, kCTFontOrientationHorizontal, &g, nullptr, 1);
                // whole pixels around the image, y up from the baseline
                const int x0 = (int)std::floor(box.origin.x), y0 = (int)std::floor(box.origin.y);
                const int x1 = (int)std::ceil(box.origin.x + box.size.width), y1 = (int)std::ceil(box.origin.y + box.size.height);
                const int w = x1 - x0, h = y1 - y0;
                bool drawn = false;
                if (w > 0 && h > 0 && w <= 4096 && h <= 4096)
                {
                    std::vector<std::uint8_t> rgba((std::size_t)w * (std::size_t)h * 4, 0);
                    CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                    CGContextRef cg = CGBitmapContextCreate(rgba.data(), (size_t)w, (size_t)h, 8, (size_t)w * 4, srgb,
                                                            (CGBitmapInfo)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
                    CGColorSpaceRelease(srgb);
                    if (cg)
                    {
                        const CGPoint at = CGPointMake(-x0, -y0);
                        CTFontDrawGlyphs(sized, &g, &at, 1, cg);
                        CGContextRelease(cg);
                        // rows top first already (a bitmap context's memory); premultiplied -> straight
                        for (std::size_t i = 0; i < rgba.size(); i += 4)
                            if (const unsigned a = rgba[i + 3]; a > 0 && a < 255)
                                for (int c = 0; c < 3; ++c)
                                    rgba[i + (std::size_t)c] = (std::uint8_t)std::min(255u, (rgba[i + (std::size_t)c] * 255u + a / 2) / a);
                        out.left = x0;
                        out.top = -y1;
                        out.width = w;
                        out.height = h;
                        out.channels = 4;
                        out.pixels = std::move(rgba);
                        drawn = true;
                    }
                }
                CFRelease(sized);
                return drawn;
            }

        private:
            CTFontRef font_;
        };
    }

    std::unique_ptr<PlatformFace> OpenPlatformFace(std::string_view path)
    {
        if (!path.starts_with(kCoreTextFontScheme))
            return nullptr;
        const std::string_view name = path.substr(kCoreTextFontScheme.size());
        CFStringRef ps = CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(name.data()), (CFIndex)name.size(), kCFStringEncodingUTF8, false);
        if (!ps)
            return nullptr;
        CTFontRef probe = CTFontCreateWithName(ps, 12.0, nullptr);
        CTFontRef font = nullptr;
        if (probe)
        {
            // Core Text answers an unknown name with another font: only the one asked for
            CFStringRef got = CTFontCopyPostScriptName(probe);
            if (got && CFStringCompare(got, ps, 0) == kCFCompareEqualTo)
                font = CTFontCreateCopyWithAttributes(probe, (CGFloat)CTFontGetUnitsPerEm(probe), nullptr, nullptr);
            if (got)
                CFRelease(got);
            CFRelease(probe);
        }
        CFRelease(ps);
        if (!font)
            return nullptr;
        return std::unique_ptr<PlatformFace>(new CoreTextFace(font));
    }
#else
    std::unique_ptr<PlatformFace> OpenPlatformFace(std::string_view) { return nullptr; }
#endif
}
