cmake_minimum_required(VERSION 3.16)
project(cpp_lib LANGUAGES CXX)

add_subdirectory(Core/Math/crc)

# add_subdirectory(... weitere module ...)
option(CPPLIB_BUILD_TESTS "Build cpp-lib unit tests" OFF)

if(PROJECT_IS_TOP_LEVEL AND CPPLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
