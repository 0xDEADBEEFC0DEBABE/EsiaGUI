// Esia - a MOCK of the few Foundation declarations metal_device.mm uses, for the offline Objective-C++ check on hosts
// without Apple's SDK (the esia_rhi_metal_objc_mock_check test, ../../CMakeLists.txt). Not Apple's header: written
// from Apple's documentation; see Metal/Metal.h for what the check does and does not prove.
// UNVERIFIED: needs macOS - a Linux-only stand-in for Apple's header.
#pragma once
#include <cstddef>
#include <cstdint>

typedef unsigned long NSUInteger;
typedef long NSInteger;
typedef bool BOOL;
#define YES true
#define NO false
#define nil nullptr
#define NS_ENUM(type, name) enum name : type name; enum name : type
#define NS_OPTIONS(type, name) enum name : type name; enum name : type
#define API_AVAILABLE(...)

typedef struct _NSRange
{
    NSUInteger location;
    NSUInteger length;
} NSRange;
inline NSRange NSMakeRange(NSUInteger loc, NSUInteger len) { return NSRange{loc, len}; }

@protocol NSObject
@end

__attribute__((objc_root_class))
@interface NSObject <NSObject>
+ (instancetype)new;
+ (instancetype)alloc;
- (instancetype)init;
@end

@interface NSString : NSObject
+ (instancetype)stringWithUTF8String:(const char*)s;
@property (readonly) const char* UTF8String;
- (BOOL)isEqualToString:(NSString*)other;
@end

@interface NSError : NSObject
@property (readonly, copy) NSString* localizedDescription;
@end

@interface NSData : NSObject
@property (readonly) const void* bytes;
@property (readonly) NSUInteger length;
@end

typedef struct
{
    unsigned long state;
    __unsafe_unretained id* itemsPtr;
    unsigned long* mutationsPtr;
    unsigned long extra[5];
} NSFastEnumerationState;

@protocol NSFastEnumeration
- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState*)state objects:(__unsafe_unretained id[])buffer count:(NSUInteger)len;
@end

@interface NSArray<ObjectType> : NSObject <NSFastEnumeration>
@property (readonly) NSUInteger count;
@end
