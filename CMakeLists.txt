cmake_minimum_required(VERSION 3.16)
project(cpp_lib LANGUAGES CXX)

# -------------------------------------------------
# Embedded Template Library (ETL)
# -------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    etl
    GIT_REPOSITORY https://github.com/ETLCPP/etl
    GIT_TAG 20.48.1
)

FetchContent_MakeAvailable(etl)

# -------------------------------------------------
# Target platform
# -------------------------------------------------
set(
    CPPLIB_TARGET
    "AUTO"
    CACHE STRING
    "Target platform: AUTO, STM32, LINUX or WINDOWS"
)

# Zeigt in CMake-GUIs eine Auswahlliste an
set_property(
    CACHE CPPLIB_TARGET
    PROPERTY STRINGS
    AUTO
    STM32
    LINUX
    WINDOWS
)

# Eingabe unabhängig von Groß-/Kleinschreibung machen
string(TOUPPER "${CPPLIB_TARGET}" CPPLIB_TARGET)

# Zielsystem automatisch bestimmen
if(CPPLIB_TARGET STREQUAL "AUTO")
    if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
        set(CPPLIB_TARGET "STM32")

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CPPLIB_TARGET "WINDOWS")

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CPPLIB_TARGET "LINUX")

    else()
        message(
            FATAL_ERROR
            "Could not detect target platform from "
            "CMAKE_SYSTEM_NAME='${CMAKE_SYSTEM_NAME}'. "
            "Set CPPLIB_TARGET manually."
        )
    endif()
endif()

# Eingabe validieren
set(CPPLIB_VALID_TARGETS
    STM32
    LINUX
    WINDOWS
)

if(NOT CPPLIB_TARGET IN_LIST CPPLIB_VALID_TARGETS)
    message(
        FATAL_ERROR
        "Invalid CPPLIB_TARGET='${CPPLIB_TARGET}'. "
        "Valid targets are: STM32, LINUX, WINDOWS."
    )
endif()

message(STATUS "cpp-lib target platform: ${CPPLIB_TARGET}")

add_subdirectory(Core)

# -------------------------------------------------
# Platform-specific implementations
# -------------------------------------------------
if(CPPLIB_TARGET STREQUAL "STM32")
    message(STATUS "Building STM32 HAL adapters")

    add_subdirectory(Hal/stm32)

elseif(CPPLIB_TARGET STREQUAL "LINUX")
    message(STATUS "Building Linux platform adapters")

elseif(CPPLIB_TARGET STREQUAL "WINDOWS")
    message(STATUS "Building Windows platform adapters")

    add_subdirectory(Hal/windows)
endif()

# -------------------------------------------------
# Unit tests
# -------------------------------------------------
option(
    CPPLIB_BUILD_TESTS
    "Build cpp-lib unit tests"
    OFF
)

option(
    CPPLIB_BUILD_HARDWARETESTS
    "Run Hardware Tests"
    OFF
)

if(CPPLIB_BUILD_TESTS)
    if(CPPLIB_TARGET STREQUAL "STM32")
        message(
            FATAL_ERROR
            "CPPLIB_BUILD_TESTS cannot be enabled for the STM32 target. "
            "Build the tests with CPPLIB_TARGET=WINDOWS or LINUX."
        )
    endif()

    FetchContent_Declare(
        googletest
        URL
        https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(
        gtest_force_shared_crt
        ON
        CACHE BOOL
        ""
        FORCE
    )

    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    enable_testing()

    add_subdirectory(Hal/mock/mock_spi)
    add_subdirectory(Helper)

    if(CPPLIB_BUILD_HARDWARETESTS)
        add_subdirectory(tests/Hardware)
    endif()

    add_subdirectory(tests)
endif()