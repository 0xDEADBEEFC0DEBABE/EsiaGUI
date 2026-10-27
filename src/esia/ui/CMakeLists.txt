# The widget layer (include/esia/ui): theme, springs, styles, auto layout, controls, containers, lists, data widgets and
# docking, drawn with the Painter - EsiaGUI's liquid glass on Esia's core and renderer. src/esia/CMakeLists.txt adds this
# directory when it exists.
add_library(esia_ui STATIC
    theme.cpp
    anim.cpp
    ui.cpp
    controls.cpp
    layout.cpp
    containers.cpp
    selection.cpp
    lists.cpp
    navigation.cpp
    text_field.cpp
    number.cpp
    color.cpp
    table.cpp
    text_editor.cpp
    dock.cpp
    popups.cpp
    charts.cpp
    plot.cpp
    overlays.cpp)
target_link_libraries(esia_ui PUBLIC esia_render esia_text)
esia_target_defaults(esia_ui)
add_library(esia::ui ALIAS esia_ui)
