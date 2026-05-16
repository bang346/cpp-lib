cmake_minimum_required(VERSION 3.23)

add_library(delay_if STATIC
    delay_if.cpp
)

add_library(Interface::delay_if ALIAS delay_if)

target_include_directories(delay_if PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(delay_if PUBLIC cxx_std_17)
