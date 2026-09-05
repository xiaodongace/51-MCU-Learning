#include "Buzzer.h"
#include "GPIO.h"
#include "NVIC.h"
#include "Switch.h" // 配置外设引脚
#include "STC8H_PWM.h"
#include "Delay.h"


#define BUZZER P00

//			 C	 D     E 	F	 G	  A	   B	 C`
static u16 code hz2[] = {523, 587, 659, 698, 784, 880, 988, 1047};

//			             C`	   D`     E`   F`	  G`	A`	  B`    C``
static u16 code hz[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976, 2093};

// 1 -  7
// 8 - 15
//16 - 23
static u16 code FREQS[] = {
    523 * 1, 587 * 1, 659 * 1, 698 * 1, 784 * 1, 880 * 1, 988 * 1,
    523 * 2, 587 * 2, 659 * 2, 698 * 2, 784 * 2, 880 * 2, 988 * 2,
    523 * 4, 587 * 4, 659 * 4, 698 * 4, 784 * 4, 880 * 4, 988 * 4,
    523 * 8, 587 * 8, 659 * 8, 698 * 8, 784 * 8, 880 * 8, 988 * 8,
};

// 初始化蜂鸣器GPIO，仅供本模块内部使用。
static void Buzzer_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure; //结构定义
    GPIO_InitStructure.Pin = GPIO_Pin_0; //指定要初始化的IO,
    GPIO_InitStructure.Mode = GPIO_OUT_PP; //指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(GPIO_P0, &GPIO_InitStructure); //初始化
}

// 配置蜂鸣器PWM频率，仅供本模块内部使用。
static void Buzzer_PWM_Config(u16 hz_output) {
    PWMx_InitDefine PWMx_InitStructure;

    u16 period = (MAIN_Fosc / hz_output);

    // 配置PWM5
    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE1;
    //模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
    PWMx_InitStructure.PWM_Duty = 0; //PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = ENO5P;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM5, &PWMx_InitStructure); //初始化PWM,  PWMA,PWMB

    // 配置PWMB
    PWMx_InitStructure.PWM_Period = period - 1; //周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0; //死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE; //主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE; //使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMB, &PWMx_InitStructure); //初始化PWM通用寄存器,  PWMA,PWMB

    // 切换PWM通道
    PWM5_SW(PWM5_SW_P00); //PWM5_SW_P20,PWM5_SW_P17,PWM5_SW_P00,PWM5_SW_P74

    // 初始化PWMB的中断
    NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
}


// 初始化蜂鸣器GPIO和PWM输出。
void Buzzer_Init(void) {
    EAXSFR(); /* 扩展寄存器使能 */
    Buzzer_GPIO_Init();
    Buzzer_PWM_Config(1000);
}

// 按照指定频率播放声音
// 按指定频率播放声音。
void Buzzer_Play(u16 hz_value) {
    //    PWM_config(hz_value);
    u16 period = (MAIN_Fosc / hz_value);
    u16 duty = period * 0.02f; // 这里的0.02可以决定音量大小

    // 设置PWMB输出Period (1/频率)
    PWMB_AutoReload(period - 1); //周期设置
    // 设置PWM5的占空比
    PWMB_Duty5(duty);
    // 启用PWMB通道5的输出使能
    PWMB_CC5E_Enable();
}

// 按照指定音调播放声音
// 按音调编号播放预设音符。
void Buzzer_Beep(u16 tone) {
    // 1,2,3,4,5,6,7,8
    // tone -> idx -> hz
    Buzzer_Play(FREQS[tone - 1]);
}

// 停止蜂鸣器输出。
void Buzzer_Stop(void) {
    //    PWMB_BrakeOutputEnable(DISABLE); // 关闭PWMB输出使能
    PWMB_CC5E_Disable(); // 单独关闭PWMB的通道5输出
}


/*************************************************************************************
驱动无源蜂鸣器

P00 推挽输出模式

无源蜂鸣器: 直接通电不能响, 必须要按照一定频率输出才能驱动
有源蜂鸣器: 直接拉高蒂安瓶就可以驱动(内部有振荡源)

使用PWM5驱动蜂鸣器
*************************************************************************************/
static u8 code notes[] = {
    L5,N1,N1,N3,  N6,N3,N5,    N5,N6,N5,N3, N4,N3,N2,
    L6,N2,N2,N4,  N7,N7,N6,N5, N4,N0,N4,N3, L6,L7,N1,
    N2, N2,N0,    L5,N1,N1,N3, N6,N3,N5,
    N5,N6,N5,N3,  N4,N3,N2,    L6,N2,N2,N4, N7,N7,N6,N5,
    N4,N4,N3,     L7,N2, N1,   N1,N0,L5,

};
// 2/4  -> 四分音节为一拍, 每小节2拍
// 时延数组 1->100ms
static u8 code durations[] = {
    3,1,3,1,      3,1,4,       3,1,3,1,   3,1,4,
    3,1,3,1,      3,1,3,1,     2,2,3,1,   2,4,2,
    8, 4,4,       3,1,3,1,     3,1,4,
    3,1,3,1,      3,1,3,       3,1,3,1,   3,1,3,1,
    4,3,1,        4,4, 8,      4,3,1,
};



// 播放预设旋律，作为蜂鸣器集成测试。
void Buzzer_Test(void) {
    u8 len = 0, i = 0;

    // 蜂鸣器初始化
    Buzzer_Init();

    len = sizeof(notes) / sizeof(notes[0]);
    for(i = 0; i < len; i++){
        // 按照指定音调输出
        Buzzer_Beep(notes[i]);

        // 每个音调后, 做休眠
        delay_X_ms(durations[i] * 100);

        // 音调之间做短暂间隔
        Buzzer_Stop();
        delay_ms(50);
    }
    Buzzer_Stop();
}
