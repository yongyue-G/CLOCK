#include "Homepage.h"
#include "gui_guider.h"
#include <stdio.h>

// ÉùÃ÷Íâ²¿Éú³ÉµÄ UI ¶ÔÏó
extern lv_ui guider_ui;
static const char *week_map[7] = {"æ˜ŸæœŸä¸€", "æ˜ŸæœŸäºŒ","æ˜ŸæœŸä¸‰","æ˜ŸæœŸå››","æ˜ŸæœŸäº”","æ˜ŸæœŸå…­","æ˜ŸæœŸæ—¥"};//Ö¸Õë£¿

void u_homepage(void* pvParameters)
{
  EventBits_t wait_bits = EVT_GUI_INIT_DONE;
	EventBits_t bits=xEventGroupWaitBits(g_sys_event,wait_bits,pdFALSE,//²»×Ô¶¯Çå³ı±êÖ¾Î»
																		pdTRUE,//allÎ»
																		portMAX_DELAY);
    if(!guider_ui.screen_home) 
    {
        // ºô½Ğ GUI Guider Éú³É screen_home ½çÃæ
        setup_scr_screen_home(&guider_ui); 
    }
    // ÏÖÔÚËü²»ÊÇ¿ÕÖ¸ÕëÁË£¬°²È«¼ÓÔØ£¡
    lv_scr_load(guider_ui.screen_home);
    u_refresh_homepage_lvgl(&weather_info);  
lv_task_handler();		
    xEventGroupSetBits(g_sys_event,EVT_HOMEPAGE_DONE);   
    vTaskDelete(NULL);                                                       
}
/* -----------------------------------------------------------
 * Ä£·ÂÔ­×÷Õß·ç¸ñµÄ LVGL ¸üĞÂ½Ó¿Ú
 * ----------------------------------------------------------- */

/* ¸üĞÂÊ±¼äÏÔÊ¾ */
void u_update_time_lvgl(time_t *tm)
{
    // ¶ÔÓ¦¿Ø¼ş£ºscreen_home_time
    lv_label_set_text_fmt(guider_ui.screen_home_time, "%02d:%02d", tm->hour, tm->min);
}

/* ¸üĞÂÈÕÆÚºÍĞÇÆÚÏÔÊ¾ */
void u_update_date_lvgl(time_t *tm)
{
    // ¸üĞÂÈÕÆÚ£ºscreen_home_date (¸ñÊ½: 2026.05.14)
    lv_label_set_text_fmt(guider_ui.screen_home_date, "%04d.%02d.%02d", tm->year, tm->month, tm->day);

    // ¸üĞÂĞÇÆÚ£ºscreen_home_xingqi
    if (tm->week >= 1 && tm->week <= 7)
    {
        lv_label_set_text(guider_ui.screen_home_xingqi         , week_map[tm->week - 1]);
    }
}

/* ¸üĞÂ³ÇÊĞÏÔÊ¾ */
void u_update_city_lvgl(char *city)
{
    // ¶ÔÓ¦¿Ø¼ş£ºscreen_home_city_show
    lv_label_set_text(guider_ui.screen_home_city_show, city);
}

/* ¸üĞÂÌìÆøÃèÊöÏÔÊ¾ */
void u_update_weather_lvgl(char *weather_str)
{

    lv_label_set_text(guider_ui.screen_home_tianqi_show, weather_str);
}

/* æ›´æ–°å®¤å?–ç¯å¢ƒæ˜¾ç¤? */
void u_update_outdoor_lvgl(float tmp)
{
    int tmp_int = (int)tmp;                               // æå–æ•´æ•°éƒ¨åˆ†
    int tmp_dec = (int)(tmp * 10) % 10;                   // æå–ç¬?ä¸€ä½å°æ•?
    if (tmp_dec < 0) tmp_dec = -tmp_dec;                  // ğŸš¨ é˜²æ?¢é›¶ä¸‹æ¸©åº¦æ—¶å°æ•°å¸¦è´Ÿå?
    
    // å¯¹åº”æ§ä»¶ï¼šscreen_home_outdoor_show
    lv_label_set_text_fmt(guider_ui.screen_home_outdoor_show, "%d.%d", tmp_int, tmp_dec);
}

/* æ›´æ–°å®¤å†…ç?å¢ƒæ˜¾ç¤ºï¼ˆæ¸©æ¹¿åº¦ï¼‰ */
void u_update_indoor_lvgl(float tmp, float humi)
{
    // å¤„ç†å®¤å†…æ¸©åº¦
    int tmp_int = (int)tmp;
    int tmp_dec = (int)(tmp * 10) % 10;
    if (tmp_dec < 0) tmp_dec = -tmp_dec;

    // å¤„ç†å®¤å†…æ¹¿åº¦
    int humi_int = (int)humi;
    int humi_dec = (int)(humi * 10) % 10;
    if (humi_dec < 0) humi_dec = -humi_dec; 

    // å¯¹åº”æ§ä»¶ï¼šscreen_home_indoor_show å’? screen_home_shidu_show
    lv_label_set_text_fmt(guider_ui.screen_home_indoor_show, "%d.%d", tmp_int, tmp_dec);
    lv_label_set_text_fmt(guider_ui.screen_home_shidu_show, "%d.%d", humi_int, humi_dec);
}

/* * ¸¨Öúº¯Êı£º½«ÌìÆøÊı×Ö´úÂë×ª»»ÎªÖĞÎÄ×Ö·û´®
 * ·µ»ØÖµ£ºconst char* (³£Á¿×Ö·û´®Ö¸Õë£¬¼«ÆäÊ¡ÄÚ´æÇÒ°²È«)
 */
