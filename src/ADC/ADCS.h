#ifndef __ADCS_H
#define __ADCS_H

#include "config.h"

/* 当前工程使用的ADC通道与物理引脚映射。 */
#define ADC_CHANNEL_P04 12
#define ADC_CHANNEL_P05 13

// 初始化默认的P0.5电位器ADC通道。
void ADC_Init(void);

/* 初始化指定ADC通道及其模拟输入引脚。 */
void ADC_InitChannel(u8 channel);

/* 读取指定ADC通道的一次12位转换结果。 */
u16 ADC_Read(u8 channel);

#endif /* __ADCS_H */
