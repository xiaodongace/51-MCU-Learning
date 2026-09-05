#ifndef __LED_CYCLE_LAB_H
#define __LED_CYCLE_LAB_H

/*
 * 初始化“按键切换单灯”练习。
 *
 * 完成 LED、按键的基础初始化，设置上电默认点亮 LED1，并读取一次按键
 * 初始状态，避免上电时按住按键就被误认为一次有效按下。
 */
void LedCycleLab_Init(void);

/*
 * 执行一次练习任务。
 *
 * main() 的 while(1) 中应持续调用本函数。函数会检查 KEY1 是否发生一次
 * 完整的“按下再松开”，每次完整按下将显示切换到下一盏 LED。函数本身
 * 不包含无限循环，便于以后和其他任务并行运行。
 */
void LedCycleLab_Task(void);


#endif
