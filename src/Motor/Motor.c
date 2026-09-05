#include "Motor.h"
#include "GPIO.h"
#include "Delay.h"
#include "NVIC.h"
#include "STC8H_PWM.h"
#include "Switch.h"

#define PERIOD (MAIN_Fosc / 1000)
static PWMx_Duty xdata dutyB;
static bit motor_pwm_initialized = 0;

/* 配置马达控制引脚为推挽输出，仅供本模块内部调用。 */
static void Motor_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure; // 结构定义
    GPIO_InitStructure.Pin = GPIO_Pin_1; // 指定要初始化的IO
    GPIO_InitStructure.Mode = GPIO_OUT_PP; // 指定IO的输入或输出方式
    GPIO_Inilize(GPIO_P0, &GPIO_InitStructure); // 初始化
}

/* 初始化马达控制GPIO。 */
void Motor_Init(void) {
    Motor_GPIO_Init();                   // 配置P01为马达控制输出。
}

/* 直接启动震动马达，适用于全速开关控制。 */
void Motor_Start(void) {
    P01 = 1;
}

/* 直接停止震动马达。 */
void Motor_Stop(void) {
    P01 = 0;
}

/* 以“运行1秒、停止1秒”的方式测试马达GPIO控制。 */
void Motor_Test(void) {
    Motor_Start();
    delay_X_ms(1000);

    Motor_Stop();
    delay_X_ms(1000);
}


/* 配置马达使用的PWM6通道，仅供本模块内部调用。 */
static void Motor_PWM_Config(void) {
    PWMx_InitDefine PWMx_InitStructure;

    // 配置PWM6
    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty = dutyB.PWM6_Duty; //PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = ENO6P;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM6, &PWMx_InitStructure);
    // 配置PWMB
    PWMx_InitStructure.PWM_Period = PERIOD - 1; //周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0; //死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE; //主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE; //使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMB, &PWMx_InitStructure); //初始化PWM通用寄存器,  PWMA,PWMB

    // 切换PWM通道
    PWM6_SW(PWM6_SW_P01); //PWM6_SW_P21,PWM6_SW_P54,PWM6_SW_P01,PWM6_SW_P75

    // 初始化PWMB的中断
    NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
}

/* 按0~100%的百分比设置震动马达PWM占空比。 */
void Motor_SetSpeed(u8 percent) {
    if (percent > 100) {
        percent = 100;                    // 防止调用者传入超过100的非法占空比。
    }

    if (motor_pwm_initialized == 0) {
        dutyB.PWM6_Duty = 0;               // 首次使用PWM时先设置为0%，避免马达突然启动。
        Motor_PWM_Config();               // 初始化PWM6和PWMB的公共寄存器。
        motor_pwm_initialized = 1;        // 标记PWM已经完成初始化。
    }

    dutyB.PWM6_Duty = PERIOD * percent / 100; // 将百分比换算为PWM计数值。
    UpdatePwm(PWM6, &dutyB);               // 将新的计数值写入PWM6比较寄存器。

    if (percent == 0) {
        PWMB_CC6E_Disable();               // 0%时关闭PWM输出，确保马达停止。
    } else {
        PWMB_CC6E_Enable();                // 非0%时打开PWM输出。
    }
}


/* 让马达占空比在0~100%之间循环变化，用于测试PWM调速效果。 */
void Motor_PWM_Test(void) {
    int duty_percent = 0, dir = 1;

    dutyB.PWM6_Duty = 0;                   // 从0%占空比开始渐变。
    Motor_PWM_Config();                    // 初始化马达PWM通道。

    while (1) {
        duty_percent += dir;               // 按当前方向改变占空比。
        if (duty_percent >= 100) dir = -1; // 到达最大值后改为递减。
        else if (duty_percent <= 0) dir = 1; // 到达最小值后改为递增。

        if (duty_percent == 0) {
            delay_X_ms(500);               // 在完全停止状态额外停留500ms。
        }

        dutyB.PWM6_Duty = PERIOD * duty_percent / 100; // 百分比换算为PWM计数值。
        UpdatePwm(PWM6, &dutyB);             // 更新PWM6输出强度。
        delay_ms(20);                        // 每20ms改变一次，形成平滑渐变。
    }
}
