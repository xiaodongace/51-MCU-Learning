#include "Timers.h"
#include "Timer.h"
#include "NVIC.h"

volatile u16 system_ms = 0; // 系统时间

/* 配置Timer1为1 ms自动重装系统节拍。 */
static void Timer_Config(void) {
    TIM_InitTypeDef TIM_InitStructure; //结构定义
    /* Timer0 由 RTX51 Tiny 调度器占用；应用层节拍固定使用 Timer1。 */
    TIM_InitStructure.TIM_Mode = TIM_16BitAutoReload;
    //指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
    TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T; //指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
    TIM_InitStructure.TIM_ClkOut = DISABLE; //是否输出高速脉冲, ENABLE或DISABLE
    TIM_InitStructure.TIM_Value = 65536UL - (MAIN_Fosc / 1000UL); //初值,
    TIM_InitStructure.TIM_Run = ENABLE; //是否初始化后启动定时器, ENABLE或DISABLE
    Timer_Inilize(Timer1, &TIM_InitStructure);
    /* 开启 Timer1 中断后，Timer1_ISR_Handler 每 1 ms 递增 system_ms。 */
    NVIC_Timer1_Init(ENABLE,Priority_0);
}

// 初始化应用层定时器模块。
void Timers_Init(void) {
    /* 在这里完成 Timer 模块的初始化。 */
    Timer_Config();
}

u16 Timers_GetSystemMs(void) {
    return system_ms;
}
