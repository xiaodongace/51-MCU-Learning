#ifndef __LED_FLIP_LAB_H
#define __LED_FLIP_LAB_H


/* 初始化LED翻转实验、公共按键事件和系统定时器。 */
void LedFlipLab_Init(void);

/* 在主循环中扫描按键事件，并处理LED翻转或复位。 */
void LedFlipLab_Task(void);


#endif
