#include "BuzzerToneLab.h"
#include "Buzzer.h"
#include "Key.h"

void BuzzerToneLab_Init(void) {
    Buzzer_Init();
    Key_Init();
    Buzzer_Stop();
}


/*
* **要求**：

- 上电后蜂鸣器必须停止。
- KEY1 按下后播放一个指定音调。
- KEY1 松开后停止蜂鸣器。
- 重复按下和松开时，蜂鸣器状态必须正确恢复。
- 不允许直接在业务代码中操作蜂鸣器底层寄存器，应调用模块公开接口。

**验收标准**：按键按住才有声音，松开立即停止；上电不会自动发声。
 */
void BuzzerToneLab_Task(void) {
    if (Key_IsPressed(0)) {
        Buzzer_Beep(1);
    } else {
        Buzzer_Stop();
    }
}