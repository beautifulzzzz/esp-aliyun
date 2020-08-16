/*************************************************************************
	> File Name: app_wifi.c
	> Author: 
	> Mail: 
	> Created Time: 2019年09月08日 星期日 05时22分21秒
 ************************************************************************/
#include <string.h>
#include "esp_wifi.h"
#include "app_uart.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

static const char* TAG = "app wifi";
esp_err_t app_wifi_event_handler(void *ctx, system_event_t *event);

extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;//没连接上wifi不会执行MQTT

void app_wifi_init(void){
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(app_wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_wifi_set_name_password(char *name, int name_len, char *password, int password_len){
    static char pre_name[40];
    static char pre_pass[40];

    if(!memcmp(name,pre_name,name_len) && !memcmp(password,pre_pass,password_len))return;
    memcpy(pre_name,name,name_len);
    memcpy(pre_pass,password,password_len);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "鼎典设计",
            .password = "57177001",
        },
    };

    memset(wifi_config.sta.ssid,0,sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.ssid,name,name_len);
    memset(wifi_config.sta.password,0,sizeof(wifi_config.sta.password));
    memcpy(wifi_config.sta.password,password,password_len);

    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_connect() );   
}

esp_err_t app_wifi_event_handler(void *ctx, system_event_t *event){
    char send_data[40];
    int send_data_len;
    send_data_len = sprintf(send_data,"WiFiState:%d",event->event_id);
    app_uart_send_data(send_data,send_data_len);
    /*
     * 4->7 (已经输入好了wifi密码，4连接，7得到ip）
     * 将wifi热点关闭：5周期输出（disconnect)
     */
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP8266 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;

        default:
            break;
    }

    return ESP_OK;
}


