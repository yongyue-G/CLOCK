#include "stm32f4xx.h"                  // Device header
#include "AT.h"
#include "log.h"
#include "Timer.h"
#include "main.h"
#include "wifi_usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
uint16_t at_rev_len=0;
volatile uint8_t at_rx_busy_flag=0;
USART_Config AT_usartc;
uint8_t ATINIT=0;
uint8_t ATECHO=0;
AT_Status at_status=AT_IDLE;
static char at_buff[AT_BUFF_SIZE+1];//ÎªÊ²Ã´ÊÇchar²»ÊÇstring

typedef struct 
{
   const char* str;
   AT_Status Status;
}AT_StatusMap;

static const AT_StatusMap at_status_map[]={
    { "OK",        AT_OK    },
	{ "ready",     AT_READY },
	{ "ERROR",     AT_ERROR },
	{ "busy p...", AT_BUSY  },
};
/* ================= AT ³õÊ¼»¯ ================= */
uint8_t AT_Init(void)
{
    if(ATINIT==1) return 1;
    usart_default_config(&AT_usartc,USART1);
    AT_usartc.rx_buf=(uint8_t*)at_buff;//ÎªÊ²Ã´uint
	AT_usartc.rx_buf_size=AT_BUFF_SIZE;
    usart_init(&AT_usartc);
	myDMA_Init(&AT_usartc);
	
    ATINIT=1;//ÌáÔçÊÍ·Å£¬ÎªÁËAT_Wait_SendÖĞµÄAT_IS_Busy·¢ËÍAT
    log("AT Hardware Layer Initialized!");
    if(AT_Wait_Send(AT_INIT_TIMEOUT)==0)
    {
        ATINIT=0;
        LOG("AT init failed");
        return 0;
    }
    if(ATECHO==0)//¹Ø»ØÏÔ
        AT_Transceive("ATE0",AT_RECV_TIMEOUT);
    else 
        AT_Transceive("ATE1",AT_RECV_TIMEOUT);
		// Ô­×÷Õß AT.c µÚ 59 ĞĞ×óÓÒ

		return 1;
}
/* ================= AT »ù´¡²Ù×÷ ================= */
void at_usart_send_str(const char* ch)
//"AT\r\n" ÕâÖÖÖ±½ÓĞ´ÔÚË«ÒıºÅÀïµÄ½Ğ×Ö·û´®³£Á¿¡£ÔÚµ¥Æ¬»úÀï£¬ËüÃÇÊÇ±»Ö±½ÓÉÕÂ¼ÔÚ Flash£¨Ö»¶Á´æ´¢Æ÷£© ÀïµÄ¡£²»¼Óconst±àÒëÆ÷»á±¨¾¯¸æ£¬ÒòÎªËü¾õµÃÄã°ÑÒ»¸öÖ»¶ÁµÄ¶«Î÷£¬´«¸øÁËÒ»¸ö¡°¿ÉÄÜ»áĞŞ¸ÄËü¡±µÄº¯Êı
{
    while(*ch)//ÔÚ C ÓïÑÔÖĞ£¬ËùÎ½µÄ¿Õ×Ö·û´® ""£¬ÔÚÄÚ´æÀïÆäÊµ²¢²»ÊÇ¡°Ê²Ã´¶¼Ã»ÓĞ¡±£¬Ëü°üº¬ÁËÒ»¸öÒş²ØµÄÓÄÁé×Ö·û¡ª¡ª'\0'£¨×Ö·û´®½áÊø·û£©¡£ËüµÄ ASCII ÂëÊıÖµ¸ÕºÃ¾ÍÊÇ 0¡£
    {
        while(USART_GetFlagStatus(AT_usartc.usartx,USART_FLAG_TXE)==RESET);//È·±£Data RegisterÒÑ¾­¿ÕÁË
        USART_SendData(AT_usartc.usartx,(uint8_t)*ch++);//µÈÍ¬ÓÚUSART_SendData(AT_usartc.usartx,(uint8_t)*ch);ch++;chÊÇµØÖ·
        //pointer?ÕâÊÇÒ»¸ö×¨ÃÅÕë¶ÔÖĞÎÄ×Ö·ûºÍÌØÊâ¶ş½øÖÆÊı¾İµÄ·ÀÀ×Éè¼Æ¡£Ç¿ĞĞ°ş¶áËüµÄ¡°Õı¸ººÅ¡±ÊôĞÔ
        //while(USART_GetFlagStatus(AT_usartc.usartx,USART_FLAG_TC)==RESET);//µÈ´ıËÍ´ï²»ÓÃ¼Ó£¬Ö»µÈ´ıUSART_FLAG_TXE£¬Á½Ö¡Êı¾İÖ®¼äÎŞ·ìÏÎ½Ó£¬´®¿Ú´ø¿íÀûÓÃÂÊ 100%£¡
    }
}

