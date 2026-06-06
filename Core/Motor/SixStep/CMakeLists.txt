cmake_minimum_required(VERSION 3.23)

add_library(sixstep STATIC
    sixstep.cpp
)

add_library(motor::sixstep ALIAS sixstep)

target_include_directories(sixstep PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(sixstep PUBLIC cxx_std_17)
