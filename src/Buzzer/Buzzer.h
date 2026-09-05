#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "Config.h"

#define N0 0

#define	L1	1
#define	L2	2
#define	L3	3
#define	L4	4
#define	L5	5
#define	L6	6
#define	L7	7

#define	N1	L1 + 7
#define	N2	L2 + 7
#define	N3	L3 + 7
#define	N4	L4 + 7
#define	N5	L5 + 7
#define	N6	L6 + 7
#define	N7	L7 + 7

#define	H1	N1 + 7
#define	H2	N2 + 7
#define	H3	N3 + 7
#define	H4	N4 + 7
#define	H5	N5 + 7
#define	H6	N6 + 7
#define	H7	N7 + 7

// 初始化蜂鸣器GPIO和PWM输出。
void Buzzer_Init(void);

// 按指定频率播放声音。
void Buzzer_Play(u16 hz_value);

// 按照指定音调播放声音
void Buzzer_Beep(u16 tone);

// 停止蜂鸣器输出。
void Buzzer_Stop(void);

// 集成测试蜂鸣器
void Buzzer_Test(void);

#endif
