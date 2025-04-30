# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(NEN C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(NEN 
    src/main.cpp
    src/registers.cpp
    src/flash.cpp
    src/CAN_module.cpp
    
    # Library executables
    # MCP2515
    include/mcp2515/mcp2515.cpp
    # SH1107
    include/sh1107/sh110x.cpp
    include/sh1107/display_graphics.cpp
    include/sh1107/display_print.cpp
    include/sh1107/display_fonts.cpp
    )

pico_set_program_name(NEN "NEN")
pico_set_program_version(NEN "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(NEN 0)
pico_enable_stdio_usb(NEN 1)

# Add the standard library to the build
target_link_libraries(NEN
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(NEN PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add MCP2515 Lib
target_include_directories(NEN PUBLIC include/)
target_include_directories(NEN PUBLIC src/)

# Add any user requested libraries
target_link_libraries(NEN 
        hardware_spi
        hardware_i2c
        )

pico_add_extra_outputs(NEN)

