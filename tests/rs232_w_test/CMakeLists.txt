# ---------- rs232_w_test ----------
add_executable(rs232_w_test
    rs232_w_test.cpp
)
target_link_libraries(rs232_w_test PRIVATE
    gmock_main
    windows::rs232_w
    Communication::BusMaster
    Communication::coder
    Communication::frame
)
gtest_discover_tests(rs232_w_test)