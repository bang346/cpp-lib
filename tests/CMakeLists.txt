include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ---------- CRC ----------
add_executable(test_crc
  crc/test_crc.cpp
)
target_link_libraries(test_crc PRIVATE
  math::crc
  GTest::gtest_main
)
gtest_discover_tests(test_crc)

# # ---------- SPI_IF ----------
# add_executable(test_spi_if
# spi_if/test_spi_if.cpp
# )
# target_link_libraries(test_spi_if PRIVATE
# yourlib::spi_if
# GTest::gtest_main
# )
# gtest_discover_tests(test_spi_if)
