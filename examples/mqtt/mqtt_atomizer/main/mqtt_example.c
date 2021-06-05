/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "app_entry.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"

#include "board.h"
#include "cpt_rtc.h"

#if 1
    #define PRODUCT_KEY                     "a10zictDXlp"
    #define PRODUCT_SECRET                  "8yDKzx1K6KC6RnMh"
    #define al_device128 1
#if al_device121
    char device_name[20] = "00000121";     
    char device_secret[80] ="1cde0b8e7b9f688ce8110f712c57aa3c";
#elif al_device122
    char device_name[20] = "00000122";     
    char device_secret[80] ="6eb193f7c1a3f430b0b412e6ed439514";
#elif al_device123
    char device_name[20] = "00000123";     
    char device_secret[80] ="8afc44c65c4af6a808fe5f074dbeef81";
#elif al_device124
    char device_name[20] = "00000124";     
    char device_secret[80] ="9ca559b3fa5a4a0bdca1eaf37781b389";
#elif al_device125
    char device_name[20] = "00000125";     
    char device_secret[80] ="c924632016e6780fe434475dfe151069";
#elif al_device126
    char device_name[20] = "00000126";     
    char device_secret[80] ="fa8cbb276a91c9f3dae861b1e43ea61c";
#elif al_device127
    char device_name[20] = "00000127";     
    char device_secret[80] ="291db7499752585d5a22e92d01f38ca6";
#elif al_device128
    char device_name[20] = "00000128";     
    char device_secret[80] ="e65c18b8c69fea44092e3c6ea43524f6";
#elif al_device129
    char device_name[20] = "00000129";     
    char device_secret[80] ="bcdd69e17ea1cc9cf26ee5fc1dc7cdf4";
#elif al_device130
    char device_name[20] = "00000130";     
    char device_secret[80] ="994ac284597a82dc7c071992fac064e0";
#else
#endif

#else
    #define PRODUCT_KEY                     "a1QynQ2BCLJ"
    #define PRODUCT_SECRET                  "GGhRmKHLS632t8WX"
    #define al_device1 1
    #if al_device1
        char device_name[20] = "device1";     
        char device_secret[80] ="TbJkO9e2cMusqkDzXdasoVVJe7OdgjXx";
    #elif al_device2
        char device_name[20] = "device2";     
        char device_secret[80] ="139ce198184a2fa37d753de23a3e3750";
    #else

    #endif
#endif

