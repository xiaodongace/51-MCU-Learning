#include "CarBuzzer.h"
#include "Buzzer.h"


/* 兰花草主旋律音符 */
u8 code LanHuaCao_Notes[] = {
    // 我从山中来
    N6, N3, N3, N3, N3, N2,

    // 带着兰花草
    N1, N2, N1, N7, N6,

    // 家中无富贵
    N6, N6, N6, N6, N6, N5,

    // 口袋无财宝
    N3, N5, N5, N4, N3,

    // 寒风终刺骨
    N3, N6, N6, N5, N3, N2,

    // 勤为好仕途
    N1, N2, N1, N7, N6, N3,

    // 博得明月出
    N3, N1, N1, N7, N6, N3,

    // 用兰花换锦服
    N2, N1, N7, N5, N6
};

/* 每个数字代表100ms */
u8 code LanHuaCao_Durations[] = {
    // 我从山中来
    3, 3, 3, 3, 4, 3,

    // 带着兰花草
    3, 3, 3, 3, 6,

    // 家中无富贵
    3, 3, 3, 3, 4, 3,

    // 口袋无财宝
    3, 3, 3, 4, 6,

    // 寒风终刺骨
    3, 3, 3, 3, 4, 3,

    // 勤为好仕途
    3, 3, 3, 3, 4, 3,

    // 博得明月出
    3, 3, 3, 3, 4, 3,

    // 用兰花换锦服
    3, 3, 3, 4, 8
};

// 1 -  7
// 8 - 15
//16 - 23
static u16 code FREQS[] = {
    523 * 1, 587 * 1, 659 * 1, 698 * 1, 784 * 1, 880 * 1, 988 * 1,
    523 * 2, 587 * 2, 659 * 2, 698 * 2, 784 * 2, 880 * 2, 988 * 2,
    523 * 4, 587 * 4, 659 * 4, 698 * 4, 784 * 4, 880 * 4, 988 * 4,
    523 * 8, 587 * 8, 659 * 8, 698 * 8, 784 * 8, 880 * 8, 988 * 8,
};

static void CarBuzzer_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure; //结构定义
    GPIO_InitStructure.Pin = GPIO_Pin_4; //指定要初始化的IO,
    GPIO_InitStructure.Mode = GPIO_OUT_PP; //指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure); //初始化
}

// 配置蜂鸣器PWM频率，仅供本模块内部使用。
static void Buzzer_PWM_Config(u16 hz_output) {
    PWMx_InitDefine PWMx_InitStructure;

    u16 period = (MAIN_Fosc / hz_output);

    // 配置PWM5
    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE1;
    //模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
    PWMx_InitStructure.PWM_Duty = 0; //PWM占空比时间, 0~Period
    PWMx_InitStructure.PWM_EnoSelect = ENO8P;
    //输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
    PWM_Configuration(PWM8, &PWMx_InitStructure); //初始化PWM,  PWMA,PWMB

    // 配置PWMB
    PWMx_InitStructure.PWM_Period = period - 1; //周期时间,   0~65535
    PWMx_InitStructure.PWM_DeadTime = 0; //死区发生器设置, 0~255
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE; //主输出使能, ENABLE,DISABLE
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE; //使能计数器, ENABLE,DISABLE
    PWM_Configuration(PWMB, &PWMx_InitStructure); //初始化PWM通用寄存器,  PWMA,PWMB

    // 切换PWM通道
    PWM8_SW(PWM8_SW_P34); //PWM5_SW_P20,PWM5_SW_P17,PWM5_SW_P00,PWM5_SW_P74

    // 初始化PWMB的中断
    NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
}

void CarBuzzer_Init() {
    CarBuzzer_GPIO_Init();
    Buzzer_PWM_Config(1000);
}

// 按照指定频率播放声音
// 按指定频率播放声音。
void CarBuzzer_Play(u16 hz_value) {
    //    PWM_config(hz_value);
    u16 period = (MAIN_Fosc / hz_value);
    u16 duty = period * 0.02f; // 这里的0.02可以决定音量大小

    // 设置PWMB输出Period (1/频率)
    PWMB_AutoReload(period - 1); //周期设置
    // 设置PWM8的占空比
    PWMB_Duty8(duty);
    // 启用PWMB通道8的输出使能
    PWMB_CC8E_Enable();
}

// 按照指定音调播放声音
// 按音调编号播放预设音符。
void CarBuzzer_Beep(u16 tone) {
    // 1,2,3,4,5,6,7,8
    // tone -> idx -> hz
    CarBuzzer_Play(FREQS[tone - 1]);
}

// 停止蜂鸣器输出。
void CarBuzzer_Stop(void) {
    //    PWMB_BrakeOutputEnable(DISABLE); // 关闭PWMB输出使能
    PWMB_CC8E_Disable(); // 单独关闭PWMB的通道8输出
}


void CarBuzzer_Alarm() {
    // 659 * 2
    CarBuzzer_Play(659 * 2);
    os_wait2(K_TMO, 20);
    CarBuzzer_Stop();
    os_wait2(K_TMO, 10);

    // 587 * 2
    CarBuzzer_Play(587 * 2);
    os_wait2(K_TMO, 20);
    CarBuzzer_Stop();
    os_wait2(K_TMO, 10);

    // 523 * 2
    CarBuzzer_Play(523 * 2);
    os_wait2(K_TMO, 20);
    CarBuzzer_Stop();
    os_wait2(K_TMO, 10);
}


void CarBuzzer_Test_Beep() {
    u8 i;
    u8 len = sizeof(LanHuaCao_Notes) / sizeof(LanHuaCao_Notes[0]);

    for (i = 0; i < len; i++) {
        CarBuzzer_Beep(LanHuaCao_Notes[i]);
        os_wait2(K_TMO, LanHuaCao_Durations[i] * 15);

        CarBuzzer_Stop();
        os_wait2(K_TMO, 5);
    }
}