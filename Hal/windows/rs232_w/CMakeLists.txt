cmake_minimum_required(VERSION 3.23)

add_library(rs232_w STATIC
    rs232_w.cpp
)

add_library(windows::rs232_w ALIAS rs232_w)

target_include_directories(rs232_w PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(rs232_w PUBLIC
    Interface::bus_if
)
target_compile_features(rs232_w PUBLIC cxx_std_17)
