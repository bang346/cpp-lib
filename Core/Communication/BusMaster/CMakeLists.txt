cmake_minimum_required(VERSION 3.23)

add_library(BusMaster STATIC
    BusMaster.cpp
)

add_library(Communication::BusMaster ALIAS BusMaster)

target_include_directories(BusMaster PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(BusMaster PUBLIC
    Communication::binary_container
    Communication::archive
    math::crc
    Interface::bus_if
    Interface::coder_if
    Communication::frame
)
target_compile_features(BusMaster PUBLIC cxx_std_17)
