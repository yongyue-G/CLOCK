#include "stm32f4xx.h"                  // Device header
#include "log.h"
#include "wifi_usart.h"
#include <stdio.h>
#include <string.h>
static USART_Config LOG_usart;
static char log_buff[LOG_BUFF_SIZE+1];//为什么是char不是string
uint16_t log_rev_len=0;
uint8_t log_rx_busy_flag=0;
uint8_t LOGINIT=0;
char ssid[128]="";
char password[128]="12345678";

void log_Init(void)
{
	if(LOGINIT==1) return;
  usart_default_config(&LOG_usart,USART2);
	LOG_usart.rx_buf=(uint8_t*)log_buff;
	LOG_usart.rx_buf_size=LOG_BUFF_SIZE;
  usart_init(&LOG_usart);
	myDMA_Init(&LOG_usart);
	LOGINIT=1;
}

PUTCHAR_PROTOTYPE
{
	if(!LOGINIT) return ch;//防御性编程，用来预防在初始化之前printf卡死
	while(USART_GetFlagStatus(LOG_usart.usartx,USART_FLAG_TXE)==RESET);//确保Data Register已经空了
    USART_SendData(LOG_usart.usartx,(uint8_t)ch);
    while(USART_GetFlagStatus(LOG_usart.usartx,USART_FLAG_TC)==RESET);//等待送达
    return ch;
}

void USART2_IRQHandler(void)        
{
	if(USART_GetITStatus(USART2,USART_IT_IDLE)==SET)//USART_IT_IDLE 不是USART_Flag_IDLE
	{
		uint32_t clear_temp=USART2->SR;
		clear_temp=USART2->DR;
		if(log_rx_busy_flag==0)
		{
			// 计算接收到的字节数 = 总容量 - 剩余计数
			log_rev_len=LOG_BUFF_SIZE-DMA_GetCurrDataCounter(LOG_usart.dma_rx_stream);
			// 3. 停止 DMA，防止数据在处理时被新来的覆盖
			DMA_Cmd(LOG_usart.dma_rx_stream, DISABLE);
			log_rx_busy_flag=1;
						
		}
		
	}
	//不使用DMA
	// if(USART_GetITStatus(USART2,USART_FLAG_RXNE)==SET)//串口接收数据
	// {
	// 	uint8_t res=USART_ReceiveData(USART2);
	// 	if(wifi_rx_len<WIFI_RX_BUFF&&!wifi_rx_compete)
	// 	{
	// 		wifi_rx_buff[wifi_rx_len++]=res;
	// 	}		
	// }
	// if(USART_GetITStatus(USART2,USART_FLAG_IDLE)==SET)//接受完数据
	// {
	// 	// F4 清除空闲中断标志位的特殊动作：先读 SR，再读 DR
	// 	uint32_t clear_temp=USART2->SR;
	// 	clear_temp=USART2->DR;
	// 	wifi_rx_compete=1;
	// }
} 

 
void Log_Rx(void)//接受信息统一格式
{
	DMA_Cmd(LOG_usart.dma_rx_stream,DISABLE);
	while(LOG_usart.dma_rx_stream->CR& DMA_SxCR_EN)
	//SxCR = Stream x Control Register即 「第 x 个数据流的控制寄存器」
	//等待关闭DMA，因为关闭是一个过程，DMA_SxCR_EN是一个宏，
	//while中不能只有他，因为他是个固定值，相当于掩码，只有读取寄存器&掩码才可以

	//处理字符串
	if(log_rev_len>0 && log_rev_len<LOG_BUFF_SIZE)
		log_buff[log_rev_len+1]='\0';
	else 
		log_buff[LOG_BUFF_SIZE]='\0';
	
	log_process(log_buff);
	log_rev_len=0;
	//清除DMA并从新启动
	DMA_ClearFlag(LOG_usart.dma_rx_stream,
					DMA_FLAG_FEIF5                
					|DMA_FLAG_DMEIF5                
					|DMA_FLAG_TEIF5                   
					|DMA_FLAG_HTIF5          
					|DMA_FLAG_TCIF5);
	//从新配置，使用结构提不可行
	// DMA_InitStructure.DMA_Memory0BaseAddr=(uint32_t)log_buff;
	// DMA_InitStructure.DMA_BufferSize=LOG_BUFF_SIZE;
	LOG_usart.dma_rx_stream->M0AR=(uint32_t)log_buff;
	LOG_usart.dma_rx_stream->NDTR=LOG_BUFF_SIZE;//NDTR	Number of Data Transfer Register	剩余待传输的数据个数
	DMA_Cmd(LOG_usart.dma_rx_stream,ENABLE);

}

void log_process(char* line)
{
	if(!line||line[0]=='\0')//空和遇到结尾符号
	{
		return;
	}
	char ssid_buff[128]={0};
	char password_buff[128]={0};
	//表示没有成功提取2个项目 → 格式错误//从字符数组中格式化读取数组，而不是从标准输入（通常是键盘）中读取数据
	if(sscanf(line,"WIFI: \"%127[^\"]\" \"%127[^\"]\"",ssid_buff,password_buff)!=2)
	{
		log("WIFI Info Error. Received SSID='%s', PASSWORD='%s'.", ssid_buff, password_buff);
		log("Correct format: WIFI: \"SSID\" \"PASSWORD\". Example: WIFI: \"MyWiFi\" \"12345678\"");
	}
	else
	{
		strncpy(ssid,ssid_buff,sizeof(ssid)-1);
		ssid[sizeof(ssid)-1]='\0';
		strncpy(password,password_buff,sizeof(password)-1);	
		ssid[sizeof(password)-1]='\0';

		log("WIFI Info Changed. SSID='%s',PASSWORD='%s'",ssid,password);
	}
}
