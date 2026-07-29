# ---------- test_communication ----------
add_executable(test_communication
    test_communication.cpp
)

target_link_libraries(test_communication PRIVATE
    gmock_main
    Communication::BusMaster
    Communication::coder
    hal::mock_spi
)

gtest_discover_tests(test_communication)
