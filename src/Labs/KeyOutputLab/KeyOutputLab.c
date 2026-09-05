#include "KeyOutputLab.h"
#include "Key.h"
#include "Motor.h"
#include "Buzzer.h"
#include "Delay.h"

#define KEY_OUTPUT_COUNT 2
#define KEY_STATE_UP     0
#define KEY_STATE_DOWN   1

/* 保存KEY1和KEY2上一次已经处理的逻辑状态。 */
static u8 last_key_states;

/* 读取指定按键保存的上一次逻辑状态。 */
static u8 KeyOutputLab_GetLastState(u8 key_index) {
    /* 每个按键占用一个二进制位，1表示按下，0表示松开。 */
    return (last_key_states >> key_index) & 0x01;
}

/* 保存指定按键本轮已经确认的逻辑状态。 */
static void KeyOutputLab_SetLastState(u8 key_index, u8 pressed) {
    if (pressed != 0) {
        /* 按下时把对应位置1，避免按住期间反复启动外设。 */
        last_key_states |= (0x01 << key_index);
    }
    else {
        /* 松开时把对应位清零，允许下一次按下再次触发。 */
        last_key_states &= ~(0x01 << key_index);
    }
}

/* 执行按键按下后的马达或蜂鸣器业务动作。 */
static void KeyOutputLab_HandlePressed(u8 key_index) {
    if (key_index == 0) {
        Motor_Start(); /* KEY1按下后启动震动马达。 */
    }
    else if (key_index == 1) {
        Buzzer_Play(1000); /* KEY2按下后播放1 kHz声音。 */
    }
}

/* 执行按键松开后的马达或蜂鸣器停止动作。 */
static void KeyOutputLab_HandleReleased(u8 key_index) {
    if (key_index == 0) {
        Motor_Stop(); /* KEY1松开后停止震动马达。 */
    }
    else if (key_index == 1) {
        Buzzer_Stop(); /* KEY2松开后停止蜂鸣器。 */
    }
}

/* 初始化按键控制马达和蜂鸣器实验，并建立安全的输出状态。 */
void KeyOutputLab_Init(void) {
    /* 各外设分别调用自己的初始化接口，Lab只负责组合调用顺序。 */
    Key_Init();
    Motor_Init();
    Buzzer_Init();

    /* 上电时先停止两个输出，并把按键历史状态设为全部松开。 */
    Motor_Stop();
    Buzzer_Stop();
    last_key_states = 0;
}

/* 扫描按键，并在状态边沿出现时控制马达和蜂鸣器。 */
void Key_Controller_Buzzer_Or_Motor(void) {
    u8 i;
    u8 pressed;

    /* KEY1和KEY2分别对应马达和蜂鸣器，两个按键互不影响。 */
    for (i = 0; i < KEY_OUTPUT_COUNT; i++) {
        pressed = Key_IsPressed(i);

        if ((pressed != 0) && (KeyOutputLab_GetLastState(i) == KEY_STATE_UP)) {
            /* 检测到松开到按下的变化，只执行一次启动动作。 */
            KeyOutputLab_HandlePressed(i);
            KeyOutputLab_SetLastState(i, KEY_STATE_DOWN);
        }
        else if ((pressed == 0) && (KeyOutputLab_GetLastState(i) == KEY_STATE_DOWN)) {
            /* 检测到按下到松开的变化，只执行一次停止动作。 */
            KeyOutputLab_HandleReleased(i);
            KeyOutputLab_SetLastState(i, KEY_STATE_UP);
        }
    }

    /* 保留原实验的简单阻塞消抖行为。 */
    delay_ms(10);
}
