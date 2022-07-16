cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(scd4x_7seg C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(scd4x_7seg)

target_sources(scd4x_7seg PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/scd4x_i2c.c
	${CMAKE_CURRENT_LIST_DIR}/sensirion_common.c
	${CMAKE_CURRENT_LIST_DIR}/sensirion_i2c_hal.c
	${CMAKE_CURRENT_LIST_DIR}/sensirion_i2c.c
        )

pico_enable_stdio_usb(scd4x_7seg 1)
pico_enable_stdio_uart(scd4x_7seg 0)

pico_add_extra_outputs(scd4x_7seg)

target_link_libraries(scd4x_7seg 
    pico_stdlib
    hardware_i2c
    hardware_gpio
)