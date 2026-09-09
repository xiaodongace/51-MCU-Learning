#ifndef __UART_COMMAND_H
#define __UART_COMMAND_H

#include "Config.h"

/*
 * 串口命令类型。
 *
 * UART_COMMAND_TYPE_NONE 表示当前没有解析出有效的外设命令；
 * LED 和 MOTOR 分别对应题22中的两类控制命令。
 */
typedef enum {
    UART_COMMAND_TYPE_NONE = 0,
    UART_COMMAND_TYPE_LED,
    UART_COMMAND_TYPE_MOTOR
} UartCommandType;

/*
 * 串口命令解析状态。
 *
 * 解析模块只返回状态，不直接向串口发送错误文本；
 * UARTS.c 根据状态生成适合上位机显示的提示。
 */
typedef enum {
    UART_COMMAND_STATUS_OK = 0,
    UART_COMMAND_STATUS_EMPTY,
    UART_COMMAND_STATUS_FORMAT_ERROR,
    UART_COMMAND_STATUS_UNKNOWN_COMMAND,
    UART_COMMAND_STATUS_MISSING_PARAMETER,
    UART_COMMAND_STATUS_NON_DIGIT,
    UART_COMMAND_STATUS_OUT_OF_RANGE
} UartCommandStatus;

/*
 * 串口命令解析结果。
 *
 * type    ：解析出的外设类型。
 * percent ：外设参数，题22规定合法范围为0~100。
 */
typedef struct {
    UartCommandType type;
    u8 percent;
} UartCommandResult;

/*
 * 解析一条已经接收完成的串口命令。
 *
 * message：指向UART接收缓冲区的首地址，当前工程为xdata缓冲区。
 * length ：有效数据长度，不要求数据以字符串结束符'\0'结尾。
 * result ：输出解析后的命令类型和百分比；失败时type保持NONE。
 * 返回值：具体解析状态，不操作UART、LED、Motor或其他硬件。
 */
UartCommandStatus UartCommand_Parse(u8 xdata *message,
                                    u8 length,
                                    UartCommandResult *result);


#endif /* __UART_COMMAND_H */
