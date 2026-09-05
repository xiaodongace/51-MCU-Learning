#include "RtcDisplayLab.h"
#include "IIC.h"
#include "EXTIS.h"
#include "Exti.h"
#include "Delay.h"
#include "oled.h"

/* 测试RTC时间、闹钟和定时器的I2C读写及外部中断处理。 */
void Test_I2C_ReadOrWrite(void) {
    Clock_t clock;
    Alarm_t alarm;

    /* Lab统一初始化I2C、RTC中断输入和总中断。 */
    IIC_Init();
    EXTI_Init();
    EA = 1;

    /* 使用十进制准备RTC初始时间，再由IIC模块转换成BCD写入。 */
    clock.year = 2026;
    clock.month = 8;
    clock.day = 27;
    clock.week = 4;
    clock.hour = 23;
    clock.minute = 59;
    clock.second = 55;
    I2C_Set_Clock(clock);

    /* 配置原测试使用的闹钟条件，并打开闹钟中断。 */
    alarm.minute = 0;
    alarm.hour = 0;
    alarm.day = 28;
    alarm.week = -2;
    I2C_Set_Alarm(alarm);
    I2C_Enable_Alarm(ENABLE);

    /* 以64 Hz计数64次，保留原测试约1秒到期的RTC定时器配置。 */
    I2C_Set_Timer(HZ64, 64);
    I2C_Enable_Timer(ENABLE);

    while (1) {
        /* 每秒读取并打印一次RTC日期时间。 */
        I2C_Get_Clock(&clock);
        printf("%04d-%02d-%02d ", (int)clock.year, (int)clock.month, (int)clock.day);
        printf("%02d:%02d:%02d ", (int)clock.hour, (int)clock.minute, (int)clock.second);
        printf("week->%d\r\n", (int)clock.week);

        /* INT3唤醒后读取RTC标志，并在处理后清除本次唤醒来源。 */
        if (WakeUpSource == 4) {
            WakeUpSource = 0;
            on_int3_call();
        }

        delay_ms(250);
        delay_ms(250);
        delay_ms(250);
        delay_ms(250);
    }
}

/* 在OLED上持续显示通过I2C读取的RTC日期和时间。 */
void I2COLedTest(void) {
    int counter = 1006;
    char text_buffer[32];
    Clock_t clock;

    /* 该测试需要RTC中断和OLED显示，因此由Lab统一完成组合初始化。 */
    EA = 1;
    IIC_Init();
    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);

    /* 建立一个确定的初始日期时间，保留原测试数据。 */
    clock.year = 2026;
    clock.month = 8;
    clock.day = 27;
    clock.week = 4;
    clock.hour = 23;
    clock.minute = 59;
    clock.second = 55;
    I2C_Set_Clock(clock);

    while (1) {
        /* RTC模块只提供时间数据，OLED模块只负责显示，格式组合留在Lab。 */
        I2C_Get_Clock(&clock);
        OLED_ShowString(0, 0, "Test...", 16);

        sprintf(text_buffer, "%04d-%02d-%02d", (int)clock.year, (int)clock.month, (int)clock.day);
        OLED_ShowString(0, 2, text_buffer, 16);

        sprintf(text_buffer, "%02d:%02d:%02d W:%d", (int)clock.hour, (int)clock.minute,
                (int)clock.second, (int)clock.week);
        OLED_ShowString(0, 4, text_buffer, 16);

        sprintf(text_buffer, "%d", counter--);
        OLED_ShowString(0, 6, "    ", 16);
        OLED_ShowString(0, 6, text_buffer, 16);

        /* 保留原测试每秒更新一次显示的节奏。 */
        delay_ms(250);
        delay_ms(250);
        delay_ms(250);
        delay_ms(250);
    }
}
