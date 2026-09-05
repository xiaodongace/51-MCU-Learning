#include "KeyLedLab.h"

#include "Delay.h"
#include "Key.h"
#include "LED.h"


static u8 key1_locked = 0; // 防止按住 KEY1 时重复切换
static u8 key2_locked = 0; // 防止按住 KEY1 时重复切换

/*
* **要求**：

- KEY1 按下时点亮全部 LED，松开时熄灭全部 LED。
- KEY2 按下时只点亮 LED1，松开时熄灭全部 LED。
- 初始化时必须调用 `Key_Init()`，确认按键配置为带上拉输入。
- 在代码注释中写清楚“松开为 1、按下为 0”的原因。
- 暂时允许使用简单延时消抖，但要说明延时对主循环响应的影响。

**验收标准**：两个按键能够独立控制对应输出，不按键时 LED 不自行变化。
 */
void KeyLedLab_Init() {
    LED_Init();
    Key_Init();
}


void KeyLedLab_Task() {
    if (Key_IsPressed(0)) {
        if (key1_locked == 0) {
            // 消抖
            delay_ms(10);
            // key1被确认按下
            if (Key_IsPressed(0)) {
                key1_locked = 1;
                LED_SetAll(1);
            }
        }
    } else if (key1_locked != 0) {
        // 消抖
        delay_ms(10);
        // 确认松开
        if (!Key_IsPressed(0)) {
            key1_locked = 0;
            LED_SetAll(0);
        }
    }


    if (Key_IsPressed(1)) {
        if (key2_locked == 0) {
            delay_ms(10);
            // key2被确认按下
            if (Key_IsPressed(1)) {
                key2_locked = 1;
                LED_ShowOnly(0);
            }
        }
    } else if (key2_locked != 0) {
        delay_ms(10);
        if (!Key_IsPressed(1)) {
            key2_locked = 0;
            LED_SetAll(0);
        }
    }
}


