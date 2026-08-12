# ---------- rs232_w_test ----------
add_executable(rs232_w_test
    rs232_w_test.cpp
)
target_link_libraries(rs232_w_test PRIVATE
    gmock_main
    windows::rs232_w
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::frame
    helper::prittyprinting
)
gtest_discover_tests(rs232_w_test)