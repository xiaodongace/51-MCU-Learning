#include "AdcModeLab.h"
#include "ADCS.h"
#include "Key.h"
#include "LED.h"
#include "Motor.h"
#include "Timers.h"

/* 当前模式是本Lab唯一描述模式状态的变量。 */
static AdcModeLab_Mode current_mode = ADC_MODE_LED;

/*
 * 初始化本题依赖的公共模块，并将所有可能产生动作的输出置为安全状态。
 * 本Lab按独立练习运行，因此不依赖SelfTest或其他业务模块提前完成初始化。
 */
void AdcModeLab_Init(void) {
    /* Key_Scan()的非阻塞消抖需要Timers_GetSystemMs()提供系统时间。 */
    Timers_Init();

    /* P0.5/ADC13是本题电位器输入，ADC模块负责模拟输入配置。 */
    ADC_InitChannel(ADC_CHANNEL_P05);

    /* 按键模块只负责按键输入和事件消抖。 */
    Key_Init();

    /* 两个外设都由各自模块初始化，Lab不直接操作任何PWM寄存器。 */
    LED_SetBrightness(0);
    Motor_Init();
    Motor_SetSpeed(0);

    /* 默认从LED亮度模式开始，避免上一次运行留下模式状态。 */
    current_mode = ADC_MODE_LED;
}

/* 返回当前模式，避免其他模块直接访问本Lab的内部状态变量。 */
AdcModeLab_Mode AdcModeLab_GetMode(void) {
    return current_mode;
}

/* 模式切换显示具体实现 */
void AdcModeLab_Task(void) {
    u16 adc_value;
    u8 percent;
    // 扫秒按键事件 非阻塞消抖
    Key_Scan();

    // 检查key1是否生成了有效事件
    if (Key_GetPressEvent(0)) {
        // 如果当前模式为LED 则替换为Motor 并将LED熄灭
        if (current_mode == ADC_MODE_LED) {
            current_mode = ADC_MODE_MOTOR;
            LED_SetBrightness(0);
        } else {
            // 反之
            current_mode = ADC_MODE_LED;
            Motor_SetSpeed(0);
        }
    }

    // 读取ADC值 并转换为百分比 公式为 -> 读取的ADC_Value * 百分比 / 电位器最大值
    adc_value = ADC_Read(ADC_CHANNEL_P05);
    percent = (u8)(((u32)adc_value * 100) / 2660);
    // 限制最大百分比
    if (percent > 100) {
        percent = 100;
    }

    // 根据模式启用LED和Motor
    if (current_mode == ADC_MODE_LED) {
        Motor_SetSpeed(0);
        LED_SetBrightness(percent);
    } else {
        Motor_SetSpeed(percent);
        LED_SetBrightness(0);
    }
}
