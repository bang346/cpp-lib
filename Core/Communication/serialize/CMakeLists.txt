cmake_minimum_required(VERSION 3.23)

add_library(serialize STATIC
    serialize.cpp
)

add_library(Communication::serialize ALIAS serialize)

target_include_directories(serialize PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(serialize PRIVATE
    Communication::binary_container
)

target_compile_features(serialize PUBLIC cxx_std_17)
