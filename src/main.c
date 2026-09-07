// #include "Config.h"
//
// #include "LedPwmLab.h"
//
// // 程序入口：完成系统基础设置并选择当前测试功能。
// int main(void) {
//
// 	EAXSFR();
// 	/* 所有外设和定时器初始化完成后，再开启总中断。 */
// 	EA = 1;
//
// 	LedPwmLab_Init();
// 	while (1) {
// 		LedPwmLab_Task();
// 	}
//
//
// 	/*
// 	 * LED按键控制反转 非阻塞消抖
// 	 */
// 	{
// 		// LedFlipLab_Init();
// 		// while (1) {
// 		// 	LedFlipLab_Task();
// 		// }
// 	}
//
// 	/*
// 		- KEY1 按下时点亮全部 LED，松开时熄灭全部 LED。
// 		- KEY2 按下时只点亮 LED1，松开时熄灭全部 LED。
// 	 */
// 	{
// 		// KeyLedLab_Init();
// 		// while (1) {
// 		// 	KeyLedLab_Task();
// 		// }
// 	}
//
// 	/*
// 	 * LED间隔1秒闪烁
// 	 */
// 	{
// 		// LedBasicLab_Init();
// 		// while (1) {
// 		// 	LedBasicLab_Task();
// 		// }
// 	}
//
// 	/*
// 	 * 通过旋转电位器修改占空比控制震动幅度
// 	 * 使用时包含 AdcMotorLab.h。
// 	 */
// 	{
// 		// // 开启全局中断
// 		// EA = 1;
// 		// EAXSFR(); // 拓展寄存器使能
// 		// while(1) {
// 		// 	ADC_Upt_PWM_Motor();
// 		// }
// 	}
//
//
// 	/*
// 	 * 数码管跑马灯实现
// 	 */
// 	{
// 		// // 开启全局中断
// 		// EA = 1;
// 		// EAXSFR(); // 拓展寄存器使能
// 		// Nixie_Init();
// 		// while(1) {
// 		// 	Nixie_Running_Test();
// 		// }
// 	}
//
//
// 	/*
// 	 * 使用1号和2号独立按键控制两个外设
// 	 * 按下按键1马达震动	按键2蜂鸣器响起
// 	 * 使用时包含 KeyOutputLab.h。
// 	 */
// 	{
// 		// // 开启全局中断
// 		// EA = 1;
// 		// EAXSFR(); // 拓展寄存器使能
// 		// KeyOutputLab_Init();
// 		// while(1) {
// 		// 	Key_Controller_Buzzer_Or_Motor();
// 		// }
// 	}
//
//
// 	/*
// 	 * 通过串口打印姓名拼音和当前温度格式为 -> name:26
// 	 * 在温度高于初始温度2度以后闪烁8个LED
// 	 * 使用时包含 UartDeviceLab.h。
// 	 */
// 	{
// 		// // 开启全局中断
// 		// EA = 1;
// 		// EAXSFR(); // 拓展寄存器使能
// 		// UartDeviceLab_Init();
// 		// Factory_Init();
// 		// while(1) {
// 		// 	Read_Uart_Message();
// 		// 	Factory_Task();
// 		// }
// 	}
//
//
// 	/*
// 	 * PWM动态控制震动马达频率
// 	 */
// 	{
// 		// // 开启全局中断
// 		// EA = 1;
// 		// EAXSFR(); // 拓展寄存器使能
// 		// Motor_Init();
// 		// while(1) {
// 		// 	Motor_PWM_Test();
// 		// }
// 	}
//
//
// 	/*
// 	 * 偶数LED点亮熄灭
// 	 */
// 	{
// 	// 	LED_Init();
// 	// 	while(1) {
// 	// 		test_LED();
// 	// 	}
// 	}
//
//
// 	/*
// 	 * 使用热敏电阻获取传感器采集到的环境温度数据并通过数码管展示
// 	 */
// 	{
// 	// 	int num_one = 0;
// 	// 	int num_two = 0;
// 	// 	int temperature = 0;
// 	// 	u16 elapsed_ms = 0;
// 	// 	// 开启全局中断
// 	// 	EA = 1;
// 	// 	EAXSFR(); // 拓展寄存器使能
// 	//
// 	// 	NTC_Init();
// 	// 	Nixie_Init();
// 	// 	while(1) {
// 	// 		// 持续刷新数码管
// 	// 		NIXIE_display(num_one, 0);
// 	// 		delay_ms(2);
// 	//
// 	// 		NIXIE_display(num_two, 1);
// 	// 		delay_ms(2);
// 	//
// 	// 		elapsed_ms += 4;
// 	//
// 	// 		// 每500ms读取一次温度
// 	// 		if (elapsed_ms >= 500) {
// 	// 			elapsed_ms = 0;
// 	//
// 	// 			temperature = NTC_GetTemperature();
// 	// 			num_one = temperature / 10;
// 	// 			num_two = temperature % 10;
// 	// 		}
// 	// 	}
// 	}
// }
