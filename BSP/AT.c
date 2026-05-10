#include "stm32f4xx.h"                  // Device header
#include "AT.h"
#include "log.h"
#include "Timer.h"
#include "main.h"
#include "wifi_usart.h"
#include <stdio.h>
#include <string.h>
uint16_t at_rev_len=0;
uint8_t at_rx_busy_flag=0;
USART_Config AT_usartc;
uint8_t ATINIT=0;
uint8_t ATECHO=0;
AT_Status at_status=AT_IDLE;
static char at_buff[AT_BUFF_SIZE+1];//为什么是char不是string

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
/* ================= AT 初始化 ================= */
uint8_t AT_Init(void)
{
    if(ATINIT==1) return 1;
    usart_default_config(&AT_usartc,USART1);
    AT_usartc.rx_buf=(uint8_t*)at_buff;//为什么uint
	AT_usartc.rx_buf_size=AT_BUFF_SIZE;
    usart_init(&AT_usartc);
	myDMA_Init(&AT_usartc);
    ATINIT=1;//提早释放，为了AT_Wait_Send中的AT_IS_Busy发送AT
    log("AT Hardware Layer Initialized!");
    if(AT_Wait_Send(AT_INIT_TIMEOUT)==0)
    {
        ATINIT=0;
        LOG("AT init failed");
        return 0;
    }
    if(ATECHO==0)//关回显
        AT_Transceive("ATE1",AT_RECV_TIMEOUT);
    else 
        AT_Transceive("ATE0",AT_RECV_TIMEOUT);
		return 1;
}
/* ================= AT 基础操作 ================= */
void at_usart_send_str(const char* ch)
//"AT\r\n" 这种直接写在双引号里的叫字符串常量。在单片机里，它们是被直接烧录在 Flash（只读存储器） 里的。不加const编译器会报警告，因为它觉得你把一个只读的东西，传给了一个“可能会修改它”的函数
{
    while(*ch)//在 C 语言中，所谓的空字符串 ""，在内存里其实并不是“什么都没有”，它包含了一个隐藏的幽灵字符——'\0'（字符串结束符）。它的 ASCII 码数值刚好就是 0。
    {
        while(USART_GetFlagStatus(AT_usartc.usartx,USART_FLAG_TXE)==RESET);//确保Data Register已经空了
        USART_SendData(AT_usartc.usartx,(uint8_t)*ch++);//等同于USART_SendData(AT_usartc.usartx,(uint8_t)*ch);ch++;ch是地址
        //pointer?这是一个专门针对中文字符和特殊二进制数据的防雷设计。强行剥夺它的“正负号”属性
        //while(USART_GetFlagStatus(AT_usartc.usartx,USART_FLAG_TC)==RESET);//等待送达不用加，只等待USART_FLAG_TXE，两帧数据之间无缝衔接，串口带宽利用率 100%！
    }
}

void AT_SendCRLF(void)
{
    at_usart_send_str("\r\n");
}

void AT_Send(const char* cmd) //单片机向外发送
{
    if(ATINIT==0) return;
    at_usart_send_str(cmd);
    AT_SendCRLF();
}

void AT_Reset(void)//软件重启
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
    while(!AT_IS_Busy()||at_status!=AT_OK)
    {
        at_status=AT_BUSY;//用于初始化刚刚进入busy的条件判断的复位
        if(IS_TIMEOUT(tick,timeout))
        {
            return 0;//发送失败
        }
    }
    return 1;
}

AT_Status AT_Parse(void)//解析接收的数据
{
    for(int i=0;i<sizeof(at_status_map)/sizeof(at_status_map[0]);i++)
    {
        if(strstr((char*)at_buff,at_status_map[i].str)!=NULL)
        {
            at_status=at_status_map[i].Status;
            at_rx_busy_flag=0;
            // 重新清理状态，防止之前有残留标志
            DMA_ClearFlag(AT_usartc.dma_rx_stream, DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2);
            DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
            return at_status;
        }
    }
          DMA_ClearFlag(AT_usartc.dma_rx_stream, DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2);
    DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
    return AT_UNKNOWN;
}

