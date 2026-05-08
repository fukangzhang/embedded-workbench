cmake_minimum_required(VERSION 3.20)

project(embedded_workbench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_library(embedded_drivers
    drivers/src/sensor_sample.c
)

target_include_directories(embedded_drivers
    PUBLIC
        drivers/include
)

add_library(embedded_app
    app/src/app_info.c
    app/src/alarm_state.c
)

target_include_directories(embedded_app
    PUBLIC
        app/include
)

target_link_libraries(embedded_app
    PUBLIC
        embedded_drivers
)

add_executable(host_sim
    tools/host_sim/main.c
)

target_link_libraries(host_sim
    PRIVATE
        embedded_app
)

add_executable(test_app_info
    tests/test_app_info.c
)

target_link_libraries(test_app_info
    PRIVATE
        embedded_app
)

add_test(NAME app_info COMMAND test_app_info)

add_executable(test_sensor_sample
    tests/test_sensor_sample.c
)

target_link_libraries(test_sensor_sample
    PRIVATE
        embedded_drivers
)

add_test(NAME sensor_sample COMMAND test_sensor_sample)

add_executable(test_alarm_state
    tests/test_alarm_state.c
)

target_link_libraries(test_alarm_state
    PRIVATE
        embedded_app
)

add_test(NAME alarm_state COMMAND test_alarm_state)
