set(FREERTOS_KERNEL_PATH "" CACHE PATH "Path to an external FreeRTOS-Kernel checkout")
set(FREERTOS_PORT "GCC_ARM_CM3" CACHE STRING "FreeRTOS portable layer")
set(FREERTOS_HEAP "heap_4" CACHE STRING "FreeRTOS heap implementation")

if(NOT FREERTOS_KERNEL_PATH)
    message(FATAL_ERROR "Set FREERTOS_KERNEL_PATH to a FreeRTOS-Kernel checkout.")
endif()

if(NOT EXISTS "${FREERTOS_KERNEL_PATH}/include/FreeRTOS.h")
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH does not look like FreeRTOS-Kernel: ${FREERTOS_KERNEL_PATH}")
endif()

if(FREERTOS_PORT STREQUAL "GCC_ARM_CM3")
    set(FREERTOS_PORT_DIR "${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM3")
elseif(FREERTOS_PORT STREQUAL "GCC_ARM_CM4F")
    set(FREERTOS_PORT_DIR "${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM4F")
else()
    message(FATAL_ERROR "Unsupported FREERTOS_PORT=${FREERTOS_PORT}.")
endif()

add_library(freertos_kernel
    "${FREERTOS_KERNEL_PATH}/croutine.c"
    "${FREERTOS_KERNEL_PATH}/event_groups.c"
    "${FREERTOS_KERNEL_PATH}/list.c"
    "${FREERTOS_KERNEL_PATH}/queue.c"
    "${FREERTOS_KERNEL_PATH}/stream_buffer.c"
    "${FREERTOS_KERNEL_PATH}/tasks.c"
    "${FREERTOS_KERNEL_PATH}/timers.c"
    "${FREERTOS_KERNEL_PATH}/portable/MemMang/${FREERTOS_HEAP}.c"
    "${FREERTOS_PORT_DIR}/port.c"
)

target_include_directories(freertos_kernel
    PUBLIC
        "${CMAKE_SOURCE_DIR}/firmware/libc/include"
        "${FREERTOS_KERNEL_PATH}/include"
        "${FREERTOS_PORT_DIR}"
        "${CMAKE_SOURCE_DIR}/firmware/config"
)

target_link_libraries(freertos_kernel
    PUBLIC
        embedded_freestanding_support
)
