cmake_minimum_required(VERSION 3.23)

add_library(cntr_system STATIC
    cntr_system.cpp
)

add_library(control::cntr_system ALIAS cntr_system)

target_include_directories(cntr_system PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(cntr_system PUBLIC cxx_std_17)
