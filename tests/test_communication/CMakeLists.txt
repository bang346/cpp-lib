# ---------- test_communication ----------
add_executable(test_communication
    test_communication.cpp
)

target_link_libraries(test_communication PRIVATE
    gmock_main
    Communication::archive
    Communication::binary_container
    Communication::serialize
)

gtest_discover_tests(test_communication)
