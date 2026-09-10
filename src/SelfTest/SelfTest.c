#include "SelfTest.h"
#include "Key.h"
#include "Timers.h"
#include "SpiOled.h"
#include "IIC.h"
#include "I2C.h"
#include "LED.h"
#include "NTC.h"
#include "ADCS.h"
#include "Motor.h"
#include "Nixie.h"
#include "DHT11.h"
#include "Buzzer.h"
#include "oled.h"

/* SPI 菜单的七个自检项目。编号与题目中的菜单编号一致。 */
#define SELFTEST_ITEM_LED        0
#define SELFTEST_ITEM_NTC        1
#define SELFTEST_ITEM_MOTOR      2
#define SELFTEST_ITEM_RTC        3
#define SELFTEST_ITEM_NIXIE      4
#define SELFTEST_ITEM_DHT11      5
#define SELFTEST_ITEM_KEY_BUZZER 6
#define SELFTEST_ITEM_COUNT      7

/* 两种界面状态：浏览菜单，或查看选中项目的详情页。 */
#define SELFTEST_PAGE_MENU       0
#define SELFTEST_PAGE_DETAIL     1

/* Key 模块索引：索引与板子上的 KEY1 至 KEY4 一一对应。 */
#define SELFTEST_KEY_NEXT        0
#define SELFTEST_KEY_PREVIOUS    1
#define SELFTEST_KEY_ENTER       2
#define SELFTEST_KEY_BACK        3

/* RTX Timer0 每个节拍约 5 ms；下列常量同时约束界面刷新和外设节拍。 */
#define SELFTEST_REFRESH_TICKS     100  /* 约 0.5 秒：LED、NTC、马达、RTC、数码管 */
#define SELFTEST_DHT_REFRESH_TICKS 200  /* 约 1 秒：满足 DHT11 两次读取的最小间隔 */
#define SELFTEST_BEEP_TICKS        30   /* 约 150 ms：一次独立按键提示音 */

/* 板载电位器在 P0.5 的实测满量程约为 2660，不是 ADC 理论值 4095。 */
#define SELFTEST_POT_ADC_FULL_SCALE 2660U

/* task_1 把按下的蜂鸣器按键转换为音调编号，task_2 实际播放。 */
#define SELFTEST_BUZZER_NONE 0
#define SELFTEST_BUZZER_NOTE_C N1
#define SELFTEST_BUZZER_NOTE_D N2
#define SELFTEST_BUZZER_NOTE_E N3

/*
 * 本次烧录使用的本机时间（Asia/Shanghai）：2026-09-07 18:04:15，星期一。
 * PCF8563 的 Timer Counter（0x0F）在本工程未使用，借其保存本固件的校时标记。
 * 标记存在时普通复位不会回写该编译时间，RTC 将继续依靠后备电源自行走时。
 */
#define SELFTEST_RTC_SYNC_MARKER_ADDR 0x0F
#define SELFTEST_RTC_SYNC_MARKER      0x1F

/* task_1 写入，task_2 读取；u8 读写在 8051 上是原子的。 */
static volatile u8 selected_item = SELFTEST_ITEM_LED;
static volatile u8 current_page = SELFTEST_PAGE_MENU;
static bit led_test_active = 0;
/* 当前累加点亮的 LED 数量，范围为 0~8。 */
static u8 led_count = 0;
static bit ntc_test_active = 0;
static bit motor_test_active = 0;
static bit nixie_test_active = 0;
static bit dht_test_active = 0;
static bit buzzer_test_active = 0;
/* 数码管闭环跑马灯位置：顶部向右、右侧向下、底部向左、左侧向上。 */
static u8 nixie_position = 0;
static volatile u8 buzzer_note_request = SELFTEST_BUZZER_NONE;

/*
 * 把当前 SPI 菜单索引转换成 I2C OLED 上的英文项目名。
 * 该函数只写第 2 行，不清屏，也不修改 selected_item；菜单页和兜底详情页都复用它。
 * I2C OLED 使用 16 像素高字体，因此可用的行坐标只有 0、2、4、6。
 */
