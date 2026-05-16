#include "Init_Page.h"
#include "stm32f4xx.h"
#include "wifi_usart.h"
#include "log.h"
#include "timer.h"
#include "AT.h"
#include "LCD.h"
#include "DHT11.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "gui_guider.h"           // Gui Guider 生成的界面和控件的声明
#include "events_init.h"          // Gui Guider 生成的初始化事件、回调函数
lv_ui guider_ui;

void boot_delay_ms(uint32_t ms)
{
    uint32_t t = 0;
    while(t < ms)
    {
        lv_task_handler(); // 告诉 LVGL 快去画图
        delay_ms(5);       // CPU 真实延时 5ms
        t += 5;
    }
}

void update_init_progress(uint8_t percent)
{
    // 【关键修改】：这里的 LV_ANIM_OFF 绝对不能改！
    lv_bar_set_value(guider_ui.screen_init_bar_progress, percent, LV_ANIM_OFF);
    lv_label_set_text_fmt(guider_ui.screen_init_label_progress, "%d%%", percent);
    
    // 连呼两次，确保 LVGL 内部的排版计算彻底完成
    lv_task_handler(); 
    lv_task_handler();
}



void Init_page(void)
{
			LCD_Init(NULL);
			lv_init();
			lv_port_disp_init();// 初始化lvgl显示设备
			setup_ui(&guider_ui);           // 初始化 UI
			events_init(&guider_ui);       // 初始化 事件
			update_init_progress(20);
			log_Init();
			update_init_progress(40);
			AT_Init();
			update_init_progress(60);
			DHT11_GPIO_Init();
			update_init_progress(80);
    if(!guider_ui.screen_home) 
    {
        // 呼叫 GUI Guider 生成 screen_home 界面
        setup_scr_screen_home(&guider_ui); 
    }
    // 现在它不是空指针了，安全加载！
    lv_scr_load(guider_ui.screen_home);
}
