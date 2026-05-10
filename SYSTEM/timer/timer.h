#ifndef __TIMER_H
#define __TIMER_H
extern volatile uint32_t ms_tick;
#define NOW() ms_tick
void TIM4_Init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void TIM4_IRQHandler(void);

#endif
