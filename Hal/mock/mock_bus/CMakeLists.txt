add_library(mock_bus STATIC
    mock_bus.cpp
)

add_library(hal::mock_bus ALIAS mock_bus)

target_include_directories(mock_bus PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(mock_bus PUBLIC
    Interface::bus_if
    Interface::async_bus_if
    gmock
)

target_compile_features(mock_bus PUBLIC cxx_std_17)