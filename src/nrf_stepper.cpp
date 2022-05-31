#include "pico/stdlib.h"

#include<string>
#include <stdio.h>


/**
 * Copyright (c) 2019, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Arduino/004
 * Example of stepper motor (28BYJ-48) controller. 
 */
 
class StepperMotor {
  public:
    StepperMotor(uint8_t _in1, uint8_t _in2, uint8_t _in3, uint8_t _in4):
      in1(_in1), in2(_in2), in3(_in3), in4(_in4) {

    gpio_init(in1);
    gpio_set_dir(in1, GPIO_OUT);
    gpio_init(in2);
    gpio_set_dir(in2, GPIO_OUT);
    gpio_init(in3);
    gpio_set_dir(in3, GPIO_OUT);
    gpio_init(in4);
    gpio_set_dir(in4, GPIO_OUT);
    
      dir = 1;
      seq = 0;
    }

    /* Move one step */
    void move(void) {
      seq = (seq + (dir?1:-1)) & 7;

      switch (seq) {
      /* Pattern for 8 microsteps: A-AB-B-BC-C-CD-D-DA */
      /*      [        A        ][         B        ][         C        ][         D        ] */
      case 0: gpio_put(in1,1);gpio_put(in2,0);gpio_put(in3,0);gpio_put(in4,0);break;
      case 1: gpio_put(in1,1);gpio_put(in2,1);gpio_put(in3,0);gpio_put(in4,0);break;
      case 2: gpio_put(in1,0);gpio_put(in2,1);gpio_put(in3,0);gpio_put(in4,0);break;      
      case 3: gpio_put(in1,0);gpio_put(in2,1);gpio_put(in3,1);gpio_put(in4,0);break;
      case 4: gpio_put(in1,0);gpio_put(in2,0);gpio_put(in3,1);gpio_put(in4,0);break;
      case 5: gpio_put(in1,0);gpio_put(in2,0);gpio_put(in3,1);gpio_put(in4,1);break;      
      case 6: gpio_put(in1,0);gpio_put(in2,0);gpio_put(in3,0);gpio_put(in4,1);break;      
      case 7: gpio_put(in1,1);gpio_put(in2,0);gpio_put(in3,0);gpio_put(in4,1);break;
      }
    }

    /* Set direction. 
       1 - clock wise
       0 - counter clock wise
    */
    void setDir(uint8_t _dir) {
      dir = !!_dir;
    }
  
  private:
    uint8_t in1;
    uint8_t in2;
    uint8_t in3;
    uint8_t in4;
    uint8_t dir;
    uint8_t seq;
};


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

