add_library(mock_spi STATIC
    mock_spi.cpp
)

add_library(hal::mock_spi ALIAS mock_spi)

target_include_directories(mock_spi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(mock_spi PUBLIC
    Interface::bus_if
    gmock
)

target_compile_features(mock_spi PUBLIC cxx_std_17)