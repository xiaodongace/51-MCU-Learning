#ifndef __UARTS_H
#define __UARTS_H

#include "Config.h"

// 初始化UART通信和串口接收中断。
void UART_Init(void);

// 将当前接收缓冲区中的数据原样发送回串口。
void Out_Uart_Message(void);

/*
 * 每约10 ms检查一次接收超时；连续约50 ms没有新字节时，
 * 解析当前单字节 LED 命令并清理接收状态。应在普通 main 循环中重复调用。
 */
void UART_MessageTask(void);

#endif /* __UARTS_H */