void AT_SendCRLF(void)
{
    at_usart_send_str("\r\n");
}

void AT_Send(const char* cmd) //µ¥Æ¬»úÏòÍâ·¢ËÍ
{
    if(ATINIT==0) return;
    at_usart_send_str(cmd);
    AT_SendCRLF();
}

void AT_Reset(void)//Èí¼şÖØÆô
{
    AT_Send("AT+RST");
}

uint8_t AT_IS_Busy(void)
{
    if(at_status!=AT_BUSY)
        return 0;
    AT_Transceive("AT",AT_RECV_TIMEOUT);
    return (at_status== AT_BUSY);
}

uint8_t AT_Wait_Send(uint32_t timeout)
{
    uint32_t tick = NOW();
    while(AT_IS_Busy()||at_status!=AT_OK)
    {
        at_status=AT_BUSY;//ÓÃÓÚ³õÊ¼»¯¸Õ¸Õ½øÈëbusyµÄÌõ¼şÅĞ¶ÏµÄ¸´Î»
        if(IS_TIMEOUT(tick,timeout))
        {
            return 0;//·¢ËÍÊ§°Ü
        }
    }
    return 1;
}

AT_Status AT_Parse(void)//½âÎö½ÓÊÕµÄÊı¾İ
{
    for(int i=0;i<sizeof(at_status_map)/sizeof(at_status_map[0]);i++)
    {
        if(strstr((char*)at_buff,at_status_map[i].str)!=NULL)
        {
            at_status=at_status_map[i].Status;
            at_rx_busy_flag=0;
            // ÖØĞÂÇåÀí×´Ì¬£¬·ÀÖ¹Ö®Ç°ÓĞ²ĞÁô±êÖ¾
            DMA_ClearFlag(AT_usartc.dma_rx_stream, DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2);
            DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
            return at_status;
        }
    }
          DMA_ClearFlag(AT_usartc.dma_rx_stream, DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2);
//    DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
		at_status = AT_UNKNOWN;
		// ğŸš¨ å¿…é¡»æŠŠå…¨å±€å˜é‡æ›´æ–°ä¸ºæœªçŸ¥ï¼Œå¦åˆ™å®ƒä¼šç»§æ‰¿ä¸Šä¸€æ¬¡çš„ OKï¼
    return AT_UNKNOWN;
}

void AT_Recv(uint32_t timeout)//½ÓÊÕ
{
    at_rx_busy_flag=0;
    at_rev_len=0;
	// 1. ?? ±ØĞëÏÈÇ¿ÖÆÍ£Ö¹ DMA£¬²ÅÄÜ¶¯ËüµÄ¼Ä´æÆ÷ºÍÇå¿ÕÄÚ´æ£¡
    DMA_Cmd(AT_usartc.dma_rx_stream, DISABLE);
    memset(at_buff, 0, AT_BUFF_SIZE);
    //´ÓĞÂ¿ªÆô±ØĞëclearËùÓĞ±êÖ¾
    DMA_ClearFlag(AT_usartc.dma_rx_stream,
					DMA_FLAG_FEIF2                
					|DMA_FLAG_DMEIF2               
					|DMA_FLAG_TEIF2                   
					|DMA_FLAG_HTIF2          
					|DMA_FLAG_TCIF2);
    AT_usartc.dma_rx_stream->M0AR=(uint32_t)at_buff;
	AT_usartc.dma_rx_stream->NDTR=AT_BUFF_SIZE;//NDTR	Number of Data Transfer Register	Ê£Óà´ı´«ÊäµÄÊı¾İ¸öÊı
	DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
    uint32_t tick = NOW();
    while(IS_TIMEOUT(tick,timeout)==0)
    {
        if(at_rx_busy_flag==1)//esp32»Ø¸´²¢ËµÍêÁË
        {
					printf("AT_Recv: %s\r\n", at_buff);
            at_buff[at_rev_len]='\0';
            return;
        }
    }
    log("AT Command Timeout! CMD");
}
/* ================= AT ºËĞÄ²Ù×÷ ================= */
AT_Status AT_Transceive(const char*cmd, uint16_t timeout)
{
    
    AT_Send(cmd);
    AT_Recv(timeout);
    AT_Parse();
    return (at_status == AT_OK);
}

