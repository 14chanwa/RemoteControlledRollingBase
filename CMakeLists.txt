cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

set(CMAKE_CXX_STANDARD 17)

project(nrf)

pico_sdk_init()


add_executable(nrf_tx
src/nrf_tx.cpp
src/NRF24.cpp
)
target_include_directories(nrf_tx PUBLIC
inc
)

target_link_libraries(nrf_tx
pico_stdlib
hardware_spi
hardware_adc
)

# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf_tx 1)
pico_enable_stdio_uart(nrf_tx 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf_tx)


add_executable(nrf_rx
src/nrf_rx.cpp
src/NRF24.cpp
)
target_include_directories(nrf_rx PUBLIC
inc
)

target_link_libraries(nrf_rx
pico_stdlib
hardware_spi
hardware_adc
)

# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf_rx 1)
pico_enable_stdio_uart(nrf_rx 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf_rx)





add_executable(nrf_stepper
src/nrf_stepper.cpp
)
target_include_directories(nrf_stepper PUBLIC
inc
)

target_link_libraries(nrf_stepper
pico_stdlib
)

# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf_stepper 1)
pico_enable_stdio_uart(nrf_stepper 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf_stepper)