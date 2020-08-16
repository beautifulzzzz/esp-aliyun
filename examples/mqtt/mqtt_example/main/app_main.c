/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"


#include "esp_log.h"

#include "app_entry.h"
#include "platform_hal.h"

#include "app_wifi.h"
#include "app_uart.h"



/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;//没连接上wifi不会执行MQTT
static const char* TAG = "app main";


void mqtt_task(void* parameter){
    while(1) {
        ESP_LOGI(TAG, "wait wifi connect...");
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

        app_main_paras_t paras;
        const char* argv[] = {"main", "loop"};
        paras.argc = 2;
        paras.argv = argv; 
    
        linkkit_main((void *)&paras);
    }
}

void uart_task(void* parameter){
    app_uart_run();
}

void app_main(){
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "esp-aliyun verison: %s", HAL_GetEAVerison());
    ESP_LOGI(TAG, "iotkit-embedded version: %s", HAL_GetIEVerison());

    app_uart_init();
    app_wifi_init();

    // SNTP service uses LwIP, please allocate large stack space.
    xTaskCreate(mqtt_task, "mqtt_task", 10240, NULL, 5, NULL);
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
}

