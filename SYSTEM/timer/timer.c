#include "stm32f4xx.h"                  // Device header
#include "freertos.h"
#include "task.h"
#include "timer.h"
#include "lvgl.h"  // ±ØÐë¼ÓÉÏÕâ¸ö£¬²ÅÄÜÈÏÊ¶ lv_tick_inc

void TIM4_Init(void)//Êµ¼ÊÊÇÎªÁËÑÓÊ±²»ÊÇÖÐ¶Ï
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//1·ÖÆµ
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//ÏòÉÏ¼ÆÊý
    TIM_TimeBaseStructure.TIM_Prescaler=84-1;
    TIM_TimeBaseStructure.TIM_Period=0xFFFF;
    
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
//		// ÏÈÇå³ý¸üÐÂÖÐ¶Ï±êÖ¾£¨½¨Òé£©
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

//		// Ê¹ÄÜ TIM4 µÄ¸üÐÂÖÐ¶Ï
//		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

//		// ÉèÖÃ NVIC ÖÐ¶ÏÓÅÏÈ¼¶£¨Ò»¶¨ÒªÐ´£©
//		NVIC_InitTypeDef NVIC_InitStructure;
//		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM4,ENABLE);
}


extern void xPortSysTickHandler(void);
//systick ÖÐ¶Ï·þÎñº¯Êý,Ê¹ÓÃ OS Ê±ÓÃµ½
void SysTick_Handler(void)
{ 
 if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//ÏµÍ³ÒÑ¾­ÔËÐÐ
 {
    xPortSysTickHandler();
 }
 lv_tick_inc(1);
}
void delay_us(uint32_t us)
{
    // ÄãµÄ TIM4 ÒÑ¾­ÅäÖÃÎª 84·ÖÆµ (1MHz)£¬ËùÒÔ TIM4->CNT Ã¿¹ý 1 Î¢Ãë¼Ó 1
    uint16_t start = TIM4->CNT; 
    
    // Ô­µØËÀµÈ£¬Ö±µ½¶¨Ê±Æ÷×ß¹ýµÄ²îÖµ´ïµ½Ö¸¶¨µÄÎ¢ÃëÊý
    // (ÀûÓÃ 16 Î»ÎÞ·ûºÅÊýµÄÒç³öÌØÐÔ£¬ÄÄÅÂ¶¨Ê±Æ÷ÖÐÍ¾¹éÁãÁËÒ²ÄÜËã³öÕýÈ·²îÖµ)
    while((uint16_t)(TIM4->CNT - start) < us); 
}

void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}


////²»ÄÜ´òÁ½·Ý¹¤
//void TIM4_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//	{
//		ms_tick++;  // Ã¿ 1 ms Ôö¼ÓÒ»´Î

//		lv_tick_inc(1);
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//	}
//}

volatile uint32_t FreeRTOSRunTimeTicks = 0; // å…¨å±€çš„é«˜ç²¾åº¦æ—¶é—´å˜é‡

// è¿™ä¸ªå‡½æ•°å°±æ˜¯æä¾›ç»™ FreeRTOSConfig.h è°ƒç”¨çš„
void ConfigureTimeForRunTimeStats(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // å‡è®¾ç³»ç»Ÿæ—¶é’Ÿ 168MHzï¼ŒAPB1 ä¸º 84MHz
    TIM_TimeBaseStructure.TIM_Period = 50 - 1;       // 50 å¾®ç§’è§¦å‘ä¸€æ¬¡
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;    // åˆ†é¢‘åŽæ˜¯ 1MHz (1å¾®ç§’è·‘ä¸€æ¬¡)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // ä¼˜å…ˆçº§å¯ä»¥è®¾ç½®é«˜ä¸€ç‚¹
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

// TIM3 ä¸­æ–­æœåŠ¡å‡½æ•°
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        FreeRTOSRunTimeTicks++; // é«˜é¢‘è‡ªå¢žï¼Œæä¾›æžå…¶ç²¾å‡†çš„æ—¶é—´åŸºå‡†
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
