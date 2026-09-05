#include "LedBasicLab.h"
#include "Delay.h"
#include "LED.h"

/*
* **要求**：

- 创建或完善一个 LED 测试任务。
- 上电后所有 LED 必须保持熄灭，不能出现随机亮灯。
- 按照“全灭 1 秒、全亮 1 秒、全灭 1 秒”的顺序循环运行。
- 必须先调用 `LED_Init()`，再调用 LED 控制函数。
- 尽量通过 `LED_SetAll()` 控制全部 LED，不要在业务代码中重复操作每个引脚。

**验收标准**：8 个 LED 状态一致，亮灭逻辑与代码注释一致；复位后立即处于确定的熄灭状态。
 */
void LedBasicLab_Init() {
    LED_Init();
    P45 = 0;    // 开启LED总开关
    LED_SetAll(0);  // 默认上电后状态为关闭
}

// 所有LED闪烁
void LedBasicLab_Task() {
    LED_SetAll(1);
    delay_X_ms(1000);
    LED_SetAll(0);
    delay_X_ms(1000);
}







