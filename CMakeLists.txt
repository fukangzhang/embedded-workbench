cmake_minimum_required(VERSION 3.20)

project(embedded_workbench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_library(embedded_app
    app/src/app_info.c
)

target_include_directories(embedded_app
    PUBLIC
        app/include
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
