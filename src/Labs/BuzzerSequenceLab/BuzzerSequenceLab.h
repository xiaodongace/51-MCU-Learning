#ifndef __Buzzer_Sequence_Lab_H
#define __Buzzer_Sequence_Lab_H


/* 初始化LED、按键、蜂鸣器、定时器和旋律状态机。 */
void BuzzerSequenceLab_Init(void);

/* 在主循环中反复调用，处理按键并推进旋律状态机。 */
void BuzzerSequenceLab_Task(void);

/* 运行支持开始、暂停、继续和停止的新旋律状态机。 */
void New_SequenceTask(void);

#endif
