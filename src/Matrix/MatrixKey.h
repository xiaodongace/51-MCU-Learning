#ifndef __MATRIX_KEY_H
#define __MATRIX_KEY_H

#include "config.h"

// 给一种函数:  没有返回值, 有3个u8参数的函数, 定义一个类型 MK_func_cb
typedef void (* MK_func_cb)(u8, u8, u8);

void MatrixKey_Init(void);

/* 设置矩阵按键状态变化后的回调函数。 */
void MatrixKey_SetCallback(MK_func_cb callback);

/* 扫描16个矩阵按键，并通知已注册的状态变化回调。 */
void MK_Scan(void);

#endif /* __MATRIX_KEY_H */
