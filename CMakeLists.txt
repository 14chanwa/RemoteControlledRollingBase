cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

set(CMAKE_CXX_STANDARD 17)

project(nrf C CXX ASM)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

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
src/StepperMotor.cpp
src/NRF24.cpp
src/nrf_stepper.cpp
)
target_include_directories(nrf_stepper PUBLIC
inc
)

target_link_libraries(nrf_stepper
pico_stdlib
pico_multicore
hardware_spi
)

# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf_stepper 1)
pico_enable_stdio_uart(nrf_stepper 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf_stepper)





add_executable(nrf_servo)

# pico_generate_pio_header(nrf_servo ${CMAKE_CURRENT_LIST_DIR}/src/nrf_servo.pio)


target_sources(nrf_servo PRIVATE 
    # src/StepperMotor.cpp
    src/NRF24.cpp
    src/nrf_servo.cpp)

target_include_directories(nrf_servo PUBLIC
inc
)

target_link_libraries(nrf_servo
pico_stdlib
pico_multicore
hardware_spi
hardware_pwm
# hardware_pio
)

# pico_add_extra_outputs(nrf)

# enable usb output, disable uart output
pico_enable_stdio_usb(nrf_servo 0)
pico_enable_stdio_uart(nrf_servo 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(nrf_servo)