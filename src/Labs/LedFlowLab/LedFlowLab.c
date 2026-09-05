#include "LedFlowLab.h"

#include "Delay.h"
#include "Key.h"
#include "LED.h"

/*
 * 本练习的需求：
 *
 * 1. 8 盏 LED 中始终只点亮一盏。
 * 2. LED 每隔约 200 ms 移动一格。
 * 3. KEY1 每按下一次，改变 LED 的移动方向。
 * 4. KEY2 每按下一次，在“运行”和“暂停”之间切换。
 * 5. 按住按键时只能触发一次，必须松开后才能再次触发。
 *
 * 当前版本使用 delay_ms(200) 完成最容易理解的“定时”效果。
 * 这属于阻塞式入门版本：LED 移动期间，按键响应可能会稍慢。学习定时器
 * 后，再把这段延时改成基于 1 ms 系统节拍的非阻塞任务。
 */

#define LED_COUNT       8   /* LED 的逻辑编号范围是 0~7。 */
#define DIRECTION_KEY   0   /* KEY1：切换正向/反向。 */
#define PAUSE_KEY       1   /* KEY2：切换运行/暂停。 */

/*
 * 当前 LED 的逻辑编号，而不是芯片的实际引脚编号。
 *
 * 例如 current_led == 0 表示 LED1，current_led == 7 表示 LED8。
 * LED1~LED8 实际分布在不同端口，具体引脚映射由 LED_ShowOnly() 隐藏在
 * LED 模块内部，因此本练习只处理 0~7 这样的逻辑编号。
 */
static u8 current_led;

/*
 * 移动方向标志：
 *     0：编号递减，LED1 -> LED8 的反向移动；
 *     1：编号递增，LED1 -> LED8 的正向移动。
 */
static u8 direction;

/*
 * 暂停标志：
 *     0：流水灯正在运行；
 *     1：保持当前 LED，不再自动移动。
 */
static u8 paused;

/*
 * 两个按键各自使用一个锁定标志，不能共用一个变量。
 *
 * key_locked[0] 对应 KEY1，key_locked[1] 对应 KEY2。
 * 标志为 0 表示按键已经松开，可以接受下一次按下；标志为 1 表示本次
 * 按下已经处理过，即使按键仍然保持低电平，也不能重复触发。
 */
static u8 key_locked[2];

/*
 * 检测某个按键是否产生了“一次有效按下”事件。
 *
 * 与直接判断 Key_IsPressed() 不同，本函数不会在按住按键期间连续返回 1。
 * 它把一个完整按键动作拆成两个阶段：
 *
 * 1. 按下阶段：发现按下后等待 10 ms，再次确认仍然按下；确认成功后返回 1
 *    并锁定该按键。
 * 2. 松开阶段：发现按键松开后等待 10 ms，再次确认已经松开；确认成功后
 *    解锁该按键，允许下一次按下触发。
 *
 * 这里使用的是适合入门的阻塞式消抖。函数不理解 KEY1 是“方向键”还是
 * KEY2 是“暂停键”，它只负责把物理电平转换为一次性的按键事件，具体业务
 * 由调用者决定。
 */
static u8 LedFlowLab_KeyPressedOnce(u8 key_index)
{
    if (Key_IsPressed(key_index)) {
        /* 当前为按下，且这次按下还没有被处理。 */
        if (key_locked[key_index] == 0) {
            delay_ms(10); /* 等待机械触点抖动结束。 */

            if (Key_IsPressed(key_index)) {
                key_locked[key_index] = 1;
                return 1; /* 只在确认成功的这一刻产生一次事件。 */
            }
        }
    } else if (key_locked[key_index] != 0) {
        /* 当前为松开，只有稳定松开后才解除锁定。 */
        delay_ms(10);

        if (!Key_IsPressed(key_index)) {
            key_locked[key_index] = 0;
        }
    }

    return 0; /* 当前没有新的有效按下事件。 */
}

/*
 * 让 LED 移动一格，并处理两端边界。
 *
 * 正向移动时，LED8 的下一盏是 LED1；反向移动时，LED1 的下一盏是 LED8。
 * 先修改逻辑编号，再调用 LED_ShowOnly()，由 LED 模块负责“先全灭、再点亮
 * 目标灯”的实际 GPIO 操作。
 */
static void LedFlowLab_MoveOneStep(void)
{
    if (direction != 0) {
        current_led++;

        if (current_led >= LED_COUNT) {
            current_led = 0; /* 正向越过 LED8 后回到 LED1。 */
        }
    } else {
        if (current_led == 0) {
            current_led = LED_COUNT - 1; /* 反向越过 LED1 后回到 LED8。 */
        } else {
            current_led--;
        }
    }

    LED_ShowOnly(current_led);
}

/*
 * 初始化流水灯练习。
 *
 * 初始化函数只调用一次，不能放在 Task() 中；否则每次扫描都会把
 * current_led、direction 和 paused 重新设置，状态永远无法保留下来。
 */
void LedFlow_Init(void)
{
    LED_Init();
    Key_Init();

    current_led = 0; /* 上电从 LED1 开始。 */
    direction = 1;   /* 默认采用编号递增方向。 */
    paused = 0;      /* 默认自动运行。 */

    key_locked[0] = 0;
    key_locked[1] = 0;

    LED_ShowOnly(current_led);
}

/*
 * 执行一次流水灯任务。
 *
 * main() 只需要在 while(1) 中不断调用本函数。每次调用依次完成：
 *
 * 1. 检查 KEY1，必要时翻转 direction；
 * 2. 检查 KEY2，必要时翻转 paused；
 * 3. 若没有暂停，等待 200 ms 并移动一格；
 * 4. 若处于暂停状态，立即返回并保持当前 LED。
 */
void LED_Flow_Task(void)
{
    /* KEY1 是边沿事件：每次完整按下只反转一次方向。 */
    if (LedFlowLab_KeyPressedOnce(DIRECTION_KEY)) {
        direction = !direction;
    }

    /* KEY2 也是边沿事件：每次完整按下只切换一次暂停状态。 */
    if (LedFlowLab_KeyPressedOnce(PAUSE_KEY)) {
        paused = !paused;
    }

    /* 暂停时不改变 current_led，也不等待下一步。 */
    if (paused != 0) {
        return;
    }

    /* 入门版本用阻塞延时形成约 200 ms 的步进周期。 */
    delay_ms(200);
    LedFlowLab_MoveOneStep();
}
