#include "Exti.h"
#include "NVIC.h"
#include "EXTIS.h"
#include "GPIO.h"

/******************** INT配置 ********************/
void Exti_config(void) {
    EXTI_InitTypeDef Exti_InitStructure; //结构定义

    Exti_InitStructure.EXTI_Mode = EXT_MODE_Fall; //中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
    Ext_Inilize(EXT_INT3, &Exti_InitStructure); //初始化
    NVIC_INT3_Init(ENABLE,Priority_0); //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3
}


void EXTI_Init(void) {
    /*
        外部中断引脚:
            INT0	P3.2	支持上升沿和下降沿中断
            INT1	P3.3	支持上升沿和下降沿中断
            INT2	P3.6	只支持下降沿中断
            INT3	P3.7	只支持下降沿中断
            INT4	P3.0	只支持下降沿中断
     */
    GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
    GPIO_InitStructure.Pin  = GPIO_Pin_7;		//指定要初始化的IO,
    GPIO_InitStructure.Mode = GPIO_PullUp;	    //指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);//初始化

    Exti_config();
}
