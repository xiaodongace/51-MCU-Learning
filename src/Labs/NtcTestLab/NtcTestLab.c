#include "NtcTestLab.h"
#include "NTC.h"

/* 读取一次NTC温度并通过已经初始化的串口输出。 */
void Test_ADC_LED(void) {
    int temperature;

    /* NTC模块内部负责选择并初始化自己的ADC输入通道。 */
    NTC_Init();
    temperature = NTC_GetTemperature();

    /* 串口初始化由调用这个测试的入口负责，避免NTC模块依赖UART模块。 */
    printf("温度：%d \n", temperature);
}
