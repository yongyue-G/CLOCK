#include "stm32f4xx.h"                  // Device header
#include "SPI.h"
void SPI_Default_Config(SPI_Config* spic)//默认初始化
{
    spic->SPIx=SPI1;
    spic->GPIOx=GPIOB;

    spic->CLK=GPIO_Pin_3;
    spic->MISO=GPIO_Pin_4;
    spic->MOSI=GPIO_Pin_5;
    spic->CS=GPIO_Pin_8;
    //SPI_InitTypeDef SPI_InitStructure;在结构体中有
    (spic->SPI_InitStructure).SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;//速度拉满
    (spic->SPI_InitStructure).SPI_CPHA=SPI_CPHA_2Edge;//
    (spic->SPI_InitStructure).SPI_CPOL=SPI_CPOL_High;//时钟极性 - 空闲为高电平
    (spic->SPI_InitStructure).SPI_CRCPolynomial=7;
    (spic->SPI_InitStructure).SPI_DataSize=SPI_DataSize_8b;
    (spic->SPI_InitStructure).SPI_Direction=SPI_Direction_2Lines_FullDuplex;//双向
    (spic->SPI_InitStructure).SPI_FirstBit=SPI_FirstBit_MSB;
    (spic->SPI_InitStructure).SPI_Mode=SPI_Mode_Master;
    (spic->SPI_InitStructure).SPI_NSS=SPI_NSS_Soft;
    
    //执行与配置分离
    //SPI_Init(spic->SPIx,&spic->SPI_InitStructure)
    // SPI_Cmd(spic->SPIx,ENABLE);
}

void SPI_GPIO_Config(SPI_Config* spic)
{
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;//复用
    GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin=spic->MOSI|spic->CLK;
    GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;//上拉输入
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;//屏幕要最快
    GPIO_Init(spic->GPIOx,&GPIO_InitStructure);

    //由于上方配置过现在只需要修改不一样的就行
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//普通
    GPIO_InitStructure.GPIO_Pin=spic->CS;
    GPIO_Init(spic->GPIOx,&GPIO_InitStructure);

    // //防止误触放在LCD初始化中
    // GPIO_SetBits(spic->OUTGPIOx,spic->LCD_RST_Pin|spic->LCD_CS_Pin);

}

void SPI_AF_Config(SPI_Config* spic)
{
    GPIO_PinAFConfig(spic->GPIOx,GPIO_PinSource3,GPIO_AF_SPI1);
    GPIO_PinAFConfig(spic->GPIOx,GPIO_PinSource4,GPIO_AF_SPI1);
    GPIO_PinAFConfig(spic->GPIOx,GPIO_PinSource5,GPIO_AF_SPI1);
}

void MySPI_Init(SPI_Config* spic)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);

    SPI_GPIO_Config(spic);
    SPI_AF_Config(spic);

    SPI_Init(spic->SPIx,&spic->SPI_InitStructure);
    SPI_Cmd(spic->SPIx,ENABLE);
}

u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//等待发送区空	  
	SPIx->DR=Byte;	 	//发送一个byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完一个byte  
	return SPIx->DR;          	     //返回收到的数据			
} 

void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
{
	SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2	
	}
	else//低速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
	SPIx->CR1|=1<<6; //SPI设备使能
} 
