# ---------- Motor ----------
add_executable(test_motor
    test_motor.cpp
)

target_link_libraries(test_motor PRIVATE
    motor::sixstep
    gmock_main
)

gtest_discover_tests(test_motor)