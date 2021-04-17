/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "app_entry.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "app_entry.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "sc";

void wifi_task(void * parm);
static void sc_callback(smartconfig_status_t status, void *pdata);
extern esp_err_t app_nvs_set_reset(uint8_t *reset);
extern esp_err_t app_nvs_get_reset(uint8_t *reset);

static void wifi_connection(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "tao",
            .password = "123456789",
        },
    };

    extern esp_err_t app_nvs_get_ssid_password(uint8_t *ssid, uint8_t *password);
    app_nvs_get_ssid_password(&wifi_config.sta.ssid, &wifi_config.sta.password);
    ESP_LOGI(TAG,"Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    esp_wifi_connect();
}

uint8_t wifi_state = 3;
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:{
        uint8_t reset = 0;
        if(ESP_OK == app_nvs_get_reset(&reset) && reset == 3){//已经配网则直接连接
            wifi_connection();
            wifi_state = 1;
        }else{//否则进入配网状态
            ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
            ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
            wifi_state = 0;
        }
        xTaskCreate(wifi_task, "wifi_task", 10240, NULL, 3, NULL);
        }break;
    case SYSTEM_EVENT_STA_GOT_IP:
        wifi_state = 2;
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:{
            ESP_LOGI(TAG, "SC_STATUS_LINK");

            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
           
            //将设备名字放在ssid后8字节,这里用于判断和取出
            char name[9] = "ffffffff";
            extern char device_name[];
            int strlen = 0;
            while(wifi_config->sta.ssid[strlen++] != 0x00);
            strlen--;
            if(strlen < 8)break;
            for(int i=0;i<8;i++){
                name[i] = wifi_config->sta.ssid[strlen-8+i];      
                if(name[i] != device_name[i])break;
            }
            wifi_config->sta.ssid[strlen-8] = 0x00;
#define TEST_DEBUG 0
#if TEST_DEBUG
            memcpy(wifi_config->sta.ssid,"1202",5);
            memcpy(wifi_config->sta.password,"1234567890",11);
#endif
            extern esp_err_t app_nvs_set_ssid_password(uint8_t *ssid, uint8_t *password);
            app_nvs_set_ssid_password(wifi_config->sta.ssid,wifi_config->sta.password);

            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            }
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}

void wifi_task(void * parm)
{
    EventBits_t uxBits;

    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");

            uint8_t reset = 3;
            app_nvs_set_reset(&reset);

            app_main_paras_t paras;
            const char* argv[] = {"main", "loop"};
            paras.argc = 2;
            paras.argv = argv; 
            linkkit_main((void *)&paras);
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}