static void SelfTest_ShowItemName(void)
{
    switch (selected_item) {
    case SELFTEST_ITEM_LED:
        OLED_ShowString(0, 2, "LED", 16);
        break;
    case SELFTEST_ITEM_NTC:
        OLED_ShowString(0, 2, "NTC", 16);
        break;
    case SELFTEST_ITEM_MOTOR:
        OLED_ShowString(0, 2, "POT+MOTOR", 16);
        break;
    case SELFTEST_ITEM_RTC:
        OLED_ShowString(0, 2, "RTC", 16);
        break;
    case SELFTEST_ITEM_NIXIE:
        OLED_ShowString(0, 2, "NIXIE", 16);
        break;
    case SELFTEST_ITEM_DHT11:
        OLED_ShowString(0, 2, "DHT11", 16);
        break;
    default:
        OLED_ShowString(0, 2, "KEY+BUZZER", 16);
        break;
    }
}

/*
 * 绘制 I2C OLED 的菜单页完整静态内容。
 * 调用前无需自行清屏；本函数会清屏，再显示当前项目名及 K1~K3 的操作提示。
 * SPI OLED 的中文菜单由 SpiOled_ShowMenu() 负责，本函数只负责辅助详情屏。
 */
static void SelfTest_DrawMenuPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "SELF TEST", 16);
    SelfTest_ShowItemName();
    OLED_ShowString(0, 4, "K1/K2:MOVE", 16);
    OLED_ShowString(0, 6, "K3:ENTER", 16);
}

/*
 * 刷新 LED 详情页的 CNT 行。
 * led_count 是当前实际点亮数量，范围为 0~8；本函数只显示计数，不改变灯状态。
 * 只覆盖这一行而不清屏，避免定时刷新时产生可见闪烁。
 */
static void SelfTest_ShowLedCount(void)
{
    char text[16];

    sprintf(text, "CNT: %d        ", (int)led_count);
    OLED_ShowString(0, 2, text, 16);
}

/*
 * 在首次进入 LED 详情页时准备 GPIO 与运行状态。
 * 重复调用不会重新初始化，防止 task_2 每次刷新页面都让流水灯回到起点。
 * LED_SetAll(0) 明确关闭所有 LED，保证第一个节拍由 LED1 开始。
 */
static void SelfTest_StartLedTest(void)
{
    if (led_test_active == 0) {
        LED_Init();
        LED_SetAll(0);
        led_count = 0;
        led_test_active = 1;
    }
}

/*
 * 离开 LED 项目时执行的收尾操作。
 * 关闭全部 LED、复位流水位置并清除活动标志；因此切换菜单或进入其他项目不会残留灯光。
 */
static void SelfTest_StopLedTest(void)
{
    if (led_test_active != 0) {
        LED_SetAll(0);
        led_count = 0;
        led_test_active = 0;
    }
}

/*
 * 首次进入 NTC 页面时初始化温度采样所需的 ADC。
 * ntc_test_active 仅表示本页已完成初始化；实际温度读取由 task_2 周期调用
 * SelfTest_ShowNtcTemperature() 完成。
 */
static void SelfTest_StartNtcTest(void)
{
    if (ntc_test_active == 0) {
        NTC_Init();
        ntc_test_active = 1;
    }
}

/*
 * 标记 NTC 页面已经退出。
 * ADC 无需关闭，因为它是可被其他模拟量功能重新初始化的共享外设；停止周期读取即可。
 */
static void SelfTest_StopNtcTest(void)
{
    ntc_test_active = 0;
}

/*
 * 首次进入电位器+马达页面时配置 ADC13/P0.5，并把马达 PWM 先设为 0%。
 * 这样不会把上一个项目留下的 PWM 占空比带入本页面；后续占空比由周期采样更新。
 * motor_test_active 防止刷新详情页时重复初始化 ADC。
 */
static void SelfTest_StartMotorTest(void)
{
    if (motor_test_active == 0) {
        ADC_InitChannel(ADC_CHANNEL_P05);
        Motor_SetSpeed(0);
        motor_test_active = 1;
    }
}

/*
 * 离开电位器+马达页面时强制停止马达。
 * 这是安全收尾：无论用户从菜单切换到哪里，PWM6 都不会继续以最后一次占空比运转。
 */
static void SelfTest_StopMotorTest(void)
{
    if (motor_test_active != 0) {
        Motor_SetSpeed(0);
        motor_test_active = 0;
    }
}

