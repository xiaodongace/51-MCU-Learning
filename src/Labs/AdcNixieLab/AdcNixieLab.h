#ifndef __ADC_NIXIE_LAB_H
#define __ADC_NIXIE_LAB_H

#include "Config.h"

/* 初始化 ADC 和数码管 */
void AdcNixieLab_Init();

/* 周期采样并刷新显示 */
void AdcNixieLab_Task();

#endif /* __ADC_NIXIE_LAB_H */
