#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "wifi_usart.h"
#include "log.h"
#include "timer.h"
#include "AT.h"
#include "LCD.h"
#include "SPI.h"
#include "LCD_GUI.h"
#include "DHT11.h"
#include "Init_Page.h"
#include "Homepage.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "gui_guider.h"           // Gui Guider ���ɵĽ���Ϳؼ�������
#include "events_init.h"          // Gui Guider ���ɵĳ�ʼ���¼����ص�����
weather_info_t weather_info={
	.city="徐州",
	.humidity=0.0,
	.tem_indoor=0.0,
	.tem_outdoor=0.0,
	.weather=0,
	.time={
		.week = 3,
		.month = 1,
		.sec = 0,
		.min = 0,
		.hour = 0,
		.day = 1,
		.year = 2026,
	},
};
#define BASE_PRIORITY 1
char ssid[64] = "HONOR 90 Pro";       
char password[64] = "12345678"; 
SemaphoreHandle_t sem_at;
SemaphoreHandle_t sem_ui;
EventGroupHandle_t g_sys_event;
xTaskHandle xLogTaskHandle= NULL;
xTaskHandle xATInitTaskHandle= NULL;
xTaskHandle xWIFITaskHandle= NULL;
xTaskHandle xGetTimeTaskHandle= NULL;
xTaskHandle xHTTPTaskHandle= NULL;
extern void vTaskRun_LogRx(void *pvParameters);
extern void u_initpage(void* pvParameters);
extern void u_homepage(void* pvParameters);
void vTaskRun_UI(void* pvParameters);
void vTaskRun_Test(void* parameters);
	void vTaskRun_AT_Init(void* parameters);
	void vTaskRun_WIFI(void* pvParameters);
	void vTaskRun_AT_Get_Time(void* pvParameters);
	void vTaskRun_AT_HTTP(void* pvParameters);
void vTaskRun_Exception(void* pvParameters);
void vTaskRun_Time_tick(void* pvParameters);
void vTaskRun_DHT11(void* pvParameters);

// 定义一个全局变量，记录空闲任务跑了多少圈
volatile uint32_t ulIdleCycleCount = 0;

// 这是 FreeRTOS 强制要求你实现的名字，一个字母都不能错
void vApplicationIdleHook(void)
{
    // CPU 只要一闲下来，就会疯狂执行这句代码
    ulIdleCycleCount++; 
}


