cmake_minimum_required(VERSION 3.23)

add_library(clark_park STATIC
    clark_park.cpp
)

add_library(math::clark_park ALIAS clark_park)

target_include_directories(clark_park PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(clark_park PUBLIC cxx_std_17)
