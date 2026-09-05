#ifndef __MOTOR_H
#define __MOTOR_H

#include "config.h"

// 初始化电机GPIO。
void Motor_Init(void);

// 启动震动马达。
void Motor_Start(void);

// 停止震动马达。
void Motor_Stop(void);

// 按百分比设置震动马达PWM占空比，范围为0~100。
void Motor_SetSpeed(u8 percent);

// 执行电机GPIO启停测试。
void Motor_Test(void);

// 执行电机PWM渐变调速测试。
void Motor_PWM_Test(void);

#endif /* __MOTOR_H */
