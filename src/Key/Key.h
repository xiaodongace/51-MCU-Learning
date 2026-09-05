#ifndef __KEY_H
#define __KEY_H

#include "Config.h"

// 初始化按键输入模块。
void Key_Init(void);

/*
 * 扫描KEY1至KEY3并更新公共非阻塞消抖状态。
 * 使用按键事件的任务应在每轮主循环先调用一次本函数。
 */
void Key_Scan(void);

/*
 * 读取并消费指定按键的一次性按下事件。
 * key_index：0、1、2分别对应KEY1、KEY2、KEY3；返回非零表示发生一次按下。
 */
u8 Key_GetPressEvent(u8 key_index);

/*
 * 查询指定按键的当前物理状态。
 *
 * key_index：按键编号；当前模块支持 0（KEY1）和 1（KEY2）和 2（KEY3）。
 * 返回值：非零表示按下，0 表示松开或编号无效。
 *
 * 本函数只读取当前电平，不做消抖，也不会记录按下事件。需要非阻塞消抖
 * 和“按一次只触发一次”时，应在主循环先调用 Key_Scan()，再由业务模块
 * 调用 Key_GetPressEvent() 读取一次性按下事件。
 */
u8 Key_IsPressed(u8 key_index);

#endif /* __KEY_H */