/*
 * 首次进入数码管页面时初始化串行驱动，并清屏后从闭环路径的顶部第 1 位开始。
 * 后续的 20 个路径位置由 task_2 递增；本函数仅负责“进入页面”的一次性状态建立。
 */
static void SelfTest_StartNixieTest(void)
{
    if (nixie_test_active == 0) {
        Nixie_Init();
        Nixie_Clear();
        nixie_position = 0;
        nixie_test_active = 1;
    }
}

/*
 * 离开数码管页面时关掉段选和位选。
 * 这样数码管不会在进入其他项目后继续显示最后一个跑马灯段。
 */
static void SelfTest_StopNixieTest(void)
{
    if (nixie_test_active != 0) {
        Nixie_Clear();
        nixie_test_active = 0;
    }
}

/*
 * 首次进入温湿度页面时把 P4.6 配置为 DHT11 单总线的上拉输入/输出模式。
 * DHT11 无专用外设需要关闭；退出时只停止 task_2 的周期读取即可。
 */
static void SelfTest_StartDhtTest(void)
{
    if (dht_test_active == 0) {
        DHT11_Init();
        dht_test_active = 1;
    }
}

/*
 * 标记温湿度页面已经退出。
 * 该函数不改变 P4.6 电平，避免在传感器总线空闲时人为输出额外脉冲。
 */
static void SelfTest_StopDhtTest(void)
{
    dht_test_active = 0;
}

/*
 * 首次进入键盘蜂鸣器页面时初始化 PWM5 并确保蜂鸣器静音。
 * 蜂鸣器与马达同属 PWMB，因此详情页切换时必须先停止马达；音调请求由 task_1 写入，
 * 实际 PWM 操作仍由 task_2 执行，避免两个任务同时改硬件寄存器。
 */
static void SelfTest_StartBuzzerTest(void)
{
    if (buzzer_test_active == 0) {
        Buzzer_Init();
        Buzzer_Stop();
        buzzer_note_request = SELFTEST_BUZZER_NONE;
        buzzer_test_active = 1;
    }
}

/*
 * 离开蜂鸣器页面时停止 PWM 输出，并丢弃尚未播放的按键音请求。
 * 这样按住或刚松开按键后再切换页面，不会在其他项目中突然响铃。
 */
static void SelfTest_StopBuzzerTest(void)
{
    if (buzzer_test_active != 0) {
        Buzzer_Stop();
        buzzer_note_request = SELFTEST_BUZZER_NONE;
        buzzer_test_active = 0;
    }
}

/*
 * 判断 task_2 在一次 os_wait2() 返回后，用户是否仍停留在指定详情页。
 * 参数 item 是 SELFTEST_ITEM_* 编号；返回 1 表示可继续刷新该项目，0 表示应重绘新页面。
 * 该检查防止按键切换页面后仍用旧项目的数据覆盖新界面。
 */
static bit SelfTest_IsCurrentDetail(u8 item)
{
    return ((current_page == SELFTEST_PAGE_DETAIL) && (selected_item == item));
}

/*
 * 绘制 LED 自检页的静态骨架。
 * 动态的 CNT 值由 SelfTest_ShowLedCount() 更新，实际 LED 点亮由 task_2 调用 LED_ShowOnly() 完成。
 */
