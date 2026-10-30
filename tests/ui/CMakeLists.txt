# The widget layer's tests (src/esia/ui), without a GPU: tests/CMakeLists.txt adds this directory with the layer.
esia_add_test(esia_ui_tests test_ui.cpp test_data_widgets.cpp test_plot.cpp)
target_link_libraries(esia_ui_tests PRIVATE esia::ui)
target_include_directories(esia_ui_tests PRIVATE ${ESIA_ROOT}/src/esia/ui)   # ui_internal.hpp (detail::FormatV, detail::PlotTicks)

# the draw memos against drawing every widget (UiDesc::drawMemo): the FreeType text system with the test fonts
if(TARGET esia_text_ft)
    esia_add_test(esia_ui_memo_tests test_draw_memo.cpp)
    target_link_libraries(esia_ui_memo_tests PRIVATE esia::ui esia_text_ft)
    target_include_directories(esia_ui_memo_tests PRIVATE ${ESIA_ROOT}/src/esia/ui)   # ui_internal.hpp (the memo mode)
    target_compile_definitions(esia_ui_memo_tests PRIVATE ESIA_TEST_FONT_DIR="${ESIA_ROOT}/tests/fonts")
    set_tests_properties(esia_ui_memo_tests PROPERTIES ENVIRONMENT "ESIA_TEXT_RENDERING=;ESIA_UI_MEMO=")
endif()
