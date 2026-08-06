cmake_minimum_required(VERSION 3.23)

add_library(async_bus_if STATIC
    async_bus_if.cpp
)

add_library(Interface::async_bus_if ALIAS async_bus_if)

target_include_directories(async_bus_if PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(async_bus_if PUBLIC cxx_std_17)
