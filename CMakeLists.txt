cmake_minimum_required(VERSION 3.16)
project(cpp_lib LANGUAGES CXX)

# Math
add_subdirectory(Core/Math/crc)
add_subdirectory(Core/Math/clark_park)
add_subdirectory(Core/Math/svpwm)

# Control
add_subdirectory(Core/Control/pid)
add_subdirectory(Core/Control/cntr_system)

# HAL
add_subdirectory(Hal/stm32/stm32_spi)

# Helper
add_subdirectory(Helper/csv_parser)

# add_subdirectory(... weitere module ...)
option(CPPLIB_BUILD_TESTS "Build cpp-lib unit tests" OFF)

# To build with tests run: cmake -S . -B build -DCPPLIB_BUILD_TESTS=ON
if(PROJECT_IS_TOP_LEVEL AND CPPLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
