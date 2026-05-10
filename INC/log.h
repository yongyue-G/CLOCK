#ifndef __LOG_H
#define __LOG_H
#include "stdio.h"
#include "stm32f4xx.h"
extern uint8_t log_rx_busy_flag;
extern uint16_t log_rev_len;//注意数据类型
void log_Init(void);
void log_DMA_Init(void);
void USART2_IRQHandler(void) ;
int fputc(int ch,FILE *f);
void Log_Rx(void);
void log_process(char* line);
#define LOG printf
#define log(fmt, ...) LOG("[LOG] " fmt "\r\n", ##__VA_ARGS__)
#define LOG_BUFF_SIZE 512
#define LOG_USART SERIAL_USART
#define PUTCHAR_PROTOTYPE int fputc(int ch,FILE *f)
#endif
