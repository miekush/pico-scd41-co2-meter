//Raspberry Pi Pico SCD41 7 Segment
//Device: RP2040, SCD41, HS420561K-32 (CC 7-Segment Display)
//File: main.c
//Author: Mike Kushnerik
//Date: 7/15/2022

//I2C Bus Wiring (default):
//SDA = GP4
//SCL = GP5
//see sensirion_i2c_hal_init(void) in "sensirion_i2c_hal.c" to change from default

#include <stdio.h>
#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/* //7 segment wiring (adjust as needed):
#define a_pin       6
#define b_pin       7
#define c_pin       8
#define d_pin       9
#define e_pin       10
#define f_pin       11
#define g_pin       12
#define digit_1     13
#define digit_2     14
#define digit_3     15
#define digit_4     16 */

//7 segment wiring (adjust as needed):
#define a_pin       14
#define b_pin       7
#define c_pin       10
#define d_pin       13
#define e_pin       15
#define f_pin       12
#define g_pin       8
#define digit_1     16
#define digit_2     11
#define digit_3     9
#define digit_4     6

//rgb led pins:
#define led_r       2
#define led_g       1
#define led_b       0

//create mask for segment pins
const uint32_t segment_mask =   (1 << a_pin) |
                                (1 << b_pin) |
                                (1 << c_pin) |
                                (1 << d_pin) |
                                (1 << e_pin) |
                                (1 << f_pin) |
                                (1 << g_pin);

//create mask for digit pins
const uint32_t digit_mask =     (1 << digit_1) |
                                (1 << digit_2) |
                                (1 << digit_3) |
                                (1 << digit_4);

const uint32_t rgb_mask =       (1 << led_r) |
                                (1 << led_g) |
                                (1 << led_b);

//LUT for segment pins
//i.e. segment_pins[5] will display the value 5 on the display digit
const uint32_t segment_pins[10] =
{
    //0
    ((1<<a_pin) | (1<<b_pin) | (1<<c_pin) | (1<<d_pin) | (1<<e_pin) | (1<<f_pin)),
    //1
    ((1<<b_pin) | (1<<c_pin)),
    //2
    ((1<<a_pin) | (1<<b_pin) | (1<<d_pin) | (1<<e_pin) | (1<<g_pin)),
    //3
    ((1<<a_pin) | (1<<b_pin) | (1<<c_pin) | (1<<d_pin) | (1<<g_pin)),
    //4
    ((1<<b_pin) | (1<<c_pin) | (1<<f_pin) | (1<<g_pin)),
    //5
    ((1<<a_pin) | (1<<c_pin) | (1<<d_pin) | (1<<f_pin) | (1<<g_pin)),
    //6
    ((1<<a_pin) | (1<<c_pin) | (1<<d_pin) | (1<<e_pin) | (1<<f_pin) | (1<<g_pin)),
    //7
    ((1<<a_pin) | (1<<b_pin) | (1<<c_pin)),
    //8
    ((1<<a_pin) | (1<<b_pin) | (1<<c_pin) | (1<<d_pin) | (1<<e_pin) | (1<<f_pin) | (1<<g_pin)),
    //9
    ((1<<a_pin) | (1<<b_pin) | (1<<c_pin) | (1<<d_pin) | (1<<f_pin) | (1<<g_pin))
};

//LUT for digit pins
//i.e. digit_pins[0] is the most significant digit of the display
uint32_t digit_pins[4] =
{
    digit_1,
    digit_2,
    digit_3,
    digit_4
};

uint16_t co2 = 9999;
int32_t temperature = 0;
int32_t humidity = 0;
bool timerFlag = false;

//function prototypes
void init_display(void);
void update_display(uint16_t value);
void init_rgb(void);
void update_rgb(uint16_t value);

bool timer_cb(struct repeating_timer *t);

int main() 
{
    stdio_init_all();
    init_display();
    init_rgb();    

    sensirion_i2c_hal_init();

    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();

    scd4x_start_periodic_measurement();

    //timer to poll scd41 every 5s
    struct repeating_timer timer;
    add_repeating_timer_ms(5000, timer_cb, NULL, &timer);

    while(1)
    {
        update_display(co2);
        update_rgb(co2);

        if(timerFlag)
        {
            //read scd41
            int16_t error = scd4x_read_measurement(&co2, &temperature, &humidity);
            //clear flag
            timerFlag = false;
        }
    }

    return 0;
}

//function to init the 7 segment display
void init_display(void)
{
    //init 7 segment output pins
    gpio_init_mask(segment_mask | digit_mask);
    //set pins as output
    gpio_set_dir_out_masked(segment_mask | digit_mask);
}

//function to init the rgb led
void init_rgb(void)
{
    //init the rgb output pins
    gpio_init_mask(rgb_mask);
    //set pins as output
    gpio_set_dir_out_masked(rgb_mask);
}

//function to update the value of the 7 segment display
void update_display(uint16_t value)
{
    //turn all digits off
    gpio_put_masked(digit_mask, digit_mask);
    //turn all segments off
    gpio_clr_mask(segment_mask);

    int i = 0;
    do
    {
        //turn all digits off
        gpio_put_masked(digit_mask, digit_mask);
        //select digit
        gpio_put(digit_pins[3-i], 0);
        //write digit value
        gpio_put_masked(segment_mask, segment_pins[value % 10]);
        //2000us seems to work well :p
        sleep_us(2000);
        i++;
    } while (value /= 10);
}

//function to update the air quality RGB led
void update_rgb(uint16_t value)
{
    //good air quality
    if(value < 800)
    {
        gpio_put(led_r, 0);
        gpio_put(led_g, 1);
        gpio_put(led_b, 0);
    }
    //average air quality
    else if (value >= 800 && value < 1000)
    {
        gpio_put(led_r, 1);
        gpio_put(led_g, 1);
        gpio_put(led_b, 0);
    }
    //bad air quality
    else
    {
        gpio_put(led_r, 1);
        gpio_put(led_g, 0);
        gpio_put(led_b, 0);        
    }
}

//timer interrupt callback to poll scd41
bool timer_cb(struct repeating_timer *t)
{
    //set flag
    timerFlag = true;
    return true;
}