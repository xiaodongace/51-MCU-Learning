#include "BuzzerArrayLab.h"
#include "Buzzer.h"
#include "Delay.h"
#include "Key.h"
#include "LED.h"


/* 旋律中共有三个音符。 */
#define NOTE_COUNT 3

/* Buzzer_Beep() 接收的是预设音调编号，而不是频率值。 */
#define NOTE_C5 1
#define NOTE_D5 2
#define NOTE_E5 3

static u8 code notes[NOTE_COUNT] = {
    NOTE_C5,
    NOTE_D5,
    NOTE_E5
};

/* 当前正在播放的音符下标。播放结束或被停止后必须恢复为 0。 */
static u8 note_index;

/*
 * KEY1 播放锁：0 表示已松开、允许启动一次旋律；1 表示本次按下
 * 已经处理。它避免 KEY1 一直按住时，旋律播放完成后又立刻重播。
 */
static u8 key1_locked;

void BuzzerArrayLab_Init() {
    /* 为下一次播放建立确定的初始状态。 */
    note_index = 0;
    key1_locked = 0;

    /* 初始化本实验所使用的输入和输出模块。 */
    Key_Init();
    LED_Init();
    /* LED 在本题中用作“正在播放”的指示：上电时默认熄灭。 */
    LED_SetAll(DISABLE);
    Buzzer_Init();
    /* 蜂鸣器 PWM 已配置，但先关闭输出，保证上电不会发声。 */
    Buzzer_Stop();
}

/*
 * 阻塞等待 ms 毫秒，并且每等待 10 ms 就检查一次 KEY2。
 *
 * 返回 0：等待时间完整结束，KEY2 没有被确认按下。
 * 返回 1：KEY2 经 10 ms 简单消抖后确认按下，外层应立即停止旋律。
 *
 * 参数 ms 直接保存“剩余等待时间”，因此不需要额外的 elapsed_ms。
 * 例如传入 250 时：250 -> 240 -> ... -> 10 -> 0，正好等待 25 个 10 ms。
 * 此函数仍会阻塞主循环；它只是将最长按键响应时间缩短到约 10 ms。
 */
static u8 BuzzerArrayLab_WaitAndCheckStop(u16 ms) {
    while (ms >= 10) {
        /* PWM 已在后台持续输出当前音符；CPU 在这里仅等待 10 ms。 */
        delay_ms(10);
        ms -= 10;

        /* KEY2 的下标为 1；Key_IsPressed() 返回非零表示按下。 */
        if (Key_IsPressed(1)) {
            /* 再等待一次并复查，滤除 KEY2 按下瞬间的机械抖动。 */
            delay_ms(10);
            if (Key_IsPressed(1)) {
                /* 把“停止请求”交给外层统一处理 LED、蜂鸣器和下标。 */
                return 1;
            }
        }
    }

    /* 剩余等待时间已耗尽，说明当前音符可以正常结束。 */
    return 0;
}

/*
* **要求**：

- 建立一个至少包含 3 个音符的数组，例如 `C5、D5、E5`。
- KEY1 按下后从数组下标 0 开始播放。
- 当前音符播放结束后，下标加 1，播放下一个音符。
- 播放到最后一个音符后，关闭蜂鸣器并把下标恢复为 0。
- 播放结束后再次按 KEY1，必须从第一个音符重新开始。
- 明确处理 `note_index >= NOTE_COUNT` 的边界。

**验收标准**：音符顺序正确，数组不会越界，播放完成后状态能够复位。
 */
void BuzzerArrayLab_Task() {
    /* KEY1 被检测为按下后，只有未锁定时才允许启动新的一轮播放。 */
    if (Key_IsPressed(0)) {
        if (key1_locked == 0) {
            /* 对 KEY1 的按下进行一次简单的 10 ms 确认。 */
            delay_ms(10);

            /* 第二次仍按下，才把这次按下视为有效。 */
            if (Key_IsPressed(0)) {
                /* 立刻锁定；即使旋律播放完成，KEY1 按住时也不会重播。 */
                key1_locked = 1;

                /* 用所有 LED 表示“旋律正在播放”。 */
                LED_SetAll(ENABLE);

                /*
                 * note_index 从 0 依次递增到 NOTE_COUNT - 1。
                 * 循环条件保证访问 notes[note_index] 前，下标始终合法。
                 */
                for (note_index = 0; note_index < NOTE_COUNT; note_index++) {
                    /* 选择当前数组元素对应的音调，并启动该音符。 */
                    Buzzer_Beep(notes[note_index]);

                    /*
                     * 当前音符保持约 250 ms。等待中若 KEY2 被确认按下，
                     * 函数返回 1，说明本次旋律需要立刻中止。
                     */
                    if (BuzzerArrayLab_WaitAndCheckStop(250)) {
                        /* KEY2 中止路径：关闭所有输出并恢复下标。 */
                        LED_SetAll(DISABLE);
                        Buzzer_Stop();
                        note_index = 0;

                        /*
                         * 退出整个 Task，而不是只退出 if 或 for；
                         * 这样后面的音符不会继续被播放。
                         */
                        return;
                    }
                }

                /* 正常播放到最后一个音符后，也恢复初始状态。 */
                note_index = 0;
                Buzzer_Stop();
                LED_SetAll(DISABLE);
            }
        }
    } else if (key1_locked != 0) {
        /* KEY1 已从按下变为松开，准备解除播放锁。 */
        delay_ms(10);

        /* 松开状态稳定 10 ms 后才解锁，避免松开抖动导致提前重播。 */
        if (!Key_IsPressed(0)) {
            key1_locked = 0;
        }
    }
}


