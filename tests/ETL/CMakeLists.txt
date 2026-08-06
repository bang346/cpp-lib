# ---------- test_circular_buffer ----------
add_executable(test_circular_buffer
    test_circular_buffer.cpp
)
target_link_libraries(test_circular_buffer PRIVATE
    gmock_main
    etl::etl
    helper::prittyprinting
)
gtest_discover_tests(test_circular_buffer)