void sys_init(void *pvParameters)
{
	EventBits_t init_bits = xEventGroupWaitBits(g_sys_event, EVT_HOMEPAGE_DONE,
	                                            pdFALSE,  // 不清除
	                                            pdTRUE,   // 等待所有位
	                                            portMAX_DELAY);
	xTaskCreate(vTaskRun_Test, "TaskRun_Test", 512, NULL, BASE_PRIORITY+1 , NULL);
	if(!(xEventGroupGetBits(g_sys_event)&EVT_AT_INITED))
		xTaskCreate(vTaskRun_AT_Init, "vTaskRun_AT_Init", 512, NULL, BASE_PRIORITY + 6, &xATInitTaskHandle);
	xTaskCreate(vTaskRun_WIFI, "vTaskRun_WIFI", 512, NULL, BASE_PRIORITY + 2, NULL);
	
	// 创建任务
    xTaskCreate(vTaskRun_Time_tick, "TimeTick", 128, NULL, BASE_PRIORITY + 7, NULL);
    xTaskCreate(vTaskRun_Exception, "Exception", 256, NULL, BASE_PRIORITY + 7, NULL);

	if (!(xEventGroupGetBits(g_sys_event) & EVT_AT_INITED))
		xTaskCreate(vTaskRun_AT_Init, "AT_Init", 512, NULL, BASE_PRIORITY + 6, NULL);

    xTaskCreate(vTaskRun_UI, "UI", 512, NULL, BASE_PRIORITY + 5, NULL);

    xTaskCreate(vTaskRun_LogRx, "LogRx", 512, NULL, BASE_PRIORITY + 4, &xLogTaskHandle);

    xTaskCreate(vTaskRun_WIFI, "WIFI", 512, NULL, BASE_PRIORITY + 3, &xWIFITaskHandle);
   	xTaskCreate(vTaskRun_DHT11, "DHT11", 512, NULL, BASE_PRIORITY + 3, NULL);

    xTaskCreate(vTaskRun_AT_Get_Time, "AT_Time", 512, NULL, BASE_PRIORITY + 2, &xGetTimeTaskHandle);
	BaseType_t xReturn = pdFALSE;
    xReturn=xTaskCreate(vTaskRun_AT_HTTP, "AT_HTTP", 512,NULL, BASE_PRIORITY + 2, &xHTTPTaskHandle);
		if (xReturn != pdPASS)
{
    // 🚨 如果在这里打断点停住了，说明 Heap 空间不够了，HTTP 任务创建失败！
    printf("ERROR: AT_HTTP Task Create Failed due to OOM!\r\n"); 
}
	
  	vTaskDelete(NULL);
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	TIM4_Init();
	sem_at=xSemaphoreCreateMutex();
	sem_ui=xSemaphoreCreateMutex();
	g_sys_event=xEventGroupCreate();
	
//	log_Init();
//		xTaskCreate(vTaskRun_Test, "TaskRun_Test", 512, NULL, BASE_PRIORITY , NULL);
	xTaskCreate(u_initpage, "u_initpage", 1024, NULL, BASE_PRIORITY, NULL);

	xTaskCreate(u_homepage, "u_homepage", 1024, NULL, BASE_PRIORITY, NULL);

	xTaskCreate(sys_init, "sys_init", 1024, NULL, BASE_PRIORITY, NULL);
	vTaskStartScheduler();
	while(1)
	{
		printf("error...\r\n");
		delay_ms(1000);
	}
}
void vTaskRun_Exception(void* pvParameters)
{
	while(1)
	{
		EventBits_t bits = xEventGroupWaitBits(g_sys_event, EVT_EXCEPTION,
		                                       pdTRUE,   // 自动清除
		                                       pdFALSE,  // 等待任意一个
		                                       portMAX_DELAY);
		if(!(bits&EVT_AT_INITED))
		{
			// AT初始化异常
			AT_Reset();
			log("EXCEPTION: AT_INIT failed, triggering reset!");
			vTaskDelay(pdMS_TO_TICKS(3000));  // 延时避免打印过快
		}
	}
}
void vTaskRun_AT_Get_Time(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(TIME_GET_TIME));
		if(!xSemaphoreTake(sem_at,TIME_SEM_TAKE)) continue;
		if(AT_WIFI_Info(ssid)!=WIFI_Connected)
		{
			xEventGroupSetBits(g_sys_event,EVT_WIFI_NEED_CONNECT);
			xSemaphoreGive(sem_at);//因为后续continue回到开头需要释放锁
			continue;
		}
		time_t revc_time=weather_info.time;//增加局部缓冲
		if(AT_Get_Time(&revc_time)==1)
		{
			
			xEventGroupSetBits(g_sys_event,EVT_TIME_READY);
			xEventGroupSetBits(g_sys_event,EVT_DATE_READY);
			weather_info.time=revc_time;
		}
		xSemaphoreGive(sem_at);
	}
}
void vTaskRun_AT_HTTP(void* pvParameters)
{
	while(1)
	{
		
		if(!xSemaphoreTake(sem_at,TIME_SEM_TAKE)) 
		{
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}
		if(AT_WIFI_Info(ssid)!=WIFI_Connected)
		{
			xEventGroupSetBits(g_sys_event,EVT_WIFI_NEED_CONNECT);
			xSemaphoreGive(sem_at);//因为后续continue回到开头需要释放锁
			vTaskDelay(pdMS_TO_TICKS(1000));
			continue;
		}
		if(AT_HTTP_Request(URL,&weather_info)==1)
		{
			xEventGroupSetBits(g_sys_event,EVT_WEATHER_READY);
		}
		xSemaphoreGive(sem_at);
		vTaskDelay(pdMS_TO_TICKS(TIME_HTTP));
		
	}
}
//void vTaskRun_Test(void* parameters)
//{
//    // 定义一个足够大的数组来存放生成的表格文本
//    // 任务越多，这个数组就需要越大，400 字节通常够用
//    char pcWriteBuffer[400]; 

//    while(1)
//    {
//        // 延时 5 秒打印一次报表
//        vTaskDelay(pdMS_TO_TICKS(5000));

