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


double stop_value = 1500.0;
double posturn_value = 1700.0;
double negturn_value = 1300.0;



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

            int activation_offset = 300;

            if (result_x > 2048+activation_offset)
            {
                // forward
                run_x = true;
                run_y = false;
                direction_x = true;
                direction_y = true;
            }
            else if (result_x < 2048-activation_offset)
            {
                // backward
                run_x = true;
                run_y = false;
                direction_x = false;
                direction_y = true;
            }
            else if (result_y > 2048+activation_offset)
            {
                // rotate left
                run_x = false;
                run_y = true;
                direction_x = true;
                direction_y = true;
            }
            else if (result_y < 2048-activation_offset)
            {
                // rotate right
                run_x = false;
                run_y = true;
                direction_x = true;
                direction_y = false;
            }
            else
            {
                run_x = false;
                run_y = false;
            }

            // if (result_y > 2048-100 & result_y < 2048+100)
            // {
            //     if (result_x > 2048+100)
            //     {
            //         // forward
            //         run_x = true;
            //         run_y = true;
            //         direction_x = false;
            //         direction_y = true;
            //     }
            //     else if(result_x < 2048-100)
            //     {
            //         // backward
            //         run_x = true;
            //         run_y = true;
            //         direction_x = true;
            //         direction_y = false;
            //     } else {
            //         run_x = false;
            //         run_y = false;
            //     }
            // } else {
            //     if (result_y > 2048+100)
            //     {
            //         // rotate left
            //         run_x = true;
            //         run_y = true;
            //         direction_x = true;
            //         direction_y = true;
            //     }
            //     else if(result_y < 2048-100)
            //     {
            //         // rotate right
            //         run_x = true;
            //         run_y = true;
            //         direction_x = false;
            //         direction_y = false;
            //     }
            // }

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

class Servo
{
    public:
        uint gpio_;

        Servo(uint gpio);

        uint getSliceNum();
        uint getChan();

        /// @brief Sends a target to the servo in us
        /// @param target should be a target in us
        void setTargetUs(uint32_t target);

};

uint Servo::getSliceNum() { return(pwm_gpio_to_slice_num(gpio_)); }
uint Servo::getChan() { return(pwm_gpio_to_channel(gpio_)); }

Servo::Servo(uint gpio) : gpio_(gpio) 
{
    gpio_set_function(gpio_, GPIO_FUNC_PWM);
    pwm_set_phase_correct(getSliceNum(), false);

    // frequency = 50 Hz, period = 20ms
    // stop signal = 1500 us = 1500/20000 duty cycle
    pwm_init(getSliceNum(), getChan(), stop_value/20000.);
    pwm_set_enabled(getSliceNum(), true);
}


void Servo::setTargetUs(uint32_t target)
{
    double d = target / 20000.;
    uint32_t level = static_cast<uint32_t>(round(wrap * d));
    pwm_set_chan_level(getSliceNum(), getChan(), level);
};


int main(){

    stdio_init_all();
    multicore_launch_core1(core1_entry);

    Servo rightMotor(0);
    Servo leftMotor(1);


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
                    rightMotor.setTargetUs((1 - abs_speed_x) * stop_value + posturn_value * abs_speed_x);
                    leftMotor.setTargetUs((1 - abs_speed_x) * stop_value + negturn_value * abs_speed_x);
                }
                else
                {
                    rightMotor.setTargetUs((1 - abs_speed_x) * stop_value + negturn_value * abs_speed_x);
                    leftMotor.setTargetUs((1 - abs_speed_x) * stop_value + posturn_value * abs_speed_x);
                }   
            }
            else if (run_y)
            {
                if (direction_y)
                {
                    leftMotor.setTargetUs((1 - abs_speed_y) * stop_value + negturn_value * abs_speed_y);
                    rightMotor.setTargetUs((1 - abs_speed_y) * stop_value + negturn_value * abs_speed_y);
                }
                else
                {
                    leftMotor.setTargetUs((1 - abs_speed_y) * stop_value + posturn_value * abs_speed_y);
                    rightMotor.setTargetUs((1 - abs_speed_y) * stop_value + posturn_value * abs_speed_y);
                }   
            }
            else
            {
                leftMotor.setTargetUs(stop_value);
                rightMotor.setTargetUs(stop_value);
            }


        sleep_us(period_current); // max 100Hz (1ms)
    }

    return 0;
}

