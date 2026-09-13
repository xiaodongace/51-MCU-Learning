#include "Config.h"
#include "SelfTest.h"
#include "Light.h"
#include "UARTS.h"
#include "CarKey.h"
#include "Battery.h"
#include "CarBuzzer.h"
#include "Ultrasonic.h"
#include "CarMotor.h"



void sys_init() {
    EAXSFR();
    EA = 1; // 使能总中断

    // 库函数初始化
    UART_Init();

    // 外设初始化
    Light_Init();
    CarKey_init();
    Battery_init();
    CarBuzzer_Init();
    Ultrasonic_init();
    CarMotors_init();

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
int value = 0;
char speed = 20;
void CarKey_on_keydown() {
    // 0~100  100~0渐变速度
    #if 0
    // 电机正反转 控制边界条件
    if (flag) {
        value += 10;
        if (value >= 100) {
            value = 100;
            flag = 0;   // 开始递减
        }
    } else {
        value -= 10;
        if (value <= 0) {
            value = 0;
            flag = 1;   // 开始递增
        }
    }
    printf("当前value为 -> %d\n", (int)value);
    CarPWM_Init(value);
#else
    switch(flag){
    case 1:
        CarMotors_forward(speed, LEFT_M);
        break;
    case 2:
        CarMotors_forward(speed, MID_M);
        break;
    case 3:
        CarMotors_forward(speed, RIGHT_M);
        break;
    case 4:
        CarMotors_stop();
        break;
    default:  break;
    }
    flag++;
    if (flag > 3) flag = 1;
#endif
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
 * Demo练习main函数入口
 */
#if 0
void main_start(void) RTX_TASK(0)
{
    TempAlarmLab_Init();
    while (1) {
        TempAlarmLab_SampleTask();
        TempAlarmLab_AlarmTask();
    }
}
#endif


/*
 * RTX51 Tiny 的任务 0 仅做一次性初始化和任务创建。
 *
 * 任务 1（SelfTest_InputTask）：扫描 key1~key4，产生界面事件。
 * 任务 2（SelfTest_ViewTask）：独占初始化并刷新 SPI 菜单和 I2C 详情屏。
 */
#if 0
void main_start(void) RTX_TASK(0)
{
    // 初始化任务基本环境
    SelfTest_Init();

    // 创建任务
    os_create_task(1);
    os_create_task(2);

    // 销毁任务
    os_delete_task(0);
}
#endif

