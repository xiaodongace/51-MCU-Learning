#include    "Config.h"
#include    "UART.h"
#include    "NVIC.h"
#include    "UARTS.h"
#include    "Switch.h"
#include    "Delay.h"

/*
 * UART1 接收中断每收到一个字节都会把 COM1.RX_TimeOut 重置为 TimeOutSet1。
 * EchoTask 每 10 ms 将它减 1；当前 TimeOutSet1 为 5，因此约 50 ms 没有
 * 新字节到达时，认为当前这一段数据已经接收完成并执行回显。
 */
#define UART_ECHO_TASK_PERIOD_MS 10


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
 * 每 10 ms 检查一次接收空闲时间；一段数据接收完成后原样回显。
 * 本函数应在普通 main 循环中反复调用，不依赖 RTX51 的任务等待接口。
 */
void UART_EchoTask(void) {
    /* RX_TimeOut 归零表示约 50 ms 内没有收到新字节。 */
    if ((COM1.RX_TimeOut > 0) && (--COM1.RX_TimeOut == 0)) {
        if (COM1.RX_Cnt > 0) {
            /* 先回显完整缓冲区，再清空长度，避免本条数据被直接丢弃。 */
            Out_Uart_Message();

            /* 本条消息处理完成后清零长度，等待下一条消息。 */
            COM1.RX_Cnt = 0;
        }
    }

    /* 轮询周期定义了 RX_TimeOut 的时间单位，修改此值时应同步检查超时含义。 */
    delay_ms(UART_ECHO_TASK_PERIOD_MS);
}