static void SelfTest_DrawLedPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "LED", 16);
    OLED_ShowString(0, 2, "CNT: 0", 16);
    OLED_ShowString(0, 4, "FLOWING", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 读取一次 NTC 模块的整数摄氏度，并只覆盖 I2C OLED 的温度数据行。
 * NTC_GetTemperature() 内部负责 ADC 换算；本函数不重新初始化 NTC，也不清屏。
 * %3d 固定数值宽度，能避免温度位数减少时保留旧字符。
 */
static void SelfTest_ShowNtcTemperature(void)
{
    char text[16];
    int temperature = NTC_GetTemperature();

    /* %3d 始终占用三格，避免数值位数变少时遗留旧字符。 */
    sprintf(text, "TEMP:%3dC", temperature);
    /* 单次覆盖写入，不能先写一行空格，否则会形成可见的空白闪烁。 */
    OLED_ShowString(0, 2, text, 16);
}

/*
 * 绘制 NTC 详情页的静态内容和默认占位符。
 * 进入页面后的下一次 task_2 刷新会以真实 TEMP 值覆盖第 2 行。
 */
static void SelfTest_DrawNtcPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "NTC", 16);
    OLED_ShowString(0, 2, "TEMP:--C", 16);
    OLED_ShowString(0, 4, "UPD: 0.5S", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 读取 P0.5 电位器、按实测满量程映射为 0~100%，并同时更新马达 PWM 和 I2C 屏。
 * 原始 ADC 值只用于换算，不显示给用户；超过实测上限的值被钳位为 100%。
 * 本函数由 task_2 定时调用，始终只覆盖 DUTY 行，因而不会产生整屏闪烁。
 */
static void SelfTest_ShowMotorValues(void)
{
    char text[16];
    u16 adc_value = ADC_Read(ADC_CHANNEL_P05);
    u8 speed_percent;

    /*
     * 按板载电位器的实际 0~2660 量程映射为 0~100%。
     * 读数略高于实测上限时直接钳位，避免超过 100% 或马达接口收到非法值。
     */
    if (adc_value >= SELFTEST_POT_ADC_FULL_SCALE) {
        speed_percent = 100;
    }
    else {
        speed_percent = (u8)(((u32)adc_value * 100UL) /
                             SELFTEST_POT_ADC_FULL_SCALE);
    }

    Motor_SetSpeed(speed_percent);
    /*
     * C51 的 sprintf 对 u8 变参取值不可靠，先提升为 int 后使用 %d。
     * 末尾补齐到固定 15 个字符，数值由 100 变回 0 时也不会残留旧数字或 %。
     */
    sprintf(text, "DUTY:%3d%%      ", (int)speed_percent);
    OLED_ShowString(0, 2, text, 16);
}

/*
 * 绘制电位器+马达详情页的静态框架。
 * 第 2 行先显示占位符，随后 SelfTest_ShowMotorValues() 将其替换为实时 DUTY 百分比。
 */
static void SelfTest_DrawMotorPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "POT+MOTOR", 16);
    OLED_ShowString(0, 2, "DUTY:---%", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 从 PCF8563 读取当前日期时间，并覆盖 RTC 详情页的日期、时间两行。
 * 本函数只读 RTC，绝不会校时；因此每次进入页面都能继续显示后备电池维持的真实走时。
 * 所有 u8 字段先转为 int，以适配 Keil C51 的可变参数格式化规则。
 */
static void SelfTest_ShowRtcTime(void)
{
    char date_text[16];
    char time_text[16];
    Clock_t clock;

    I2C_Get_Clock(&clock);
    /*
     * Keil C51 的变参函数不会像桌面编译器那样可靠地提升 u8。
     * 不先转为 int 时，month/day 可能从错误的栈位置取值而显示成几千。
     */
    sprintf(date_text, "%04d-%02d-%02d", (int)clock.year,
            (int)clock.month, (int)clock.day);
    sprintf(time_text, "%02d:%02d:%02d", (int)clock.hour,
            (int)clock.minute, (int)clock.second);
    OLED_ShowString(0, 2, date_text, 16);
    OLED_ShowString(0, 4, time_text, 16);
}

/*
 * 绘制 RTC 详情页的静态骨架与日期/时间占位符。
 * 真正的日期时间由下一次 SelfTest_ShowRtcTime() 调用覆盖。
 */
static void SelfTest_DrawRtcPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "RTC", 16);
    OLED_ShowString(0, 2, "---- -- --", 16);
    OLED_ShowString(0, 4, "--:--:--", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 仅在本固件第一次发现校时标记不匹配时，写入本机当前时间。
 * 不在 RTC 页面里调用本函数，避免用户每次进入页面都把秒数重置。
 */
static void SelfTest_SyncRtcOnFirstFirmwareStart(void)
{
    u8 marker;
    Clock_t clock;

    I2C_ReadNbyte(PCF8563_ADDR, SELFTEST_RTC_SYNC_MARKER_ADDR, &marker, 1);
    if (marker == SELFTEST_RTC_SYNC_MARKER) {
        return;
    }

    clock.year = 2026;
    clock.month = 9;
    clock.day = 7;
    clock.week = 1;   /* PCF8563: 0 为周日，1 为周一。 */
    clock.hour = 18;
    clock.minute = 4;
    clock.second = 15;
    I2C_Set_Clock(clock);

    /* 自检程序不使用 PCF8563 定时器，关闭它以保证校时标记不会被倒计时改写。 */
    I2C_Enable_Timer(DISABLE);
    marker = SELFTEST_RTC_SYNC_MARKER;
    I2C_WriteNbyte(PCF8563_ADDR, SELFTEST_RTC_SYNC_MARKER_ADDR, &marker, 1);
}

/*
 * 将 nixie_position 的当前闭环位置送入数码管驱动，并在 I2C OLED 显示路径状态。
 * 0~7 是顶部向右，8~9 是右侧向下，10~17 是底部向左，18~19 是左侧向上。
 * 本函数不递增位置；递增由 task_2 在一次显示完成后统一处理。
 */
static void SelfTest_ShowNixieRunningLight(void)
{
    char text[16];

    Nixie_ShowRunningLight(nixie_position);
    if (nixie_position < 8) {
        /* 所有文本补齐至 15 字符，直接覆盖旧内容，不先清屏，因此不闪烁。 */
        sprintf(text, "TOP: %d         ", (int)(nixie_position + 1));
    }
    else if (nixie_position < 10) {
        sprintf(text, "RIGHT: DOWN    ");
    }
    else if (nixie_position < 18) {
        sprintf(text, "BOTTOM: %d      ", (int)(18 - nixie_position));
    }
    else {
        sprintf(text, "LEFT: UP       ");
    }
    OLED_ShowString(0, 2, text, 16);
}

/*
 * 绘制数码管详情页的静态说明和初始路径状态。
 * 跑马灯实际输出由 SelfTest_ShowNixieRunningLight() 周期更新。
 */
static void SelfTest_DrawNixiePage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "NIXIE", 16);
    OLED_ShowString(0, 2, "TOP: 1", 16);
    OLED_ShowString(0, 4, "TOP-R-BOT-L", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 发起一次 DHT11 单总线采样，并在成功时覆盖温度、湿度两行。
 * getHumidityAndTemperature() 返回 SUCCESS(0) 才使用输出参数；任意通讯或校验错误时
 * 直接显示负错误码，避免使用未初始化的 humidity/temperature 数据，也便于定位失败阶段。
 * 该函数不含 UART printf，保证 DHT 失败时仍能回到 RTX 调度和响应 KEY4。
 */
static void SelfTest_ShowDhtData(void)
{
    char text[16];
    float humidity;
    float temperature;
    int8 result = getHumidityAndTemperature(&humidity, &temperature);

    if (result != SUCCESS) {
        /* 直接显示 -1/-2/-3，便于区分无响应、时序异常和校验失败。 */
        sprintf(text, "ERR:%d          ", (int)result);
        OLED_ShowString(0, 4, text, 16);
        return;
    }

    sprintf(text, "TEMP:%3dC", (int)temperature);
    OLED_ShowString(0, 2, text, 16);
    sprintf(text, "HUM:%3d%%       ", (int)humidity);
    OLED_ShowString(0, 4, text, 16);
}

/*
 * 绘制 DHT11 详情页的标题、两个数据占位符和返回提示。
 * 后续采样成功会覆盖 TEMP/HUM 行；失败则由 SelfTest_ShowDhtData() 显示 READ ERROR。
 */
static void SelfTest_DrawDhtPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "DHT11", 16);
    OLED_ShowString(0, 2, "TEMP:--C", 16);
    OLED_ShowString(0, 4, "HUM: --%", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 更新蜂鸣器页最后一次播放的音符提示。
 * 参数 note 使用 SELFTEST_BUZZER_NOTE_*；NONE 时保持横线，C/D/E 时只覆盖最后一个字符。
 * 声音本身不在本函数产生，PWM 输出由 task_2 的 Buzzer_Beep() 负责。
 */
static void SelfTest_ShowBuzzerNote(u8 note)
{
    OLED_ShowString(0, 6, "NOTE: - ", 16);
    if (note == SELFTEST_BUZZER_NOTE_C) {
        OLED_ShowString(48, 6, "C", 16);
    }
    else if (note == SELFTEST_BUZZER_NOTE_D) {
        OLED_ShowString(48, 6, "D", 16);
    }
    else if (note == SELFTEST_BUZZER_NOTE_E) {
        OLED_ShowString(48, 6, "E", 16);
    }
}

/*
 * 绘制键盘蜂鸣器页的按键-音符对照关系和初始“未播放”状态。
 * K4 的退出逻辑不在这里处理，而由 task_1 改变 current_page 后通知 task_2 重绘。
 */
static void SelfTest_DrawBuzzerPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "KEY+BUZZER", 16);
    OLED_ShowString(0, 2, "K1:C K2:D", 16);
    OLED_ShowString(0, 4, "K3:E K4:EXIT", 16);
    SelfTest_ShowBuzzerNote(SELFTEST_BUZZER_NONE);
}

