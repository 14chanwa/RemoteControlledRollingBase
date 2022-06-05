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
                    direction_x = false;
                    direction_y = true;
                }
                else if(result_x < 1000)
                {
                    // backward
                    run_x = true;
                    run_y = true;
                    direction_x = true;
                    direction_y = false;
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
                    direction_x = true;
                    direction_y = true;
                }
                else if(result_y < 1000)
                {
                    // rotate right
                    run_x = true;
                    run_y = true;
                    direction_x = false;
                    direction_y = false;
                }
            }

        }
        
        // printf("Heartbeat\n");
        // sleep_ms(1000);
    }
}


StepperMotor stepper_left(2, 3, 4, 5);
StepperMotor stepper_right(21, 20, 19, 18);

const double period_start = 1000;
const double period_target = 700;
const double period_step = 10;
double period_current = 1000;
// decrease 50 each 100 ms

double time_since_last_period_update = 0;
const double time_between_period_updates = 50;

bool current_direction_x = direction_x;
bool current_direction_y = direction_y;
bool current_run_x = run_x;
bool current_run_y = run_y;

volatile bool timer_fired = false;

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    printf("Timer %d fired!\n", (int) id);
    timer_fired = true;
    // Can return a value here in us to fire in the future
    return 0;
}

int main(){


    stdio_init_all();

    multicore_launch_core1(core1_entry);


    // uint8_t dir_right = 1;
    // uint8_t dir_left = 1;
    while (1) {

        if (current_direction_x != direction_x ||
                current_direction_y != direction_y ||
                current_run_x != run_x ||
                current_run_y != run_y) 
        {
            // Update
            current_direction_x = direction_x;
            current_direction_y = direction_y;
            current_run_x = run_x;
            current_run_y = run_y;

            /* Change direction */
            stepper_right.setDir(direction_x);
            stepper_left.setDir(direction_y);

            // Reset timers
            period_current = period_start;
            add_alarm_in_ms(50, alarm_callback, NULL, false);
        }

        // move motors and wait
        if (run_x)
            stepper_right.move();
        if (run_y)
            stepper_left.move();
        sleep_us(period_current); // max 100Hz (1ms)

        // update period
        if (timer_fired) 
        {
            // reset timer
            timer_fired = false;

            // update period
            if (period_current > period_target) 
            {
                period_current = std::max(period_current - period_step, period_target);
                add_alarm_in_ms(50, alarm_callback, NULL, false);
            }
        }

    }

    return 0;
}

