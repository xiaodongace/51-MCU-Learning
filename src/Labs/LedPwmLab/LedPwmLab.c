#include "LedPwmLab.h"
#include "Key.h"
#include "LED.h"
#include "Timers.h"
#include "Delay.h"

static u8 pwm_num = 0;  // 动态pwm值


/*
 * 初始化固定档位LED亮度实验，并建立0%亮度和公共按键事件的初始状态。
 * PWM通道由首次LED_SetBrightness()配置，后续切换只更新占空比。
 */
void LedPwmLab_Init(void) {
    /* 软件档位从0%开始，并由同一次调用同步到实际PWM输出。 */
    pwm_num = 0;

    /* Key_Init()建立按键稳定状态，Timers_Init()提供10 ms消抖时间基准。 */
    Key_Init();
    Timers_Init();

    /* 首次调用完成LED PWM一次性配置，并输出确定的0%亮度。 */
    LED_SetBrightness(pwm_num);
}

/*
 * 使用KEY1的一次性按下事件，在0%、25%、50%、75%、100%之间循环切换。
 * 按键消抖由Key模块非阻塞完成，本任务只管理亮度档位和LED输出。
 */
void LedPwmLab_Task(void) {
    /* 每轮主循环更新公共按键状态，不使用delay_ms()阻塞其他任务。 */
    Key_Scan();

    /* 读取并消费KEY1事件，按住按键不会连续切换档位。 */
    if (Key_GetPressEvent(0) != 0) {
        /* 100%之后明确回到0%，保证输出始终处于题目要求的五个档位。 */
        if (pwm_num >= 100) {
            pwm_num = 0;
        } else {
            pwm_num += 25;
        }

        /* LED接口负责范围保护、低电平反向换算和占空比更新。 */
        LED_SetBrightness(pwm_num);
    }
}

/* 让LED亮度在0~100%之间循环变化，保留原来的PWM呼吸灯测试入口。 */
void PWM_Controller_LED_Test(void) {
    u8 brightness = 0;
    int8 direction = 1;

    /* LED模块负责PWM初始化，本测试只负责产生逐步变化的逻辑亮度。 */
    LED_SetBrightness(brightness);

    while (1) {
        if (direction > 0) {
            brightness++;
            if (brightness >= 100) {
                brightness = 100;
                direction = -1;
            }
        }
        else {
            brightness--;
            if (brightness == 0) {
                direction = 1;
            }
        }

        /* 每10 ms更新1%，形成约2秒的完整明暗循环。 */
        LED_SetBrightness(brightness);
        delay_ms(10);
    }
}
