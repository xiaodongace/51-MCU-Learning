#ifndef __LED_PWM_LAB_H
#define __LED_PWM_LAB_H


/* 初始化固定档位LED PWM实验。 */
void LedPwmLab_Init(void);

/* 在主循环中扫描按键并切换LED亮度档位。 */
void LedPwmLab_Task(void);

/* 让LED亮度在0~100%之间循环变化，用于PWM呼吸灯测试。 */
void PWM_Controller_LED_Test(void);

#endif /* __LED_PWM_LAB_H */
