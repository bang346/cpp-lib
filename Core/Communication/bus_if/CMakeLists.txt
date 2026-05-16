cmake_minimum_required(VERSION 3.23)

add_library(bus_if STATIC
    bus_if.cpp
)

add_library(Communication::bus_if ALIAS bus_if)

target_include_directories(bus_if PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(bus_if PUBLIC cxx_std_17)
