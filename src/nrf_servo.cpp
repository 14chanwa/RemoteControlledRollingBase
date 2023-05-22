#include "pico/stdlib.h"
#include "pico/multicore.h"

#include<string>
#include<cmath>

#include <stdio.h>
#include "hardware/pwm.h"
#include "NRF24.h"


#include "hardware/clocks.h"


const uint32_t wrap = 65465;

uint32_t pwm_init(uint slice_num,
       uint chan,double d)
{
 uint32_t clock = 125000000;
 pwm_set_clkdiv_int_frac(slice_num, 38, 3);
 pwm_set_wrap(slice_num, 65465);

 uint32_t level = static_cast<uint32_t>(round(wrap * d));
 pwm_set_chan_level(slice_num, chan, level);
 return wrap;
}


double stop_value = 1455.0;
double posturn_value = 2000.0;
double negturn_value = 1000.0;



bool direction_x = false;
bool direction_y = false;

// between 0 and 1
double abs_speed_x = 0.0;
double abs_speed_y = 0.0;

double compute_speed(int reading)
{
    return std::min(1.0 * abs(reading - 2048.0), 2048.0 - 246.0) / (2048.0-246.0);
}

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
            printf(buffer2);
            printf("\n");

            abs_speed_x = compute_speed(result_x);
            abs_speed_y = compute_speed(result_y);

            printf("Compute speed: ");
            printf(std::to_string(abs_speed_x).c_str());
            printf(" ");
            printf(std::to_string(abs_speed_y).c_str());
            printf("\n");

            if (result_y > 2048-100 & result_y < 2048+100)
            {
                if (result_x > 2048+100)
                {
                    // forward
                    run_x = true;
                    run_y = true;
                    direction_x = false;
                    direction_y = true;
                }
                else if(result_x < 2048-100)
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
                if (result_y > 2048+100)
                {
                    // rotate left
                    run_x = true;
                    run_y = true;
                    direction_x = true;
                    direction_y = true;
                }
                else if(result_y < 2048-100)
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
    // stop signal = 1500 us = 1500/20000 duty cycle
    pwm_init(slice_num,chan, stop_value/20000.);
    pwm_set_enabled(slice_num, true);

    while (1) {


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
                    double d = ((1 - abs_speed_x) * stop_value + posturn_value * abs_speed_x) / 20000.;
                    uint32_t level = static_cast<uint32_t>(round(wrap * d));
                    pwm_set_chan_level(slice_num, chan, level);
                }
                else
                {
                    double d = ((1 - abs_speed_x) * stop_value + negturn_value * abs_speed_x) / 20000.;
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


        sleep_us(period_current); // max 100Hz (1ms)
    }

    return 0;
}

