#include "pico/stdlib.h"

#include<string>
#include <stdio.h>
#include "StepperMotor.h"


/**
 * Copyright (c) 2019, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Arduino/004
 * Example of stepper motor (28BYJ-48) controller. 
 */


StepperMotor stepper_left(2, 3, 4, 5);
StepperMotor stepper_right(21, 20, 19, 18);


int main(){


    stdio_init_all();

    uint8_t dir_right = 1;
    uint8_t dir_left = 1;
    while (1) {
        /* Do full turn */
        for (uint16_t i = 0; i < 4096; ++i) {
            stepper_right.move();
            stepper_left.move();
            sleep_us(1500); // max 100Hz (1ms)
        }
        /* Change direction */
        stepper_right.setDir(dir_right^=1);
        stepper_left.setDir(dir_left^=1);
    }

    return 0;
}

