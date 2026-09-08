#ifndef __ADC_MODE_LAB_H
#define __ADC_MODE_LAB_H

/* ADC数据的两种业务使用模式。整个Lab只允许处于其中一种模式。 */
typedef enum {
    ADC_MODE_LED = 0,
    ADC_MODE_MOTOR = 1
} AdcModeLab_Mode;

/* 初始化本题使用的ADC、按键、LED和马达，并建立安全初始输出。 */
void AdcModeLab_Init(void);

/* 执行一次按键处理、ADC采样和当前模式对应的输出分发。 */
void AdcModeLab_Task(void);

/* 读取当前模式，供显示层或测试代码查询。 */
AdcModeLab_Mode AdcModeLab_GetMode(void);

#endif /* __ADC_MODE_LAB_H */
