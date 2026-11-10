# This backend is only linked into the test executable; it is never installed.
add_library(tm1637_test_wiringpi SHARED fakeWiringPi.cpp)
set_target_properties(tm1637_test_wiringpi PROPERTIES OUTPUT_NAME wiringPi)
target_link_libraries(tm1637_test_wiringpi PRIVATE Threads::Threads)

add_executable(tm1637_device_tests device_tests.cpp)
target_include_directories(tm1637_device_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_device_tests PRIVATE TM1637Pi tm1637_test_wiringpi dl)
add_test(NAME device_protocol COMMAND tm1637_device_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_traces.txt)
add_test(NAME device_protocol_worker COMMAND tm1637_device_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_traces.txt --worker)

add_executable(tm1637_command_tests command_tests.cpp)
target_include_directories(tm1637_command_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_command_tests PRIVATE TM1637Pi dl)
add_test(NAME command_batches COMMAND tm1637_command_tests)

add_executable(tm1637_worker_tests worker_tests.cpp)
target_include_directories(tm1637_worker_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_worker_tests PRIVATE TM1637Pi Threads::Threads)
add_test(NAME worker_lifecycle COMMAND tm1637_worker_tests)
set_tests_properties(worker_lifecycle PROPERTIES TIMEOUT 20)

add_library(tm1637_test_gpiod SHARED fakeGpiod.cpp)
set_target_properties(tm1637_test_gpiod PROPERTIES OUTPUT_NAME gpiod SOVERSION 3)
add_executable(tm1637_backend_tests backend_tests.cpp)
target_include_directories(tm1637_backend_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(tm1637_backend_tests PRIVATE TM1637Pi tm1637_test_gpiod ${CMAKE_DL_LIBS})
add_test(NAME backend_cleanup COMMAND tm1637_backend_tests)

add_test(NAME say_synchronous COMMAND tm1637_say
    --gpio-lib GpioWiringPi --delay-time 5 --count 1 A)
set_tests_properties(say_synchronous PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:tm1637_test_wiringpi>"
    TIMEOUT 10)
