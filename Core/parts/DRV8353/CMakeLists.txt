cmake_minimum_required(VERSION 3.23)

add_library(DRV8353 STATIC
    DRV8353.cpp
)

add_library(parts::DRV8353 ALIAS DRV8353)

target_include_directories(DRV8353 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(DRV8353 PRIVATE Interface::bus_if)

target_compile_features(DRV8353 PUBLIC cxx_std_17)
