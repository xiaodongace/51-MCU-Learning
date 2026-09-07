#include "LED.h"
#include "GPIO.h"
#include "Delay.h"
#include "STC8H_PWM.h"
#include "NVIC.h"
#include "Switch.h"

#define LED_PWM_FREQUENCY 1000
#define LED_PWM_PERIOD ((MAIN_Fosc / LED_PWM_FREQUENCY) - 1)

#define LED_SW P45
#define LED1 P27
#define LED2 P26
#define LED3 P15
#define LED4 P14
#define LED5 P23
#define LED6 P22
#define LED7 P21
#define LED8 P20

/* LED PWM的比较值和一次性初始化标志只属于LED模块。 */
static PWMx_Duty led_pwm_duty;
static u8 led_pwm_initialized = 0;

/* 一次性配置8盏LED使用的4组PWMA通道。 */
static void LED_PWM_Config(void) {
    PWMx_InitDefine PWMx_InitStructure;

    /* 每组P/N输出连接两盏LED，调整N端极性后让同组波形方向一致。 */
    PWMx_InitStructure.PWM_Mode = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty = led_pwm_duty.PWM1_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO1P | ENO1N;
    PWM_Configuration(PWM1, &PWMx_InitStructure);
    PWMA_CC1P_HighValid();
    PWMA_CC1NP_LowValid();

    PWMx_InitStructure.PWM_Duty = led_pwm_duty.PWM2_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO2P | ENO2N;
    PWM_Configuration(PWM2, &PWMx_InitStructure);
    PWMA_CC2P_HighValid();
    PWMA_CC2NP_LowValid();

    PWMx_InitStructure.PWM_Duty = led_pwm_duty.PWM3_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO3P | ENO3N;
    PWM_Configuration(PWM3, &PWMx_InitStructure);
    PWMA_CC3P_HighValid();
    PWMA_CC3NP_LowValid();

    PWMx_InitStructure.PWM_Duty = led_pwm_duty.PWM4_Duty;
    PWMx_InitStructure.PWM_EnoSelect = ENO4P | ENO4N;
    PWM_Configuration(PWM4, &PWMx_InitStructure);
    PWMA_CC4P_HighValid();
    PWMA_CC4NP_LowValid();

    /* 四组LED通道都属于PWMA，不能在这里修改蜂鸣器和马达使用的PWMB。 */
    PWMx_InitStructure.PWM_Period = LED_PWM_PERIOD;
    PWMx_InitStructure.PWM_DeadTime = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable = ENABLE;
    PWM_Configuration(PWMA, &PWMx_InitStructure);

    /* 将四组PWMA通道映射到LED实际连接的八个引脚。 */
    PWM1_SW(PWM1_SW_P20_P21);
    PWM2_SW(PWM2_SW_P22_P23);
    PWM3_SW(PWM3_SW_P14_P15);
    PWM4_SW(PWM4_SW_P26_P27);
    NVIC_PWM_Init(PWMA, DISABLE, Priority_0);
}

