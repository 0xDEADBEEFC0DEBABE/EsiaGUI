# showcase: WGT's demo on Esia (its panels, the dock, the island, the status bar), in the frame of glass_window
# (glass_app). Captures compare with WGT's reference screenshots (branch reference/wgt-1.1).
if(NOT TARGET glass_app)
    message(STATUS "showcase: needs glass_window's frame (Win32 + a window-capable backend, or macOS + Metal) - not built")
    return()
endif()
if(APPLE)
    enable_language(OBJCXX)   # per directory: the rules for .mm files
    add_executable(showcase main.cpp image_file_apple.mm image_file.hpp)
    set_source_files_properties(image_file_apple.mm PROPERTIES COMPILE_OPTIONS "${ESIA_METAL_OBJC_OPTIONS}")
    target_link_libraries(showcase PRIVATE glass_app esia::ui "-framework ImageIO" "-framework CoreGraphics" "-framework Foundation")
    # iOS: the wallpaper is one of the building Mac's (iOS has none an app can read), a JPEG of 2560 pixels in the bundle
    set(_wallpaper)
    if(IOS)
        foreach(_source "/System/Library/Desktop Pictures/Sonoma.heic" "/System/Library/Desktop Pictures/Mac Blue.heic")
            if(EXISTS "${_source}")
                set(_wallpaper ${CMAKE_CURRENT_BINARY_DIR}/wallpaper.jpg)
                add_custom_command(OUTPUT ${_wallpaper}
                                   COMMAND sips -s format jpeg -s formatOptions 90 -Z 2560 ${_source} --out ${_wallpaper}
                                   DEPENDS ${_source} VERBATIM)
                target_sources(showcase PRIVATE ${_wallpaper})
                break()
            endif()
        endforeach()
    endif()
    glass_ios_app(showcase "Esia Showcase" showcase RESOURCES ${_wallpaper})
else()
    add_executable(showcase main.cpp image_file.cpp image_file.hpp)
    target_link_libraries(showcase PRIVATE glass_app esia::ui windowscodecs ole32)
endif()
esia_target_defaults(showcase)
