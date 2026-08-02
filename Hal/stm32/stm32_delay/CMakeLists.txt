cmake_minimum_required(VERSION 3.23)

add_library(stm32_delay STATIC
    stm32_delay.cpp
)

add_library(hal::stm32_delay ALIAS stm32_delay)

target_include_directories(stm32_delay PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(stm32_delay PUBLIC
    Interface::delay_if
    stm32cubemx
)

target_compile_features(stm32_delay PUBLIC cxx_std_17)
