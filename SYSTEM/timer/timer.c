#include "stm32f4xx.h"                  // Device header
#include "timer.h"
volatile uint32_t ms_tick=0;
void TIM4_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//1分频
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseStructure.TIM_Prescaler=84-1;
    TIM_TimeBaseStructure.TIM_Period=0xFFFF;
    
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);

    TIM_Cmd(TIM4,ENABLE);
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

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		ms_tick++;  // 每 1 ms 增加一次
	}
}