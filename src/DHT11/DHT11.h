#ifndef __DHT11_H
#define __DHT11_H

#include "Config.h"

/* 初始化DHT11单总线引脚。 */
void DHT11_Init(void);

/* 读取并打印一次DHT11温湿度，保留为传感器测试入口。 */
void DHT11_Task(void);

int8 getHumidityAndTemperature(float* p_humidity, float* p_temperature);

#endif
