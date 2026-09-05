#ifndef __TIMERS_H
#define __TIMERS_H

#include "Config.h"

// 初始化应用层定时器模块。
void Timers_Init(void);

u16 Timers_GetSystemMs(void);

#endif /* __TIMERS_H */
