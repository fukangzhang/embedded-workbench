cmake_minimum_required(VERSION 3.20)

project(embedded_workbench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(EW_BUILD_HOST_TOOLS "Build host-side simulation tools" ON)
option(EW_BUILD_TESTS "Build host-side tests" ON)
option(EW_BUILD_FIRMWARE "Build target firmware ELF" OFF)
option(EW_USE_FREERTOS "Link target firmware with external FreeRTOS-Kernel" OFF)
option(EW_FREERTOS_START_SCHEDULER "Call vTaskStartScheduler from target firmware" OFF)

if(EW_BUILD_TESTS)
    enable_testing()
endif()

add_library(embedded_bsp
    bsp/src/board_profile.c
)

target_include_directories(embedded_bsp
    PUBLIC
        bsp/include
)

add_library(embedded_freestanding_support
    firmware/libc/src/memory.c
)

target_include_directories(embedded_freestanding_support
    PUBLIC
        firmware/libc/include
)

add_library(embedded_drivers
    drivers/src/command_parser.c
    drivers/src/sensor_sample.c
)

target_include_directories(embedded_drivers
    PUBLIC
        drivers/include
)

add_library(embedded_app
    app/src/alarm_output.c
    app/src/alarm_output_sink.c
    app/src/app_info.c
    app/src/alarm_state.c
    app/src/command_handler.c
    app/src/response_format.c
    app/src/rtos_task_model.c
    app/src/rtos_port.c
)

if(EW_USE_FREERTOS)
    include(cmake/FreeRTOSKernel.cmake)
    target_sources(embedded_app
        PRIVATE
            app/src/rtos_port_freertos.c
    )
    target_link_libraries(embedded_app
        PRIVATE
            freertos_kernel
    )
endif()

target_include_directories(embedded_app
    PUBLIC
        app/include
)

target_link_libraries(embedded_app
    PUBLIC
        embedded_drivers
)

if(EW_BUILD_HOST_TOOLS)
    add_executable(host_sim
        tools/host_sim/main.c
    )

    target_link_libraries(host_sim
        PRIVATE
            embedded_app
    )
endif()

if(EW_BUILD_FIRMWARE)
    add_executable(embedded_firmware
        firmware/src/main.c
        firmware/startup/startup_stm32f401re.c
    )

    target_link_libraries(embedded_firmware
        PRIVATE
            embedded_app
            embedded_bsp
            embedded_drivers
            embedded_freestanding_support
    )

    if(EW_USE_FREERTOS)
        target_link_libraries(embedded_firmware
            PRIVATE
                freertos_kernel
        )
        target_compile_definitions(embedded_firmware
            PRIVATE
                EW_FIRMWARE_USE_FREERTOS=1
        )
        if(EW_FREERTOS_START_SCHEDULER)
            target_compile_definitions(embedded_firmware
                PRIVATE
                    EW_FIRMWARE_START_FREERTOS_SCHEDULER=1
            )
        endif()
    endif()

    target_link_options(embedded_firmware
        PRIVATE
            -nostdlib
            -Wl,-T,${CMAKE_SOURCE_DIR}/firmware/linker/stm32f401re.ld
            -Wl,-e,Reset_Handler
            -Wl,--gc-sections
    )

    set_target_properties(embedded_firmware PROPERTIES
        OUTPUT_NAME embedded_firmware
        SUFFIX ".elf"
    )
endif()

if(EW_BUILD_TESTS)
    add_executable(test_app_info
        tests/test_app_info.c
    )

    target_link_libraries(test_app_info
        PRIVATE
            embedded_app
    )

    add_test(NAME app_info COMMAND test_app_info)

    if(EW_BUILD_HOST_TOOLS)
        add_test(NAME host_sim_smoke COMMAND host_sim)
    endif()

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

    add_executable(test_alarm_output
        tests/test_alarm_output.c
    )

    target_link_libraries(test_alarm_output
        PRIVATE
            embedded_app
    )

    add_test(NAME alarm_output COMMAND test_alarm_output)

    add_executable(test_alarm_output_sink
        tests/test_alarm_output_sink.c
    )

    target_link_libraries(test_alarm_output_sink
        PRIVATE
            embedded_app
    )

    add_test(NAME alarm_output_sink COMMAND test_alarm_output_sink)

    add_executable(test_command_parser
        tests/test_command_parser.c
    )

    target_link_libraries(test_command_parser
        PRIVATE
            embedded_drivers
    )

    add_test(NAME command_parser COMMAND test_command_parser)

    add_executable(test_command_handler
        tests/test_command_handler.c
    )

    target_link_libraries(test_command_handler
        PRIVATE
            embedded_app
    )

    add_test(NAME command_handler COMMAND test_command_handler)

    add_executable(test_response_format
        tests/test_response_format.c
    )

    target_link_libraries(test_response_format
        PRIVATE
            embedded_app
    )

    add_test(NAME response_format COMMAND test_response_format)

    add_executable(test_rtos_task_model
        tests/test_rtos_task_model.c
    )

    target_link_libraries(test_rtos_task_model
        PRIVATE
            embedded_app
    )

    add_test(NAME rtos_task_model COMMAND test_rtos_task_model)

    add_executable(test_board_profile
        tests/test_board_profile.c
    )

    target_link_libraries(test_board_profile
        PRIVATE
            embedded_bsp
    )

    add_test(NAME board_profile COMMAND test_board_profile)

    add_executable(test_rtos_port
        tests/test_rtos_port.c
    )

    target_link_libraries(test_rtos_port
        PRIVATE
            embedded_app
    )

    add_test(NAME rtos_port COMMAND test_rtos_port)
endif()
