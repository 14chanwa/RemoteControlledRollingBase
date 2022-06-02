#include "pico/stdlib.h"
#include "pico/multicore.h"

#include<string>
#include <stdio.h>
#include "StepperMotor.h"
#include "NRF24.h"


/**
 * Copyright (c) 2019, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Arduino/004
 * Example of stepper motor (28BYJ-48) controller. 
 */


bool direction_x = false;
bool direction_y = false;

bool run_x = false;
bool run_y = false;


void core1_entry() {

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    NRF24 nrf(spi1, 9, 8);
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

            sscanf(buffer, "%d %d", &result_y, &result_x);
            sprintf(buffer2, "%d %d", result_x, result_y);

            printf("NewMessage");
            // printf(buffer);
            // printf("\n");
            printf(buffer2);
            printf("\n");


            if (result_y > 1000 & result_y < 3000)
            {
                if (result_x > 3000)
                {
                    // forward
                    run_x = true;
                    run_y = true;
                    direction_x = true;
                    direction_y = false;
                }
                else if(result_x < 1000)
                {
                    // backward
                    run_x = true;
                    run_y = true;
                    direction_x = false;
                    direction_y = true;
                } else {
                    run_x = false;
                    run_y = false;
                }
            } else {
                if (result_y > 3000)
                {
                    // rotate left
                    run_x = true;
                    run_y = true;
                    direction_x = false;
                    direction_y = false;
                }
                else if(result_y < 1000)
                {
                    // rotate right
                    run_x = true;
                    run_y = true;
                    direction_x = true;
                    direction_y = true;
                }
            }

        }
        
        // printf("Heartbeat\n");
        // sleep_ms(1000);
    }
}


StepperMotor stepper_left(2, 3, 4, 5);
StepperMotor stepper_right(21, 20, 19, 18);


int main(){


    stdio_init_all();

    multicore_launch_core1(core1_entry);


    // uint8_t dir_right = 1;
    // uint8_t dir_left = 1;
    while (1) {

        /* Change direction */
        stepper_right.setDir(direction_x);
        stepper_left.setDir(direction_y);
        if (run_x)
            stepper_right.move();
        if (run_y)
            stepper_left.move();
        sleep_us(1000); // max 100Hz (1ms)
    }

    return 0;
}

