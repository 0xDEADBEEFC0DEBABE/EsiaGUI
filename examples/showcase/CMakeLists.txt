# showcase: WGT's demo on Esia (its panels, the dock, the island, the status bar), in the frame of glass_window
# (glass_app). Captures compare with WGT's reference screenshots (branch reference/wgt-1.1).
if(NOT TARGET glass_app)
    message(STATUS "showcase: needs glass_window's frame (the Win32 platform layer and a window-capable backend) - not built")
    return()
endif()
add_executable(showcase main.cpp image_file.cpp image_file.hpp)
target_link_libraries(showcase PRIVATE glass_app esia::ui windowscodecs ole32)
esia_target_defaults(showcase)
