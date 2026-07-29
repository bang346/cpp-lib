cmake_minimum_required(VERSION 3.23)

add_library(coder_if STATIC
    coder_if.cpp
)

add_library(Interface::coder_if ALIAS coder_if)

target_include_directories(coder_if PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(coder_if PUBLIC cxx_std_17)
