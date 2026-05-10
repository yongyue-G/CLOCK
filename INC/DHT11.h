#ifndef __DHT11_H
#define __DHT11_H

#define GPIO_DHT11_Port GPIOA
#define GPIO_DHT11_Pin GPIO_Pin_3

#define DHT11_OUT(x) do { \
    if(x) GPIOA->BSRRL = GPIO_Pin_3; \
    else  GPIOA->BSRRH = GPIO_Pin_3; \
} while(0)

// 2. 读取引脚电平状态
#define DHT11_IN     ((GPIOA->IDR & GPIO_Pin_3) ? 1 : 0)

#define DHT11_HARDWARE_ERR 1
#define DHT11_CHECRSUM_ERR 2
#define DHT11_SUCCESS 0

extern float temperature;
extern float humidity;
void DHT11_GPIO_Init(void);
void DHT11_Mode_OUT(void);
void DHT11_Mode_IN(void);
unsigned int DHT11_Read_Data(void);

#endif
