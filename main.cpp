/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/stdlib.h>
#include "stdio.h"
#include <stdint.h>
#include "hardware/i2c.h"

#include "CCJEncoder.h"

//oled
#include <string.h>

extern "C"{ 
#include "ssd1306.h"
#include "image.h"
#include "acme_5_outlines_font.h"
#include "bubblesstandard_font.h"
#include "crackers_font.h"
#include "BMSPA_font.h"
#include "renew_font.h"
#include "fivexfive_font.h"
}


//assumes PICO with built in LED PIN.
#define BUTTON_PIN 5
#define LED_PIN 25 //LED pin on PICO dev board

CCJISREncoder <1> encoder1;
uint64_t blinkTime = 300000;
bool ledState = 0;

const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[6]= {acme_font, bubblesstandard_font, crackers_font, BMSPA_font, renew_font, fivexfive_font};

char EncAsString[8] = "";
 



int main() {
    //initialize I2C OLED
    i2c_init(i2c1,400000);
    gpio_set_function(2,GPIO_FUNC_I2C);
    gpio_set_function(3,GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);

    ssd1306_t disp;  // instantiate from struct
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp); //blank display
    int count = 0;
    stdio_init_all();
    encoder1.init(10,15);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    absolute_time_t lastBlink = get_absolute_time();




while (2) {
        absolute_time_t curTime = get_absolute_time(); // current time
        // Check time since last blink, if time since last blink is larger than blinkTime, then blink.
        if ((absolute_time_diff_us(lastBlink, curTime)) > blinkTime)
        {
            lastBlink = get_absolute_time(); // update blink time
            ledState = 1-ledState;                 // Flip ledState between 0 and 1
            gpio_put(LED_PIN, ledState);    // update led
        }
 

        if (encoder1.check())  // we call CCJ algo
            {
               int a = encoder1.value();     
               printf("Value is: %d \n",a);  // print out value via uart.

//includes int to string conv snprintf()
         
            snprintf(EncAsString, 11, "%d", a);
            strcat(EncAsString,"Encr:");
            ssd1306_clear(&disp);
            ssd1306_draw_string_with_font(&disp, 8, 24, 2, fonts[4], EncAsString);
            ssd1306_show(&disp);  
        //  printf("Frequency %u Hz\n", edge_counter_frequency());

            }

        }   

}