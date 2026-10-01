# workbench: Esia's data widgets (tables, trees, number, vector and color fields, the multi-line editor) in a dock space,
# in the frame of glass_window (glass_app).
if(NOT TARGET glass_app)
    message(STATUS "workbench: needs glass_window's frame - not built")
    return()
endif()
add_executable(workbench main.cpp)
target_link_libraries(workbench PRIVATE glass_app esia::ui)
esia_target_defaults(workbench)
if(COMMAND glass_ios_app)
    glass_ios_app(workbench "Esia Workbench" workbench)
endif()
