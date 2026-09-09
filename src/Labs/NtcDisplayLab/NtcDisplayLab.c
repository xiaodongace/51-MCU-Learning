#include "NtcDisplayLab.h"
#include "NTC.h"
#include "Nixie.h"
#include "UARTS.h"
#include "Timers.h"

static u8 NixieShow_arr[3] = {0, 0, 0};
static u8 display_position;
static u16 sys_time;

void NtcDisplayLab_Init() {
    EAXSFR();
    EA = 1;

    Timers_Init();
    NTC_Init();
    Nixie_Init();
    Nixie_Clear();
    UART_Init();

    sys_time = 0;
    display_position = 0;
}

void NtcDisplayLab_SampleTask() {
    int temperature = 0;
    u16 now = Timers_GetSystemMs();
    if (now - sys_time >= 500) {
        sys_time = now;
        temperature = NTC_GetTemperature();
        printf("当前温度为： %d\n", temperature);

        if (temperature < 0) {
            NixieShow_arr[0] = 21;  // 21表示为负数符号
            // temperature = abs(temperature);
            temperature = -temperature;
            NixieShow_arr[1] = (u8)(temperature / 10) % 10;
            NixieShow_arr[2] = (u8)temperature % 10;
        } else {
            NixieShow_arr[0] = (u8)(temperature / 100) % 10;
            NixieShow_arr[1] = (u8)(temperature / 10) % 10;
            NixieShow_arr[2] = (u8)temperature % 10;
        }
    }
}

void NtcDisplayLab_DisplayTask() {
    if (display_position == 0) {
        NIXIE_display(NixieShow_arr[0], 0);
    } else if (display_position == 1) {
        NIXIE_display(NixieShow_arr[1], 1);
    } else {
        NIXIE_display(NixieShow_arr[2], 2);
    }
    display_position++;
    if (display_position > 2) {
        display_position = 0;
    }
}