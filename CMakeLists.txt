# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(RT C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Cambia HARDWARE_DEFAULT con HARDWARE_V2 per cambiare la versione hardware
add_definitions(-DHARDWARE_DEFAULT)

# Add executable. Default name is the project name, version 0.1
add_executable(RT 
    main.c
    system/scheduler_core.c
    system/initcalls.c
    system/terminal.c
    platform/hardware.c
    platform/driver_led.c
    app/task_led.c
    app/task_terminal.c
    app/terminal/cmd.c
    )

pico_set_program_name(RT "RT")
pico_set_program_version(RT "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(RT 1)
pico_enable_stdio_usb(RT 0)

# Add the standard library to the build
target_link_libraries(RT
        pico_stdlib
        pico_multicore
        hardware_pwm
        pico_time
        )

# Add the standard include files to the build
target_include_directories(RT PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/system
  ${CMAKE_CURRENT_LIST_DIR}/platform
  ${CMAKE_CURRENT_LIST_DIR}/app
)

pico_add_extra_outputs(RT)

