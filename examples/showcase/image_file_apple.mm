// showcase - image files (the wallpaper) through ImageIO on macOS and iOS, as straight-alpha RGBA8 (image_file.hpp). A
// relative path that is no file from the working directory is looked up in the app bundle's resources (iOS).
#include "image_file.hpp"
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>

namespace showcase
{
    bool LoadImageFile(const std::wstring& path, ImageFile& out, int maxSide)
    {
        @autoreleasepool
        {
            // wchar_t is UTF-32 on Apple platforms
            NSString* p = [[NSString alloc] initWithBytes:path.data() length:path.size() * sizeof(wchar_t) encoding:NSUTF32LittleEndianStringEncoding];
            if (!p)
                return false;
            if (!p.absolutePath && ![NSFileManager.defaultManager fileExistsAtPath:p])
                if (NSString* inBundle = [NSBundle.mainBundle pathForResource:p ofType:nil])
                    p = inBundle;
            CGImageSourceRef src = CGImageSourceCreateWithURL((__bridge CFURLRef)[NSURL fileURLWithPath:p], nullptr);
            if (!src)
                return false;
            CGImageRef img = nullptr;
            if (maxSide > 0)
            {
                // a thumbnail no larger than maxSide on its longer side (never enlarged), EXIF orientation applied
                NSDictionary* o = @{(__bridge NSString*)kCGImageSourceCreateThumbnailFromImageAlways: @YES,
                                    (__bridge NSString*)kCGImageSourceCreateThumbnailWithTransform: @YES,
                                    (__bridge NSString*)kCGImageSourceThumbnailMaxPixelSize: @(maxSide)};
                img = CGImageSourceCreateThumbnailAtIndex(src, 0, (__bridge CFDictionaryRef)o);
            }
            else
                img = CGImageSourceCreateImageAtIndex(src, 0, nullptr);
            CFRelease(src);
            if (!img)
                return false;

            const int w = (int)CGImageGetWidth(img), h = (int)CGImageGetHeight(img);
            std::vector<std::uint8_t> rgba((std::size_t)w * (std::size_t)h * 4);
            CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
            CGContextRef ctx = CGBitmapContextCreate(rgba.data(), (size_t)w, (size_t)h, 8, (size_t)w * 4, srgb,
                                                     (CGBitmapInfo)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
            CGColorSpaceRelease(srgb);
            if (!ctx)
            {
                CGImageRelease(img);
                return false;
            }
            CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), img);
            CGContextRelease(ctx);
            CGImageRelease(img);
            // Core Graphics draws premultiplied: back to straight alpha
            for (std::size_t i = 0; i < rgba.size(); i += 4)
                if (const unsigned a = rgba[i + 3]; a > 0 && a < 255)
                    for (int c = 0; c < 3; ++c)
                        rgba[i + (std::size_t)c] = (std::uint8_t)std::min(255u, (rgba[i + (std::size_t)c] * 255u + a / 2) / a);
            out.width = w;
            out.height = h;
            out.rgba = std::move(rgba);
            return true;
        }
    }
}
