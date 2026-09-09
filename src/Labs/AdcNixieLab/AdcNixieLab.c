#include "AdcNixieLab.h"
#include "ADCS.h"
#include "Nixie.h"

// 当前正在刷新的数码管位置
static u8 display_position;

void AdcNixieLab_Init() {
    ADC_InitChannel(ADC_CHANNEL_P05);
    Nixie_Init();
    Nixie_Clear();
    display_position = 0;
}

void AdcNixieLab_Task() {
    u16 adc_value = ADC_Read(ADC_CHANNEL_P05);
    u8 thousands = adc_value / 1000;        // 千位
    u8 hundreds = (adc_value / 100) % 10;   // 百位
    u8 tens = (adc_value / 10) % 10;        // 十位
    u8 ones = adc_value % 10;               // 个位

    if (display_position == 0) {
        NIXIE_display(thousands, 0);
    }else if (display_position == 1) {
        NIXIE_display(hundreds, 1);
    } else if (display_position == 2) {
        NIXIE_display(tens, 2);
    } else {
        NIXIE_display(ones, 3);
    }

    display_position++;
    if (display_position > 4) {
        display_position = 0;
    }
}