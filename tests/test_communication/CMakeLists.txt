# ---------- test_communication ----------
add_executable(test_communication
    test_communication.cpp
)

target_link_libraries(test_communication PRIVATE
    gmock_main
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::coder
    hal::mock_spi
    Communication::frame
    hal::mock_spi
)

gtest_discover_tests(test_communication)

# ---------- testFrameV1 ----------
add_executable(testFrameV1
    testFrameV1.cpp
)

target_link_libraries(testFrameV1 PRIVATE
    gmock_main
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::coder
    hal::mock_spi
    Communication::frame
)

gtest_discover_tests(testFrameV1)
