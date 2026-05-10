#include "stm32f4xx.h"                  // Device header
#include "LCD.h"
#include "sys.h"
#include "Timer.h"
#include "LCD_GUI.h"
#include "test.h"


//========================variable==========================//
u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};//定义颜色数组
//=====================end of variable======================//


void DrawTestPage(u8 *str)
{
//绘制固定栏up
LCD_Clear(WHITE);
LCD_Fill(0,0,lcddev.width,20,BLUE);
//绘制固定栏down
LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
POINT_COLOR=WHITE;
Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//居中显示
Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE,"www.lcdwiki.com",16,1);//居中显示
//绘制测试区域
//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
}

void main_test(void)
{
	DrawTestPage("综合测试程序");	
	Gui_StrCenter(0,23,RED,BLUE,"全动电子",16,1);//居中显示
	Gui_StrCenter(0,40,RED,BLUE,"综合测试程序",16,1);//居中显示	
	Gui_StrCenter(0,57,GREEN,BLUE,"1.8\" ST7735S",16,1);//居中显示
	Gui_StrCenter(0,74,GREEN,BLUE,"128X160",16,1);//居中显示
	Gui_StrCenter(0,91,BLUE,BLUE,"2018-12-03",16,1);//居中显示
	delay_ms(1500);		
	delay_ms(1500);
}


void Test_Color(void)
{
	//DrawTestPage("测试1:纯色填充测试");
	LCD_Fill(0,0,lcddev.width,lcddev.height,WHITE);
	Show_Str(20,30,BLUE,YELLOW,"BL Test",16,1);delay_ms(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,RED);
	Show_Str(20,30,BLUE,YELLOW,"RED ",16,1);delay_ms(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,GREEN);
	Show_Str(20,30,BLUE,YELLOW,"GREEN ",16,1);delay_ms(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height,BLUE);
	Show_Str(20,30,RED,YELLOW,"BLUE ",16,1);delay_ms(800);
}


void Test_FillRec(void)
{
	u8 i=0;
	DrawTestPage("GUI矩形填充测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawRectangle(lcddev.width/2-40+(i*16),lcddev.height/2-40+(i*13),lcddev.width/2-40+(i*16)+30,lcddev.height/2-40+(i*13)+30); 
	}
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	{
		POINT_COLOR=ColorTab[i];
		LCD_DrawFillRectangle(lcddev.width/2-40+(i*16),lcddev.height/2-40+(i*13),lcddev.width/2-40+(i*16)+30,lcddev.height/2-40+(i*13)+30); 
	}
	delay_ms(1500);
}

void Test_Circle(void)
{
	u8 i=0;
	DrawTestPage("GUI画圆填充测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++)  
		gui_circle(lcddev.width/2-40+(i*15),lcddev.height/2-25+(i*13),ColorTab[i],15,0);
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	  	gui_circle(lcddev.width/2-40+(i*15),lcddev.height/2-25+(i*13),ColorTab[i],15,1);
	delay_ms(1500);
}


void English_Font_test(void)
{
	DrawTestPage("英文显示测试");
	Show_Str(10,22,BLUE,YELLOW,"6X12:abcdefgh01234567",12,0);
	Show_Str(10,34,BLUE,YELLOW,"6X12:ABCDEFGH01234567",12,1);
	Show_Str(10,47,BLUE,YELLOW,"6X12:~!@#$%^&*()_+{}:",12,0);
	Show_Str(10,60,BLUE,YELLOW,"8X16:abcde01234",16,0);
	Show_Str(10,76,BLUE,YELLOW,"8X16:ABCDE01234",16,1);
	Show_Str(10,92,BLUE,YELLOW,"8X16:~!@#$%^&*()",16,0); 
	delay_ms(1200);
}

void Test_Triangle(void)
{
	u8 i=0;
	DrawTestPage("GUI Tri填充测试");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Draw_Triangel(lcddev.width/2-40+(i*15),lcddev.height/2-12+(i*11),lcddev.width/2-25-1+(i*15),lcddev.height/2-12-26-1+(i*11),lcddev.width/2-10-1+(i*15),lcddev.height/2-12+(i*11));
	}
	delay_ms(1500);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for(i=0;i<5;i++)
	{
		POINT_COLOR=ColorTab[i];
		Fill_Triangel(lcddev.width/2-40+(i*15),lcddev.height/2-12+(i*11),lcddev.width/2-25-1+(i*15),lcddev.height/2-12-26-1+(i*11),lcddev.width/2-10-1+(i*15),lcddev.height/2-12+(i*11));
	}
	delay_ms(1500);
}

void Chinese_Font_test(void)
{	
	DrawTestPage("中文显示测试");
	Show_Str(10,25,BLUE,YELLOW,"16X16:全动电子欢迎",16,0);
	Show_Str(10,45,BLUE,YELLOW,"24X24:中文测试",24,1);
	Show_Str(10,70,BLUE,YELLOW,"32X32:字体测试",32,1);
	delay_ms(1200);
}


//void Pic_test(void)
//{
//	DrawTestPage("图片显示测试");
//	//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
//	Gui_Drawbmp16(20,30,gImage_qq);
//	Show_Str(20+12,75,BLUE,YELLOW,"QQ",16,1);
//	Gui_Drawbmp16(70,30,gImage_qq);
//	Show_Str(70+12,75,BLUE,YELLOW,"QQ",16,1);
////	Gui_Drawbmp16(150,30,gImage_qq);
////	Show_Str(150+12,75,BLUE,YELLOW,"QQ",16,1);
//	delay_ms(1200);
//}

//void Rotate_Test(void)
//{
//	u8 i=0;
//	u8 *Direction[4]={"Rotation:0","Rotation:90","Rotation:180","Rotation:270"};
//	
//	for(i=0;i<4;i++)
//	{
//	LCD_direction(i);
//	DrawTestPage("屏幕旋转测试");
//	Show_Str(20,30,BLUE,YELLOW,Direction[i],16,1);
//	Gui_Drawbmp16(30,50,gImage_qq);
//	delay_ms(1000);delay_ms(1000);
//	}
//	LCD_direction(USE_HORIZONTAL);
//}