// 初始化LED GPIO。
void LED_Init(void) {
    /* 在这里完成 LED 模块的初始化。 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_InitStructure.Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Inilize(GPIO_P2, &GPIO_InitStructure);

    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_InitStructure.Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Inilize(GPIO_P1, &GPIO_InitStructure);

    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_InitStructure.Pin = GPIO_Pin_5;
    GPIO_Inilize(GPIO_P4, &GPIO_InitStructure);
}


// 执行默认方向的LED流水灯测试。
void Flowing_LED_Test(void) {
    LED_Init();
    P27 = !P27;
    delay_X_ms(500);
    P26 = !P26;
    delay_X_ms(500);
    P15 = !P15;
    delay_X_ms(500);
    P14 = !P14; //
    delay_X_ms(500);
    P23 = !P23;
    delay_X_ms(500);
    P22 = !P22; //
    delay_X_ms(500);
    P21 = !P21;
    delay_X_ms(500);
    P20 = !P20; //
    delay_X_ms(500);
}

// 按指定方向执行一次LED流水灯，reverse为非零时反向运行。
void LED_Flowing_Test(u8 reverse) {
    u8 i;

    LED_Init();
    LED_SW = 0;
    for (i = 0; i < 8; i++) {
        u8 led = reverse ? (7 - i) : i;

        LED1 = (led == 0) ? 0 : 1;
        LED2 = (led == 1) ? 0 : 1;
        LED3 = (led == 2) ? 0 : 1;
        LED4 = (led == 3) ? 0 : 1;
        LED5 = (led == 4) ? 0 : 1;
        LED6 = (led == 5) ? 0 : 1;
        LED7 = (led == 6) ? 0 : 1;
        LED8 = (led == 7) ? 0 : 1;
        delay_ms(250);
        delay_ms(250);
    }

    LED1 = 1;
    LED2 = 1;
    LED3 = 1;
    LED4 = 1;
    LED5 = 1;
    LED6 = 1;
    LED7 = 1;
    LED8 = 1;
}

// 同时控制8个LED，非零为点亮，零为熄灭。
void LED_SetAll(u8 on) {
    // P45为LED总使能，低电平有效。
    LED_SW = 0;
    LED1 = on ? 0 : 1;
    LED2 = on ? 0 : 1;
    LED3 = on ? 0 : 1;
    LED4 = on ? 0 : 1;
    LED5 = on ? 0 : 1;
    LED6 = on ? 0 : 1;
    LED7 = on ? 0 : 1;
    LED8 = on ? 0 : 1;
}

/* 按0~100%的逻辑百分比设置8盏LED的PWM亮度。 */
void LED_SetBrightness(u8 percent) {
    /* 超过100%的输入统一钳位，防止比较值越过PWM周期。 */
    if (percent > 100) {
        percent = 100;
    }

    /* PWM硬件只在第一次调光时初始化，后续调用只更新比较值。 */
    if (led_pwm_initialized == 0) {
        LED_Init();
        LED_PWM_Config();
        LED_SW = 0;
        led_pwm_initialized = 1;
    }

    /* LED低电平点亮，因此逻辑亮度要反向换算为硬件高电平占空比。 */
    led_pwm_duty.PWM1_Duty = LED_PWM_PERIOD * (100 - percent) / 100;
    led_pwm_duty.PWM2_Duty = LED_PWM_PERIOD * (100 - percent) / 100;
    led_pwm_duty.PWM3_Duty = LED_PWM_PERIOD * (100 - percent) / 100;
    led_pwm_duty.PWM4_Duty = LED_PWM_PERIOD * (100 - percent) / 100;

    /* 一次更新四组比较值，让八盏LED同步切换亮度。 */
    UpdatePwm(PWMA, &led_pwm_duty);
}


// 翻转8个LED的当前状态并等待500ms，仅保留给旧版测试代码使用。
void all_LED_Flashing(void) {
    P27 = !P27;
    P26 = !P26;
    P15 = !P15;
    P14 = !P14; //
    P23 = !P23;
    P22 = !P22; //
    P21 = !P21;
    P20 = !P20; //
    delay_X_ms(500);
}


// 偶数流水灯点亮熄灭
void test_LED(void) {
    LED_Init();
    P45 = 0;
    while (1) {
        LED2 = 0;
        delay_X_ms(500);
        LED4 = 0;
        delay_X_ms(500);
        LED6 = 0;
        delay_X_ms(500);
        LED8 = 0;
        delay_X_ms(500);

        LED2 = 1;
        LED4 = 1;
        LED6 = 1;
        LED8 = 1;
        delay_X_ms(500);
    }
}


/*
 * 仅点亮一盏 LED。
 *
 * LED1~LED8 分布在 P1、P2 的不同引脚，无法直接使用一个端口的位掩码
 * 数组来统一访问。因此在 LED 模块内部使用 switch，将逻辑编号映射到实际
 * 引脚；外部调用者只需要知道 0~7 的编号，不需要知道任何硬件连接细节。
 *
 * 第一步调用 LED_SetAll(0)，把所有 LED 的输出置为高电平（熄灭）。
 * 第二步仅把目标 LED 置为低电平（点亮）。这样即使上一次点亮的是其他
 * LED，也不会残留两盏灯同时亮的状态。
 */
void LED_ShowOnly(u8 index) {
    LED_SetAll(0); /* 先建立“全部熄灭”的确定初始状态。 */

    switch (index) {
    /* 逻辑编号 0 对应第 1 盏 LED。 */
    case 0: LED1 = 0;
        break;
    case 1: LED2 = 0;
        break;
    case 2: LED3 = 0;
        break;
    case 3: LED4 = 0;
        break;
    case 4: LED5 = 0;
        break;
    case 5: LED6 = 0;
        break;
    case 6: LED7 = 0;
        break;
    case 7: LED8 = 0;
        break;
    default:
        /* 非法编号：前面已经全部熄灭，这里无需额外操作。 */
        break;
    }
}

/*
 * 累加式跑灯：先统一熄灭，再点亮 LED1~LEDcount。
 * 这样每次更新都得到确定的输出状态，不会因上一次状态而残留多余灯光。
 */
void LED_ShowCount(u8 count) {
    if (count > 8) {
        count = 8;
    }

    LED_SetAll(0);
    if (count >= 1) LED1 = 0;
    if (count >= 2) LED2 = 0;
    if (count >= 3) LED3 = 0;
    if (count >= 4) LED4 = 0;
    if (count >= 5) LED5 = 0;
    if (count >= 6) LED6 = 0;
    if (count >= 7) LED7 = 0;
    if (count >= 8) LED8 = 0;
}
