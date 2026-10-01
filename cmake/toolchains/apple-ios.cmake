# iOS on Apple silicon iPhones and iPads (arm64), with Xcode's clang and the iPhoneOS SDK. The examples become
# app bundles, signed after the link when the Mac has a provisioning profile for them (tools/ios/codesign.py).
#
#   cmake --preset ios && cmake --build --preset ios
#   xcrun devicectl device install app --device <name or UDID> build/ios/bin/showcase.app
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET 16.0 CACHE STRING "")
execute_process(COMMAND xcrun --sdk iphoneos --show-sdk-path OUTPUT_VARIABLE _esia_sdk OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT _esia_sdk)
    message(FATAL_ERROR "iOS: no iPhoneOS SDK (install Xcode and select it: sudo xcode-select -s /Applications/Xcode.app)")
endif()
# by name: CMake picks the target from it (-mios-simulator-version-min for the simulator)
set(CMAKE_OSX_SYSROOT ${ESIA_IOS_PLATFORM} CACHE STRING "")
# Xcode's clang, not Homebrew LLVM's: the SDK's frameworks and Apple's linker are what an app is built with
execute_process(COMMAND xcrun --sdk iphoneos -f clang OUTPUT_VARIABLE _esia_cc OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND xcrun --sdk iphoneos -f clang++ OUTPUT_VARIABLE _esia_cxx OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_C_COMPILER ${_esia_cc})
set(CMAKE_CXX_COMPILER ${_esia_cxx})
set(CMAKE_OBJCXX_COMPILER ${_esia_cxx})
# try_compile builds static libraries: an executable would need signing
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
