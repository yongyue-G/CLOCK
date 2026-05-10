#include "stm32f4xx.h"
#include "wifi_usart.h"
#include "log.h"
#include "timer.h"
#include "AT.h"
#include "main.h"
#include "LCD.h"
#include "SPI.h"
#include "LCD_GUI.h"
#include "DHT11.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "gui_guider.h"           // Gui Guider 生成的界面和控件的声明
#include "events_init.h"          // Gui Guider 生成的初始化事件、回调函数
weather_info_t weather_info={
	.city="徐州",
	.humidity=0.0,
	.tem_indoor=0.0,
	.tem_outdoor=0.0,
	.weather=0,
	.time={
		.week = 3,
		.month = 1,
		.sec = 0,
		.min = 0,
		.hour = 0,
		.day = 1,
		.year = 2026,
	},
};//只要初始化一个其他默认初始化为0
lv_ui guider_ui;

int main(void)
{


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	TIM4_Init();
			LCD_Init(NULL);
			lv_init();
			lv_port_disp_init();// 初始化lvgl显示设备
			lv_demo_widgets();
			setup_ui(&guider_ui);           // 初始化 UI
			events_init(&guider_ui);       // 初始化 事件

//	log_Init();
//	AT_Init();
//	DHT11_GPIO_Init();

	while(1)
	{
		lv_task_handler();
	}
	

}

