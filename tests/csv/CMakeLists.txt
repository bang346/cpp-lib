# ---------- CSV ----------
add_executable(test_csv
    test_csv.cpp
)
target_link_libraries(test_csv PRIVATE
    helper::csv_parser
    GTest::gtest_main
)

target_compile_definitions(test_csv PRIVATE
    TEST_CSV_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data.csv"
)

gtest_discover_tests(test_csv)