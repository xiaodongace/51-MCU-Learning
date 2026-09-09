#include "TempAlarmLab.h"
#include "Config.h"
#include "Timers.h"
#include "NTC.h"
#include "LED.h"
#include "Buzzer.h"
#include "UARTS.h"

/* 温度达到阈值加2℃时进入报警，降低到阈值减1℃时解除报警。 */
#define TEMP_ALARM_THRESHOLD      30
#define TEMP_ALARM_ENTER_TEMP     (TEMP_ALARM_THRESHOLD + 2)
#define TEMP_ALARM_EXIT_TEMP      (TEMP_ALARM_THRESHOLD - 1)

/* 各任务的执行间隔，单位为毫秒。 */
#define TEMP_SAMPLE_INTERVAL      500
#define LED_FLASH_INTERVAL        500
#define BUZZER_OFF_INTERVAL       400
#define BUZZER_ON_INTERVAL        100

int current_temperature = 0; /* 当前温度。使用有符号类型以支持负温度。 */
u8 alarm_active = 0;          /* 当前是否处于报警状态。 */
u16 last_sample_ms = 0;       /* 上次采样时间。 */
u16 last_led_ms = 0;          /* 上次LED更新时间。 */
u16 last_buzzer_ms = 0;       /* 上次蜂鸣器更新时间。 */
u8 buzzer_active = 0;         /* 蜂鸣器当前是否正在响。 */
u8 led_on = 0;                /* LED当前是否点亮。 */

/*
 * 初始化温度报警实验需要使用的外设，并清除报警输出状态。
 */
void TempAlarmLab_Init() {
    /* 初始化系统时间、温度传感器、LED、蜂鸣器和串口。 */
    Timers_Init();
    NTC_Init();
    LED_Init();
    Buzzer_Init();
    UART_Init();

    /* 上电后先确保LED和蜂鸣器处于关闭状态。 */
    LED_SetAll(0);
    Buzzer_Stop();

    /* 清除本题使用的所有状态和时间记录。 */
    led_on = 0;
    alarm_active = 0;
    buzzer_active = 0;
    current_temperature = 0;
    last_sample_ms = 0;
    last_led_ms = 0;
    last_buzzer_ms = 0;
}

/*
 * 按固定周期读取NTC温度，并使用滞回条件更新报警状态。
 *
 * 温度达到32℃时进入报警，温度降低到29℃时解除报警。
 * 29℃到32℃之间不改变原来的状态，避免温度在临界值附近反复切换。
 */
void TempAlarmLab_SampleTask() {
    u16 now = Timers_GetSystemMs();

    /* 未达到采样周期时立即返回，避免频繁读取NTC。 */
    if (now - last_sample_ms < TEMP_SAMPLE_INTERVAL) {
        return;
    }

    last_sample_ms = now;
    current_temperature = NTC_GetTemperature();

    /* 正常状态下，只有温度达到进入报警温度才进入报警。 */
    if (alarm_active == 0) {
        if (current_temperature >= TEMP_ALARM_ENTER_TEMP) {
            alarm_active = 1;

            /* 每次重新进入报警时，从LED熄灭、蜂鸣器停止开始新的节拍。 */
            led_on = 0;
            buzzer_active = 0;
            last_led_ms = now;
            last_buzzer_ms = now;
        }
    }
    /* 报警状态下，只有温度降低到解除报警温度才恢复正常。 */
    else {
        if (current_temperature <= TEMP_ALARM_EXIT_TEMP) {
            alarm_active = 0;
        }
    }

    /* 每次采样都输出温度、报警状态和设定阈值，方便串口观察。 */
    printf("TEMP=%d ALARM=%d LIMIT=%d\r\n",
           current_temperature,
           (int)alarm_active,
           TEMP_ALARM_THRESHOLD);
}

/*
 * 根据alarm_active执行非阻塞报警动作。
 *
 * 报警时LED每500ms翻转一次，蜂鸣器响100ms、停400ms。
 * 函数每次只做当前到期的动作，不使用长时间延时，因此不会阻塞主循环。
 */
void TempAlarmLab_AlarmTask() {
    u16 now = Timers_GetSystemMs();

    if (alarm_active == 1) {
        /* LED到达更新时间后翻转一次，实现闪烁效果。 */
        if (now - last_led_ms >= LED_FLASH_INTERVAL) {
            last_led_ms = now;
            led_on = !led_on;
            LED_SetAll(led_on);
        }

        /* 蜂鸣器停止够400ms后开始响。 */
        if (buzzer_active == 0 && now - last_buzzer_ms >= BUZZER_OFF_INTERVAL) {
            last_buzzer_ms = now;
            buzzer_active = 1;
            Buzzer_Play(1000);
        }
        /* 蜂鸣器响够100ms后停止。 */
        else if (buzzer_active == 1 && now - last_buzzer_ms >= BUZZER_ON_INTERVAL) {
            Buzzer_Stop();
            buzzer_active = 0;
            last_buzzer_ms = now;
        }
    }
    else {
        /* 非报警状态下，确保LED和蜂鸣器都关闭。 */
        if (led_on != 0) {
            led_on = 0;
            LED_SetAll(DISABLE);
        }

        if (buzzer_active != 0) {
            buzzer_active = 0;
            Buzzer_Stop();
        }
    }
}
