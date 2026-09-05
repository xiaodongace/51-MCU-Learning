#ifndef __NTC_H
#define __NTC_H

#include "Config.h"

// 求绝对值
#define abs(x)	((x > 0) ? (x) : (-(x)))

// 初始化NTC使用的ADC输入通道。
void NTC_Init(void);

// 获取当前温度值，单位为摄氏度。
int NTC_GetTemperature(void);

#endif
