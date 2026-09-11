#ifndef __CAR_BUZZER_H
#define __CAR_BUZZER_H

#include "GPIO.h"
#include "Switch.h"
#include "NVIC.h"
#include "STC8H_PWM.h"

void CarBuzzer_Init();

void CarBuzzer_Alarm();

void CarBuzzer_Test_Beep();


#endif /* __CAR_BUZZER_H */
