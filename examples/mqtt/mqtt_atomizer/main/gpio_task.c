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

//https://www.cnblogs.com/smartlife/articles/9061040.html
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

typedef struct{
    bool PowerSwitch;
    bool OilShortage;
    char SprayLevel;
    char timer[8][50];
    char time_syn[50];
}dps_s;

static const char *TAG = "APP PWM";
static int atomizer_on_off = 0;
extern dps_s dps;


void gpio_led_r(char on){
    static char _on = -1;
    if(on != _on){
        gpio_set_level(GPIO_LED_R, on);
        _on = on;
    }
}

void gpio_led_b(char on){
    static char _on = -1;
    if(on != _on){
        gpio_set_level(GPIO_LED_B, on);
        _on = on;
    }
}

void gpio_moto_ctl(char on){
    static char _on = -1;
    if(on != _on){
        gpio_set_level(GPIO_MOTO, on);
        _on = on;
    }
}

void app_atomizer_on(int enable){
    if(dps.PowerSwitch == 1){
        if(atomizer_on_off != enable){
            atomizer_on_off = enable;
            gpio_led_b(enable);
            gpio_moto_ctl(enable);
            ESP_LOGI(TAG,"pen:%d",enable);
        }
    }else{
        if(atomizer_on_off != 0){
            atomizer_on_off = 0;
            gpio_led_b(0);
            gpio_moto_ctl(0);
            ESP_LOGI(TAG,"pen:0");
        }
    }
}

void gpio_task_run(void *arg){
    int pwm_state = 0;
    int cnt = 0;
    while(1 < 2) {
        gpio_set_level(GPIO_PWM, pwm_state);
        if(atomizer_on_off == 1){
            pwm_state = !pwm_state;
            if(pwm_state != 0)
                cnt = 8;
            else
                cnt = 2;
            while(cnt--);
            os_delay_us(3);
        }else{
            pwm_state = 0;
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        //ESP_LOGI(TAG,"PWM:%d",pwm_state);
    }
}

void timing_task_run(void *arg){
    while(1 < 2) {
        vTaskDelay(15000 / portTICK_RATE_MS);
        esp_task_wdt_reset();       
    }
}

extern uint8_t wifi_state;
void button_task_run(void *arg){
    int time = 0;
    int reboot = 0;
    uint32_t cnt = 0,cnt2 = 0;
    uint8_t led_r_on = 0;
    while(1){
        switch(wifi_state){
            case 0:
                cnt++;
                if(cnt%12 == 0){
                    led_r_on = !led_r_on;
                    gpio_led_r(led_r_on);
                }
                break;//pairing
            case 1:
                gpio_led_r(0);
                break;//not get ip
            case 2:
                if(cnt2 == 0){
                    gpio_led_r(1);
                    cnt2++;
                }else if(cnt2 > 150){
                    cnt2 = 0;
                    gpio_led_r(0);   
                    wifi_state = 3;
                }else{
                    cnt2++;
                }
                break;//get ip
            default:break;
        }

        if(reboot != 0){
            reboot++;
            if(reboot > 20){
                extern void HAL_Reboot(void);
                HAL_Reboot();
            }
        }else{
            if(gpio_get_level(GPIO_KEY) == 0){      
                time += 20;
            }else{
                if(time > 0){
                    if(time < 1500){
                        dps.PowerSwitch = !dps.PowerSwitch;
                        app_atomizer_on(0);
                        ESP_LOGI(TAG,"short press");
                    }else if(time > 2500){
                        uint8_t reset = 0;
                        extern esp_err_t app_nvs_set_reset(uint8_t *reset);
                        app_nvs_set_reset(&reset);
                        extern esp_err_t app_nvs_erase_timing(void );
                        app_nvs_erase_timing();
                        ESP_LOGI(TAG,"long press");
                        
                        reboot = 1;
                    }
                }
                time = 0;
            }
        }
        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

void gpio_init(void){
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

    gpio_set_level(GPIO_PWM, 0);

    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

void gpio_task_init(void){
    xTaskCreate(gpio_task_run, "gpio_task", 4096, NULL, 1, NULL);
    xTaskCreate(timing_task_run, "timing_task", 4096, NULL, 2, NULL);
    xTaskCreate(button_task_run, "button_task", 4096, NULL, 2, NULL);
}