//        printf("===========================================\r\n");
//        printf("Task Name\tRun Time\tUsage(%%)\r\n");
//        printf("===========================================\r\n");
//        
//        // 🚨 核心调用：让系统计算并填充报表
//        vTaskGetRunTimeStats(pcWriteBuffer);
//        
//        // 打印出报表
//        printf("%s\r\n", pcWriteBuffer);
//        printf("===========================================\r\n");
//    }
//}
void vTaskRun_Test(void* parameters)
{
    UBaseType_t wifi_free, http_free;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000)); // 每 5 秒巡检一次
        if(xHTTPTaskHandle != NULL) {
            http_free = uxTaskGetStackHighWaterMark(xHTTPTaskHandle);
            printf("[HTTP Stack Remained]: %ld Bytes\r\n", http_free * 4);
        }
    }
}
//void vTaskRun_Test(void* parameters)
//{
//	whlie(1)
//	{
//		printf("running\r\n");
//		delay_ms(2000);
//	}
//	
//}
//// 假设你的单片机在啥业务都不跑时，1秒钟能加到这个数（你需要先测出这个基准值）
//// 测基准值的方法：把 sys_init 里的网络、UI等任务全注释掉，看打印出的是多少，填在这里
//#define MAX_IDLE_COUNT_PER_SEC 3992832 

//void vTaskRun_Test(void* parameters)
//{
//    uint32_t current_idle_count = 0;
//    float cpu_usage = 0.0f;

//    while(1)
//    {
//        // 1. 清零计数器
//        ulIdleCycleCount = 0;

//        // 2. 绝对延时 1000 毫秒 (让系统去跑业务任务和空闲任务)
//        vTaskDelay(pdMS_TO_TICKS(1000));

//        // 3. 抓取这 1 秒内的空闲计数值
//        current_idle_count = ulIdleCycleCount;
//				printf("count:%d\r\n", current_idle_count);
////        // 4. 计算 CPU 占用率
//        // 占用率 = (最大算力 - 剩余算力) / 最大算力 * 100%
//        if (current_idle_count <= MAX_IDLE_COUNT_PER_SEC)
//        {
//            cpu_usage = (float)(MAX_IDLE_COUNT_PER_SEC - current_idle_count) / MAX_IDLE_COUNT_PER_SEC * 100.0f;
//        }
//        else
//        {
//            cpu_usage = 0.0f; // 防止异常越界
//        }

//        //5. 打印震撼的数据
//        printf("--- CPU Usage: %.2f%% ---\r\n", cpu_usage);
//    }
//}
void vTaskRun_AT_Init(void* parameters)
{
	TickType_t tick = xTaskGetTickCount();//累计时间
	while(1)
	{
		if(xSemaphoreTake(sem_at,TIME_SEM_TAKE)==pdTRUE)//得到串口
		{
			uint8_t AT_Seccess=AT_Init();
			xSemaphoreGive(sem_at);//无论是否成功都释放
			if(AT_Seccess==1)
			{
				xEventGroupSetBits(g_sys_event,EVT_AT_INITED);
				vTaskDelete(NULL);
			}
		}

		if(IS_TIMEOUT(tick,TIME_EXCEPTION)==1)//超时
		{
			xEventGroupClearBits(g_sys_event,EVT_AT_INITED);
			xEventGroupSetBits(g_sys_event,EVT_EXCEPTION);
			tick = xTaskGetTickCount();//必须重置
		}
		vTaskDelay(pdMS_TO_TICKS(TIME_CHECK_AT));  // 重试间隔
	}
}
void vTaskRun_UI(void* pvParameters)
{
	xEventGroupWaitBits(g_sys_event, EVT_HOMEPAGE_DONE, pdFALSE, pdTRUE, portMAX_DELAY);
	while (1)
	{
		EventBits_t bits = xEventGroupWaitBits(g_sys_event, EVT_TIME_READY | EVT_INDOOR_READY |
	                            EVT_WEATHER_READY|EVT_DATE_READY, pdTRUE, pdFALSE, pdMS_TO_TICKS(10));
		if (bits != 0)
		{
			if (xSemaphoreTake(sem_ui, portMAX_DELAY))
			{

				if (bits & EVT_TIME_READY)
				{
					u_update_time_lvgl(&weather_info.time);
				}

				if (bits & EVT_DATE_READY)
				{
					u_update_date_lvgl(&weather_info.time);
				}

				if (bits & EVT_INDOOR_READY)
				{
					u_update_indoor_lvgl(weather_info.tem_indoor, weather_info.humidity);
				}


				if (bits & EVT_WEATHER_READY)
				{
					u_update_city_lvgl(weather_info.city);
					const char *weather_zh = get_weather_string(weather_info.weather); 
					u_update_weather_lvgl((char *)weather_zh); // 强转一下防止报 warning
					u_update_outdoor_lvgl(weather_info.tem_outdoor);
				}

				xSemaphoreGive(sem_ui);
			}
		}
		if (xSemaphoreTake(sem_ui, portMAX_DELAY))
        {
            lv_task_handler(); 
            xSemaphoreGive(sem_ui);
        }
	}
}
void vTaskRun_WIFI(void* pvParameters)//软件错误不用exception
{
	EventBits_t wifi_wait_bits = EVT_WIFI_NEED_CONNECT;
	bool need_notify=true;
	while(1)
	{
		EventBits_t bits=xEventGroupWaitBits(g_sys_event,wifi_wait_bits,pdFALSE,//不自动清除标志位
																		pdFALSE,//任意一个位
																		portMAX_DELAY);
		if(xSemaphoreTake(sem_at,TIME_SEM_TAKE)==pdTRUE)//得到串口
		{
			WIFI_Status wifi_stat = AT_WIFI_Connect(ssid, password, NULL);
			if(wifi_stat == WIFI_Connected)
			{
				xEventGroupClearBits(g_sys_event,EVT_WIFI_NEED_CONNECT);
				log(">>> WiFi Connected Successfully!");
				need_notify=true;
			}
			else
			{
				if(need_notify==true)
				{
					need_notify=false;
					log("WIFI disconnect, SSID: %s, PASSWORD: %s", ssid, password);
					
				}
				vTaskDelay(pdMS_TO_TICKS(TIME_WIFI));  // 只有不正确的时候才再次尝试，成功的情况有wait标志位阻塞
			}
			xSemaphoreGive(sem_at);
		}
		
	}
}
// 判断是否为闰年（进位天数需要）
 static uint8_t Is_Leap_Year(uint16_t year) 
 {
     return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
 }

 // 每月天数表（平年）
 static const uint8_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

 // 🚨 本地时钟自增核心函数

