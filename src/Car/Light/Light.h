#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "GPIO.h"

#define LED_LEFT  		P07
#define LED_RIGHT	 	P52

typedef enum {
    LEFT,   // 左
    RIGHT,  // 右
    ALL,    // 全部
} Light;

// 初始化
void Light_Init();
// 开灯
void Light_On(Light temp);
// 关灯
void Light_Off(Light temp);

#endif