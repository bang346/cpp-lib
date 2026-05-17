cmake_minimum_required(VERSION 3.23)

add_library(prittyprinting STATIC
    prittyprinting.cpp
)

add_library(helper::prittyprinting ALIAS prittyprinting)

target_include_directories(prittyprinting PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(prittyprinting PUBLIC cxx_std_17)
