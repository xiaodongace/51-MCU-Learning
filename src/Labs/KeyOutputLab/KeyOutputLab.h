#ifndef __KEY_OUTPUT_LAB_H
#define __KEY_OUTPUT_LAB_H

#include "Config.h"

/* 初始化按键控制马达和蜂鸣器实验。 */
void KeyOutputLab_Init(void);

/* 扫描按键，并在按下或松开状态变化时控制马达和蜂鸣器。 */
void Key_Controller_Buzzer_Or_Motor(void);

#endif /* __KEY_OUTPUT_LAB_H */
