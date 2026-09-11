#ifndef __CAR_KEY_H
#define __CAR_KEY_H

#include "GPIO.h"

#define KEY		P05
// 使用函数指针版本将以下置0
#define USE_KEYDOWN	0
#define USE_KEYUP	0
// 按下抬起函数的声明，如需使用，需打开开关，需用户在合适位置定义
void CarKey_on_keydown();
void CarKey_on_keyup();

// 初始化
void CarKey_init();
// 扫描按键
void CarKey_scan();

// 扫描按键，函数指针回调函数版本
// 不要和Key_scan()同时使用
void CarKey_scan2(void (*down)(), void (*up)());


#endif /* __CAR_KEY_H */
