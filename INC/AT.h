#ifndef __AT_H
#define __AT_H
#include "main.h"
#include"string.h"

#define AT_BUFF_SIZE 512
#define AT_INIT_TIMEOUT 3000
#define AT_RECV_TIMEOUT 1000
#define AT_WIFI_TIMEOUT 3000
#define AT_HTTP_TIMEOUT 10000
#define IS_TIMEOUT(start, ms) ((ms) ? ((uint32_t)((NOW()) - (uint32_t)(start)) >= (uint32_t)(ms)) : 0)
// #define IS_TIMEOUT(start,ms) ((ms)?((uint32_t)(NOW()-(uint32_t)start)>=(uint32_t)(ms):0))//不超时返回0
typedef enum
{
    WIFI_None,
    WIFI_Connected,
    WIFI_Error,
    WIFI_Busy,
    WIFI_Disconencted,
    WIFI_Unknow
}WIFI_Status;
typedef enum
{
    AT_OK,
    AT_IDLE,
    AT_READY,
    AT_BUSY,
    AT_ERROR,
    AT_UNKNOWN
}AT_Status;
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
    
}AT_Config;
uint8_t AT_Init(void);
void at_usart_send_str(const char* ch);
void AT_SendCRLF(void);
void AT_Send(const char* cmd);
void USART1_IRQHandler(void) ;
uint8_t AT_Wait_Send(uint32_t timeout);
uint8_t AT_IS_Busy(void);

void AT_Reset(void);
AT_Status AT_Parse(void);
void AT_Recv(uint32_t timeout);
AT_Status AT_Transceive(const char*cmd, uint16_t timeout);
WIFI_Status AT_WIFI_Info(char *ssid);
WIFI_Status AT_WIFI_Connect(char* ssid, const char* password,const char*mac);
uint8_t AT_HTTP_Request(const char*url,weather_info_t* info);
uint8_t AT_Get_Time(time_t* tm);
void AT_Show_Time(time_t* tm);
void extract_province_from_path(const char* path, char* province, size_t len);
uint8_t json_next_string(char **pp,const char*key,char*out,size_t out_len);
uint8_t prase_weather(weather_info_t *info);
uint8_t prase_time(time_t *t_tm);
#endif

