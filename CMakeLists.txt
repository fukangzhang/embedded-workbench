cmake_minimum_required(VERSION 3.20)

project(embedded_workbench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(EW_BUILD_HOST_TOOLS "Build host-side simulation tools" ON)
option(EW_BUILD_TESTS "Build host-side tests" ON)
option(EW_BUILD_FIRMWARE "Build target firmware ELF" OFF)
option(EW_USE_FREERTOS "Link target firmware with external FreeRTOS-Kernel" OFF)
option(EW_FREERTOS_START_SCHEDULER "Call vTaskStartScheduler from target firmware" OFF)
option(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT "Use STM32F401RE memory-mapped GPIO init path in firmware" OFF)

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
    drivers/src/board_digital_output.c
    drivers/src/command_parser.c
    drivers/src/digital_output.c
    drivers/src/sensor_sample.c
    drivers/src/serial_line.c
    drivers/src/stm32_board_gpio_init.c
    drivers/src/stm32_gpio_config.c
    drivers/src/stm32_gpio_output.c
    drivers/src/stm32_rcc_gpio_clock.c
    drivers/src/stm32f401re_gpio_bindings.c
)

target_include_directories(embedded_drivers
    PUBLIC
        drivers/include
)

target_link_libraries(embedded_drivers
    PUBLIC
        embedded_bsp
)

add_library(embedded_app
    app/src/alarm_output_digital_sink.c
    app/src/alarm_output.c
    app/src/alarm_output_sink.c
    app/src/alarm_output_timing.c
    app/src/app_info.c
    app/src/alarm_state.c
    app/src/command_handler.c
    app/src/command_session.c
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

    if(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT)
        target_compile_definitions(embedded_firmware
            PRIVATE
                EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT=1
        )
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
        add_test(
            NAME host_sim_script_status
            COMMAND ${CMAKE_COMMAND}
                -D HOST_SIM_EXE=$<TARGET_FILE:host_sim>
                -D HOST_SIM_INPUT=${CMAKE_SOURCE_DIR}/tests/host_sim_status_input.txt
                -P ${CMAKE_SOURCE_DIR}/tests/host_sim_script_smoke.cmake
        )
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

    add_executable(test_alarm_output_digital_sink
        tests/test_alarm_output_digital_sink.c
    )

    target_link_libraries(test_alarm_output_digital_sink
        PRIVATE
            embedded_app
    )

    add_test(NAME alarm_output_digital_sink COMMAND test_alarm_output_digital_sink)

    add_executable(test_alarm_output_timing
        tests/test_alarm_output_timing.c
    )

    target_link_libraries(test_alarm_output_timing
        PRIVATE
            embedded_app
    )

    add_test(NAME alarm_output_timing COMMAND test_alarm_output_timing)

    add_executable(test_command_parser
        tests/test_command_parser.c
    )

    target_link_libraries(test_command_parser
        PRIVATE
            embedded_drivers
    )

    add_test(NAME command_parser COMMAND test_command_parser)

    add_executable(test_digital_output
        tests/test_digital_output.c
    )

    target_link_libraries(test_digital_output
        PRIVATE
            embedded_drivers
    )

    add_test(NAME digital_output COMMAND test_digital_output)

    add_executable(test_stm32_gpio_config
        tests/test_stm32_gpio_config.c
    )

    target_link_libraries(test_stm32_gpio_config
        PRIVATE
            embedded_drivers
    )

    add_test(NAME stm32_gpio_config COMMAND test_stm32_gpio_config)

    add_executable(test_serial_line
        tests/test_serial_line.c
    )

    target_link_libraries(test_serial_line
        PRIVATE
            embedded_drivers
    )

    add_test(NAME serial_line COMMAND test_serial_line)

    add_executable(test_stm32_gpio_output
        tests/test_stm32_gpio_output.c
    )

    target_link_libraries(test_stm32_gpio_output
        PRIVATE
            embedded_drivers
    )

    add_test(NAME stm32_gpio_output COMMAND test_stm32_gpio_output)

    add_executable(test_stm32_rcc_gpio_clock
        tests/test_stm32_rcc_gpio_clock.c
    )

    target_link_libraries(test_stm32_rcc_gpio_clock
        PRIVATE
            embedded_drivers
    )

    add_test(NAME stm32_rcc_gpio_clock COMMAND test_stm32_rcc_gpio_clock)

    add_executable(test_stm32_board_gpio_init
        tests/test_stm32_board_gpio_init.c
    )

    target_link_libraries(test_stm32_board_gpio_init
        PRIVATE
            embedded_drivers
    )

    add_test(NAME stm32_board_gpio_init COMMAND test_stm32_board_gpio_init)

    add_executable(test_stm32f401re_gpio_bindings
        tests/test_stm32f401re_gpio_bindings.c
    )

    target_link_libraries(test_stm32f401re_gpio_bindings
        PRIVATE
            embedded_drivers
    )

    add_test(NAME stm32f401re_gpio_bindings COMMAND test_stm32f401re_gpio_bindings)

    add_executable(test_board_digital_output
        tests/test_board_digital_output.c
    )

    target_link_libraries(test_board_digital_output
        PRIVATE
            embedded_drivers
    )

    add_test(NAME board_digital_output COMMAND test_board_digital_output)

    add_executable(test_command_handler
        tests/test_command_handler.c
    )

    target_link_libraries(test_command_handler
        PRIVATE
            embedded_app
    )

    add_test(NAME command_handler COMMAND test_command_handler)

    add_executable(test_command_session
        tests/test_command_session.c
    )

    target_link_libraries(test_command_session
        PRIVATE
            embedded_app
    )

    add_test(NAME command_session COMMAND test_command_session)

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
