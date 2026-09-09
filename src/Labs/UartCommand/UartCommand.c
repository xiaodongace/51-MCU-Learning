#include "UartCommand.h"

/*
 * 题22命令格式：
 *
 *     LED:0~100
 *     MOTOR:0~100
 *
 * 本文件只负责把字节序列转换为“命令类型 + 百分比”。
 * 真正的LED亮度和马达速度控制由UARTS.c调用对应模块接口完成。
 */

/*
 * 解析一条已经接收完成的串口命令。
 *
 * 函数不依赖COM1或RX1_Buffer，因此解析逻辑可以单独测试；
 * 参数通过手动逐位检查，确保“60abc”不会被错误解析成60。
 */
UartCommandStatus UartCommand_Parse(u8 xdata *message,
                                    u8 length,
                                    UartCommandResult *result)
{
    u8 colon_index = 0;
    u8 command_length;
    u8 parameter_start;
    u8 i;
    u8 digit;
    u16 parsed_value = 0;
    UartCommandType command_type = UART_COMMAND_TYPE_NONE;

    /*
     * result是调用者提供的输出空间。
     * 如果没有输出空间，函数无法履行接口约定，因此直接返回格式错误。
     */
    if (result == 0) {
        return UART_COMMAND_STATUS_FORMAT_ERROR;
    }

    /*
     * 每次解析前先清空上一次结果，避免调用者误用旧的命令类型或参数。
     * 只有全部检查通过后，下面才会写入有效的解析结果。
     */
    result->type = UART_COMMAND_TYPE_NONE;
    result->percent = 0;

    /* 没有数据时，不进入后续的缓冲区访问。 */
    if ((message == 0) || (length == 0)) {
        return UART_COMMAND_STATUS_EMPTY;
    }

    /*
     * 很多串口助手会在发送内容后自动追加CR、LF或CR/LF。
     * 这些字符只表示行结束，不属于百分比参数，所以从有效长度中去除。
     */
    while ((length > 0) &&
           ((message[length - 1] == '\r') || (message[length - 1] == '\n'))) {
        length--;
    }

    if (length == 0) {
        return UART_COMMAND_STATUS_EMPTY;
    }

    /*
     * 查找命令名与参数之间的冒号。
     * 没找到冒号，或者冒号位于第一个字符，说明整体格式不完整。
     */
    while ((colon_index < length) && (message[colon_index] != ':')) {
        colon_index++;
    }

    if ((colon_index == 0) || (colon_index >= length)) {
        return UART_COMMAND_STATUS_FORMAT_ERROR;
    }

    command_length = colon_index;
    parameter_start = colon_index + 1;

    /* 冒号后没有任何内容，属于参数缺失。 */
    if (parameter_start >= length) {
        return UART_COMMAND_STATUS_MISSING_PARAMETER;
    }

    /*
     * 命令名采用精确匹配：
     * - 只接受大写LED和MOTOR；
     * - 不接受多余字符；
     * - 不把大小写转换隐藏在解析逻辑中。
     */
    if ((command_length == 3) &&
        (message[0] == 'L') && (message[1] == 'E') && (message[2] == 'D')) {
        command_type = UART_COMMAND_TYPE_LED;
    }
    else if ((command_length == 5) &&
             (message[0] == 'M') && (message[1] == 'O') &&
             (message[2] == 'T') && (message[3] == 'O') && (message[4] == 'R')) {
        command_type = UART_COMMAND_TYPE_MOTOR;
    }
    else {
        return UART_COMMAND_STATUS_UNKNOWN_COMMAND;
    }

    /*
     * 逐个检查参数字符并进行十进制累加。
     * 使用u16作为中间值，避免u8在计算过程中发生溢出。
     */
    for (i = parameter_start; i < length; i++) {
        if ((message[i] < '0') || (message[i] > '9')) {
            return UART_COMMAND_STATUS_NON_DIGIT;
        }

        digit = message[i] - '0';
        parsed_value = parsed_value * 10 + digit;

        /*
         * 一旦超过100就立即返回，既满足题目要求，也避免继续累加更长数字。
         * 例如101、999和999999都会在超过100时被拒绝。
         */
        if (parsed_value > 100) {
            return UART_COMMAND_STATUS_OUT_OF_RANGE;
        }
    }

    /* 所有检查完成后，才把临时结果提交给调用者。 */
    result->type = command_type;
    result->percent = (u8)parsed_value;
    return UART_COMMAND_STATUS_OK;
}
