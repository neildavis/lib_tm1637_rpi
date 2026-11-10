# This backend is only linked into the test executable; it is never installed.
add_library(tm1637_test_wiringpi SHARED fakeWiringPi.cpp)
set_target_properties(tm1637_test_wiringpi PROPERTIES OUTPUT_NAME wiringPi)

add_executable(tm1637_device_tests device_tests.cpp)
target_include_directories(tm1637_device_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_device_tests PRIVATE TM1637Pi tm1637_test_wiringpi dl)
add_test(NAME device_protocol COMMAND tm1637_device_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_traces.txt)

add_executable(tm1637_command_tests command_tests.cpp)
target_include_directories(tm1637_command_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_command_tests PRIVATE TM1637Pi dl)
add_test(NAME command_batches COMMAND tm1637_command_tests)
