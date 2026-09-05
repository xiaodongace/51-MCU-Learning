#ifndef __IIC_H
#define __IIC_H

#include "Config.h"

// 时钟结构体
typedef struct Clock
{
    u16 year;
    u8 month;
    u8 day;
    u8 week;
    u8 hour;
    u8 minute;
    u8 second;
} Clock_t;

// 闹钟结构体
typedef struct Alarm
{
    // [-128, 127]
    int8 minute;
    int8 hour;
    int8 day;
    int8 week;
} Alarm_t;

typedef enum TimerFreq
{
    HZ4096 = 0,
    HZ64 = 1,
    HZ1 = 2,
    HZ1_60 = 3
} TimerFreq_t;

// 设备写地址 0xA2;
#define PCF8563_ADDR    (0x51 << 1)
// 寄存器地址: 从秒钟寄存器开始读
#define PCF8563_REG    0x02

// 初始化引脚和I2C、串口
void IIC_Init(void);

// 设置时间
void I2C_Set_Clock(Clock_t clock);

// 读取时间
void I2C_Get_Clock(Clock_t* clock);

// 设置闹钟
void I2C_Set_Alarm(Alarm_t alarm);

// 启用闹钟
void I2C_Enable_Alarm(u8 enable);

// 清理闹钟标记
void I2C_Clear_Alarm(void);

// 设置定时器
void I2C_Set_Timer(TimerFreq_t freq, u8 conut);

// 启用定时器
void I2C_Enable_Timer(u8 enable);

// 清理定时器标记
void I2C_Clear_Timer(void);

// Alarm和Timer中断回调
void on_int3_call(void);

#endif /* __IIC_H */