/*
 * 根据 selected_item 建立一个详情页，并确保所有非当前项目的外设已经安全停止。
 * 每个分支只启动本项目需要的资源，再绘制对应静态页面；实时数据留给 task_2 主循环刷新。
 * 这是外设互斥的中心入口，尤其防止马达和蜂鸣器同时占用 PWMB。
 */
static void SelfTest_DrawDetailPage(void)
{
    if (selected_item == SELFTEST_ITEM_LED) {
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SelfTest_StartLedTest();
        SelfTest_DrawLedPage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_NTC) {
        SelfTest_StopLedTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SelfTest_StartNtcTest();
        SelfTest_DrawNtcPage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_MOTOR) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SelfTest_StartMotorTest();
        SelfTest_DrawMotorPage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_RTC) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SelfTest_DrawRtcPage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_NIXIE) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SelfTest_StartNixieTest();
        SelfTest_DrawNixiePage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_DHT11) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopBuzzerTest();
        SelfTest_StartDhtTest();
        SelfTest_DrawDhtPage();
        return;
    }

    if (selected_item == SELFTEST_ITEM_KEY_BUZZER) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StartBuzzerTest();
        SelfTest_DrawBuzzerPage();
        return;
    }

    /* 若从已实现项目切到其他详情页，确保旧项目不再运行。 */
    SelfTest_StopLedTest();
    SelfTest_StopNtcTest();
    SelfTest_StopMotorTest();
    SelfTest_StopNixieTest();
    SelfTest_StopDhtTest();
    SelfTest_StopBuzzerTest();
    OLED_Clear();
    OLED_ShowString(0, 0, "SELF TEST", 16);
    SelfTest_ShowItemName();
    OLED_ShowString(0, 4, "READY", 16);
    OLED_ShowString(0, 6, "K4:BACK", 16);
}

