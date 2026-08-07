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

gtest_discover_tests(test_BusMaster)

# Adapt the target name and paths to your tests/CMakeLists.txt.
add_executable(test_BusMasterOwn
    test_BusMasterOwn.cpp
)

target_link_libraries(test_BusMasterOwn
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::binary_container
    Communication::frame
    hal::mock_bus
    hal::mock_spi
    etl::etl
)

target_compile_features(test_BusMasterOwn PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(test_BusMasterOwn)

# Adapt the target name and paths to your tests/CMakeLists.txt.
add_executable(test_BusMasterComplete
    test_BusMasterComplete.cpp
)

target_link_libraries(test_BusMasterComplete
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    Communication::BusMasterTransmit
    Communication::BusMasterReceive
    Communication::binary_container
    Communication::frame
    hal::mock_bus
    hal::mock_spi
    etl::etl
)

target_compile_features(test_BusMasterComplete PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(test_BusMasterComplete)