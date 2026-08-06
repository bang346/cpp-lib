# Adapt the target name and paths to your tests/CMakeLists.txt.
add_executable(test_BusMaster
    test_BusMaster.cpp
)

target_link_libraries(test_BusMaster
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::binary_container
    Communication::frame
    Interface::bus_if
    etl::etl
)

target_compile_features(test_BusMaster PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(test_BusMaster)