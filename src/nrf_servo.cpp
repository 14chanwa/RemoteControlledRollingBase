#include "pico/stdlib.h"
#include "pico/multicore.h"

#include<string>
#include<cmath>

#include <stdio.h>
#include "hardware/pwm.h"
#include "NRF24.h"


#include "hardware/clocks.h"
// #include "hardware/pio.h"
#include "pico/stdlib.h"
// #include "nrf_servo.pio.h"

// #define MIN_DC 1200
// #define MAX_DC 1700
// const uint SERVO_PIN = 0;

// // Write `period` to the input shift register
// void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
//     pio_sm_set_enabled(pio, sm, false);
//     pio_sm_put_blocking(pio, sm, period);
//     pio_sm_exec(pio, sm, pio_encode_pull(false, false));
//     pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
//     pio_sm_set_enabled(pio, sm, true);
// }

// // Write `level` to TX FIFO. State machine will copy this into X.
// void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
//     pio_sm_put_blocking(pio, sm, level);
// }



const uint32_t wrap = 65465;

uint32_t pwm_set_freq_duty(uint slice_num,
       uint chan,double d)
{
 uint32_t clock = 125000000;
//  uint32_t divider16 = clock / f / 4096 + 
//                            (clock % (f * 4096) != 0);
//  if (divider16 / 16 == 0)
//     divider16 = 16;

//  uint32_t wrap = clock * 16 / divider16 / f - 1;
 pwm_set_clkdiv_int_frac(slice_num, 38, 3);
 pwm_set_wrap(slice_num, 65465);

 uint32_t level = static_cast<uint32_t>(round(wrap * d));

//  printf("wrap: ");
//  printf(std::to_string(wrap).c_str());
//  printf(" -- level: ");
//  printf(std::to_string(level).c_str());
//  printf(" -- length: ");
//  printf(std::to_string(20000.0 * level / wrap).c_str());
//  printf("\n");
 pwm_set_chan_level(slice_num, chan, level);
 return wrap;
}


double stop_value = 1455.0;
double posturn_value = 2000.0;
double negturn_value = 1000.0;



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

    int result_x, result_y;

    while(1){
        if (nrf.newMessage() == 1) {
            nrf.receiveMessage(buffer);            
            gpio_put(LED_PIN, ledstatus);
            ledstatus = !ledstatus;

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


// StepperMotor stepper_left(2, 3, 4, 5);
// StepperMotor stepper_right(21, 20, 19, 18);

const double period_start = 1000;
const double period_target = 700;
const double period_step = 10;
double period_current = 100;
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


    gpio_set_function(0, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    uint chan = pwm_gpio_to_channel(0);
    pwm_set_phase_correct(slice_num, false);

    // frequency = 50 Hz, period = 20ms
    // stop signal = 1500 us = 15/20000 duty cycle
    pwm_set_freq_duty(slice_num,chan, stop_value/20000.);
    pwm_set_enabled(slice_num, true);
    // return 0;


    // PIO pio = pio0;
    // int sm = 0;
    // uint offset = pio_add_program(pio, &pico_servo_pio_program);

    // float freq = 50.0f; // servo except 50Hz
    // uint clk_div = 64;  // make the clock slower

    // pico_servo_pio_program_init(pio, sm, offset, clk_div, SERVO_PIN);

    // uint cycles = clock_get_hz(clk_sys) / (freq * clk_div);
    // uint32_t period = (cycles -3) / 3;  
    // pio_pwm_set_period(pio, sm, period);

    // uint level;
    // int ms = (MAX_DC - MIN_DC) / 2;
    // bool clockwise = false;

//   while (true) {
//     level = (ms / 20000.f) * period;
//     pio_pwm_set_level(pio, sm, level);

//     if (ms <= MIN_DC || ms >= MAX_DC) {
//         clockwise = !clockwise;
//     }

//     if (clockwise) {
//         ms -= 100;
//     } else {
//         ms += 100;
//     }

//     sleep_ms(500);
//   }



    // multicore_launch_core1(core1_entry);


    // // // Tell GPIO 0 and 1 they are allocated to the PWM
    // // gpio_set_function(0, GPIO_FUNC_PWM);
    // // gpio_set_function(1, GPIO_FUNC_PWM);

    // // // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    // // uint slice_num = pwm_gpio_to_slice_num(0);

    // // // Set period of 4 cycles (0 to 3 inclusive)
    // // pwm_set_wrap(slice_num, 3);
    // // // Set channel A output high for one cycle before dropping
    // // pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // // // Set initial B output high for three cycles before dropping
    // // pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
    // // // Set the PWM running
    // // pwm_set_enabled(slice_num, true);
    // // /// \end::setup_pwm[]

    // const uint LED_PIN = 0;
    // gpio_init(LED_PIN);
    // gpio_set_dir(LED_PIN, GPIO_OUT);
    // while (true) {
    //     gpio_put(LED_PIN, 1);
    //     sleep_ms(2000);
    //     gpio_put(LED_PIN, 0);
    //     sleep_ms(2000);
    // }



    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.


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

            if (run_x)
            {
                if (direction_x)
                {
                    double d = posturn_value/20000.;
                    uint32_t level = static_cast<uint32_t>(round(wrap * d));
                    pwm_set_chan_level(slice_num, chan, level);
                }
                else
                {
                    double d = negturn_value/20000.;
                    uint32_t level = static_cast<uint32_t>(round(wrap * d));
                    pwm_set_chan_level(slice_num, chan, level);
                }   
            }
            else
            {
                
                double d = stop_value/20000.;
                uint32_t level = static_cast<uint32_t>(round(wrap * d));
                pwm_set_chan_level(slice_num, chan, level);
            }

            // stepper_right.setDir(direction_x);
            // stepper_left.setDir(direction_y);

            // // Reset timers
            // period_current = period_start;
            // add_alarm_in_ms(10, alarm_callback, NULL, false);
        }

        // move motors and wait
        // if (run_x)
        //     stepper_right.move();
        // if (run_y)
        //     stepper_left.move();
        sleep_us(period_current); // max 100Hz (1ms)

        // // update period
        // if (timer_fired) 
        // {
        //     // reset timer
        //     timer_fired = false;

        //     // update period
        //     if (period_current > period_target) 
        //     {
        //         period_current = std::max(period_current - period_step, period_target);
        //         add_alarm_in_ms(10, alarm_callback, NULL, false);
        //     }
        // }

    }

    return 0;
}

