#include "LedFlipLab.h"

#include "Key.h"
#include "LED.h"
#include "Timers.h"

/*
 * 由本实验保存的 LED“逻辑状态”。
 * 0 表示所有 LED 应熄灭，1 表示所有 LED 应点亮。
 * 它不是从硬件读取出来的值，而是下一次翻转时所依据的记录。
 */
static u8 led_state = 0;

/*
 * 初始化LED翻转实验使用的外设、公共按键事件状态和1 ms系统时间。
 * 按键消抖的内部数据统一由Key模块管理，本Lab只消费按下事件。
 */
void LedFlipLab_Init(void) {
    /* 先配置 LED、按键对应的硬件引脚。 */
    LED_Init();
    Key_Init();

    /* 启动1 ms定时器，之后Key_Scan()可以执行10 ms非阻塞消抖。 */
    Timers_Init();

    /* 上电后建立全部LED熄灭的确定输出状态。 */
    led_state = 0;
    LED_SetAll(led_state);
}


/*
* **目标**：练习按键边沿检测和“按住不重复触发”。

**要求**：

- 每按下一次 KEY1，只翻转一次 LED 全亮/全灭状态。
- 按住 KEY1 不允许持续翻转。
- 必须保存上一次按键状态。
- 只有检测到“松开 → 按下”变化时才执行翻转。
- 松开后再次按下，才能产生下一次动作。

**验收标准**：快速观察时 LED 不会因为按住按键而反复闪烁；每次完整按下动作只触发一次。
 */
void LedFlipLab_Task(void) {
    /* 公共Key模块扫描所有按键并生成经过10 ms消抖的按下事件。 */
    Key_Scan();

    /* 读取时自动消费事件；KEY1每次有效按下只翻转一次LED状态。 */
    if (Key_GetPressEvent(0) != 0) {
        /* 0 变 1、1 变 0：KEY1 每次有效按下都让所有 LED 翻转。 */
        led_state = !led_state;
        LED_SetAll(led_state);
    }

    /* KEY2的有效按下事件用于恢复到全部熄灭的确定状态。 */
    if (Key_GetPressEvent(1) != 0) {
        led_state = 0;
        LED_SetAll(led_state);
    }
}
