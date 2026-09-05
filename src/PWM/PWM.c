#include "PWM.h"
#include "STC8H_PWM.h"
#include "NVIC.h"
#include "Switch.h"

/**************************
    MAIN_Fosc       FREQ
    24 000 000    /   1000    =   24 000
    从数0到24000为1ms
    (MAIN_Fosc / FREQ) - 1 因为0也算1位
 **************************/
#define FREQ		1000    // 输出频率
#define PERIOD 	((MAIN_Fosc / FREQ) - 1)	// 一个周期的计数值
// 初始化PWM4互补输出的测试配置。
void PWM_Init(void) {
    PWMx_InitDefine PWMx_InitStructure;

    /*
     模式：
         ● 冻结: CCMRn_FREEZE
         ● 匹配时设置通道 n 的输出为有效电平: CCMRn_MATCH_VALID
         ● 匹配时设置通道 n 的输出为无效电平: CCMRn_MATCH_INVALID
         ● 翻转: CCMRn_ROLLOVER
         ● 强制为无效电平:  CCMRn_FORCE_INVALID
         ● 强制为有效电平:  CCMRn_FORCE_VALID
         ● PWM 模式 1: CCMRn_PWM_MODE1
         ● PWM 模式 2: CCMRn_PWM_MODE2
  */
    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE2;

    /**************************
        PWM_Duty: 高电平计数值 -> 占空比  高电平占一个周期的比例值
        PWM_Period: 一个周期的计数值(高电平+低电平) 通过主频 / 频率
    **************************/
    PWMx_InitStructure.PWM_Duty = PERIOD * 0.5f; // PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = ENO4P | ENO4N;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM4, &PWMx_InitStructure);

    // 配置PWMA
    PWMx_InitStructure.PWM_Period = PERIOD; //周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0; //死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE; //主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE; //使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMA, &PWMx_InitStructure); //初始化PWM通用寄存器,  PWMA,PWMB

    // 切换PWM4选择PWM4_SW_P26_P27
    PWM4_SW(PWM4_SW_P26_P27); //PWM4_SW_P16_P17,PWM4_SW_P26_P27,PWM4_SW_P66_P67,PWM4_SW_P34_P33

    // 初始化PWMA的中断
    NVIC_PWM_Init(PWMA,DISABLE,Priority_0);
}