/* * è¾…åŠ©å‡½æ•°ï¼šå°†å¤©æ°”æ•°å­—ä»£ç è½?æ?ä¸ºä¸­æ–‡å­—ç¬¦ä¸²
 * è¿”å›å€¼ï¼šconst char* (å¸¸é‡å­—ç?¦ä¸²æŒ‡é’ˆï¼Œæå…¶çœå†…å­˜ä¸”å®‰å…?)
 */
/**
 * æ ¹æ®å¿ƒçŸ¥å¤©æ°”APIè¿”å›çš„å¤©æ°”ä»£ç ï¼Œè·å–å¯¹åº”çš„ä¸­æ–‡å­—ç¬¦ä¸²
 * @param weather_code å¤©æ°”ä»£ç ï¼ˆå¿ƒçŸ¥å¤©æ°”APIä¸? code å­—æ?µçš„å€¼ï¼‰
 * @return å¯¹åº”çš„ä¸­æ–‡å­—ç¬¦ä¸²
 */
/**
 * æ ¹æ®å¿ƒçŸ¥å¤©æ°”APIè¿”å›çš„å¤©æ°”ä»£ç ï¼Œè·å–å¯¹åº”çš„ä¸­æ–‡å­—ç¬¦ä¸²
 * @param weather_code å¤©æ°”ä»£ç ï¼ˆå¿ƒçŸ¥å¤©æ°”APIä¸­ code å­—æ®µçš„å€¼ï¼‰
 * @return å¯¹åº”çš„ä¸­æ–‡å­—ç¬¦ä¸²
 */
const char* get_weather_string(int weather_code)
{
    switch (weather_code)
    {
        // æ™´ï¼ˆåŒºåˆ†å›½å†…å¤–ã€ç™½å¤©å¤œæ™šï¼‰
        case 0:  return "æ™´";      // å›½å†…åŸå¸‚ç™½å¤©æ™´
        case 1:  return "æ™´";      // å›½å†…åŸå¸‚å¤œæ™šæ™´
        case 2:  return "æ™´";      // å›½å¤–åŸå¸‚ç™½å¤©æ™´
        case 3:  return "æ™´";      // å›½å¤–åŸå¸‚å¤œæ™šæ™´
        
        // äº‘é‡ç›¸å…³
        case 4:  return "å¤šäº‘";
        case 5:  return "æ™´é—´å¤šäº‘";
        case 6:  return "æ™´é—´å¤šäº‘";
        case 7:  return "å¤§éƒ¨å¤šäº‘";
        case 8:  return "å¤§éƒ¨å¤šäº‘";
        case 9:  return "é˜´";
        
        // é›¨
        case 10: return "é˜µé›¨";
        case 11: return "é›·é˜µé›¨";
        case 12: return "é›·é˜µé›¨ä¼´æœ‰å†°é›¹";
        case 13: return "å°é›¨";
        case 14: return "ä¸­é›¨";
        case 15: return "å¤§é›¨";
        case 16: return "æš´é›¨";
        case 17: return "å¤§æš´é›¨";
        case 18: return "ç‰¹å¤§æš´é›¨";
        case 19: return "å†»é›¨";
        
        // é›ªä¸é›¨å¤¹é›ª
        case 20: return "é›¨å¤¹é›ª";
        case 21: return "é˜µé›ª";
        case 22: return "å°é›ª";
        case 23: return "ä¸­é›ª";
        case 24: return "å¤§é›ª";
        case 25: return "æš´é›ª";
        
        // æ²™å°˜/æµ®å°˜
        case 26: return "æµ®å°˜";
        case 27: return "æ‰¬æ²™";
        case 28: return "æ²™å°˜æš´";
        case 29: return "å¼ºæ²™å°˜æš´";
        
        // é›¾/éœ¾
        case 30: return "é›¾";
        case 31: return "éœ¾";
        
        // é£
        case 32: return "é£";
        case 33: return "å¤§é£";
        case 34: return "é£“é£";
        case 35: return "çƒ­å¸¦é£æš´";
        case 36: return "é¾™å·é£";
        
        // æ¸©åº¦æç«¯
        case 37: return "å†·";
        case 38: return "çƒ­";
        
        // æœªçŸ¥
        case 99: return "æœªçŸ¥";
        
        default: return "æœªçŸ¥å¤©æ°”";
    }
}

/* -----------------------------------------------------------
 * ×ÜË¢ĞÂº¯Êı£ºÖ±½Ó´«ÈëÄãµÄ weather_info_t ½á¹¹Ìå
 * ----------------------------------------------------------- */
void u_refresh_homepage_lvgl(weather_info_t *info)
{
    // Ö»ÓĞµ±Ö÷Ò³Ãæ¼ÓÔØÁË²ÅÖ´ĞĞË¢ĞÂ£¬·ÀÖ¹²Ù×÷¿ÕÖ¸Õë
    if (guider_ui.screen_home == NULL) return;

    u_update_time_lvgl(&info->time);
    u_update_date_lvgl(&info->time);
    u_update_city_lvgl(info->city);
	const char *weather_zh = get_weather_string(info->weather); 
    u_update_weather_lvgl((char *)weather_zh); // Ç¿×ªÒ»ÏÂ·ÀÖ¹±¨ warning
    u_update_outdoor_lvgl(info->tem_outdoor);
    u_update_indoor_lvgl(info->tem_indoor, info->humidity);
}
