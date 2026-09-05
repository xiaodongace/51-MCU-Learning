#include "LedModelLab.h"
#include "LED.h"
#include "Key.h"
#include "Delay.h"


static u8 current_mode = 0; // 保存当前模式
static u8 key_locked = 0; // 防止按住 KEY1 时重复切换

// 处理 KEY1 的一次有效按下，更新 current_mode，并处理模式循环边界
static void LedModelLab_HandleKey(void) {
    if (Key_IsPressed(0)) {
        if (key_locked == 0) {
            delay_ms(10);

            if (Key_IsPressed(0)) {
                current_mode++;
                if (current_mode >= 4) {
                    current_mode = 0;
                }
                key_locked = 1;
            }
        }
    }
    else if (key_locked != 0) {
        delay_ms(10);
        if (!Key_IsPressed(0)) {
            key_locked = 0;
        }
    }
}

// 读取 current_mode，根据当前模式调用 LED_ShowOnly() 或 LED_SetAll() 显示对应 LED
static void LedModelLab_ShowMode(void) {
    switch (current_mode) {
        case 0:
            LED_ShowOnly(0);
            break;
        case 1:
            LED_ShowOnly(7);
            break;
        case 2:
            LED_SetAll(1);
            break;
        case 3:
            LED_SetAll(0);
            break;
    default: break;
    }
}

// LedModelLab初始化
void LedModelLab_Init() {
    LED_Init();
    Key_Init();

    LedModelLab_ShowMode();
}

/*
    要求：
        - 上电后显示 LED1。
        - KEY1 每完整按下一次，切换到下一个模式。
        - 模式循环如下：
            模式 0：仅 LED1 亮
            模式 1：仅 LED8 亮
            模式 2：8 个 LED 全亮
            模式 3：8 个 LED 全灭
            模式 0：重新开始
    限制：
        按住 KEY1 不能连续切换。
        必须使用已有的 LED_ShowOnly() 和 LED_SetAll()。
        不允许在 Lab 中直接操作 P20、P27 等 LED 引脚。
        main.c 只负责初始化和调用 LedModeLab_Task()。
 */
void LedModelLab_Task(void) {
    LedModelLab_HandleKey();
    LedModelLab_ShowMode();
}

