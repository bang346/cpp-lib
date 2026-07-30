cmake_minimum_required(VERSION 3.23)

add_library(coder STATIC
    coder.cpp
)

add_library(Communication::coder ALIAS coder)

target_include_directories(coder PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(coder PUBLIC
    math::crc
    Interface::coder_if
)

target_compile_features(coder PUBLIC cxx_std_17)