char device_link_buf[300];
static char *get_topic_update(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/%s/%s/user/update",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_topic_error(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/%s/%s/user/update/error",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_topic_get(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/%s/%s/user/get",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_property_post(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/sys/%s/%s/thing/event/property/post",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_property_post_reply(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/sys/%s/%s/thing/event/property/post_reply",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_property_set(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/sys/%s/%s/thing/service/property/set",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_info_update(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/sys/%s/%s/thing/deviceinfo/update",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_ntp_request(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/ext/ntp/%s/%s/request",PRODUCT_KEY,device_name);
    return device_link_buf;
}
static char *get_device_ntp_response(void){
    memset(device_link_buf,0,sizeof(device_link_buf));
    sprintf(device_link_buf,"/ext/ntp/%s/%s/response",PRODUCT_KEY,device_name);
    return device_link_buf;
}



#define TOPIC_UPDATE                    get_topic_update() 
#define TOPIC_ERROR                     get_topic_error()
#define TOPIC_GET                       get_topic_get()

#define DEVICE_PROPERTY_POST            get_device_property_post() 
#define DEVICE_PROPERTY_POST_REPLY      get_device_property_post_reply() 
#define DEVICE_PROPERTY_SET             get_device_property_set() 
#define DEVICE_INFO_UPDATE              get_device_info_update() 

#define DEVICE_NTP_REQUEST              get_device_ntp_request()
#define DEVICE_NTP_RESPONSE             get_device_ntp_response()


/* These are pre-defined topics */
/*
#define TOPIC_UPDATE                    "/"PRODUCT_KEY"/"DEVICE_NAME"/user/update"
#define TOPIC_ERROR                     "/"PRODUCT_KEY"/"DEVICE_NAME"/user/update/error"
#define TOPIC_GET                       "/"PRODUCT_KEY"/"DEVICE_NAME"/user/get"

#define DEVICE_PROPERTY_POST            "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define DEVICE_PROPERTY_POST_REPLY      "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define DEVICE_PROPERTY_SET             "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define DEVICE_INFO_UPDATE              "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/deviceinfo/update"
*/


#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char   **user_argv;

static const char *TAG = "MQTT";

static char update_flag = 1;
typedef struct{
    bool PowerSwitch;
    bool OilShortage;
    char SprayLevel;
    char timer[8][50];
    char time_syn[50];
}dps_s;

dps_s dps = {
    .PowerSwitch = 1,
    .OilShortage = 0,
    .SprayLevel = 0,
    .time_syn = "1512038504",
};

char mqtt_get_dp_value(const char *jsonRoot){
    // jsonRoot 是您要剖析的数据
    //首先整体判断是否为一个json格式的数据
	cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
	//如果是否json格式数据
	if (pJsonRoot !=NULL) {
        char send_data[40];
        int send_data_len;
        cJSON *pParams = cJSON_GetObjectItem(pJsonRoot, "params");
        if(pParams != NULL){

            cJSON *pValue =  cJSON_GetObjectItem(pParams, "PowerSwitch");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: PowerSwitch:%d",pValue->valueint);
                dps.PowerSwitch = pValue->valueint;      
                    
                send_data_len = sprintf(send_data,"PowerSwitch:%d",dps.PowerSwitch);
                ESP_LOGI(TAG,"%s",send_data);
                //app_uart_send_data(send_data,send_data_len);
            }

            pValue =  cJSON_GetObjectItem(pParams, "OilShortage");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: OilShortage:%d",pValue->valueint);
                dps.OilShortage = pValue->valueint;
            }

            pValue =  cJSON_GetObjectItem(pParams, "Reset");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: Reset:%d",pValue->valueint);
                if(1 == pValue->valueint){//
                    
                }
            }


            pValue =  cJSON_GetObjectItem(pParams, "SprayLevel");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: SprayLevel:%d",pValue->valueint);
                dps.SprayLevel = pValue->valueint;

                send_data_len = sprintf(send_data,"SprayLevel:%d",dps.SprayLevel);
                //app_uart_send_data(send_data,send_data_len);
            }

            pValue =  cJSON_GetObjectItem(pParams, "time_syn");
            if(pValue != NULL){
                EXAMPLE_TRACE("dp: time_syn:%s",pValue->valuestring);
                memcpy(dps.time_syn,pValue->valuestring,strlen(pValue->valuestring)+1);
                
                send_data_len = sprintf(send_data,"time_syn:%s",dps.time_syn);
                //app_uart_send_data(send_data,send_data_len);
            }

            char time_n[] = "timer1";
            for(char i=1;i<9;i++){
                time_n[5] = '0'+i;

                pValue =  cJSON_GetObjectItem(pParams, time_n);
                if(pValue != NULL){
                    EXAMPLE_TRACE("dp: timer%d:%s",i,pValue->valuestring);
                    memcpy(&(dps.timer[i-1][0]),pValue->valuestring,strlen(pValue->valuestring)+1);

                    send_data_len = sprintf(send_data,"timer%d:%s",i,&(dps.timer[i-1][0]));

                    extern int  app_timing_init_from_string(char *str, u8 str_len);
                    if(0 == app_timing_init_from_string(pValue->valuestring,strlen(pValue->valuestring))){
                        u8 timer_id = pValue->valuestring[0]-'0' - 1;
                        extern esp_err_t app_nvs_set_timing(char *timing, uint8_t index );
                        app_nvs_set_timing(pValue->valuestring,timer_id);
                    }
                }
            }
       
            update_flag = 1;
        }
    }    

    return 1;
}

char mqtt_get_ntp_value(const char *jsonRoot){
    // jsonRoot 是您要剖析的数据
    //首先整体判断是否为一个json格式的数据
	cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
	//如果是否json格式数据
	if (pJsonRoot !=NULL) {
        cJSON *pParams = cJSON_GetObjectItem(pJsonRoot, "serverSendTime");
        if(pParams != NULL){
            EXAMPLE_TRACE("ntp:%ss",pParams->valuestring);
            int str_len = strlen(pParams->valuestring); 
            if(str_len > 3){
                pParams->valuestring[str_len-3] = 0x00;
                int _date_ = atoi(pParams->valuestring);
                extern void rtc_update_time(u32 time);
                rtc_update_time(_date_);

                extern void app_timing_init_from_flash(void);
                app_timing_init_from_flash();
            }
        }
    }
    return 0;
}


