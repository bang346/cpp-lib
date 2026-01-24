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

# ---------- CSV ----------
add_executable(test_csv
  csv/test_csv.cpp
)
target_link_libraries(test_csv PRIVATE
  helper::csv_parser
  GTest::gtest_main
)

target_compile_definitions(test_csv PRIVATE
  TEST_CSV_PATH="${CMAKE_CURRENT_SOURCE_DIR}/csv/data.csv"
)

gtest_discover_tests(test_csv)

# ---------- PID ----------
add_executable(test_control
  control/test_control.cpp
)
target_link_libraries(test_control PRIVATE
  control::pid
  control::cntr_system
  helper::csv_parser
  GTest::gtest_main
)
gtest_discover_tests(test_control)
