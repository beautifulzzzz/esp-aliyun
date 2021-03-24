/*************************************************************************
	> File Name: app_nvs.c
	> Author: 
	> Mail: 
	> Created Time: Mon 09 Nov 2020 08:32:07 CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/pwm.h"
#include "cJSON.h"
#include "nvs.h"

static const char* TAG = "app nvs";

esp_err_t app_nvs_get_ssid_password(uint8_t *ssid, uint8_t *password){
    nvs_handle wifi_handle;
    size_t len_ssid = 100,len_password = 100;

    nvs_open("wifi", NVS_READONLY, &wifi_handle);

    esp_err_t ret1 = nvs_get_str(wifi_handle, "ssid", (char *)ssid, &len_ssid);
    esp_err_t ret2 = nvs_get_str(wifi_handle, "pswd", (char *)password, &len_password);

    nvs_close(wifi_handle);   

    if((ret1 == ESP_OK) && (ret2 == ESP_OK)){
        ESP_LOGI(TAG,"get_ssid = %s\r\n", ssid);
        ESP_LOGI(TAG,"get_pswd = %s\r\n", password);
        return ESP_OK;
    }

    return ESP_FAIL;
}


esp_err_t app_nvs_set_ssid_password(uint8_t *ssid, uint8_t *password){
    nvs_handle wifi_handle;

    nvs_open("wifi", NVS_READWRITE, &wifi_handle);
    esp_err_t ret1 = nvs_set_str(wifi_handle, "ssid", (char *)ssid);
    esp_err_t ret2 = nvs_set_str(wifi_handle, "pswd", (char *)password);

    ESP_LOGI(TAG,"save_ssid = %s\r\n", ssid);
    ESP_LOGI(TAG,"save_pswd = %s\r\n", password);
    nvs_close(wifi_handle);   

    return (ret1 && ret2);
}


esp_err_t app_nvs_get_reset(uint8_t *reset){
    nvs_handle reset_handle;
    size_t len_reset = 10;

    nvs_open("reset", NVS_READONLY, &reset_handle);

    esp_err_t ret1 = nvs_get_str(reset_handle, "reset", (char *)reset, &len_reset);

    nvs_close(reset_handle);   

    if(ret1 == ESP_OK){
        ESP_LOGI(TAG,"get_reset = %d\r\n", *reset);
        return ESP_OK;
    }

    return ESP_FAIL;
}


esp_err_t app_nvs_set_reset(uint8_t *reset){
    nvs_handle reset_handle;

    nvs_open("reset", NVS_READWRITE, &reset_handle);
    esp_err_t ret1 = nvs_set_str(reset_handle, "reset", (char *)reset);

    ESP_LOGI(TAG,"save_reset = %d\r\n", *reset);
    nvs_close(reset_handle);   

    return ret1;
}


