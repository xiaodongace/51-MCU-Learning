#ifndef __LED_FLOW_LAB_H
#define __LED_FLOW_LAB_H

/*
 * 初始化流水灯练习所需的 LED、按键和内部状态。
 *
 * 该函数只在 main() 进入循环前调用一次。它会设置初始方向、运行状态和
 * 当前 LED，并显示第一盏灯。
 */
void LedFlow_Init(void);

/*
 * 执行一次流水灯练习任务。
 *
 * 应在 main() 的 while(1) 中持续调用。函数负责读取两个按键、处理按键
 * 事件、切换方向或暂停状态，并在运行状态下移动 LED。
 */
void LED_Flow_Task(void);

#endif
