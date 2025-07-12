cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(my_pico_project C CXX ASM)

pico_sdk_init()

add_executable(my_pico_project
    src/main.cpp
    src/ui_pages.cpp
    lib/lcd.cpp
    lib/keypad.cpp
    lib/encoder.cpp
)

target_include_directories(my_pico_project PRIVATE
    src
    lib
)

target_link_libraries(my_pico_project pico_stdlib hardware_spi hardware_gpio)

pico_enable_stdio_uart(my_pico_project 1)
pico_enable_stdio_usb(my_pico_project 0)

pico_add_extra_outputs(my_pico_project)