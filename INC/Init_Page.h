#ifndef __INIT_PAGE_H
#define __INIT_PAGE_H
#include "stm32f4xx.h"                  // Device header

void update_init_progress(uint8_t percent);
void u_initpage(void* pvParameters);
void boot_delay_ms(uint32_t ms);

#endif
