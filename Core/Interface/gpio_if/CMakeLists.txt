cmake_minimum_required(VERSION 3.23)

add_library(gpio_if STATIC
    gpio_if.cpp
)

add_library(Interface::gpio_if ALIAS gpio_if)

target_include_directories(gpio_if PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(gpio_if PUBLIC cxx_std_17)