/*
 * 根据 current_page 刷新整套 UI。
 * task_2 是唯一的界面绘制者：菜单状态时停止所有自检外设并同时刷新 SPI/I2C 两块屏；
 * 详情状态时只进入 SelfTest_DrawDetailPage()，保留 SPI 菜单供用户查看当前选择。
 */
static void SelfTest_RefreshUi(void)
{
    if (current_page == SELFTEST_PAGE_MENU) {
        SelfTest_StopLedTest();
        SelfTest_StopNtcTest();
        SelfTest_StopMotorTest();
        SelfTest_StopNixieTest();
        SelfTest_StopDhtTest();
        SelfTest_StopBuzzerTest();
        SpiOled_ShowMenu(selected_item);
        SelfTest_DrawMenuPage();
    }
    else {
        SelfTest_DrawDetailPage();
    }
}

/*
 * 在 main_start() 创建 RTX 任务前完成一次全局基础初始化。
 * EAXSFR() 打开扩展 SFR 访问，Timers_Init() 建立按键扫描时基，Key_Init() 配置四个独立按键；
 * 最后开启总中断，使 RTX 定时节拍和按键相关中断能够运行。
 */
void SelfTest_Init(void)
{
    EAXSFR();
    Timers_Init();
    Key_Init();
    EA = 1;
}

/*
 * RTX task_1：以一个 RTX 节拍扫描四个独立按键，并把按键事件转换为状态变量和信号。
 * 菜单页：K1/K2 修改 selected_item，K3 进入详情；详情页：K4 返回菜单。
 * 蜂鸣器页额外把 K1/K2/K3 转为 C/D/E 音符请求。此任务绝不操作 OLED/PWM，
 * 以保证 task_2 独占显示和外设输出，避免并发写硬件。
 */
