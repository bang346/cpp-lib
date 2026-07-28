cmake_minimum_required(VERSION 3.23)

add_library(archive STATIC
    archive.cpp
)

add_library(Communication::archive ALIAS archive)

target_include_directories(archive PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(archive PUBLIC cxx_std_17)
