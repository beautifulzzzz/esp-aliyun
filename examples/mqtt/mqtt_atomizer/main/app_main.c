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
#include "nvs_flash.h"

#include "esp_log.h"

#include "app_entry.h"
#include "platform_hal.h"




/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;//没连接上wifi不会执行MQTT
static const char* TAG = "app main";
extern void initialise_wifi(void);
extern void gpio_task_init(void);

void app_main(){
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "esp-aliyun verison: %s", HAL_GetEAVerison());
    ESP_LOGI(TAG, "iotkit-embedded version: %s", HAL_GetIEVerison());

    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    gpio_task_init();
}

