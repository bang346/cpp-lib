# ---------- CRC ----------
add_executable(test_crc
    test_crc.cpp
)
target_link_libraries(test_crc PRIVATE
    math::crc
    GTest::gtest_main
)
gtest_discover_tests(test_crc)