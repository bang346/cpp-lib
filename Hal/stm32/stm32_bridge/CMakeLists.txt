cmake_minimum_required(VERSION 3.23)

add_library(stm32_bridge STATIC
    stm32_bridge.cpp
)

add_library(hal::stm32_bridge ALIAS stm32_bridge)

target_include_directories(stm32_bridge PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(stm32_bridge PUBLIC cxx_std_17)
