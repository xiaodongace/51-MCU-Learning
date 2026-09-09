#ifndef __TEMP_ALARM_LAB_H
#define __TEMP_ALARM_LAB_H

/* 初始化温度报警模块使用的定时器、NTC、LED、蜂鸣器和串口。 */
void TempAlarmLab_Init();

/* 定期读取温度，并根据滞回条件更新报警状态。 */
void TempAlarmLab_SampleTask();

/* 根据当前报警状态，执行LED闪烁和蜂鸣器间歇报警。 */
void TempAlarmLab_AlarmTask();

#endif /* __TEMP_ALARM_LAB_H */
