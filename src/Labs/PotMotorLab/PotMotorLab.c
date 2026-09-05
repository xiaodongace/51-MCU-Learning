#include "PotMotorLab.h"
#include "ADCS.h"
#include "Motor.h"


/* 初始化P0.5电位器ADC输入和马达PWM，并以0%速度保证上电安全。 */
void PotMotorLab_Init(void) {
    /* ADC13对应P0.5电位器，ADC模块负责完成模拟输入与ADC硬件配置。 */
    ADC_InitChannel(ADC_CHANNEL_P05);

    /* 马达模块负责配置自身PWM输出，Lab不直接操作PWM寄存器。 */
    Motor_Init();

    /* 初始化完成后立即明确输出0%，避免复位后马达意外高速运转。 */
    Motor_SetSpeed(0);
}

/* 采样电位器ADC值，将其线性映射为0~100%的马达速度并持续更新输出。 */
void PotMotorLab_Task(void) {
    u8 motor_percent = 0;
    u16 adc_value = ADC_Read(ADC_CHANNEL_P05);

    /* 12位ADC的有效结果范围是0~4095，限制异常高值以保护后续映射计算。 */
    if (adc_value > 4095) {
        adc_value = 4095;
    }

    /* 先提升为u32再乘100，避免16位环境中4095 * 100的中间结果溢出。 */
    /* 公式将0~4095的ADC刻度线性缩放为Motor_SetSpeed()需要的0~100%。 */
    motor_percent = (u8)(((u32)adc_value * 100) / 4095);

    /* 对映射结果再次限幅，使接口即使在后续计算调整后仍不会收到非法百分比。 */
    if (motor_percent > 100) {
        motor_percent = 100;
    }

    /* 将百分比交给马达模块，由其更新PWM占空比。 */
    Motor_SetSpeed(motor_percent);
}
