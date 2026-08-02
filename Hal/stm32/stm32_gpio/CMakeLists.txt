cmake_minimum_required(VERSION 3.23)

add_library(stm32_gpio STATIC EXCLUDE_FROM_ALL
    stm32_gpio.cpp
)

add_library(hal::stm32_gpio ALIAS stm32_gpio)

target_include_directories(stm32_gpio PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(stm32_gpio PUBLIC
    Interface::gpio_if
    stm32cubemx
)

target_compile_features(stm32_gpio PUBLIC cxx_std_17)
