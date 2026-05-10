#ifndef __SPI_H
#define __SPI_H

typedef struct 
{
    SPI_TypeDef* SPIx;
    GPIO_TypeDef* GPIOx;
    uint16_t CLK;
    uint16_t MOSI;
    uint16_t MISO;
    uint16_t CS;
    SPI_InitTypeDef SPI_InitStructure;
}SPI_Config;


void SPI_Default_Config(SPI_Config* spic);
void SPI_GPIO_Config(SPI_Config* spic);
void SPI_AF_Config(SPI_Config* spic);
void MySPI_Init(SPI_Config* spic);

u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte);
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet);

#endif
