#include "AlarmLab.h"
#include "Key.h"
#include "Buzzer.h"
#include "LED.h"

static void AlarmLab_Stop() {
    LED_SetAll(DISABLE);
    Buzzer_Stop();
}

static void AlarmLab_Start() {
    LED_SetAll(ENABLE);
    Buzzer_Beep(1);
}

void AlarmLab_Init() {
    Key_Init();
    LED_Init();
    Buzzer_Init();

    AlarmLab_Stop();
}


/***
 *要求**：
- KEY1 按下时 LED 全亮并启动蜂鸣器。
- KEY1 松开时 LED 全灭并停止蜂鸣器。
- 将“开始报警”和“停止报警”分别封装成函数。
- 停止函数必须同时关闭所有相关外设。
- 检查如果 LED 已经亮着或蜂鸣器已经停止，重复调用停止函数是否安全。

**验收标准**：LED 和蜂鸣器状态始终一致，不出现 LED 已灭但蜂鸣器仍响的情况。
*/
void AlarmLab_Task() {
    if (Key_IsPressed(0)) {
        AlarmLab_Start();
    } else {
        AlarmLab_Stop();
    }
}


