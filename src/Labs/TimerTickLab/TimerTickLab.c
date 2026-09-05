#include "TimerTickLab.h"
#include "Timers.h"
#include "LED.h"
#include "Buzzer.h"

static u16 led_last_ms;    // led上次执行时间
static u8 led_state;   // led状态

static u16 buzzer_last_ms;  // 上一次开始蜂鸣的时间
static u16 buzzer_start_ms;  // 当前蜂鸣开始的时间
static u8 buzzer_active;    // 当前是否正在蜂鸣

void TimerTickLab_Init(void) {
    EAXSFR();		/* 扩展寄存器访问使能 */
    EA = 1;

    led_last_ms = 0;
    led_state = 0;

    buzzer_last_ms = 0;
    buzzer_start_ms = 0;
    buzzer_active = 0;

    LED_Init();
    LED_SetAll(0);
    Buzzer_Init();
    Buzzer_Stop();
    Timers_Init();
}


void TimerTickLab_Task(void) {
    u16 system_ms = Timers_GetSystemMs();

    if (system_ms - led_last_ms >= 300) {
        led_last_ms = system_ms;
        led_state = !led_state;
        LED_SetAll(led_state);
    }

    if (system_ms - buzzer_last_ms >= 700 && buzzer_active == 0) {
        Buzzer_Beep(1);
        buzzer_start_ms = system_ms;
        buzzer_active = 1;
    }

    if (buzzer_active == 1 && system_ms - buzzer_start_ms >= 100) {
        Buzzer_Stop();
        buzzer_active = 0;
        buzzer_last_ms = system_ms;
    }
}


/*
* **要求**：

- 蜂鸣器停止时等待 700 ms 后启动。
- 每次启动蜂鸣器时记录 `buzzer_start_ms`。
- 蜂鸣器持续 100 ms 后停止，并把 `buzzer_active` 置 0。
- 700 ms 的启动判断和 100 ms 的停止判断必须是两个独立的 `if`。
- 初始状态必须为停止，首次动作必须等待完整的 700 ms。

**验收标准**：蜂鸣器呈现“等待 700 ms、响 100 ms、继续等待”的周期效果。
 */
void New_TimerTick() {
    u16 now = Timers_GetSystemMs();

    if (now - led_last_ms >= 300) {
        led_last_ms = now;
        led_state = !led_state;
        LED_SetAll(led_state);
    }

    if (now - buzzer_last_ms >= 700 && buzzer_active == 0) {
        Buzzer_Beep(1);
        buzzer_start_ms = now;
        buzzer_active = 1;
    }

    if (now - buzzer_start_ms >= 100 && buzzer_active == 1) {
        Buzzer_Stop();
        buzzer_active = 0;
        buzzer_last_ms = now;
    }
}




