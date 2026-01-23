cmake_minimum_required(VERSION 3.23)

add_library(pid STATIC
    pid.cpp
)

add_library(control::pid ALIAS pid)

target_include_directories(pid PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(pid PUBLIC cxx_std_17)