void AT_Recv(uint32_t timeout)//接收
{
    at_rx_busy_flag=0;
    at_rev_len=0;
    memset(at_buff, 0, AT_BUFF_SIZE);
    //从新开启必须clear所有标志
    DMA_ClearFlag(AT_usartc.dma_rx_stream,
					DMA_FLAG_FEIF2                
					|DMA_FLAG_DMEIF2               
					|DMA_FLAG_TEIF2                   
					|DMA_FLAG_HTIF2          
					|DMA_FLAG_TCIF2);
    AT_usartc.dma_rx_stream->M0AR=(uint32_t)at_buff;
	AT_usartc.dma_rx_stream->NDTR=AT_BUFF_SIZE;//NDTR	Number of Data Transfer Register	剩余待传输的数据个数
	DMA_Cmd(AT_usartc.dma_rx_stream,ENABLE);
    uint32_t tick = NOW();
    while(IS_TIMEOUT(tick,timeout)==0)
    {
        if(at_rx_busy_flag==1)//esp32回复并说完了
        {
            at_buff[at_rev_len]='\0';
            return;
        }
    }
    log("AT Command Timeout! CMD");
}
/* ================= AT 核心操作 ================= */
AT_Status AT_Transceive(const char*cmd, uint16_t timeout)
{
    
    AT_Send(cmd);
    AT_Recv(timeout);
    AT_Parse();
    return at_status;
}

void USART1_IRQHandler(void) //接收来自单片机外的的信息，一旦 ESP32 闭嘴（总线空闲），我们就立刻拉闸停止 DMA，并立起旗子。      
{
	if(USART_GetITStatus(USART1,USART_IT_IDLE)==SET)//USART_IT_IDLE 不是USART_Flag_IDLE
	{
		uint32_t clear_temp=USART1->SR;
		clear_temp=USART1->DR;
		if(at_rx_busy_flag==0)
		{
			// 计算接收到的字节数 = 总容量 - 剩余计数
			at_rev_len=AT_BUFF_SIZE-DMA_GetCurrDataCounter(AT_usartc.dma_rx_stream);
			// 3. 停止 DMA，防止数据在处理时被新来的覆盖
			DMA_Cmd(AT_usartc.dma_rx_stream, DISABLE);
			at_rx_busy_flag=1;		
		}
	}
}

