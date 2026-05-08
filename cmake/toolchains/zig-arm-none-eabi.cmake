set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ZIG_EXE ZIG_AR ZIG_RANLIB)

set(ZIG_EXE "$ENV{ZIG_EXE}" CACHE FILEPATH "Path to zig executable")
set(ZIG_AR "$ENV{ZIG_AR}" CACHE FILEPATH "Path to zig ar wrapper")
set(ZIG_RANLIB "$ENV{ZIG_RANLIB}" CACHE FILEPATH "Path to zig ranlib wrapper")

if(NOT ZIG_EXE)
    message(FATAL_ERROR "Set ZIG_EXE to the zig executable path.")
endif()

if(NOT ZIG_AR)
    message(FATAL_ERROR "Set ZIG_AR to a wrapper that runs zig ar.")
endif()

if(NOT ZIG_RANLIB)
    message(FATAL_ERROR "Set ZIG_RANLIB to a wrapper that runs zig ranlib.")
endif()

set(CMAKE_C_COMPILER ${ZIG_EXE})
set(CMAKE_C_COMPILER_ARG1 cc)
set(CMAKE_AR ${ZIG_AR})
set(CMAKE_RANLIB ${ZIG_RANLIB})
set(CMAKE_C_FLAGS_INIT "-target thumb-freestanding-eabi -mcpu=cortex_m4")
