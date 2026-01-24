cmake_minimum_required(VERSION 3.23)
include(FetchContent)

# Fetch csv-parser and make it available to my-executable
FetchContent_Declare(
    csv
    GIT_REPOSITORY https://github.com/vincentlaucsb/csv-parser.git
    GIT_SHALLOW TRUE
    GIT_TAG 2.2.1
)

FetchContent_MakeAvailable(csv)

add_library(csv_parser STATIC
    csv_parser.cpp
)
message(" Zeile: " ${CMAKE_CURRENT_LIST_DIR})
add_library(helper::csv_parser ALIAS csv_parser)

target_include_directories(csv_parser
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(csv_parser
    PUBLIC
    csv
)
target_compile_features(csv_parser PUBLIC cxx_std_17)
