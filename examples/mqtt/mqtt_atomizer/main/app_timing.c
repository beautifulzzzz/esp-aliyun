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
#include "cpt_timing.h"

static const char *TAG = "APP TIMING";

static u8 rtc_update_flag = 0;
#define MAX_NUM_TIMING 16
static struct ITiming mITiming[MAX_NUM_TIMING];
typedef struct{
    u8 is_on[MAX_NUM_TIMING/2];
    u16 from[MAX_NUM_TIMING/2];//
    u16 to[MAX_NUM_TIMING/2];//
    u8 day[MAX_NUM_TIMING/2];
    u16 duty[MAX_NUM_TIMING/2];//每隔n分钟
    u16 keep[MAX_NUM_TIMING/2];//单次持续喷m秒
}my_timing_s;

my_timing_s my_timing;

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

extern void app_atomizer_on(int enable);
void rtc_task_run(void *arg){
    u8 times = 0;
    u32 second = 0;
    u8 count = 0;
    u8 step = 0;//0: pen; 1: not pen
    while(1<2){
        vTaskDelay(200 / portTICK_RATE_MS);
        times++;
        if(times > 4){
            times = 0;
            if(rtc_update_flag){
                cpt_rtc.run();
                mITimingFunc.run(mITiming,MAX_NUM_TIMING);

                cpt_rtc_date_s cpt_rtc_date;
                cpt_rtc.get_date(&cpt_rtc_date);
                /*
                   ESP_LOGI(TAG,"%04d-%02d-%02d %02d:%02d:%02d",
                   cpt_rtc_date.year,
                   cpt_rtc_date.month,
                   cpt_rtc_date.monthday,
                   cpt_rtc_date.hour,
                   cpt_rtc_date.minute,
                   cpt_rtc_date.second);
                   */
                int i;
                for(i=0;i<MAX_NUM_TIMING/2;i++){
                    u16 now = cpt_rtc_date.hour*60+cpt_rtc_date.minute;
                    if(now >= my_timing.from[i] && now < my_timing.to[i] && ((1<<(8-cpt_rtc_date.weekday)) & my_timing.day[i]) != 0){
                        ESP_LOGI(TAG,"on:%d",i);

                        if(step == 0){//pen
                            app_atomizer_on(1);
                            second++;
                            if(second >= my_timing.keep[i]){
                                step = 1;
                                second = 0;
                            }
                        }else{//not pen
                            app_atomizer_on(0);
                            second++;
                            if(second >= my_timing.duty[i]){
                                step = 0;
                                second = 0;
                            }
                        }
                        //ESP_LOGI(TAG,"step:%d,second:%d",step,second);
                        break;
                    }
                }
                if(i==MAX_NUM_TIMING/2){
                    ESP_LOGI(TAG,"not on all");
                    step = 0;
                    second = 0;
                    app_atomizer_on(0);
                }
            }
        }
    } 
}

u8 timing_start_cb(u8 index){
    ESP_LOGI(TAG,"TIMING[%d] CB!\n",index);
    cpt_rtc_date_s date2;
    cpt_rtc.get_date(&date2);

    ESP_LOGI(TAG,"%04d/%02d/%02d %02d:%02d:%02d [星期:%d]\n",
            date2.year,
            date2.month,
            date2.monthday,
            date2.hour,
            date2.minute,
            date2.second,
            date2.weekday);

    return 1;
}

u8 timing_end_cb(u8 index){
    ESP_LOGI(TAG,"TIMING[%d] CB!\n",index);
    cpt_rtc_date_s date2;
    cpt_rtc.get_date(&date2);

    ESP_LOGI(TAG,"%04d/%02d/%02d %02d:%02d:%02d [星期:%d]\n",
            date2.year,
            date2.month,
            date2.monthday,
            date2.hour,
            date2.minute,
            date2.second,
            date2.weekday);

    return 1;
}

u8 timing_work_mode_set(u8 timer_id,u8 is_on,u16 time_from,u16 time_to,u8 is_repet,u8 day, u16 _duty, u16 _keep){
	if(timer_id >= 8)return 0;
	mITimingFunc.set(&mITiming[2*timer_id],1,time_from,is_repet,day,&timing_start_cb);
	mITimingFunc.set(&mITiming[2*timer_id+1],1,time_to,is_repet,day,&timing_end_cb);

    my_timing.is_on[timer_id] = is_on;
    my_timing.from[timer_id] = time_from;
    my_timing.to[timer_id] = time_to;
    my_timing.day[timer_id] = day;
	my_timing.duty[timer_id] = _duty;
    my_timing.keep[timer_id] = _keep;

    return 1;
}

