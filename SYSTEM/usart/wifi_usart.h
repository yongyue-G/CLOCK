#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include <stdbool.h>

typedef struct
{
    USART_TypeDef* usartx;
    USART_InitTypeDef USART_InitStructure;
    GPIO_TypeDef* port;
    uint16_t tx;
    uint16_t rx;
    IRQn_Type IRQn;
    DMA_Stream_TypeDef* dma_rx_stream;
	uint32_t            dma_rx_channel;//注意数据类型
	uint32_t            dma_rx_rcc;//这是什么
    uint8_t* rx_buf;         // 接收缓冲区首地址指针
    uint16_t rx_buf_size;    // 接收缓冲区的大小
}USART_Config;

// extern uint8_t wifi_rx_buff[WIFI_RX_BUFF];
// extern uint16_t wifi_rx_len;
// extern bool wifi_rx_compete;
void usart_init(USART_Config* usartc);
void usart_default_config(USART_Config* usartc, USART_TypeDef* usart_num);
void myDMA_Init(USART_Config* usartc);
// void wifi_clear_rx_data(void);
// void USART1_IRQHandler(void);
// void wifi_send_data(char *res);      
#endif