// void SelfTest_InputTask(void) RTX_TASK(1)
// {
//     while (1) {
//         Key_Scan();
//
//         if (current_page == SELFTEST_PAGE_MENU) {
//             if (Key_GetPressEvent(SELFTEST_KEY_NEXT) != 0) {
//                 selected_item++;
//                 if (selected_item >= SELFTEST_ITEM_COUNT) {
//                     selected_item = SELFTEST_ITEM_LED;
//                 }
//                 os_send_signal(2);
//             }
//             else if (Key_GetPressEvent(SELFTEST_KEY_PREVIOUS) != 0) {
//                 if (selected_item == SELFTEST_ITEM_LED) {
//                     selected_item = SELFTEST_ITEM_KEY_BUZZER;
//                 }
//                 else {
//                     selected_item--;
//                 }
//                 os_send_signal(2);
//             }
//             else if (Key_GetPressEvent(SELFTEST_KEY_ENTER) != 0) {
//                 current_page = SELFTEST_PAGE_DETAIL;
//                 os_send_signal(2);
//             }
//             else {
//                 /* 菜单页忽略 KEY4，仍消费事件，避免它在进入详情页后误触发返回。 */
//                 Key_GetPressEvent(SELFTEST_KEY_BACK);
//             }
//         }
//         else {
//             /*
//              * 键盘蜂鸣器项目把 KEY1~KEY3 转换成音调请求。
//              * task_1 不直接操作 PWM，保证外设输出仍由 task_2 独占。
//              */
//             if (selected_item == SELFTEST_ITEM_KEY_BUZZER) {
//                 if (Key_GetPressEvent(SELFTEST_KEY_NEXT) != 0) {
//                     buzzer_note_request = SELFTEST_BUZZER_NOTE_C;
//                     os_send_signal(2);
//                 }
//                 else if (Key_GetPressEvent(SELFTEST_KEY_PREVIOUS) != 0) {
//                     buzzer_note_request = SELFTEST_BUZZER_NOTE_D;
//                     os_send_signal(2);
//                 }
//                 else if (Key_GetPressEvent(SELFTEST_KEY_ENTER) != 0) {
//                     buzzer_note_request = SELFTEST_BUZZER_NOTE_E;
//                     os_send_signal(2);
//                 }
//                 else if (Key_GetPressEvent(SELFTEST_KEY_BACK) != 0) {
//                     current_page = SELFTEST_PAGE_MENU;
//                     os_send_signal(2);
//                 }
//             }
//             else if (Key_GetPressEvent(SELFTEST_KEY_BACK) != 0) {
//                 current_page = SELFTEST_PAGE_MENU;
//                 os_send_signal(2);
//             }
//             else {
//                 /* 其他详情页暂不使用 KEY1、KEY2、KEY3，先消费事件。 */
//                 Key_GetPressEvent(SELFTEST_KEY_NEXT);
//                 Key_GetPressEvent(SELFTEST_KEY_PREVIOUS);
//                 Key_GetPressEvent(SELFTEST_KEY_ENTER);
//             }
//         }
//
//         /* 约每个 RTX 节拍扫描一次；Key_Scan 内部仍以 10 ms 非阻塞消抖。 */
//         os_wait2(K_TMO, 1);
//     }
// }

/*
 * RTX task_2：系统的唯一显示与自检执行任务。
 * 启动阶段初始化 SPI OLED、硬件 I2C、RTC 与 I2C OLED，然后绘制菜单首帧；循环阶段根据
 * 当前详情项目进行周期采样/输出，并用 K_SIG 打断定时等待，从而让 K4 返回无需等完整刷新周期。
 */