void vTaskRun_Time_tick(void* pvParameters)
{
    // 1. 起跑线栅栏：死等主页画好，放在死循环外面！
    xEventGroupWaitBits(g_sys_event, EVT_HOMEPAGE_DONE, pdFALSE, pdTRUE, portMAX_DELAY);
    
    // 2. 初始化绝对延时的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // 3. RTOS 任务必须有的死循环
    while (1)
    {
        // 4. 绝对延时 1000 毫秒 (1秒)。这能保证时间永不漂移！
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));

            weather_info.time.sec++; // 秒加 1

            // 6. 优化的嵌套进位逻辑（只有秒满60，才去判断分，节省CPU算力）
            if (weather_info.time.sec >= 60)
            {
                weather_info.time.sec = 0;
                weather_info.time.min++; 
                
                if (weather_info.time.min >= 60)
                {
                    weather_info.time.min = 0;
                    weather_info.time.hour++; 
                    
                    if (weather_info.time.hour >= 24)
                    {
                        weather_info.time.hour = 0;
                        weather_info.time.day++; 
                        weather_info.time.week++;
                        if (weather_info.time.week > 7) weather_info.time.week = 1;

                        uint8_t max_day = days_in_month[weather_info.time.month];
                        if (weather_info.time.month == 2 && Is_Leap_Year(weather_info.time.year)) 
                        {
                            max_day = 29; 
                        }

                        if (weather_info.time.day > max_day)
                        {
                            weather_info.time.day = 1;
                            weather_info.time.month++; 
                            
                            if (weather_info.time.month > 12)
                            {
                                weather_info.time.month = 1;
                                weather_info.time.year++; 
                            }
                        }
                        // 只有发生跨天时，才发射日期更新信号
                        xEventGroupSetBits(g_sys_event, EVT_DATE_READY);
                    }
                }
            }
        // 7. 每秒钟发射一次时间更新信号，叫醒 UI 任务刷屏幕
        xEventGroupSetBits(g_sys_event, EVT_TIME_READY);
    }
}

void vTaskRun_DHT11(void* pvParameters)
{
	while(1)
	{
		uint8_t result=DHT11_Read_Data();
		if(!result)
		{			// 读底层硬件
        
     weather_info.tem_indoor = temperature;
     weather_info.humidity = humidity;
		xEventGroupSetBits(g_sys_event, EVT_INDOOR_READY);
		
		}
		delay_ms(3000);
	}
}

// //

// //两个初始化页面函数
// //开始任务

// //异常处理(优先级最高)
// //本地时间自增(优先级最高)
// //at初始化
// //刷新ui(信号量、事件)
// //log初始化

// //wifi连接
// //温湿度检测
// //最不可控：
// //时间获取(互斥信号量)
// //天气获取(互斥信号量)


// //lvgl高频率刷新