int  app_timing_init_from_string(char *str, u8 str_len){
    //app_uart_send_data(send_data,send_data_len);
    {//timer2:2|4:00|06:20|1234567|2|10|1
        //timer1:1|01:31|01:40|1234567|1|5|1
        int pos,pos2;
        int hour,min,from;
        int hour2,min2,to;
        int repet_num,repet_flag = 0;
        int duty,keep;
        int enable;
        //timer_id
        u8 timer_id = str[0]-'0' - 1;
        if(timer_id >= 8)return 1;

        //from hour
        for(pos = 2;pos < str_len;pos++){
            if(str[pos] == ':')break;
        }
        if(pos-2 == 1){
            hour = str[pos-1]-'0';
        }else if(pos-2 ==2){
            hour = str[pos-1]-'0'+(str[pos-2]-'0')*10;
        }else{
            return 1;
        }
        //from min(pos = 3, pos2 =6)
        //from min(pos = 4, pos2 =7)
        for(pos2 = pos+1;pos2 < str_len;pos2++){
            if(str[pos2] == '|')break;
        }
        if(pos2-pos-1 == 1){
            min = str[pos2-1]-'0';
        }else if(pos2-pos-1 ==2){
            min = str[pos2-1]-'0'+(str[pos2-2]-'0')*10;
        }else{
            return 1;
        }

        //to hour (pos2 = 6,pos =9)
        for(pos = pos2+1;pos < str_len;pos++){
            if(str[pos] == ':')break;
        }
        if(pos-pos2-1 == 1){
            hour2 = str[pos-1]-'0';
        }else if(pos-pos2-1 ==2){
            hour2 = str[pos-1]-'0'+(str[pos-2]-'0')*10;
        }else{
            return 1;
        }

        //to min (pos = 9,pos2 =12)
        for(pos2 = pos+1;pos2 < str_len;pos2++){
            if(str[pos2] == '|')break;
        }
        if(pos2-pos-1 == 1){
            min2 = str[pos2-1]-'0';
        }else if(pos2-pos-1 ==2){
            min2 = str[pos2-1]-'0'+(str[pos2-2]-'0')*10;
        }else{
            return 1;
        }

        from = hour*60+min;
        to = hour2*60+min2;

        //repet (pos2 = 12, pos = 20)
        for(pos = pos2+1;pos < str_len;pos++){
            if(str[pos] == '|')break;
        }
        repet_num = pos-pos2-1; 
        if(repet_num < 8){
            repet_flag = 0;
            for(int j=0;j<repet_num;j++){
                u8 x = str[pos2+1+j]-'0';
                if(x>7)return 1;
                repet_flag = repet_flag | (1<<(8-x));
            } 
        }else{
            return 1;
        }

        //keep(pos = 20,pos2 = 22)
        for(pos2 = pos+1;pos2 < str_len;pos2++){
            if(str[pos2] == '|')break;
        }
        repet_num = pos2-pos-1; 
        if(repet_num < 4){
            keep = 0;
            for(int j=0;j<repet_num;j++){
                u8 x = str[pos+1+j]-'0';
                if(x>9)return 1;
                keep = keep*10 + x;
            } 
        }else{
            return 1;
        }

        //duty(pos2 = 22,pos =25)
        for(pos = pos2+1;pos < str_len;pos++){
            if(str[pos] == '|')break;
        }
        repet_num = pos-pos2-1; 
        if(repet_num < 8){
            duty = 0;
            for(int j=0;j<repet_num;j++){
                u8 x = str[pos2+1+j]-'0';
                if(x>9)return 1;
                duty = duty*10 + x;
            } 
        }else{
            return 1;
        }

        //enable
        enable = str[pos+1] == '1'?1:0;

        ESP_LOGI(TAG,"id:%d,from:%d,to:%d,repet:%x,duty:%d,keep:%d,enable:%d",
                timer_id,from,to,repet_flag,duty,keep,enable);

        timing_work_mode_set(timer_id,enable,from,to,1,repet_flag,duty,keep);
    }
    return 0;
}

void app_timing_init_from_flash(void){
    extern esp_err_t app_nvs_get_timing(char *timing,uint8_t index);
    char timing[100];
    for(int i=0;i<8;i++){
        if(ESP_OK == app_nvs_get_timing(timing,i)){
            app_timing_init_from_string(timing,strlen(timing));
        } 
    } 
}

void app_timing_init(void){
    cpt_rtc_params_s cpt_rtc_params;
    cpt_rtc.init(&cpt_rtc_params);

    for(int i=0;i<MAX_NUM_TIMING;i++)
        mITimingFunc.init(&mITiming[i]);

    xTaskCreate(rtc_task_run, "app rtc_task", 4096, NULL, 2, NULL);
}
