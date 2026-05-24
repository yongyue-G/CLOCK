#include "stm32f4xx.h"                  // Device header
#include "freertos.h"
#include "task.h"
#include "timer.h"
#include "lvgl.h"  // 必须加上这个，才能认识 lv_tick_inc

void TIM4_Init(void)//实际是为了延时不是中断
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//1分频
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseStructure.TIM_Prescaler=84-1;
    TIM_TimeBaseStructure.TIM_Period=0xFFFF;
    
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
//		// 先清除更新中断标志（建议）
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

//		// 使能 TIM4 的更新中断
//		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

//		// 设置 NVIC 中断优先级（一定要写）
//		NVIC_InitTypeDef NVIC_InitStructure;
//		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM4,ENABLE);
}


extern void xPortSysTickHandler(void);
//systick 中断服务函数,使用 OS 时用到
void SysTick_Handler(void)
{ 
 if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
 {
    xPortSysTickHandler();
 }
 lv_tick_inc(1);
}
void delay_us(uint32_t us)
{
    // 你的 TIM4 已经配置为 84分频 (1MHz)，所以 TIM4->CNT 每过 1 微秒加 1
    uint16_t start = TIM4->CNT; 
    
    // 原地死等，直到定时器走过的差值达到指定的微秒数
    // (利用 16 位无符号数的溢出特性，哪怕定时器中途归零了也能算出正确差值)
    while((uint16_t)(TIM4->CNT - start) < us); 
}

void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}


////不能打两份工
//void TIM4_IRQHandler(void)
//{
//	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//	{
//		ms_tick++;  // 每 1 ms 增加一次

//		lv_tick_inc(1);
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//	}
//}
