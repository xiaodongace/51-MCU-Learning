#include "UartDeviceLab.h"
#include "UARTS.h"
#include "UART.h"
#include "LED.h"

/* 保存串口命令当前设置的LED逻辑亮度。 */
static u8 xdata led_brightness = 0;

/* 初始化串口触发DHT11和LED控制实验。 */
void UartDeviceLab_Init(void) {
    /* UART和DHT11分别由自己的模块初始化，Lab只负责组织依赖关系。 */
    UART_Init();
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
