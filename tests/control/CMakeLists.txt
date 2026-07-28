# ---------- PID ----------
add_executable(test_control
    test_control.cpp
)
target_link_libraries(test_control PRIVATE
    control::pid
    control::cntr_system
    helper::csv_parser
    GTest::gtest_main
)

target_compile_definitions(test_control PRIVATE
    TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/GRK_input_output.csv "
    TEST_DATA_PIDF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/GRK_input_output_pidf.csv"
)
gtest_discover_tests(test_control)