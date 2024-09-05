# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
set(PICO_BOARD adafruit_feather_rp2040 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hello_usb C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

if (TARGET tinyusb_device)
    add_executable(hello_usb hello_usb.c )

    pico_set_program_name(hello_usb "hello_usb")
    pico_set_program_version(hello_usb "0.1")

    # Generate PIO header
    pico_generate_pio_header(hello_usb ${CMAKE_CURRENT_LIST_DIR}/blink2.pio)
    pico_generate_pio_header(hello_usb ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

    # Modify the below lines to enable/disable output over UART/USB
    #pico_enable_stdio_uart(hello_usb 0)
    pico_enable_stdio_usb(hello_usb 1)

    # pull in common dependencies
    target_link_libraries(hello_usb 
        pico_stdlib)

    # Add the standard include files to the build
    target_include_directories(hello_usb PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
    ${CMAKE_CURRENT_LIST_DIR}/include # for our common lwipopts or any other standard includes, if required
    )

    target_include_directories(hello_usb INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

    # Add any user requested libraries
    target_link_libraries(hello_usb 
        hardware_pio
        hardware_timer
        hardware_watchdog
        hardware_clocks
        pico_malloc 
        pico_mem_ops
    )

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(hello_usb)



    # add url via pico_set_program_url
elseif(PICO_ON_DEVICE)
    message("Skipping hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
