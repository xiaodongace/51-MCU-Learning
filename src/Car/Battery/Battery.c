#include "Battery.h"
#include "ADCS.h"


// 初始化
void Battery_init() {
    // 高阻输入
    P1_MODE_IN_HIZ(GPIO_Pin_3);
    // 初始化ADC
    ADC_Init();
}

// 获取电池电压
float Battery_get_voltage() {
    u16 adc_value;
    float vol;

    adc_value = ADC_Read(ADC_CH3);
    vol = adc_value * 2.5 / 4095;

    return vol * 6.1;
}