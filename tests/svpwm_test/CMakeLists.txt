# ---------- SVPWM ----------
add_executable(test_svpwm
    test_svpwm.cpp
)
target_link_libraries(test_svpwm PRIVATE
    math::svpwm
    GTest::gtest_main
)
gtest_discover_tests(test_svpwm)