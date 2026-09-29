# Esia - the Win32 platform layer (ESIA_PLATFORM_WIN32): window messages -> the core's input, PlatformRequests -> the
# window (cursor, IME, capture flags), the clipboard, per-monitor DPI and an optional app window
# (include/esia/platform/win32.hpp). Depends on the core only; the render backends know nothing of it.
add_library(esia_platform_win32 STATIC
    win32_platform.cpp
    win32_window.cpp)
target_link_libraries(esia_platform_win32 PUBLIC esia_core PRIVATE user32 imm32 dwmapi)
# Windows 10 APIs (per-monitor v2 DPI), wide-character Win32
target_compile_definitions(esia_platform_win32 PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
esia_target_defaults(esia_platform_win32)
add_library(esia::platform_win32 ALIAS esia_platform_win32)

if(BUILD_TESTING)
    # drives a real (hidden) window with the messages Windows would send; needs a desktop (under Wine: an X server)
    esia_add_test(esia_platform_win32_tests tests/test_win32_platform.cpp)
    target_link_libraries(esia_platform_win32_tests PRIVATE esia_platform_win32 user32 imm32)
    target_compile_definitions(esia_platform_win32_tests PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)
endif()
