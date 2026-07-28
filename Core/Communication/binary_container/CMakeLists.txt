cmake_minimum_required(VERSION 3.23)

add_library(binary_container STATIC
    binary_container.cpp
)

add_library(Communication::binary_container ALIAS binary_container)

target_include_directories(binary_container PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(binary_container PUBLIC cxx_std_17)
