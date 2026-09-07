#ifndef __NIXIE_H
#define __NIXIE_H

#include "config.h"

// 初始化数码管模块。
void Nixie_Init(void);

/* 让8个数码管同时按照外围段顺序运行跑马灯测试。 */
void Nixie_Running_Test(void);

#define	NIXIE_DI	P44	// 数据输入
#define	NIXIE_SCK	P42	// 移位寄存器
#define	NIXIE_RCK	P43	// 锁存寄存器

#define NIXIE_PIN_INIT() {    P4M0 &= ~0x1c; P4M1 &= ~0x1c; }

// 		u8 a_dat = 0x12;	// 0001 0010	字母位
//		u8 b_idx = 0x1F;	// 0001 1111	数字位
void NIXIE_show(u8 a_dat, u8 b_idx);

// num对应数字在数组里的位置（索引）
// id 显示在指定位置(0 -> 7)
void NIXIE_display(u8 num, u8 id);

/* 让 8 个数码管同时显示同一个十进制数字，num 有效范围为 0~9。 */
void Nixie_ShowAllDigit(u8 num);

/*
 * 显示一圈数码管跑马灯。
 * position 0~7：第 1~8 位的顶部 a 段从左向右移动；
 * position 8~9：第 8 位的右上 b 段、右下 c 段依次向下；
 * position 10~17：第 8~1 位的底部 d 段从右向左返回；
 * position 18~19：第 1 位的左下 e 段、左上 f 段依次向上。
 */
void Nixie_ShowRunningLight(u8 position);

/* 关闭全部数码管位选，用于退出自检项目时清屏。 */
void Nixie_Clear(void);

#endif /* __NIXIE_H */
