cmake_minimum_required(VERSION 3.23)

add_library(crc STATIC
    crc.cpp
)

add_library(math::crc ALIAS crc)

target_include_directories(crc PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(crc PUBLIC cxx_std_17)
