# showcase: the widget layer (esia::ui) in a window, in the frame of glass_window (glass_app).
if(NOT TARGET glass_app)
    message(STATUS "showcase: needs glass_window's frame (the Win32 platform layer and a window-capable backend) - not built")
    return()
endif()
add_executable(showcase main.cpp)
target_link_libraries(showcase PRIVATE glass_app esia::ui)
esia_target_defaults(showcase)
