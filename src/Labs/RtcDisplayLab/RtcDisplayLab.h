#ifndef __RTC_DISPLAY_LAB_H
#define __RTC_DISPLAY_LAB_H

#include "Config.h"

/* 测试RTC时间、闹钟和定时器的I2C读写及外部中断处理。 */
void Test_I2C_ReadOrWrite(void);

/* 在OLED上持续显示通过I2C读取的RTC日期和时间。 */
void I2COLedTest(void);

#endif /* __RTC_DISPLAY_LAB_H */
