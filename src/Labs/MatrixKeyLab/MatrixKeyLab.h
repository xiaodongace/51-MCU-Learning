#ifndef __MATRIX_KEY_LAB_H
#define __MATRIX_KEY_LAB_H

#include "Config.h"

/* 初始化矩阵键盘串口打印测试。 */
void MatrixKeyLab_Init(void);

/* 扫描矩阵键盘并打印本轮产生的状态变化。 */
void MatrixKeyLab_Task(void);

/* 将一次矩阵按键状态变化格式化后输出到串口。 */
void MK_Callback(u8 row, u8 col, u8 is_keyup);

#endif /* __MATRIX_KEY_LAB_H */
