#include "stm32f4xx.h"                  // Device header
#include "timer.h"
#include "lvgl.h"  // 必须加上这个，才能认识 lv_tick_inc
// 替换掉原来的 ms_tick
volatile uint32_t ms_tick = 0; 
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


// ==========================================
// 这是 ARM 内核自带的 SysTick 中断服务函数
// ==========================================
void SysTick_Handler(void)
{
    ms_tick++;       // 每 1ms 增加一次
    lv_tick_inc(1);  // ?? 给 LVGL 提供完美的 1ms 心跳！
}

void delay_us(uint32_t us)
{
    while(us)
    {
        uint32_t t=(us>60000)? 60000:us;
        uint16_t start=TIM4->CNT;
        while((uint16_t)(TIM4->CNT-start)<t);
        us-=t;
    }
}

void delay_ms(uint32_t ms)
{
    while(ms--)
    {
        delay_us(1000);
    }
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