// void SelfTest_ViewTask(void) RTX_TASK(2)
// {
//     /*
//      * 不依赖“任务刚创建时收到信号”的时序：任务 2 一开始就初始化并画首帧。
//      * 之后本任务才进入 os_wait1(K_SIG) 等待按键事件。
//      */
//     SpiOled_Init();
//
//     /*
//      * 现有 I2C OLED 的 OLED_WR_Byte() 最终调用 I2C_WriteNbyte()，
//      * 所以必须先启用硬件 I2C 并映射 P3.2/P3.3；RTC 之后也复用同一总线。
//      */
//     IIC_Init();
//     SelfTest_SyncRtcOnFirstFirmwareStart();
//     OLED_Init();
//     OLED_ColorTurn(0);
//     OLED_DisplayTurn(0);
//     SelfTest_RefreshUi();
//
//     while (1) {
//         if ((current_page == SELFTEST_PAGE_DETAIL) &&
//             (selected_item == SELFTEST_ITEM_LED)) {
//             /* 每个周期增加一盏：1→2→…→8→0，达到全亮后下一步全部熄灭。 */
//             if (led_count < 8) {
//                 led_count++;
//             }
//             else {
//                 led_count = 0;
//             }
//             LED_ShowCount(led_count);
//             SelfTest_ShowLedCount();
//             os_wait2(K_SIG | K_TMO, SELFTEST_REFRESH_TICKS);
//
//             /* 收到 KEY4 后立即熄灯并恢复菜单画面。 */
//             if ((current_page != SELFTEST_PAGE_DETAIL) ||
//                 (selected_item != SELFTEST_ITEM_LED)) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if ((current_page == SELFTEST_PAGE_DETAIL) &&
//                  (selected_item == SELFTEST_ITEM_NTC)) {
//             SelfTest_ShowNtcTemperature();
//             os_wait2(K_SIG | K_TMO, SELFTEST_REFRESH_TICKS);
//
//             if ((current_page != SELFTEST_PAGE_DETAIL) ||
//                 (selected_item != SELFTEST_ITEM_NTC)) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_MOTOR)) {
//             /* 读取电位器、更新 PWM 占空比并显示两者对应关系。 */
//             SelfTest_ShowMotorValues();
//             os_wait2(K_SIG | K_TMO, SELFTEST_REFRESH_TICKS);
//
//             if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_MOTOR) == 0) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_RTC)) {
//             /* RTC 每次只读当前时间寄存器，不会修改已保存的日期和时间。 */
//             SelfTest_ShowRtcTime();
//             os_wait2(K_SIG | K_TMO, SELFTEST_REFRESH_TICKS);
//
//             if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_RTC) == 0) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_NIXIE)) {
//             /* 顶部→右侧向下→底部→左侧向上，20 步组成完整一圈。 */
//             SelfTest_ShowNixieRunningLight();
//             nixie_position = (nixie_position + 1) % 20;
//             os_wait2(K_SIG | K_TMO, SELFTEST_REFRESH_TICKS);
//
//             if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_NIXIE) == 0) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_DHT11)) {
//             /* DHT11 读取含有严格时序，因此控制在约 1 秒采样一次。 */
//             SelfTest_ShowDhtData();
//             os_wait2(K_SIG | K_TMO, SELFTEST_DHT_REFRESH_TICKS);
//
//             if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_DHT11) == 0) {
//                 SelfTest_RefreshUi();
//             }
//         }
//         else if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_KEY_BUZZER)) {
//             /*
//              * 有音调请求时播放一个短音；等待期间收到 KEY4 信号会立刻停音并返回。
//              * 没有请求时完全挂起，不会无意义地刷新屏幕或占用 PWM。
//              */
//             if (buzzer_note_request != SELFTEST_BUZZER_NONE) {
//                 u8 note = buzzer_note_request;
//                 buzzer_note_request = SELFTEST_BUZZER_NONE;
//                 Buzzer_Beep(note);
//                 SelfTest_ShowBuzzerNote(note);
//                 os_wait2(K_SIG | K_TMO, SELFTEST_BEEP_TICKS);
//                 Buzzer_Stop();
//
//                 if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_KEY_BUZZER) == 0) {
//                     SelfTest_RefreshUi();
//                 }
//             }
//             else {
//                 os_wait1(K_SIG);
//
//                 if (SelfTest_IsCurrentDetail(SELFTEST_ITEM_KEY_BUZZER) == 0) {
//                     SelfTest_RefreshUi();
//                 }
//             }
//         }
//         else {
//             os_wait1(K_SIG);
//             SelfTest_RefreshUi();
//         }
//     }
// }
