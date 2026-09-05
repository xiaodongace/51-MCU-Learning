#include "AdcMotorLab.h"
#include "ADCS.h"
#include "Motor.h"
#include "Delay.h"

/* 使用P0.5电位器连续调节震动马达的PWM占空比。 */
void ADC_Upt_PWM_Motor(void) {
    u16 adc_value;
    u8 motor_percent;

    /* ADC和马达分别由自己的模块初始化，Lab只负责把采样结果传给马达。 */
    ADC_InitChannel(ADC_CHANNEL_P05);
    Motor_Init();
    Motor_SetSpeed(0);

    while (1) {
        /* 读取12位ADC值，并将0~4095线性映射为0~100%。 */
        adc_value = ADC_Read(ADC_CHANNEL_P05);
        motor_percent = (u8)(((u32)adc_value * 100) / 4095);

        /* 把映射结果交给马达模块，不在本Lab直接修改PWM寄存器。 */
        Motor_SetSpeed(motor_percent);
        delay_ms(20);
    }
}
