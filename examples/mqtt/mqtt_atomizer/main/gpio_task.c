/*************************************************************************
	> File Name: gpio_task.c
	> Author: 
	> Mail: 
	> Created Time: Sun 21 Mar 2021 13:54:59 HKT
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"

#define GPIO_D2             4
#define GPIO_D4             2
#define GPIO_D6             12
#define GPIO_D7             13
#define GPIO_D5             14

#define GPIO_PWM            GPIO_D4
#define GPIO_LED_R          GPIO_D6
#define GPIO_LED_B          GPIO_D7
#define GPIO_MOTO           GPIO_D2
#define GPIO_KEY            GPIO_D5

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_PWM) | (1ULL<<GPIO_LED_R) | (1ULL<<GPIO_LED_B) | (1ULL<<GPIO_MOTO))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_KEY))

void gpio_task_run(void *arg){
    int cnt = 0;
    char pwm_state = 0;
    while(1 < 2) {
        gpio_set_level(GPIO_PWM, pwm_state);
        pwm_state = !pwm_state;
        os_delay_us(4); 
    }
}

void timing_task_run(void *arg){
    while(1 < 2) {
        vTaskDelay(15000 / portTICK_RATE_MS);
        esp_task_wdt_reset();       
    }
}

void gpio_task_init(void){
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask =  GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    xTaskCreate(gpio_task_run, "gpio_task", 4096, NULL, 1, NULL);
    xTaskCreate(timing_task_run, "timing_task", 4096, NULL, 2, NULL);
}