void USART1_IRQHandler(void) //½ÓÊÕÀ´×Ôµ¥Æ¬»úÍâµÄµÄĞÅÏ¢£¬Ò»µ© ESP32 ±Õ×ì£¨×ÜÏß¿ÕÏĞ£©£¬ÎÒÃÇ¾ÍÁ¢¿ÌÀ­Õ¢Í£Ö¹ DMA£¬²¢Á¢ÆğÆì×Ó¡£      
{
	if(USART_GetITStatus(USART1,USART_IT_IDLE)==SET)//USART_IT_IDLE ²»ÊÇUSART_Flag_IDLE
	{
		uint32_t clear_temp=USART1->SR;
		clear_temp=USART1->DR;
		if(at_rx_busy_flag==0)
		{
			// ¼ÆËã½ÓÊÕµ½µÄ×Ö½ÚÊı = ×ÜÈİÁ¿ - Ê£Óà¼ÆÊı
			at_rev_len=AT_BUFF_SIZE-DMA_GetCurrDataCounter(AT_usartc.dma_rx_stream);
			// 3. Í£Ö¹ DMA£¬·ÀÖ¹Êı¾İÔÚ´¦ÀíÊ±±»ĞÂÀ´µÄ¸²¸Ç
			DMA_Cmd(AT_usartc.dma_rx_stream, DISABLE);
			at_rx_busy_flag=1;		
		}
	}
}

/* ================= WiFi ================= */
WIFI_Status AT_WIFI_Info(char *ssid)
{
    // if(at_status==AT_BUSY)at_status´æÔÚÖÍºóĞÔĞèÒªÖØĞÂÅĞ¶Ï
    if(AT_IS_Busy())
        return WIFI_Busy;
    AT_Send("AT+CWSTATE?");//²éÑ¯ ESP32 Éè±¸µÄ Wi-Fi ×´Ì¬ºÍ Wi-Fi ĞÅÏ¢
    AT_Recv(AT_RECV_TIMEOUT);
    char *p=strstr(at_buff,"CWSTATE");//strstr»á·µ»ØËüÊ×´Î³öÏÖµÄÎ»ÖÃµÄÖ¸Õë

    if(p==NULL)
        return WIFI_Unknow;
    char prased[64];
    if(sscanf(p,"CWSTATE:2,\"%63[^\"]\"",prased)==1)//Èç¹ûµ÷ÓÃ³É¹¦£¬Ôòº¯Êı·µ»Ø¶ÁÈëÊı¾İµÄ¸öÊı£»
    {  //strcmp±È½ÏÁ½¸ö×Ö·û´®ASIIC£¬¹û·µ»ØÖµĞ¡ÓÚ 0£¬Ôò±íÊ¾ str1 Ğ¡ÓÚ str2¡£
        if(ssid[0] =='\0'||strcmp(prased,ssid)==0)//Á½ÖÖÇé¿ö£¬´«ÈëÎª¿ÕºÍÏÖÔÚÁ¬½ÓµÄÓë´«Èë×Ö·ûÏàÍ¬
        {
            strcpy(ssid,prased);//ÓÃÀ´´¦Àí´«ÈëÎª¿ÕµÄ×´¿ö
            return WIFI_Connected;
        }
    }
    return WIFI_Unknow;
}

