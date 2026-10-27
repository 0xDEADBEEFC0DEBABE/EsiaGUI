# The widget layer's tests (src/esia/ui), without a GPU: tests/CMakeLists.txt adds this directory with the layer.
esia_add_test(esia_ui_tests test_ui.cpp test_data_widgets.cpp test_plot.cpp)
target_link_libraries(esia_ui_tests PRIVATE esia::ui)
target_include_directories(esia_ui_tests PRIVATE ${ESIA_ROOT}/src/esia/ui)   # ui_internal.hpp (detail::FormatV, detail::PlotTicks)
