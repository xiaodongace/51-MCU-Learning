#ifndef __UART_DEVICE_LAB_H
#define __UART_DEVICE_LAB_H

#include "Config.h"

/* 初始化串口触发DHT11和LED控制实验。 */
void UartDeviceLab_Init(void);

/* 检查接收超时，回显消息并触发一次DHT11采集。 */
void Read_Uart_Message(void);

/* 解析串口命令并控制LED流水方向。 */
void Uart_Controller_LED(void);

/* 解析串口命令并调整LED PWM亮度。 */
void Uart_Controller_PWM_LED(void);

#endif /* __UART_DEVICE_LAB_H */
