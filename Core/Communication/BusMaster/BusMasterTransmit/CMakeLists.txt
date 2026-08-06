cmake_minimum_required(VERSION 3.23)

add_library(BusMasterTransmit STATIC
    BusMasterTransmit.cpp
)

add_library(Communication::BusMasterTransmit ALIAS BusMasterTransmit)

target_include_directories(BusMasterTransmit PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(BusMasterTransmit PUBLIC
    Communication::binary_container
    Communication::archive
    math::crc
    Interface::bus_if
    Interface::coder_if
    Communication::frame
    etl::etl
    Interface::async_bus_if
)

target_compile_features(BusMasterTransmit PUBLIC cxx_std_17)
