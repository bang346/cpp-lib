cmake_minimum_required(VERSION 3.23)

add_library(stm32_spi STATIC
    stm32_spi.cpp
)

add_library(hal::stm32_spi ALIAS stm32_spi)

target_include_directories(stm32_spi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(stm32_spi PUBLIC cxx_std_17)
