#include "Config.h"
#include "SelfTest.h"
#include "Light.h"
#include "UARTS.h"
#include "CarKey.h"
#include "Battery.h"
#include "CarBuzzer.h"
#include "Ultrasonic.h"

// 左前轮 left  forward
#define 	LF_P		P16
#define 	LF_N		P17

// 右前轮 right forward
#define 	RF_P		P14
#define 	RF_N		P15

// 左后轮 left backward
#define 	LB_P		P22
#define 	LB_N		P23

// 右后轮 right backward
#define 	RB_P		P20
#define 	RB_N		P21

/*
 * Demo练习main函数入口
 */
// void main_start(void) RTX_TASK(0)
// {
//     TempAlarmLab_Init();
//     while (1) {
//         TempAlarmLab_SampleTask();
//         TempAlarmLab_AlarmTask();
//     }
// }

void sys_init() {
    EAXSFR();
    EA = 1; // 使能总中断

    // 库函数初始化
    UART_Init();

    // 电机驱动
    // P14 15 16 17
    P1_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    // P20 21 22 23
    P2_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);

    // 外设初始化
    Light_Init();
    CarKey_init();
    Battery_init();
    CarBuzzer_Init();
    Ultrasonic_init();

    printf("====sys_init====\n");
}


/*
 * 小车任务入口
 */
void main_start(void) RTX_TASK(0)
{
    // 初始化任务基本环境
    sys_init();

    // 创建任务
    os_create_task(1);
    os_create_task(2);

    // 销毁任务
    os_delete_task(0);
}


void Light_Task(void) RTX_TASK(1) {
    while (1) {
        // 左边亮灭
        Light_On(LEFT);
        os_wait2(K_TMO, 100);
        Light_Off(LEFT);
        os_wait2(K_TMO, 100);

        // 右边亮灭
        Light_On(RIGHT);
        os_wait2(K_TMO, 100);
        Light_Off(RIGHT);
        os_wait2(K_TMO, 100);

        // 全部亮灭
        Light_On(ALL);
        os_wait2(K_TMO, 100);
        Light_Off(ALL);
        os_wait2(K_TMO, 100);
    }
}

void CarKey_Task(void) RTX_TASK(2) {
    while (1) {
        // 普通版本
        // CarKey_scan();
        // 函数指针版本
        CarKey_scan2(CarKey_on_keydown, NULL);
    }
}


u8 flag = 1;
void CarKey_on_keydown() {
    switch (flag) {
    case 1:
        printf("====正转====");
        RF_P = 1;
        RF_N = 0;
        break;
    case 2:
        printf("====反转====");
        RF_P = 0;
        RF_N = 1;
        break;
    default:
        break;
    }
    flag++;
    if (flag > 2) flag = 1;
}

#if 0
void CarKey_on_keydown_bak() {
    float vol;
    float distance;
    char res;
    printf("Key -> Down\n");

    // 获取电池电压
    vol = Battery_get_voltage();
    printf("vol -> %.2f\n", vol);

    // 蜂鸣器播放
    // CarBuzzer_Test_Beep();
    CarBuzzer_Alarm();

    // 测距
    res = Ultrasonic_get_distance(&distance);
    printf("res -> %d\n", res);
    if (res == 0) {
        printf("distance -> %.2f cm\n", distance);
    }
}
#endif


#if 0
void CarKey_on_keyup() {
    printf("Key -> Up\n");
}
#endif


/*
 * RTX51 Tiny 的任务 0 仅做一次性初始化和任务创建。
 *
 * 任务 1（SelfTest_InputTask）：扫描 key1~key4，产生界面事件。
 * 任务 2（SelfTest_ViewTask）：独占初始化并刷新 SPI 菜单和 I2C 详情屏。
 */
// void main_start(void) RTX_TASK(0)
// {
//     // 初始化任务基本环境
//     SelfTest_Init();
//
//     // 创建任务
//     os_create_task(1);
//     os_create_task(2);
//
//     // 销毁任务
//     os_delete_task(0);
// }
