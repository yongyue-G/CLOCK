#ifndef __MIAN_H
#define __MIAN_H
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "task.h" 
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

typedef enum
{
    /* 阶段 1：系统与外设初始化状态 (用于开机进度条)        */
    EVT_AT_INITED         = (1 << 0),  // ESP32 模块唤醒并初始化成功
    EVT_DHT11_INITED      = (1 << 1),  // DHT11 传感器自检成功
    EVT_LOG_INITED        = (1 << 2),  // 调试串口初始化成功
    EVT_GUI_INIT_DONE     = (1 << 3),  // LVGL 框架及屏幕驱动初始化完成
    EVT_HOMEPAGE_DONE     = (1 << 4),  // LVGL 主页面加载完成 (允许数据刷屏)

    /* 阶段 2：网络状态监控 (用于断线重连与图标显示)        */
    EVT_WIFI_NEED_CONNECT = (1 << 5),  // [命令] 呼叫后台任务去连接 WiFi
    
    /* 阶段 3：传感器与网络数据就绪 (核心驱动UI刷新的信号)  */
    EVT_WEATHER_READY     = (1 << 8),  // 网络天气数据成功解析进全局结构体
    EVT_INDOOR_READY      = (1 << 9),  // DHT11 室内温湿度已更新至全局结构体
    EVT_TIME_READY        = (1 << 10), // 时间走秒或网络对时完成
    EVT_DATE_READY        = (1 << 11), // 日期发生跨天变更

    /* 阶段 4：底层系统控制                                 */
    EVT_UART_RX_READY     = (1 << 12), // 串口收到一包完整数据 (可用信号量代替，看你喜欢)
    EVT_EXCEPTION         = (1 << 13)  // 发生超时或死锁，触发异常拯救任务

} system_event_t;

extern char ssid[64];
extern char password[64];
extern EventGroupHandle_t g_sys_event;
extern SemaphoreHandle_t sem_at;
extern SemaphoreHandle_t sem_ui;
extern xTaskHandle xLogTaskHandle;
#define TIME_CHECK_AT  3000
#define TIME_EXCEPTION 30000
#define TIME_IDLE      1000
#define TIME_DHT11     2000   // 1.5秒
#define TIME_WIFI      2000   // 5秒
#define TIME_HTTP      30000  // 30秒
#define TIME_GET_TIME  10000  // 1分钟

#define CHECK_DELAY    1000  // 1000 ms

#define TIME_SEM_TAKE  portMAX_DELAY

static uint8_t Is_Leap_Year(uint16_t year) ;

void Local_Time_Tick(time_t *tm);
extern weather_info_t weather_info;
#endif
