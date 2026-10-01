// showcase - image files on Linux: PNG through the test kit's decoder (8-bit, non-interlaced: what the desktops'
// wallpapers are), JPEG through libjpeg when CMake found it (SHOWCASE_JPEG). Scaled down by area averaging.
#include "image_file.hpp"
#include "image.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#if defined(SHOWCASE_JPEG)
#include <jpeglib.h>
#endif

namespace showcase
{
    namespace
    {
        std::string Utf8(const std::wstring& w)
        {
            std::string s;
            for (wchar_t c : w)
            {
                const char32_t u = (char32_t)c;
                if (u < 0x80)
                    s += (char)u;
                else if (u < 0x800)
                    s += {(char)(0xC0 | (u >> 6)), (char)(0x80 | (u & 0x3F))};
                else if (u < 0x10000)
                    s += {(char)(0xE0 | (u >> 12)), (char)(0x80 | ((u >> 6) & 0x3F)), (char)(0x80 | (u & 0x3F))};
                else
                    s += {(char)(0xF0 | (u >> 18)), (char)(0x80 | ((u >> 12) & 0x3F)), (char)(0x80 | ((u >> 6) & 0x3F)), (char)(0x80 | (u & 0x3F))};
            }
            return s;
        }

        bool EndsWith(const std::string& s, const char* tail)
        {
            const std::string t = tail;
            return s.size() >= t.size() && std::equal(t.rbegin(), t.rend(), s.rbegin(), [](char a, char b) { return a == (char)std::tolower((unsigned char)b); });
        }

#if defined(SHOWCASE_JPEG)
        bool ReadJpeg(const std::string& path, ImageFile& out)
        {
            std::FILE* f = std::fopen(path.c_str(), "rb");
            if (!f)
                return false;
            jpeg_decompress_struct info = {};
            jpeg_error_mgr err = {};
            info.err = jpeg_std_error(&err);
            err.error_exit = [](j_common_ptr) { throw 0; };   // a broken file: give up on it
            bool ok = false;
            try
            {
                jpeg_create_decompress(&info);
                jpeg_stdio_src(&info, f);
                jpeg_read_header(&info, TRUE);
                info.out_color_space = JCS_RGB;
                jpeg_start_decompress(&info);
                out.width = (int)info.output_width;
                out.height = (int)info.output_height;
                out.rgba.assign((std::size_t)out.width * out.height * 4, 255);
                std::vector<unsigned char> row((std::size_t)out.width * 3);
                while (info.output_scanline < info.output_height)
                {
                    unsigned char* rows[] = {row.data()};
                    const int y = (int)info.output_scanline;
                    jpeg_read_scanlines(&info, rows, 1);
                    for (int x = 0; x < out.width; ++x)
                        std::copy_n(&row[(std::size_t)x * 3], 3, &out.rgba[((std::size_t)y * out.width + x) * 4]);
                }
                jpeg_finish_decompress(&info);
                ok = true;
            }
            catch (int)
            {
            }
            jpeg_destroy_decompress(&info);
            std::fclose(f);
            return ok;
        }
#endif

        // Box filter: each output pixel is the average of the source pixels it covers (whole pixels; a downscale).
        void Shrink(ImageFile& img, int maxSide)
        {
            const int side = std::max(img.width, img.height);
            if (maxSide <= 0 || side <= maxSide)
                return;
            const double k = (double)side / maxSide;
            const int w = std::max(1, (int)(img.width / k + 0.5)), h = std::max(1, (int)(img.height / k + 0.5));
            std::vector<std::uint8_t> dst((std::size_t)w * h * 4);
            for (int y = 0; y < h; ++y)
            {
                const int y0 = (int)((double)y * img.height / h), y1 = std::max(y0 + 1, (int)((double)(y + 1) * img.height / h));
                for (int x = 0; x < w; ++x)
                {
                    const int x0 = (int)((double)x * img.width / w), x1 = std::max(x0 + 1, (int)((double)(x + 1) * img.width / w));
                    unsigned sum[4] = {};
                    for (int sy = y0; sy < y1; ++sy)
                        for (int sx = x0; sx < x1; ++sx)
                            for (int c = 0; c < 4; ++c)
                                sum[c] += img.rgba[((std::size_t)sy * img.width + sx) * 4 + c];
                    const unsigned n = (unsigned)((y1 - y0) * (x1 - x0));
                    for (int c = 0; c < 4; ++c)
                        dst[((std::size_t)y * w + x) * 4 + c] = (std::uint8_t)((sum[c] + n / 2) / n);
                }
            }
            img.width = w;
            img.height = h;
            img.rgba = std::move(dst);
        }
    }

    bool LoadImageFile(const std::wstring& path, ImageFile& out, int maxSide)
    {
        const std::string file = Utf8(path);
        bool ok = false;
        if (EndsWith(file, ".png"))
        {
            esia::testkit::Image image;
            if ((ok = esia::testkit::ReadPng(file, image)))
            {
                out.width = image.width;
                out.height = image.height;
                out.rgba = std::move(image.rgba);
            }
        }
#if defined(SHOWCASE_JPEG)
        else if (EndsWith(file, ".jpg") || EndsWith(file, ".jpeg"))
            ok = ReadJpeg(file, out);
#endif
        if (ok)
            Shrink(out, maxSide);
        return ok;
    }
}
