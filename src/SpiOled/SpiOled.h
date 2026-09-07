#ifndef __SPI_OLED_H
#define __SPI_OLED_H

#include "Config.h"

/*
 * SPI OLED（带外接字库）的菜单显示接口。
 *
 * 与工程现有的 I2C OLED 驱动刻意使用不同的前缀，二者可以同时编译。
 * 该模块只负责 SPI 屏菜单，详情页仍由现有 I2C OLED 显示。
 */
void SpiOled_Init(void);
void SpiOled_Clear(void);
void SpiOled_ShowMenu(u8 selected_item);

#endif
