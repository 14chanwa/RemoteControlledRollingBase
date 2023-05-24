#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "NRF24.h"

#include<string>
#include <stdio.h>

int main(){

    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    NRF24 nrf(spi1, 10, 11, 12, 9, 8);
    nrf.config();

    nrf.modeTX();

    adc_init();
    adc_gpio_init(26);    
    adc_gpio_init(27);

    char buffer[32];

    bool led_status = false;

    while(1){

        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);

        adc_select_input(0);
        uint16_t result_x = adc_read();
        adc_select_input(1);
        uint16_t result_y = adc_read();
        // printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        sprintf(buffer, "%u %u", result_x, result_y);
        nrf.sendMessage(buffer);
        printf(buffer);
        printf("\n");
        sleep_ms(100);

        gpio_put(LED_PIN, led_status);
        led_status = !led_status;

    }

    return 0;
}

