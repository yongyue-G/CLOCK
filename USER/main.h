#ifndef __MIAN_H
#define __MIAN_H
#include "stm32f4xx.h"  
#define URL "https://api.seniverse.com/v3/weather/now.json?key=SgnM3HG8EdMC2r4ms&location=Xuzhou&language=zh-Hans&unit=c"
typedef struct
{
    uint8_t month;
    uint8_t day;
    uint16_t year;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t week;
}time_t;

typedef struct
{
	time_t time;
    char city[32];
    char province[32];
    char update[32];//数据更新时间。记录这组天气数据最后一次被气象站更新的时间戳
    float tem_outdoor;
    float humidity;
    float tem_indoor;
    uint8_t weather;//天气代码，方便后期渲染图片和查询天气
}weather_info_t;

static uint8_t Is_Leap_Year(uint16_t year) ;

void Local_Time_Tick(time_t *tm);
extern weather_info_t weather_info;
#endif
