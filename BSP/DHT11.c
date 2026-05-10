#include "stm32f4xx.h"                  // Device header
#include "DHT11.h"
#include "Timer.h"
#include "LCD.h"
#include "SPI.h"
#include "LCD_GUI.h"
#include <stdio.h>
void DHT11_GPIO_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_DHT11_Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIO_DHT11_Port,&GPIO_InitStructure);

	DHT11_OUT(1);
}

void DHT11_Mode_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;// 推挽输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_DHT11_Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIO_DHT11_Port,&GPIO_InitStructure);
}

void DHT11_Mode_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;//上拉输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin=GPIO_DHT11_Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIO_DHT11_Port,&GPIO_InitStructure);
}

float temperature;
float humidity;

unsigned int DHT11_Read_Data(void)
{
	int timeout=0;
	float small_point=0;
	long long val=0;//64位
	unsigned char verify_num=0;
	//当把一个大的整数赋值给 unsigned char 时，C 语言会自动截断取低 8 位。
//所以上面巨大的加法结果，在赋值给 verify_num 时，只保留了低 8 位：
	DHT11_OUT(0);
	delay_ms(20);//不引起调度!!!!!!!!!!!
	DHT11_OUT(1);
	delay_us(30);
	
	DHT11_Mode_IN();
	
	timeout=5000;
	while((!DHT11_IN)&&(timeout>0))//等待高电平
	{
		timeout--;
	}
	
		timeout=5000;
	while((DHT11_IN)&&(timeout>0))//等待低电平
	{
		timeout--;
	}
	
	
	for(int i=0;i<40;i++)
	{
		timeout=5000;
		while((!DHT11_IN)&&(timeout>0))
		{
			timeout--;
		}
		delay_us(40);
		if(DHT11_IN)//高电平
		{
			val=(val<<1)+1;
		}
		else 
		{
			val=(val<<1);
		}
		timeout=5000;
		while((DHT11_IN)&&(timeout>0))
		{
			timeout--;
		}
	}

	DHT11_Mode_OUT();
		DHT11_OUT(1);
	
	// 重新计算每一位
u8 h_i = (u8)(val >> 32);
u8 h_f = (u8)(val >> 24);
u8 t_i = (u8)(val >> 16);
u8 t_f = (u8)(val >> 8);
u8 check = (u8)(val & 0xFF);

verify_num = h_i + h_f + t_i + t_f;
	
	
//	//val = [ 高24位无用 ][ 湿度高8 ][ 湿度低8 ][ 温度高8 ][ 温度低8 ][ 校验和8 ]
//  //				  位63-40    	 位39-32   位31-24    位23-16   位15-8    位7-0
//	verify_num=(unsigned char)(val>>32)+(unsigned char)(val>>24)+(unsigned char)(val>>16)+(unsigned char)(val>>8);
	if(verify_num==check)
	{
				temperature = t_i+t_f*0.1;//换算为小数点
				humidity = h_i+h_f*0.1;//换算为小数点

			return DHT11_SUCCESS;
	}
	else 
	{
		printf("Error\r\n");
//		
//    // 2. 准备一个足够大的缓冲区来存放 val 的字符串形式
//    //    long long 最大值有20位，十六进制是16位，我们准备 32 字节绝对安全
//    char val_buf[32];

//    // 3. 用 sprintf 把 val 转换成十六进制字符串，这比看十进制更容易分析每一位数据
//    sprintf(val_buf, "val: 0x%016llX", val); // %016llX 表示以16进制、16位、长整型打印

//    // 4. 把转换好的字符串显示在屏幕第二行
//    LCD_ShowString(0,6*16, 16, (u8*)val_buf, 0);
		
		return DHT11_CHECRSUM_ERR;
		
	}
}
