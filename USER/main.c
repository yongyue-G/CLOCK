#include "stm32f4xx.h"
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
#include "gui_guider.h"           // Gui Guider Éú³ÉµÄ½çÃæºÍ¿Ø¼şµÄÉùÃ÷
#include "events_init.h"          // Gui Guider Éú³ÉµÄ³õÊ¼»¯ÊÂ¼ş¡¢»Øµ÷º¯Êı
weather_info_t weather_info={
	.city="å¾å·",
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
};//Ö»Òª³õÊ¼»¯Ò»¸öÆäËûÄ¬ÈÏ³õÊ¼»¯Îª0

// åˆ¤æ–­æ˜¯å¦ä¸ºé—°å¹´ï¼ˆè¿›ä½å¤©æ•°éœ€è¦ï¼‰
static uint8_t Is_Leap_Year(uint16_t year) 
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// æ¯æœˆå¤©æ•°è¡¨ï¼ˆå¹³å¹´ï¼‰
static const uint8_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// ğŸš¨ æœ¬åœ°æ—¶é’Ÿè‡ªå¢æ ¸å¿ƒå‡½æ•°
void Local_Time_Tick(time_t *tm)
{
    tm->sec++; // ç§’åŠ  1
    if (tm->sec >= 60)
    {
        tm->sec = 0;
        tm->min++; // æ»¡ 60 ç§’ï¼Œåˆ†åŠ  1
        if (tm->min >= 60)
        {
            tm->min = 0;
            tm->hour++; // æ»¡ 60 åˆ†ï¼Œæ—¶åŠ  1
            if (tm->hour >= 24)
            {
                tm->hour = 0;
                tm->day++; // æ»¡ 24 å°æ—¶ï¼Œå¤©åŠ  1
                
                // å¤„ç†æ˜ŸæœŸè¿›ä½ (1-7å¾ªç¯)
                tm->week++;
                if(tm->week > 7) tm->week = 1;

                // å¤„ç†æœˆä»½å¤©æ•°è¿›ä½
                uint8_t max_day = days_in_month[tm->month];
                if (tm->month == 2 && Is_Leap_Year(tm->year)) 
                {
                    max_day = 29; // é—°å¹´2æœˆ29å¤©
                }
                
                if (tm->day > max_day)
                {
                    tm->day = 1;
                    tm->month++; // æ»¡å½“æœˆå¤©æ•°ï¼ŒæœˆåŠ  1
                    if (tm->month > 12)
                    {
                        tm->month = 1;
                        tm->year++; // æ»¡ 12 ä¸ªæœˆï¼Œå¹´åŠ  1
                    }
                }
            }
        }
    }
}

