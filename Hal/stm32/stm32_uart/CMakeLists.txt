cmake_minimum_required(VERSION 3.23)

add_library(stm32_uart STATIC
    stm32_uart.cpp
)

add_library(hal::stm32_uart ALIAS stm32_uart)

target_include_directories(stm32_uart PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(stm32_uart PUBLIC
    Interface::bus_if
    stm32cubemx
)

target_compile_features(stm32_uart PUBLIC cxx_std_17)
