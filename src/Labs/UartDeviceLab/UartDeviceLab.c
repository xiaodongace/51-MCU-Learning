#include "UartDeviceLab.h"
#include "UARTS.h"
#include "UART.h"
#include "LED.h"
#include "DHT11.h"
#include "Delay.h"

/* 保存串口命令当前设置的LED逻辑亮度。 */
static u8 xdata led_brightness = 0;

/* 初始化串口触发DHT11和LED控制实验。 */
void UartDeviceLab_Init(void) {
    /* UART和DHT11分别由自己的模块初始化，Lab只负责组织依赖关系。 */
    UART_Init();
    DHT11_Init();
    led_brightness = 0;
}

/* 解析串口命令并控制LED流水方向。 */
void Uart_Controller_LED(void) {
    u8 uart_data = RX1_Buffer[0];

    /* 输出收到的原始字节，便于区分字符'0'和数值0。 */
    printf("收到：data = 0x%02bX\r\n", uart_data);

    /* 字符'0'或二进制0选择正向，字符'1'或二进制1选择反向。 */
    if ((uart_data == 0x00) || (uart_data == '0')) {
        LED_Flowing_Test(0);
    }
    else if ((uart_data == 0x01) || (uart_data == '1')) {
        LED_Flowing_Test(1);
    }
}

/* 解析串口命令并以10%为步长调整LED PWM亮度。 */
void Uart_Controller_PWM_LED(void) {
    /* 0命令增加亮度，并在100%处钳位。 */
    if ((RX1_Buffer[0] == 0x00) || (RX1_Buffer[0] == '0')) {
        if (led_brightness <= 90) {
            led_brightness += 10;
        }
        else {
            led_brightness = 100;
        }
    }
    /* 1命令降低亮度，并在0%处钳位。 */
    else if ((RX1_Buffer[0] == 0x01) || (RX1_Buffer[0] == '1')) {
        if (led_brightness >= 10) {
            led_brightness -= 10;
        }
        else {
            led_brightness = 0;
        }
    }

    /* 亮度换算和PWM寄存器配置仍由LED模块负责。 */
    LED_SetBrightness(led_brightness);
}

/* 检查接收超时，回显消息并触发一次DHT11采集。 */
void Read_Uart_Message(void) {
    /* RX_TimeOut递减到0表示本条串口消息已经接收完成。 */
    if ((COM1.RX_TimeOut > 0) && (--COM1.RX_TimeOut == 0)) {
        if (COM1.RX_Cnt > 0) {
            /* UART模块负责回显，DHT11模块负责采集，Lab负责组合业务。 */
            Out_Uart_Message();
            DHT11_Task();
        }

        /* 本条消息处理完成后清零长度，等待下一条消息。 */
        COM1.RX_Cnt = 0;
    }

    /* 保留原测试每10 ms检查一次消息的节奏。 */
    delay_ms(10);
}
