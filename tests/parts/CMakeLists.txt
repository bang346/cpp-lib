# ---------- PARTS ----------
add_executable(test_parts
    test_parts.cpp
)

target_link_libraries(test_parts PRIVATE
    parts::DRV8353
    helper::prittyprinting
    hal::mock_spi
    gmock_main
)

gtest_discover_tests(test_parts)