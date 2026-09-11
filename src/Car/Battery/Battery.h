#ifndef __BATTERY_H
#define __BATTERY_H


#include 	"GPIO.h"
#include	"ADC.h"
#include	"NVIC.h"

// 初始化
void Battery_init();
// 获取电池电压
float Battery_get_voltage();

#endif /* __BATTERY_H */