int main(void)
{
    
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
			SysTick_Config(SystemCoreClock / 1000);//³õÊ¼»¯ºó²ÅÄÜÓÃ
		
			
		TIM4_Init();
		Init_page();
    log("====================================");

    // 2. ÉùÃ÷½ÓÊÕÊı¾İµÄ½á¹¹ÌåÓë±äÁ¿
    char ssid[64] = "HONOR 90 Pro";         // Ìæ»»ÎªÄãµÄ WiFi ÕËºÅ
    const char* password = "12345678"; // Ìæ»»ÎªÄãµÄ WiFi ÃÜÂë


    // 3. ³õÊ¼»¯ AT Ä£¿éµ×²ã (USART1 + DMA)
    if(AT_Init() == 0)
    {
        while(1);
    }
    log(">>> AT Module Init Success.");

    WIFI_Status wifi_stat = AT_WIFI_Connect(ssid, password, NULL);
    if(wifi_stat == WIFI_Connected)
    {
        log(">>> WiFi Connected Successfully!");
    }
    else
    {
        log(">>> ERROR: WiFi Connection Failed! Status: %d", wifi_stat);
        while(1); 
    }
    
    // ¸ø ESP32 Ò»µãÊ±¼äÈ¥»¥ÁªÍøÀ­È¡Ê±¼ä
    delay_ms(2000); 

    // Ôö¼ÓÖØÊÔ»úÖÆ£¬·ÀÖ¹ÍøÂç²¨¶¯Ò»´ÎÊ§°Ü
    uint8_t time_retry = 3;
		uint8_t time_success = 0;
    while(time_retry--)
    {
        if(AT_Get_Time(&weather_info.time) == 1)
        {
            log(">>> Time Sync Success.");
					time_success=1;
            break; // ³É¹¦¾ÍÌø³öÑ­»·
        }
        else
        {
            log(">>> WARNING: Time Sync Failed. Retrying...");
            delay_ms(1000);
        }
    }

		if(time_success)
    {
        log(">>> [SUCCESS] Time Struct Members:");
        log("    Year:  %04d", weather_info.time.year);
        log("    Month: %02d", weather_info.time.month);
        log("    Day:   %02d", weather_info.time.day);
        log("    Week:  ĞÇÆÚ%d", weather_info.time.week); // 1-7 »ò 0-6
        log("    Time:  %02d:%02d:%02d", weather_info.time.hour, weather_info.time.min, weather_info.time.sec);
    }
    else
    {
        log(">>> [FAILED] Time Struct Parsing Failed after all retries.");
    }
		
    // 6. »ñÈ¡ÌìÆøĞÅÏ¢ (Ê¹ÓÃĞÄÖªÌìÆøAPI²âÊÔ)
    const char* weather_url = URL;
    uint8_t weather_success = 0;
    uint8_t weather_retry = 3; // ¸øËü 3 ´Î»ú»á
    while(weather_retry--)
    {
        if(AT_HTTP_Request(weather_url, &weather_info) == 1)
        {

					weather_success = 1;

            break; // ³É¹¦¾ÍÌø³öÑ­»·
        }
        else
        {
            log(">>> WARNING: Weather Request Failed. Retrying in 2s...");
            delay_ms(2000); // HTTP Ê§°Üºó£¬½¨ÒéÉÔÎ¢µÈ¾ÃÒ»µã£¨2Ãë£©ÔÙÖØÊÔ
        }
    }

		if(weather_success)
    {
        log(">>> [SUCCESS] Weather Struct Members:");
        log("    Province:     %s",   weather_info.province); // ½­ËÕ
        log("    City:         %s",   weather_info.city);     // ĞìÖİ
        log("    Weather Code: %d",   weather_info.weather);  // ÌìÆø´úÂëÊı×Ö
        log("    Temperature:  %.1f C", weather_info.tem_outdoor);
        log("    Last Update:  %s",   weather_info.update);   // Ê±¼ä´Á×Ö·û´®
    }
    else
    {
        log(">>> [FAILED] Weather Struct Parsing Failed after all retries.");
    }
		

    log("====================================");
		u_refresh_homepage_lvgl(&weather_info);
    // 7. Ö÷Ñ­»·
// åœ¨ main.c å¼€å¤´æˆ– main å‡½æ•°é‡Œå®šä¹‰ä¸€ä¸ªæ—¶é—´æˆ³å˜é‡
uint32_t last_dht11_time = 0; 
uint32_t last_weather_time = 0; // å¦‚æœä½ æƒ³åŠå°æ—¶æŸ¥ä¸€æ¬¡ç½‘ç»œå¤©æ°”ï¼Œä¹Ÿå¯ä»¥åŠ è¿™ä¸ª
uint32_t last_1s_tick = 0;

while(1)
{
    // ==========================================
    // 1. GUI å¿ƒè„èµ·æï¼ˆå¿…é¡»é«˜é¢‘ã€æ— é˜»å¡æ‰§è¡Œï¼ï¼‰
    // ==========================================
    lv_task_handler();
    delay_ms(5); // ç»™ LVGL ç•™ 5 æ¯«ç§’å–˜æ¯ï¼Œåƒä¸‡åˆ«å†™å¤ªå¤§ï¼

    // ==========================================
    // 2. æœ¬åœ°ä¼ æ„Ÿå™¨ï¼šæ¯éš” 2 ç§’é’Ÿï¼ˆ2000msï¼‰è¯»å–ä¸€æ¬¡ DHT11
    // ==========================================
    if (NOW() - last_dht11_time >= 2000) // NOW() è·å–å½“å‰ç³»ç»Ÿæ¯«ç§’æ•°ï¼Œä¹Ÿå¯ä»¥ç”¨è‡ªå®šä¹‰çš„ tick
    {
        last_dht11_time = NOW(); // æ›´æ–°æ—¶é—´æˆ³

        DHT11_Read_Data(); // è¯»åº•å±‚ç¡¬ä»¶
        
        // ç›´æ¥åœ¨è¿™é‡ŒæŠŠå…¨å±€å˜é‡èµ‹ç»™ä½ çš„å¤§ç»“æ„ä½“
        weather_info.tem_indoor = temperature;
        weather_info.humidity = humidity;

        // ğŸš¨ é‡ç‚¹ï¼šæ•°æ®æ›´æ–°äº†ï¼Œç«‹åˆ»é€šçŸ¥å±å¹•é‡ç»˜å¯¹åº”çš„æ–‡å­—ï¼
        // å‡è®¾ä½ æœ‰ä¸€ä¸ªä¸“é—¨åˆ·æ–°æ¸©æ¹¿åº¦çš„å‡½æ•°ï¼š
        u_refresh_homepage_lvgl(&weather_info);
    }
    
    // ==========================================
    // 3. ç½‘ç»œä¼ æ„Ÿå™¨ï¼šæ¯éš” 30 åˆ†é’Ÿï¼ˆ1800000msï¼‰è·å–ä¸€æ¬¡ç½‘ç»œå¤©æ°”
    // ==========================================
    /* if (NOW() - last_weather_time >= 1800000) 
    {
        last_weather_time = NOW();
        // é‡æ–°è°ƒç”¨ AT_HTTP_Request è·å–å¿ƒçŸ¥å¤©æ°”
        // AT_HTTP_Request(URL, &weather_info);
        // æ›´æ–°å±å¹•å¤©æ°”å›¾æ ‡å’Œæ•°æ®
    }
    */
	// ==========================================
        // 2. åŠ¨æ€èµ°è¡¨ï¼šæ¯éš” 1000ms æœ¬åœ°æ—¶é—´è‡ªå¢ 1 ç§’
        // ==========================================
        if (NOW() - last_1s_tick >= 1000)
        {
            last_1s_tick = NOW();
            
            // è°ƒç”¨è¿›ä½é€»è¾‘ï¼Œè®© current_time ç»“æ„ä½“å†…çš„æ•°å­—åŠ¨èµ·æ¥
            Local_Time_Tick(&weather_info.time);
            
            
            // è°ƒè¯•æ‰“å°ï¼Œä½ ä¼šçœ‹åˆ°ä¸²å£æ¯ç§’éƒ½åœ¨é›·æ‰“ä¸åŠ¨åœ°è¾“å‡ºé€’å¢çš„æ—¶é—´
            AT_Get_Time(&weather_info.time); 
        }
}
}