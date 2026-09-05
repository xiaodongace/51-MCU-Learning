#include "LedCycleLab.h"
#include "Delay.h"
#include "Key.h"
#include "LED.h"

#define LED_COUNT 8       /* 本练习控制 8 盏 LED，合法编号为 0~7。 */
#define NEXT_LED_KEY 0    /* 使用 Key 模块中的 KEY1 作为“切换下一盏”按键。 */

/*
 * 当前显示的 LED 逻辑编号。
 *
 * 必须是 static：Task() 每次返回后，局部变量会重新创建；static 变量则
 * 在整个程序运行期间保留数值，才能记住上一次点亮的是哪一盏 LED。
 */
static u8 current_led;

/*
 * 按键锁定标志：0 表示已松开、允许下一次按下；非零表示当前这一次按下
 * 已经处理，必须等待按键稳定松开后才能再次触发。
 *
 * 它是“按住不连跳”的关键。若没有该变量，主循环运行很快，按住按键的
 * 几十毫秒内会多次调用 Task()，LED 将连续跳过多盏。
 */
static u8 key_locked;

/*
 * 初始化所依赖的模块与状态。
 *
 * LED_ShowOnly(0) 让上电现象明确：先显示第 1 盏灯。随后读取一次 KEY1，
 * 若用户正按住按键上电，就先锁定它，必须松开后才接受新的按下动作。
 */
void LedCycleLab_Init(void) {
    LED_Init();
    Key_Init();

    current_led = 0;
    LED_ShowOnly(current_led);

    /* 若上电时按键已被按住，必须先松开，不能立即切灯。 */
    key_locked = Key_IsPressed(NEXT_LED_KEY);
}

/*
 * 处理一次按键扫描与 LED 更新。
 *
 * 按下流程：检测到 KEY1 为按下且未锁定 -> 等待 10 ms -> 再读一次确认。
 * 第二次仍为按下时，才认为这是有效按键；编号加一并在 7 后回到 0。
 *
 * 松开流程：已经锁定时，检测到 KEY1 松开 -> 等待 10 ms -> 再次确认松开，
 * 然后解锁。下一次按下才能再切换 LED。
 *
 * 这是最基础的阻塞式消抖写法。后续学习定时器后，可改为不使用
 * delay_ms() 的非阻塞式状态机，但 key_locked 的思路仍然适用。
 */
void LedCycleLab_Task(void) {
    if (Key_IsPressed(NEXT_LED_KEY)) {
        if (key_locked == 0) {
            delay_ms(10); /* 首次检测后等待，避开机械触点抖动。 */

            if (Key_IsPressed(NEXT_LED_KEY)) {
                current_led++;

                if (current_led >= LED_COUNT) {
                    current_led = 0; /* LED8 的下一次切换回到 LED1。 */
                }

                LED_ShowOnly(current_led);
                key_locked = 1; /* 本次按下已消费，按住期间不重复切换。 */
            }
        }
    }
    else if (key_locked != 0) {
        delay_ms(10); /* 对“松开”同样确认一次，避免抖动导致提前解锁。 */

        if (!Key_IsPressed(NEXT_LED_KEY)) {
            key_locked = 0; /* 已稳定松开，允许下一次按下。 */
        }
    }
}
