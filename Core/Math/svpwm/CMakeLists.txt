cmake_minimum_required(VERSION 3.23)

add_library(svpwm STATIC
    svpwm.cpp
)

add_library(math::svpwm ALIAS svpwm)

target_include_directories(svpwm PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(svpwm PUBLIC cxx_std_17)
