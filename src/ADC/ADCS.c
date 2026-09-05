#include "ADCS.h"
#include "GPIO.h"
#include "ADC.h"
#include "NVIC.h"

/* 根据ADC通道配置当前工程已经使用的模拟输入引脚。 */
static void ADC_GPIO_Config(u8 channel) {
    GPIO_InitTypeDef GPIO_InitStructure; //结构定义

    GPIO_InitStructure.Mode = GPIO_HighZ; //指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP

    /* P0.4对应ADC通道12，目前供NTC温度采集使用。 */
    if (channel == ADC_CHANNEL_P04) {
        GPIO_InitStructure.Pin = GPIO_Pin_4;
        GPIO_Inilize(GPIO_P0, &GPIO_InitStructure);
    }
    /* P0.5对应ADC通道13，目前供电位器采样使用。 */
    else if (channel == ADC_CHANNEL_P05) {
        GPIO_InitStructure.Pin = GPIO_Pin_5;
        GPIO_Inilize(GPIO_P0, &GPIO_InitStructure);
    }
}

/* 配置ADC公共采样参数、时钟、电源和中断。 */
static void ADC_Config(void) {
    ADC_InitTypeDef ADC_InitStructure; //结构定义

    ADC_InitStructure.ADC_SMPduty = 31; //ADC 模拟信号采样时间控制, 0~31（注意： SMPDUTY 一定不能设置小于 10）
    ADC_InitStructure.ADC_CsSetup = 0; //ADC 通道选择时间控制 0(默认),1
    ADC_InitStructure.ADC_CsHold = 1; //ADC 通道选择保持时间控制 0,1(默认),2,3
    ADC_InitStructure.ADC_Speed = ADC_SPEED_2X1T; //设置 ADC 工作时钟频率	ADC_SPEED_2X1T~ADC_SPEED_2X16T
    ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED; //ADC结果调整,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
    ADC_Inilize(&ADC_InitStructure); //初始化
    ADC_PowerControl(ENABLE); //ADC电源开关, ENABLE或DISABLE
    NVIC_ADC_Init(DISABLE,Priority_0); //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}

/* 初始化默认的P0.5电位器ADC通道，保留原有ADC_Init()调用方式。 */
void ADC_Init(void) {
    /* 旧代码默认使用P0.5，因此兼容接口转到通道13的统一初始化函数。 */
    ADC_InitChannel(ADC_CHANNEL_P05);
}

/* 初始化指定ADC通道及其对应的模拟输入引脚。 */
void ADC_InitChannel(u8 channel) {
    /* 引脚配置与ADC公共配置都归ADC模块统一管理，调用者无需操作寄存器。 */
    ADC_GPIO_Config(channel);
    ADC_Config();
}

/* 读取指定ADC通道的一次12位转换结果。 */
u16 ADC_Read(u8 channel) {
    /* 对底层驱动函数做一层封装，使业务和传感器模块不直接依赖底层ADC接口。 */
    return Get_ADCResult(channel);
}
