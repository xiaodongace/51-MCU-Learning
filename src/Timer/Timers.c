#include "Timers.h"
#include "Timer.h"
#include "NVIC.h"

volatile u16 system_ms = 0; // 系统时间

/* 配置Timer0为1 ms自动重装系统节拍。 */
static void Timer_Config(void) {
    TIM_InitTypeDef TIM_InitStructure; //结构定义
    //定时器0做16位自动重装, 中断频率为1000HZ
    TIM_InitStructure.TIM_Mode = TIM_16BitAutoReload;
    //指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
    TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T; //指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
    TIM_InitStructure.TIM_ClkOut = DISABLE; //是否输出高速脉冲, ENABLE或DISABLE
    TIM_InitStructure.TIM_Value = 65536UL - (MAIN_Fosc / 1000UL); //初值,
    TIM_InitStructure.TIM_Run = ENABLE; //是否初始化后启动定时器, ENABLE或DISABLE
    Timer_Inilize(Timer0, &TIM_InitStructure); //初始化Timer0	  Timer0,Timer1,Timer2,Timer3,Timer4
    NVIC_Timer0_Init(ENABLE,Priority_0); //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

// 初始化应用层定时器模块。
void Timers_Init(void) {
    /* 在这里完成 Timer 模块的初始化。 */
    Timer_Config();
}

u16 Timers_GetSystemMs(void) {
    return system_ms;
}
