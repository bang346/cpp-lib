cmake_minimum_required(VERSION 3.23)

# ---------- CRC ----------
add_executable(test_crc
  crc/test_crc.cpp
)
target_link_libraries(test_crc PRIVATE
  math::crc
  GTest::gtest_main
)
gtest_discover_tests(test_crc)

# ---------- Math ----------
add_executable(test_math
  math_test/test_math.cpp
)
target_link_libraries(test_math PRIVATE
  math::clark_park
  GTest::gtest_main
)
gtest_discover_tests(test_math)

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

target_compile_definitions(test_control PRIVATE
  TEST_DATA_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/control/GRK_input_output.csv\"
  TEST_DATA_PIDF_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/control/GRK_input_output_pidf.csv\"
)
gtest_discover_tests(test_control)

# ---------- SVPWM ----------
add_executable(test_svpwm
  svpwm_test/test_svpwm.cpp
)
target_link_libraries(test_svpwm PRIVATE
  math::svpwm
  GTest::gtest_main
)
gtest_discover_tests(test_svpwm)

# ---------- PARTS ----------
add_executable(test_parts
  parts/test_parts.cpp
)

target_link_libraries(test_parts PRIVATE
  parts::DRV8353
  helper::prittyprinting
  hal::mock_spi
  gmock_main
)

gtest_discover_tests(test_parts)