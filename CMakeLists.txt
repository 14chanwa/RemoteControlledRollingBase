cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

set(CMAKE_CXX_STANDARD 17)

project(nrf)

pico_sdk_init()


add_executable(nrf
src/main.cpp
src/NRF24.cpp
)
target_include_directories(nrf PUBLIC
inc
)

target_link_libraries(nrf
pico_stdlib
hardware_spi
)



# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf 1)
pico_enable_stdio_uart(nrf 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf)