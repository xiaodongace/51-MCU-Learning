#ifndef KEIL_IDE_COMPAT_H
#define KEIL_IDE_COMPAT_H

/*
 * 仅供 CLion/Clang 索引使用，不参与 Keil C51 编译。
 * 这些宏把 Keil C51 的存储区和寄存器声明语法降级为普通 C，
 * 让 CLion 可以继续解析 GPIO、UART、Timer 等头文件中的符号。
 */
#define sfr volatile unsigned char
#define sbit volatile unsigned char
#define bit unsigned char
#define xdata
#define idata
#define data
#define code
#define interrupt(vector)
#define using(bank)
#define _at_
#define _nop_() ((void)0)

#endif
