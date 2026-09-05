#ifndef __UARTS_H
#define __UARTS_H

#include "Config.h"

// 初始化UART通信和串口接收中断。
void UART_Init(void);

// 将当前接收缓冲区中的数据原样发送回串口。
void Out_Uart_Message(void);

#endif /* __UARTS_H */
