#ifndef __HOMEPAGE_H
#define __HOMEPAGE_H
#include "main.h"
#include "stm32f4xx.h"                  // Device header
void u_homepage(void* pvParameters);
void u_update_time_lvgl(time_t *tm);
void u_update_date_lvgl(time_t *tm);
void u_update_city_lvgl(char *city);
void u_update_weather_lvgl(char *weather_str);
void u_update_outdoor_lvgl(float tmp);
void u_update_indoor_lvgl(float tmp, float humi);
const char* get_weather_string(int weather_code);
void u_refresh_homepage_lvgl(weather_info_t *info);
#endif
