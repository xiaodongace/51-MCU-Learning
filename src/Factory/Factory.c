#include "Factory.h"
#include "Delay.h"
#include "LED.h"
#include "NTC.h"

#define FACTORY_TASK_PERIOD_MS 500
#define FACTORY_ALARM_DELTA     2
#define FACTORY_NAME            "dongcong"

static int xdata baseline_temperature;
static u16 xdata task_elapsed_ms;
static u8 xdata alarm_led_state;
static int xdata current_temperature;

/* 初始化温度监控、LED报警，并记录开机时的环境温度。 */
void Factory_Init(void) {
    NTC_Init();                         // 初始化NTC温度采集模块。
    LED_Init();                         // 初始化8个LED的GPIO。

    delay_ms(20);                       // 等待传感器和ADC稳定，避免开机瞬态影响基准值。
    baseline_temperature = NTC_GetTemperature(); // 保存开机温度，作为报警比较基准。
    task_elapsed_ms = 0;                // 清零任务计时器。
    alarm_led_state = 0;                // 报警灯初始为熄灭状态。
    current_temperature = baseline_temperature; // 初始化当前温度变量。
    LED_SetAll(0);                      // 确保所有LED上电后保持熄灭。
}

/* 每10ms调用一次；累计到500ms后打印温度并更新报警状态。 */
void Factory_Task(void) {
    task_elapsed_ms += 10;              // 这个函数约每10ms执行一次 因此累加10ms。
    if (task_elapsed_ms < FACTORY_TASK_PERIOD_MS) {
        return;                         // 未到500ms时不采样，减少ADC和串口占用。
    }
    task_elapsed_ms = 0;                // 到达周期后重新开始计时。

    current_temperature = NTC_GetTemperature(); // 读取当前环境温度。
    printf(FACTORY_NAME ":%d\r\n", current_temperature); // 按“姓名:温度”格式发送串口消息。

    if (current_temperature > (baseline_temperature + FACTORY_ALARM_DELTA)) {
        alarm_led_state = !alarm_led_state; // 每个周期翻转一次，形成500ms闪烁效果。
        LED_SetAll(alarm_led_state);       // 让8个LED同时亮或同时灭。
    }
    else {
        alarm_led_state = 0;              // 温度恢复后清除报警状态。
        LED_SetAll(0);                    // 关闭全部LED，停止报警。
    }
}
