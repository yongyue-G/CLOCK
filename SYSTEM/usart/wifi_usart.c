#include "stm32f4xx.h" 
#include "sys.h"
#include "wifi_usart.h"	
#include "AT.h"
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//ucos 使用	  
#endif


void usart_default_config(USART_Config* usartc, USART_TypeDef* usart_num)
{

	usartc->usartx = usart_num;

	if (usart_num == USART1) 
	{
		// 给 ESP32 用的高级货 (APB2 -> DMA2)
		usartc->port           = GPIOA;
		usartc->tx             = GPIO_Pin_9;
		usartc->rx             = GPIO_Pin_10;
		usartc->IRQn           = USART1_IRQn;
		usartc->dma_rx_stream  = DMA2_Stream2; 
		usartc->dma_rx_channel = DMA_Channel_4;
		usartc->dma_rx_rcc     = RCC_AHB1Periph_DMA2;
	}
	else if (usart_num == USART2) 
	{
		// 给 电脑 用的调试口 (APB1 -> DMA1)
		usartc->port           = GPIOD;
		usartc->tx             = GPIO_Pin_5;
		usartc->rx             = GPIO_Pin_6;
		usartc->IRQn           = USART2_IRQn;
		usartc->dma_rx_stream  = DMA1_Stream5; 
		usartc->dma_rx_channel = DMA_Channel_4;
		usartc->dma_rx_rcc     = RCC_AHB1Periph_DMA1;
	}

	// 3. 通用的软件参数（无论哪个串口，默认都按这个来）
	usartc->USART_InitStructure.USART_BaudRate            = 115200;
	usartc->USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	usartc->USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	usartc->USART_InitStructure.USART_Parity              = USART_Parity_No;
	usartc->USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartc->USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
}


void usart_init(USART_Config* usartc)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if (usartc->port == GPIOA) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	if (usartc->port == GPIOD) RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	if (usartc->usartx == USART1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	if (usartc->usartx == USART2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
 

	if (usartc->usartx == USART1) {
		GPIO_PinAFConfig(usartc->port, GPIO_PinSource9,  GPIO_AF_USART1); 
		GPIO_PinAFConfig(usartc->port, GPIO_PinSource10, GPIO_AF_USART1); 
	} else if (usartc->usartx == USART2) {
		GPIO_PinAFConfig(usartc->port, GPIO_PinSource5, GPIO_AF_USART2); 
		GPIO_PinAFConfig(usartc->port, GPIO_PinSource6, GPIO_AF_USART2); 
	}
	
	GPIO_InitStructure.GPIO_Pin   = usartc->tx | usartc->rx; 
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(usartc->port, &GPIO_InitStructure);


	USART_Init(usartc->usartx, &usartc->USART_InitStructure); 

	// USART_ITConfig(usartc.usartx, USART_IT_RXNE, ENABLE);不用 RXNE，改用 DMA + IDLE
	USART_ITConfig(usartc->usartx, USART_IT_IDLE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = usartc->IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // FreeRTOS 安全级别
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	

	USART_Cmd(usartc->usartx, ENABLE); //USART_Cmd 必须是整个串口初始化流程的“最后一步”。
}

void myDMA_Init(USART_Config* usartc)
{
    RCC_AHB1PeriphClockCmd(usartc->dma_rx_rcc,ENABLE); //USART2挂在AHB1上归DMA1管,但是USART1归DMA2管

	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_BufferSize=usartc->rx_buf_size;
	DMA_InitStructure.DMA_Memory0BaseAddr=(uint32_t)usartc->rx_buf;
	DMA_InitStructure.DMA_Channel=usartc->dma_rx_channel;
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralToMemory;//接收电脑按键来的数据
	DMA_InitStructure.DMA_Mode=DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&usartc->usartx->DR;
	//为什么字节
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;

	DMA_InitStructure.DMA_Priority=DMA_Priority_High;
	//VeryHigh 通常留给极其苛刻的音频 I2S 播放或者高速 ADC 连续采样，如果给串口设太高，可能会导致别的外设饿死
    DMA_InitStructure.DMA_FIFOMode=DMA_FIFOMode_Disable;//直接模式，不装箱，来一个送一个
    //DMA_InitStructure.DMA_FIFOThreshold,集装箱发车阈值,只有在上一项开启了 FIFOMode_Enable 时才有效
	//突发模式主要用于内存拷贝或者给 LCD 刷整屏像素。串口的寄存器（DR）每次只能吐 1 个字节出来
    DMA_InitStructure.DMA_MemoryBurst=DMA_MemoryBurst_Single;//(单次搬运，最规矩)
    DMA_InitStructure.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;

	DMA_Init(usartc->dma_rx_stream,&DMA_InitStructure);
	USART_DMACmd(usartc->usartx,USART_DMAReq_Rx,ENABLE);
	DMA_Cmd(usartc->dma_rx_stream, ENABLE);

}
