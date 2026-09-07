#ifndef __LED_H
#define __LED_H

#include "Config.h"

// 初始化LED GPIO。
void LED_Init(void);

// 执行一次默认方向的LED流水灯测试。
void Flowing_LED_Test(void);

// 按指定方向执行一次LED流水灯，reverse为非零时反向运行。
void LED_Flowing_Test(u8 reverse);

// 同时控制8个LED，非零为点亮，零为熄灭。
void LED_SetAll(u8 on);

// 设置LED PWM亮度，“0表示熄灭，100表示最亮”
void LED_SetBrightness(u8 percent);

// 翻转8个LED的当前状态并等待500ms
void all_LED_Flashing(void);

// 执行LED交替点亮测试
void test_LED(void);


/*
 * 仅点亮一盏 LED。
 *
 * index：LED 编号，0 对应 LED1，7 对应 LED8。
 *
 * 本开发板的 LED 使用低电平点亮；调用本函数时会先熄灭全部 8 盏，
 * 再点亮 index 指定的一盏，因此最终始终最多只有一盏 LED 亮。
 * 若 index 不在 0~7 范围内，switch 不会匹配任何 LED，全部 LED 保持熄灭。
 */
void LED_ShowOnly(u8 index);

/* 从 LED1 开始连续点亮 count 盏 LED；count=0 全灭，count=8 全亮。 */
void LED_ShowCount(u8 count);

#endif /* __LED_H */
