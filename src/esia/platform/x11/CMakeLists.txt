# Esia - the X11 platform layer (ESIA_PLATFORM_X11): a window's events -> the core's input, PlatformRequests -> the
# window (cursor), the CLIPBOARD selection, the input method (XIM), the desktop's scale and an optional app window
# (include/esia/platform/x11.hpp). Depends on the core only.
find_package(X11)
if(NOT TARGET X11::X11)
    message(STATUS "Esia: the X11 platform layer needs Xlib (libx11-dev) - not built")
    return()
endif()
add_library(esia_platform_x11 STATIC x11_platform.cpp)
target_link_libraries(esia_platform_x11 PUBLIC esia_core PRIVATE X11::X11)
esia_target_defaults(esia_platform_x11)
add_library(esia::platform_x11 ALIAS esia_platform_x11)
