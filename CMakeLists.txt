cmake_minimum_required(VERSION 3.13)

add_compile_definitions(PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/tinyusb")

include(pico_sdk_import.cmake)

project(morse)

pico_sdk_init()

add_compile_options(-Wall)

add_executable(morse src/morse.c src/tinyusb_stuff.c)
target_include_directories(morse PRIVATE src)
target_link_libraries(morse pico_stdlib tinyusb_device tinyusb_board)
pico_add_extra_outputs(morse)
