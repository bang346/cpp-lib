cmake_minimum_required(VERSION 3.23)

add_library(coder STATIC
    coder.cpp
)

add_library(Communication::coder ALIAS coder)

target_include_directories(coder PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(coder PUBLIC
    Interface::coder_if
    math::crc
)

target_compile_features(coder PUBLIC cxx_std_17)