WIFI_Status AT_WIFI_Connect(char* ssid, const char* password,const char*mac)
{
    if(AT_IS_Busy())
        return WIFI_Busy;
    if(AT_WIFI_Info(ssid)==WIFI_Connected)
        return WIFI_Connected;
    if(!AT_Transceive("AT+CWMODE=1",AT_RECV_TIMEOUT))
        return WIFI_Error;
    char cmd_buff[128];
    snprintf(cmd_buff,sizeof(cmd_buff),mac? "AT+CWJAP=\"%s\",\"%s\",\"%s\"":"AT+CWJAP=\"%s\",\"%s\"", ssid, password,mac);
    //Óë sprintf() ²»Í¬µÄÊÇ£¬snprintf() »áÏŞÖÆÊä³öµÄ×Ö·ûÊı£¬±ÜÃâ»º³åÇøÒç³ö¡£
    //snprintf() Ö»»áĞ´Èë size-1 ¸ö×Ö·û£¬²¢ÔÚ×Ö·û´®µÄÄ©Î²Ìí¼ÓÒ»¸ö¿Õ×Ö·û£¨\0£©ÒÔ±íÊ¾×Ö·û´®µÄ½áÊø¡£
    if(!AT_Transceive(cmd_buff,AT_WIFI_TIMEOUT))
        return WIFI_Error;
    return WIFI_Connected;
    //Ô­À´Ò»Ìå»¯µÄÊ±ºò
    // uint16_t timeout=1000;
    // if(AT_Transceive("AT+CWMODE=1","OK",timeout)==0)// Station Ä£Ê½
    // {
    //     log("Failed to set WiFi mode!");
    //     return WIFI_Error;
    // }
    // else
    //     log("Success to set WiFi mode!");
    // sprintf(cmd_buff,"AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    // timeout=10000;//ºÄÊ±
    // log("Connecting to WiFi: %s ... (Please wait)", ssid);
    //  if(AT_Transceive("cmd_buff","WIFI GOT IP",timeout)==0)
    //  {
    //     log("Failed to connected WiFi!");
    //     return WIFI_Disconencted;
    //  }
    //  else
    //  {
    //     log("Success to connected WiFi!");
    //     return WIFI_Connected;
    //  }
}

/* ================= Http ================= */
//uint8_t AT_HTTP_Request(const char*url,weather_info_t* info)
//{
//    if(AT_IS_Busy())
//        return 0;
//    //½ÚÊ¡Õ»Ö±½Ó´«ÈëÈ«¾Ö±äÁ¿£¬ÄÚ´æ¸´ÓÃ
//    snprintf(at_buff,sizeof(at_buff),"AT+HTTPCLIENT=2,1,\"%s\",,,2", url);

//    if(!AT_Transceive(at_buff,AT_HTTP_TIMEOUT))
//        return 0;
//    log("AT_HTTP: \r\n%s", at_buff);
//    return prase_weather(info);
//}
/* ================= HTTP ================= */
uint8_t AT_HTTP_Request(const char* url, weather_info_t* info)
{
	if (AT_IS_Busy())
		return 0;

    // ?? ĞŞ¸´µã£º×Ô¼º½¨Ò»¸ö×¨ÓÃµÄ·¢ËÍ»º³åÇø£¬¾ø¶Ô²»½èÓÃ½ÓÊÕ×¨ÓÃµÄ g_at_buf
    char cmd_buf[256]; 
	snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPCLIENT=2,1,\"%s\",,,2", url);

    // ·¢ËÍ¶ÀÁ¢µÄ cmd_buf£¬½ÓÊÕÊ±Çå¿ÕÖØĞ´ g_at_buf
	if (!AT_Transceive(cmd_buf, AT_HTTP_TIMEOUT))
		return 0;
	return prase_weather(info);
}

/* ================= TIME ================= */
uint8_t AT_Get_Time(time_t* tm)
{
    if(AT_IS_Busy())
        return 0;
    //ÅäÖÃ¶ÔÊ±·şÎñÆ÷ÓëÊ±Çø£¨Ó²ºËÅäÖÃ£©,ÕâÌõÖ¸Áî½Ğ×÷ SNTP ÅäÖÃÖ¸Áî¡£
    if (!AT_Transceive("AT+CIPSNTPCFG=1,8", AT_RECV_TIMEOUT))//Ê±ÇøÎÊÌâ
		return 0;
    //´Ë´¦²»ĞèÒªÊı¾İ½âÎö¾Í²»ÓÃAT_Transceive£¬·Ö²¼Ğ´
    AT_Send("AT+CIPSNTPTIME?");
    AT_Recv(AT_RECV_TIMEOUT);
    if(prase_time(tm)==0) return 0;
    AT_Show_Time(tm);
		return 1;
}
void AT_Show_Time(time_t* tm)
{
	log("%04d-%02d-%02d %02d:%02d:%02d",tm->year, tm->month, tm->day, tm->hour, tm->min, tm->sec);
}


