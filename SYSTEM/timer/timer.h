#ifndef __TIMER_H
#define __TIMER_H
void TIM4_Init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
//void TIM4_IRQHandler(void);
void SysTick_Handler(void);

#endif
