/*************************************************************************
	> File Name: cpt_timing.c
	> Author: 
	> Mail: 
	> Created Time: Thu 01 Aug 2019 21:36:45 CST
 ************************************************************************/
#include "board.h"
#include "cpt_rtc.h"
#include "cpt_timing.h"


void cpt_timing_init(struct ITiming *pITiming);
void cpt_timing_set(struct ITiming *pITiming,u8 enable,u16 time,u8 repet,u8 day,u8 (*cb)(u8 index));
void cpt_timing_kill(struct ITiming *pITiming);
void cpt_timing_run(struct ITiming *pITiming, u8 num);



///////////////////////////////////////////////////////////////////////
struct ITimingFunc mITimingFunc = {
    .init = &cpt_timing_init,
    .set = &cpt_timing_set,
    .kill = &cpt_timing_kill,
    .run = &cpt_timing_run,
};
///////////////////////////////////////////////////////////////////////


void cpt_timing_init(struct ITiming *pITiming){
    memset((u8 *)pITiming,0,sizeof(struct ITiming)); 
}

void cpt_timing_set(struct ITiming *pITiming,u8 enable,u16 time,u8 repet,u8 day,u8 (*cb)(u8 index)){
    pITiming->enable = (enable == 0?0:1);
    pITiming->time = time;
    pITiming->repet = (repet == 0?0:1);
    pITiming->day = day;
    pITiming->timing_end_cb = cb;
}

void cpt_timing_kill(struct ITiming *pITiming){
    pITiming->enable = 0;
}

void cpt_timing_run(struct ITiming *pITiming, u8 num){
    //需要 delay 20s 防止重复

    static u8 pre_minute = 61;//防止同一分钟执行多次动作

    //1.获取当前星期和Hour:Minute
    cpt_rtc_date_s date;
    cpt_rtc.get_date(&date);

    if(pre_minute == date.minute || date.year == 0)return;

    u16 time = (u16)date.hour*60+(u16)date.minute;
    pre_minute = date.minute;

    for(u8 i=0;i<num;i++){
        struct ITiming *p = &pITiming[i];
        if(p->enable == 1){
            //                PR_DEBUG("[T_N] %x \r\n", time);
            //                PR_DEBUG("[T_T] %x \r\n", p->time);
            //2.比较是否到达定时
            if(time == p->time){
                //PR_DEBUG("[T=T]\r\n");
                //3.分别处理重复定时和非重复定时
                if(p->repet == 1){
                    //4.判断week是否匹配
                    if((((p->day)>>(7-date.weekday))&0x01) == 0x01){
                        //5.如果匹配执行动作
                        if(p->timing_end_cb != NULL)
                            p->timing_end_cb(i);
                    }
                }else{
                    //4.判断week是否匹配
                    if((((p->day)>>(7-date.weekday))&0x01) == 0x01){
                        //5.如果匹配执行动作
                        if(p->timing_end_cb != NULL)
                            p->timing_end_cb(i);
                        //6.失能
                        p->enable = 0;
                    }
                }
            }
        }
    }
} 

