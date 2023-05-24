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

    NRF24 nrf(spi0,2, 3, 4, 5, 6);
    nrf.init();
    nrf.config();
    nrf.modeRX();

    bool ledstatus = false;

    char buffer[32];
    char buffer2[32];
    while(1){
        if (nrf.newMessage() == 1) {
            nrf.receiveMessage(buffer);            
            gpio_put(LED_PIN, ledstatus);
            ledstatus = !ledstatus;

            int result_x, result_y;

            sscanf(buffer, "%d %d", &result_x, &result_y);
            sprintf(buffer2, "%d %d", result_x, result_y);

            printf("NewMessage");
            // printf(buffer);
            // printf("\n");
            printf(buffer2);
            printf("\n");
        }
        
        // printf("Heartbeat\n");
        // sleep_ms(1000);
    }

    return 0;
}