/* ================= WiFi ================= */
WIFI_Status AT_WIFI_Info(char *ssid)
{
    // if(at_status==AT_BUSY)at_status存在滞后性需要重新判断
    if(AT_IS_Busy())
        return WIFI_Busy;
    AT_Send("AT+CWSTATE?");//查询 ESP32 设备的 Wi-Fi 状态和 Wi-Fi 信息
    AT_Recv(AT_RECV_TIMEOUT);
    char *p=strstr(at_buff,"CWSTATE");//strstr会返回它首次出现的位置的指针

    if(p==NULL)
        return WIFI_Unknow;
    char parsed[64];
    if(sscanf(p,"CWSTATE:2,\"%63[^\"]\"",parsed)==1)//如果调用成功，则函数返回读入数据的个数；
    {  //strcmp比较两个字符串ASIIC，果返回值小于 0，则表示 str1 小于 str2。
        if(ssid[0] =='\0'||strcmp(parsed,ssid)==0)//两种情况，传入为空和现在连接的与传入字符相同
        {
            strcpy(ssid,parsed);//用来处理传入为空的状况
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
    //与 sprintf() 不同的是，snprintf() 会限制输出的字符数，避免缓冲区溢出。
    //snprintf() 只会写入 size-1 个字符，并在字符串的末尾添加一个空字符（\0）以表示字符串的结束。
    if(!AT_Transceive(cmd_buff,AT_WIFI_TIMEOUT))
        return WIFI_Error;
    return WIFI_Connected;
    //原来一体化的时候
    // uint16_t timeout=1000;
    // if(AT_Transceive("AT+CWMODE=1","OK",timeout)==0)// Station 模式
    // {
    //     log("Failed to set WiFi mode!");
    //     return WIFI_Error;
    // }
    // else
    //     log("Success to set WiFi mode!");
    // sprintf(cmd_buff,"AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    // timeout=10000;//耗时
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
uint8_t AT_HTTP_Request(const char*url,weather_info_t* info)
{
    if(AT_IS_Busy())
        return 0;
    //节省栈直接传入全局变量，内存复用
    snprintf(at_buff,sizeof(at_buff),"AT+HTTPCLIENT=2,1,\"%s\",,,2", url);

    if(!AT_Transceive(at_buff,AT_HTTP_TIMEOUT))
        return 0;
    log("AT_HTTP: \r\n%s", at_buff);
    return prase_weather(info);
}


/* ================= TIME ================= */
uint8_t AT_Get_Time(time_t* tm)
{
    if(AT_IS_Busy())
        return 0;
    //配置对时服务器与时区（硬核配置）,这条指令叫作 SNTP 配置指令。
    if (!AT_Transceive("AT+CIPSNTPCFG=1,8", AT_RECV_TIMEOUT))//时区问题
		return 0;
    //此处不需要数据解析就不用AT_Transceive，分布写
    AT_Send("AT+CIPSNTPTIME?");
    AT_Transceive(at_buff,AT_RECV_TIMEOUT);
    if(parse_time(tm)==0) return 0;
    AT_Show_Time(tm);
}
void AT_Show_Time(time_t* tm)
{
    log("%04d-%02d-%02d %02d:%02d:%02d",tm->year, tm->month, tm->day, tm->hour, tm->min, tm->sec);
}


void extract_province_from_path(const char* path, char* province, size_t len)
{
    char buff[64];//为何
    strncpy(buff,path,sizeof(buff)-1);
    buff[sizeof(buff) - 1] = '\0';//为何
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
uint8_t json_next_string(char **pp,const char*key,char*out,size_t out_len)//双重指针
{
    char* p=strstr(*pp,key);//*pp内容是at_buff的起始地址
    if(!p)
        return 0;

    p=strstr(p,':');
    if(!p)
        return 0;

    p=strstr(p,'"');
    if(!p)
        return 0;

    p++;
    char* end=strstr(p,'"');
    if(!end)
        return 0;
    size_t len=(size_t)(end-p);//强行转化

    if (len >= out_len)//强行截断
	    len = out_len - 1;
    memcpy(out,p,len);
    out[len]='\0';

    **pp=end+1;

    return 1;
}
uint8_t prase_weather(weather_info_t *info)
{
    char* p=at_buff;
    char* tem[64];

    if(!json_next_string(&p,"\"name\"",info->city,sizeof(info->city)))
        return 0;

    if(!json_next_string(&p,"\"path\"",tem,sizeof(tem)))
        return 0;

    extract_province_from_path(tem,info->province,sizeof(info->province));

    if(!json_next_string(&p,"\"code\"",tem,sizeof(tem)))
        return 0;

    info->weather=atof(tem);

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
    p+=strlen(p, "+CIPSNTPTIME:");
    //因为 day 是一个普通的 8 位数字变量（uint8_t）。要想让 sscanf 把吸出来的数据直接改写到结构体内部，必须加上 &
    if(scanf(p,"%15[^ ] %15[^ ] %hhd %hhd:%hhd:%hhd %hd",week,mon,
        &t_tm->day, &t_tm->hour, &t_tm->min, &t_tm->sec,&t_tm->year) != 7)//（hhd 是专门给 8 位整数 uint8_t 用的）,[^ ]：一直往后吸，直到遇到空格为止！
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
		default: t_tm->month = 0; break;   // 非法
	}

	// 转换星期
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
		t_tm->week = 0;               // 非法

	return 1;
}
