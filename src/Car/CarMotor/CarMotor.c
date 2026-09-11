#include "CarMotor.h"
#include "STC8H_PWM.h"
#include "NVIC.h"
#include "Switch.h"
#include "GPIO.h"

#define FREQ		1000    // 输出频率
#define PERIOD 	((MAIN_Fosc / FREQ) - 1)	// 一个周期的计数值

// -100 --------- 0 --------- 100		速度
//后退最大速度	  0			前进最大速度
// 0 ----------  50 --------- 100		PWM占空比
char speed2duty(char speed) {
    // speed > 0 前进
    // speed < 0 后退
    return speed / 2 + 50;
}


static void CarMotorPWM_Init(u8 value) {
    PWMx_InitDefine PWMx_InitStructure;

    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE2;
    PWMx_InitStructure.PWM_Duty = (speed2duty(value) / 100.0) * PERIOD; // PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = (value != 0) ? (ENO1P | ENO1N) : 0;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM1, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE2;
    PWMx_InitStructure.PWM_Duty = (speed2duty(value) / 100.0) * PERIOD; // PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = (value != 0) ? (ENO2P | ENO2N) : 0;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM2, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE2;
    PWMx_InitStructure.PWM_Duty = (speed2duty(value) / 100.0) * PERIOD; // PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = (value != 0) ? (ENO3P | ENO3N) : 0;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM3, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE2;
    PWMx_InitStructure.PWM_Duty = (speed2duty(value) / 100.0) * PERIOD; // PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = (value != 0) ? (ENO4P | ENO4N) : 0;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM4, &PWMx_InitStructure);

    // 配置PWMA
    PWMx_InitStructure.PWM_Period = PERIOD; //周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0; //死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE; //主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE; //使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMA, &PWMx_InitStructure); //初始化PWM通用寄存器,  PWMA,PWMB

    PWM1_SW(PWM1_SW_P20_P21);
    PWM2_SW(PWM2_SW_P22_P23);
    PWM3_SW(PWM3_SW_P14_P15);
    PWM4_SW(PWM4_SW_P16_P17);

    // 初始化PWMA的中断
    NVIC_PWM_Init(PWMA,DISABLE,Priority_0);
}

// 初始化
void CarMotors_init() {
    // P14 15 16 17
    P1_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    // P20 21 22 23
    P2_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);

    // 默认设置为低
    LF_P = LF_N = RF_P = RF_N = LB_P = LB_N = RB_P = RB_N = 0;
}

// 前进
void CarMotors_forward(char speed) {
    CarMotorPWM_Init(speed);
}

// 后退
void CarMotors_backward(char speed) {
    CarMotorPWM_Init(-speed);
}

// 停止
void CarMotors_stop() {
    CarMotorPWM_Init(0);
}