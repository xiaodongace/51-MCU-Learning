#include    "Config.h"
#include    "UART.h"
#include    "NVIC.h"
#include    "UARTS.h"
#include    "Switch.h"


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