void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
/*
    char send_data[40];
    int send_data_len;
    send_data_len = sprintf(send_data,"MQTTState:%d",msg->event_type);
    //app_uart_send_data(send_data,send_data_len);
*/
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt     ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("----");
            EXAMPLE_TRACE("PacketId: %d", ptopic_info->packet_id);
            EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                          ptopic_info->topic_len,
                          ptopic_info->ptopic,
                          ptopic_info->topic_len);
            EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                          ptopic_info->payload_len,
                          ptopic_info->payload,
                          ptopic_info->payload_len);
            EXAMPLE_TRACE("----");

            if(memcmp(ptopic_info->ptopic,DEVICE_PROPERTY_SET,ptopic_info->topic_len) == 0)
                mqtt_get_dp_value(ptopic_info->payload);

            else if(memcmp(ptopic_info->ptopic,DEVICE_NTP_RESPONSE,ptopic_info->topic_len) == 0)
                mqtt_get_ntp_value(ptopic_info->payload);

        
            break;
        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

void mqtt_client_get_date(void *pclient){
    char msg_pub[256];
    iotx_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    /* Initialize topic information */
    memset(msg_pub, 0x0, 256);
    strcpy(msg_pub, "{\"deviceSendTime\":\"0\"}");
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    int rc = IOT_MQTT_Publish(pclient, DEVICE_NTP_REQUEST, &topic_msg);
    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", DEVICE_NTP_RESPONSE, topic_msg.payload, rc);

    IOT_MQTT_Yield(pclient, 200);
}

void mqtt_client_hello(void *pclient){
    char msg_pub[256];
    iotx_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    /* Initialize topic information */
    memset(msg_pub, 0x0, 256);
    strcpy(msg_pub, "data: hello! start!");
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    int rc = IOT_MQTT_Publish(pclient, TOPIC_UPDATE, &topic_msg);
    EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_UPDATE, topic_msg.payload, rc);

    IOT_MQTT_Yield(pclient, 200);
}

void mqtt_client_upload_all(void *pclient){
    char msg_pub[256];
    iotx_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    /* Initialize topic information */
    memset(msg_pub, 0x0, 256);
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    
    static int update_flag = 1;
    if(update_flag){
        update_flag = 0;
        
        snprintf(msg_pub, sizeof(msg_pub), 
                "{\"method\":\"thing.event.property.post\",\"id\":\"7\",\"version\":\"1.0\",\"params\":{\"MACAddress\":\"E1E1E2E3E4E7\",\"batpt\":100,\"PowerSwitch\":%d,\"OilShortage\":%d,\"SprayLevel\":%d,\"time_syn\":\"%s\"}}",
                dps.PowerSwitch, dps.OilShortage, dps.SprayLevel, dps.time_syn);

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = strlen(msg_pub);

        int rc = IOT_MQTT_Publish(pclient, DEVICE_PROPERTY_POST, &topic_msg);
        if (rc < 0) {
            EXAMPLE_TRACE("error occur when publish");
        }
        EXAMPLE_TRACE("packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);
    }

    /* handle the MQTT packet received from TCP or SSL connection */
    IOT_MQTT_Yield(pclient, 1000);
}

int mqtt_client(void)
{
    int rc, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, device_name, device_secret, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        return -1;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 24000;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //send data to topic
    /* Initialize topic information */

    //////////////////////////////////////////////////////////////////////////////////////////////
    //Subscribe a topic, then send a top
    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_PROPERTY_SET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_INFO_UPDATE, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_GET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_PROPERTY_POST_REPLY, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_PROPERTY_POST_REPLY, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }
    rc = IOT_MQTT_Subscribe(pclient, DEVICE_NTP_RESPONSE, IOTX_MQTT_QOS0, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }

    IOT_MQTT_Yield(pclient, 200);

    HAL_SleepMs(2000);

    //////////////////////////////////////////////////////////////////////////////////////////////
    //then send a top
    mqtt_client_hello(pclient);
    mqtt_client_get_date(pclient);

    do {
        /* Generate topic message */
        cnt++;
        
        mqtt_client_upload_all(pclient);
        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            //HAL_SleepMs(2000);
            //cnt = 0;
        }
        //ESP_LOGI(TAG, "min:%u heap:%u", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
    } while (cnt);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Unsubscribe(pclient, DEVICE_PROPERTY_SET);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

    return 0;
}

int linkkit_main(void *paras)
{
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = 0;
    user_argv = NULL;

    if (paras != NULL) {
        app_main_paras_t *p = (app_main_paras_t *)paras;
        user_argc = p->argc;
        user_argv = p->argv;
    }

    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(device_name);
    HAL_SetDeviceSecret(device_secret);
    HAL_SetProductSecret(PRODUCT_SECRET);
    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login  Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    mqtt_client();
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);

    EXAMPLE_TRACE("out of sample!");

    return 0;
}
