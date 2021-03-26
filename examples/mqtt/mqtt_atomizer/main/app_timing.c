/*************************************************************************
	> File Name: app_timing.c
	> Author: 
	> Mail: 
	> Created Time: Sat 27 Mar 2021 03:11:29 CST
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

#include "cpt_rtc.h"

static const char *TAG = "APP TIMING";

static u8 rtc_update_flag = 0;
void rtc_update_time(u32 time){
    rtc_update_flag = 1;
    cpt_rtc_date_s cpt_rtc_date;
    cpt_rtc.utc_to_date(time,&cpt_rtc_date);

    ESP_LOGI(TAG,"UPDATE TIME -> [%d] %04d-%02d-%02d %02d:%02d:%02d",
            time,
            cpt_rtc_date.year,
            cpt_rtc_date.month,
            cpt_rtc_date.monthday,
            cpt_rtc_date.hour,
            cpt_rtc_date.minute,
            cpt_rtc_date.second);
   
    cpt_rtc.set_date(cpt_rtc_date);
}

void rtc_task_run(void *arg){
    cpt_rtc_params_s cpt_rtc_params;
    cpt_rtc.init(&cpt_rtc_params);

    u8 times = 0;
    while(1<2){
        vTaskDelay(200 / portTICK_RATE_MS);
        times++;
        if(times > 4){
            times = 0;
            if(rtc_update_flag){
                cpt_rtc.run();

                cpt_rtc_date_s cpt_rtc_date;
                cpt_rtc.get_date(&cpt_rtc_date);
                ESP_LOGI(TAG,"%04d-%02d-%02d %02d:%02d:%02d",
                                    cpt_rtc_date.year,
                                    cpt_rtc_date.month,
                                    cpt_rtc_date.monthday,
                                    cpt_rtc_date.hour,
                                    cpt_rtc_date.minute,
                                    cpt_rtc_date.second);
            }
        }
    } 
}

void app_timing_init(void){
    xTaskCreate(rtc_task_run, "app rtc_task", 4096, NULL, 2, NULL);
}
