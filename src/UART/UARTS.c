#include    "Config.h"
#include    "UART.h"
#include    "NVIC.h"
#include    "UARTS.h"
#include    "Switch.h"
#include    "Delay.h"
#include    "LED.h"
#include    "Motor.h"
#include    "UartCommand.h"

/*
 * UART1 接收中断每收到一个字节都会把 COM1.RX_TimeOut 重置为 TimeOutSet1。
 * MessageTask 每 10 ms 将它减 1；当前 TimeOutSet1 为 5，因此约 50 ms 没有
 * 新字节到达时，认为当前这一段数据已经接收完成并开始解析命令。
 */
#define UART_MESSAGE_TASK_PERIOD_MS 10


// 初始化UART通信和串口接收中断。
void UART_Init(void) {
    /* 在这里完成 UART 模块的初始化。 */
    COMx_InitDefine COMx_InitStructure;
    COMx_InitStructure.UART_Mode = UART_8bit_BRTx; //模式, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    COMx_InitStructure.UART_BRT_Use = BRT_Timer1; //选择波特率发生器, BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
    COMx_InitStructure.UART_BaudRate = 115200ul; //波特率, 一般 110 ~ 115200
    COMx_InitStructure.UART_RxEnable = ENABLE; //接收允许,   ENABLE或DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE; //波特率加倍, ENABLE或DISABLE
    UART_Configuration(UART1, &COMx_InitStructure); //初始化串口1 UART1,UART2,UART3,UART4
    NVIC_UART1_Init(ENABLE,Priority_1); //中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

    UART1_SW(UART1_SW_P30_P31); //UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}


// 将当前接收缓冲区中的数据原样发送回串口。
void Out_Uart_Message(void) {
    u8 i;
    // RX_Cnt收到的数据个数（字节u8 - unsigned char）
    // 将收到的数据, 按字节逐个循环
    for (i = 0; i < COM1.RX_Cnt; i++) {
        TX1_write2buff(RX1_Buffer[i]); //收到的数据原样返回
    }
}

/*
 * 解析一条完整的单字节 LED 命令。
 * 合法命令为字符 '0'/'1' 或原始字节 0x00/0x01；其他长度或内容均不改变 LED 状态。
 */
static void UART_HandleLegacyLedCommand(void) {
    u8 command;

    /* 第21题的协议仅接受一个字节；串口助手测试时应关闭自动附加的 CR/LF。 */
    if (COM1.RX_Cnt != 1) {
        printf("ERR: use 0 or 1\r\n");
        return;
    }

    command = RX1_Buffer[0];
    if ((command == 0x00) || (command == '0')) {
        LED_SetAll(DISABLE);
    }
    else if ((command == 0x01) || (command == '1')) {
        LED_SetAll(ENABLE);
    }
    else {
        /* 使用 ASCII 错误文本，避免串口助手的字符编码影响显示结果。 */
        printf("ERR: use 0 or 1\r\n");
    }
}

/*
 * 根据解析状态向上位机发送明确的错误提示。
 *
 * 解析模块只负责判断错误类型；串口输出属于UART业务层，
 * 因此错误文本集中放在这里，避免UartCommand.c依赖printf。
 */
static void UART_SendCommandError(UartCommandStatus status) {
    switch (status) {
    case UART_COMMAND_STATUS_EMPTY:
        printf("ERR: empty command\r\n");
        break;
    case UART_COMMAND_STATUS_UNKNOWN_COMMAND:
        printf("ERR: unknown command\r\n");
        break;
    case UART_COMMAND_STATUS_MISSING_PARAMETER:
        printf("ERR: missing parameter\r\n");
        break;
    case UART_COMMAND_STATUS_NON_DIGIT:
        printf("ERR: parameter must be digits\r\n");
        break;
    case UART_COMMAND_STATUS_OUT_OF_RANGE:
        printf("ERR: parameter must be 0-100\r\n");
        break;
    case UART_COMMAND_STATUS_FORMAT_ERROR:
    default:
        printf("ERR: use LED:0-100 or MOTOR:0-100\r\n");
        break;
    }
}

/*
 * 处理一条已经接收完成的消息。
 *
 * 本函数只负责业务分发：
 * - UartCommand_Parse()负责解析；
 * - LED_SetBrightness()负责LED亮度实现；
 * - Motor_SetSpeed()负责马达速度实现。
 */
static void UART_HandleCommandFrame(void) {
    UartCommandResult result;
    UartCommandStatus status;

    /*
     * 保留第21题的单字节命令兼容性。
     * 这样发送字符'0'/'1'或原始字节0x00/0x01时，仍然控制全部LED。
     */
    if (COM1.RX_Cnt == 1) {
        if ((RX1_Buffer[0] == 0x00) || (RX1_Buffer[0] == '0') ||
            (RX1_Buffer[0] == 0x01) || (RX1_Buffer[0] == '1')) {
            UART_HandleLegacyLedCommand();
            return;
        }
    }

    /* 将完整消息交给纯解析模块，不在这里重复解析字符串。 */
    status = UartCommand_Parse(RX1_Buffer, COM1.RX_Cnt, &result);
    if (status != UART_COMMAND_STATUS_OK) {
        /* 非法输入只返回错误，不调用任何LED或马达控制接口。 */
        UART_SendCommandError(status);
        return;
    }

    /* 解析成功后，依据命令类型分发给对应硬件模块。 */
    if (result.type == UART_COMMAND_TYPE_LED) {
        LED_SetBrightness(result.percent);
        printf("OK: LED=%d\r\n", (int)result.percent);
    }
    else if (result.type == UART_COMMAND_TYPE_MOTOR) {
        Motor_SetSpeed(result.percent);
        printf("OK: MOTOR=%d\r\n", (int)result.percent);
    }
    else {
        /* 作为保护分支，防止未来新增类型后遗漏对应的硬件分发。 */
        UART_SendCommandError(UART_COMMAND_STATUS_FORMAT_ERROR);
    }
}


/*
 * 每 10 ms 检查一次接收空闲时间；一段数据接收完成后解析并分发串口命令。
 * 本函数应在普通 main 循环中反复调用，不依赖 RTX51 的任务等待接口。
 */
void UART_MessageTask(void) {
    /* RX_TimeOut 归零表示约 50 ms 内没有收到新字节。 */
    if ((COM1.RX_TimeOut > 0) && (--COM1.RX_TimeOut == 0)) {
        if (COM1.RX_Cnt > 0) {
            /* 完整消息才进入解析；UART中断服务函数只负责接收字节。 */
            UART_HandleCommandFrame();

            /* 本条消息处理完成后清零长度，等待下一条消息。 */
            COM1.RX_Cnt = 0;
            COM1.RX_TimeOut = 0;
        }
    }

    /* 轮询周期定义了 RX_TimeOut 的时间单位，修改此值时应同步检查超时含义。 */
    delay_ms(UART_MESSAGE_TASK_PERIOD_MS);
}
