/*************************************************************************
	> File Name: app_uart.c
	> Author: 
	> Mail: 
	> Created Time: 2019年09月08日 星期日 05时22分33秒
 ************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "app_wifi.h"

////////////////////////////////////////////////////////////////////////////////////////
//内部函数
///////////////////////////////////////////////////////////////////////////////////////
#define BUF_SIZE (1024)
//////////////////////////////////////////////////////////////////
//基本运算函数-key-value字符串匹配
//////////////////////////////////////////////////////////////////
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef char s8;
typedef short s16;
typedef int s32;
static s16 find_char(u8 *buf,u8 len,u8 data){
    for(u8 i=0;i<len;i++)
        if(buf[i] == data)return i;
    return -1;
}

//{n:tao123,p:litao123456,d:00000002,s:hjjdHOxx3t082I4rNFOWqPtlu4l1fQcg,}
//{n:@PHICOMM_82,p:xxl22901,d:00000002,s:hjjdHOxx3t082I4rNFOWqPtlu4l1fQcg,}
static u8 string_op_get_key_value_by_key(u8 key_value_buf_len, u8 *key_value_buf,
                                u8 key_len, u8 *key,
                                u8 *value_len, u8 *value){

    if(key_value_buf_len == 0 || key_len == 0 || key_value_buf_len < key_len)
        return 0;

    u8 i=0,temp = key_value_buf[0];
    key_value_buf[0] = ',';//easy for search: ,xx:xx,xx:xx,

    while(i<key_value_buf_len){
        u8 base = i;
        s16 index1 = find_char(&key_value_buf[base],key_value_buf_len,',');
        if(index1 == -1)return 0;
        s16 index2 = find_char(&key_value_buf[base+index1],key_value_buf_len,':');
        if(index2 == -1)return 0;
        if(index2 -1 != key_len){
            i = base + index2 + index1;
            continue;
        }

        u8 find = 1;
        for(u8 j=0;j<key_len;j++){
            if(key_value_buf[base+index1+1+j] != key[j]){
                find = 0;
                i = base + index2 + index1;
                break;
            }
        }

        if(find){
            s16 index3 = find_char(&key_value_buf[base+index1+index2],key_value_buf_len,',');
            if(index3 == -1)return 0;
            *value_len = index3 - 1;
            for(u8 k=0;k<*value_len;k++){
                value[k] = key_value_buf[base+index1+index2+1+k];
            }
            key_value_buf[0] = temp;
            return 1;
        }
    }
    key_value_buf[0] = temp;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
void app_uart_init(void){
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL);
}

void app_uart_send_data(char *para, int len){
    uart_write_bytes(UART_NUM_0, (const char *)para, len);   
}

void app_uart_run(void){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // Write data back to the UART
        //uart_write_bytes(UART_NUM_0, (const char *) data, len);
        if(len){
            extern char device_name[20];     
            extern char device_secret[80];
            u8 device_name_len;
            u8 device_secret_len;
            char wifi_name[40];
            u8 wifi_name_len;
            char wifi_password[40];
            u8 wifi_password_len;

            u8 key[4] = {'n','p','d','s'};

            memset(device_name,0,20);
            memset(device_secret,0,80);
            if(!string_op_get_key_value_by_key(len,data,1,&key[0],&wifi_name_len,(u8 *)wifi_name))return;
            if(!string_op_get_key_value_by_key(len,data,1,&key[1],&wifi_password_len,(u8 *)wifi_password))return;
            if(!string_op_get_key_value_by_key(len,data,1,&key[2],&device_name_len,(u8 *)device_name))return;
            if(!string_op_get_key_value_by_key(len,data,1,&key[3],&device_secret_len,(u8 *)device_secret))return;
            
            app_wifi_set_name_password(wifi_name,wifi_name_len,wifi_password,wifi_password_len); 
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

