cmake_minimum_required(VERSION 3.23)

add_library(BusMasterReceive STATIC
    BusMasterReceive.cpp
)

add_library(Communication::BusMasterReceive ALIAS BusMasterReceive)

target_include_directories(BusMasterReceive PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(BusMasterReceive PUBLIC
    Communication::binary_container
    Communication::archive
    math::crc
    Interface::bus_if
    Interface::coder_if
    Communication::frame
    etl::etl
)

target_compile_features(BusMasterReceive PUBLIC cxx_std_17)
