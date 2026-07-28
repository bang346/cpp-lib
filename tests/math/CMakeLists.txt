# ---------- Math ----------
add_executable(test_math
    test_math.cpp
)
target_link_libraries(test_math PRIVATE
    math::clark_park
    GTest::gtest_main
)
gtest_discover_tests(test_math)