void extract_province_from_path(const char* path, char* province, size_t len)
{
    char buff[64];//ÎªºÎ
    strncpy(buff,path,sizeof(buff)-1);
    buff[sizeof(buff) - 1] = '\0';//ÎªºÎ
    char* last=strrchr(buff,',');
    if(!last)
    {
        province[0]='\0';
        return;
    }

    *last='\0';

    char* second=strrchr(buff,',');
    if(!second)
        return;
    else 
        second++;
    strncpy(province,second,len-1);
    province[len-1]='\0';

}
uint8_t json_next_string(char **pp,const char*key,char*out,size_t out_len)//Ë«ÖØÖ¸Õë
{
    char* p=strstr(*pp,key);//*ppÄÚÈİÊÇat_buffµÄÆğÊ¼µØÖ·
    if(!p)
        return 0;

    p=strchr(p,':');
    if(!p)
        return 0;

    p=strchr(p,'"');
    if(!p)
        return 0;

    p++;
    char* end=strchr(p,'"');
    if(!end)
        return 0;
    size_t len=(size_t)(end-p);//Ç¿ĞĞ×ª»¯

    if (len >= out_len)//Ç¿ĞĞ½Ø¶Ï
	    len = out_len - 1;
    memcpy(out,p,len);
    out[len]='\0';

    *pp=end+1;

    return 1;
}
uint8_t prase_weather(weather_info_t *info)
{
    char* p=at_buff;
    char tem[64];//²»¼ÓĞÇºÅ

    if(!json_next_string(&p,"\"name\"",info->city,sizeof(info->city)))
        return 0;

    if(!json_next_string(&p,"\"path\"",tem,sizeof(tem)))
        return 0;

    extract_province_from_path(tem,info->province,sizeof(info->province));

    if(!json_next_string(&p,"\"code\"",tem,sizeof(tem)))
        return 0;

    info->weather=atof(tem);//ÓÃÀ´°Ñ×Ö·û´®±ä³É¸¡µãÊı

    if(!json_next_string(&p,"\"temperature\"",tem,sizeof(tem)))
        return 0;

    info->tem_outdoor=atof(tem);

    if(!json_next_string(&p,"\"last_update\"",info->update,sizeof(info->update)))
        return 0;

    return 1;
}

uint8_t prase_time(time_t *t_tm)
{
    char week[16];
    char mon[16];
    char* p=at_buff;
    p=strstr(p, "+CIPSNTPTIME:");
    if(!p)
    {
        return 0;
    }
    p+=strlen("+CIPSNTPTIME:");
    //ÒòÎª day ÊÇÒ»¸öÆÕÍ¨µÄ 8 Î»Êı×Ö±äÁ¿£¨uint8_t£©¡£ÒªÏëÈÃ sscanf °ÑÎü³öÀ´µÄÊı¾İÖ±½Ó¸ÄĞ´µ½½á¹¹ÌåÄÚ²¿£¬±ØĞë¼ÓÉÏ &
    if(sscanf(p,"%15[^ ] %15[^ ] %hhd %hhd:%hhd:%hhd %hd",week,mon,
        &t_tm->day, &t_tm->hour, &t_tm->min, &t_tm->sec,&t_tm->year) != 7)//£¨hhd ÊÇ×¨ÃÅ¸ø 8 Î»ÕûÊı uint8_t ÓÃµÄ£©,[^ ]£ºÒ»Ö±ÍùºóÎü£¬Ö±µ½Óöµ½¿Õ¸ñÎªÖ¹£¡
    return 0;
    
    switch (mon[0])
	{
		case 'J': // Jan, Jun, Jul
			if (mon[1] == 'a')       t_tm->month = 1; // Jan
			else if (mon[2] == 'n')  t_tm->month = 6; // Jun
			else                        t_tm->month = 7; // Jul
			break;
		case 'F': t_tm->month = 2; break; // Feb
		case 'M': t_tm->month = (mon[2] == 'r') ? 3 : 5; break; // Mar / May
		case 'A': t_tm->month = (mon[1] == 'p') ? 4 : 8; break; // Apr / Aug
		case 'S': t_tm->month = 9; break;  // Sep
		case 'O': t_tm->month = 10; break; // Oct
		case 'N': t_tm->month = 11; break; // Nov
		case 'D': t_tm->month = 12; break; // Dec
		default: t_tm->month = 0; break;   // ·Ç·¨
	}

	// ×ª»»ĞÇÆÚ
	if (week[0] == 'M')               // Mon
		t_tm->week = 1;
	else if (week[0] == 'T')
		t_tm->week = (week[1] == 'u') ? 2 : 4; // Tue / Thu
	else if (week[0] == 'W')          // Wed
		t_tm->week = 3;
	else if (week[0] == 'F')          // Fri
		t_tm->week = 5;
	else if (week[0] == 'S')
		t_tm->week = (week[1] == 'a') ? 6 : 7; // Sat / Sun
	else
		t_tm->week = 0;               // ·Ç·¨

	return 1;
}

