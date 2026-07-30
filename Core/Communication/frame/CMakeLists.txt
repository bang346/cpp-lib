cmake_minimum_required(VERSION 3.23)

add_library(frame STATIC
    frame.cpp
)

add_library(Communication::frame ALIAS frame)

target_include_directories(frame PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(frame PUBLIC
    Communication::binary_container
    Interface::coder_if
    Communication::archive
    Communication::coder
)

target_compile_features(frame PUBLIC cxx_std_17)
