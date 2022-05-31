#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "NRF24.h"

#include<string>
#include <stdio.h>

#define TXMODE

int main(){

    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    NRF24 nrf(spi1, 9, 8);
    nrf.config();


#ifdef TXMODE

    nrf.modeTX();

    char buffer[32];
    while(1){
        sprintf(buffer,"60");
        buffer[30] = 'R';
        buffer[31] = 'O'; // not a zero.
        nrf.sendMessage(buffer);
        sleep_ms(3000);

        gpio_put(LED_PIN, 1);

        printf("message 1\n");

        sprintf(buffer,"-60");
        buffer[30] = 'R';
        buffer[31] = 'O'; // not a zero.
        nrf.sendMessage(buffer);
        sleep_ms(3000);

        gpio_put(LED_PIN, 0);
        
        printf("message 2\n");

    }

#else

    nrf.modeRX();

    bool ledstatus = false;

    char buffer[32];
    while(1){
        if (nrf.newMessage() == 1) {
            nrf.receiveMessage(buffer);
            
            gpio_put(LED_PIN, ledstatus);
            ledstatus = !ledstatus;

            printf("NewMessage\n");
            printf(buffer);
            printf("\n");
        }
        
        printf("Heartbeat\n");
        sleep_ms(1000);
    }


#endif

    return 0;
}

