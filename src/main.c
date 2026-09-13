#include "Config.h"
#include "SelfTest.h"
#include "Light.h"
#include "UARTS.h"
#include "CarKey.h"
#include "Battery.h"
#include "CarBuzzer.h"
#include "Ultrasonic.h"
#include "CarMotor.h"
#include "Track.h"
#include "UART.h"

#define Light_Task_Id  1
#define CarKey_Task_Id  2
#define Track_Task_Id  3
#define Uart1_Task_Id  4
#define Uart2_Task_Id  5

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
    Track_init();

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
    os_create_task(Light_Task_Id);
    os_create_task(CarKey_Task_Id);
    os_create_task(Uart1_Task_Id);
    os_create_task(Uart2_Task_Id);

    // 销毁任务
    os_delete_task(0);
}


void Light_Task(void) RTX_TASK(Light_Task_Id) {
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

void CarKey_Task(void) RTX_TASK(CarKey_Task_Id) {
    while (1) {
        // 普通版本
        // CarKey_scan();
        // 函数指针版本
        CarKey_scan2(CarKey_on_keydown, NULL);
    }
}


/*
 * 测试蓝牙
 */
#if 1
void Uart1_Task(void) RTX_TASK(Uart1_Task_Id) {
    u8 i;

    while(1) {
        if(COM1.RX_TimeOut > 0) {
            //超时计数
            if(--COM1.RX_TimeOut == 0) {
                if(COM1.RX_Cnt > 0) {
                    for(i=0; i<COM1.RX_Cnt; i++)	{
                        // RX1_Buffer[i]存的是接收的每个字节，写出用 TX1_write2buff
                        TX2_write2buff(RX1_Buffer[i]);
                    }
                }
                COM1.RX_Cnt = 0;
            }
        }

        os_wait2(K_TMO, 1); // 5ms * 1
    }
}

void Uart2_Task(void) RTX_TASK(Uart2_Task_Id) {
    u8 i;

    while(1) {
        if(COM2.RX_TimeOut > 0) {
            //超时计数
            if(--COM2.RX_TimeOut == 0) {
                if(COM2.RX_Cnt > 0) {
                    for(i=0; i<COM2.RX_Cnt; i++)	{
                        // RX2_Buffer[i]存的是接收的数据，写出用 TX2_write2buff
                        TX1_write2buff(RX2_Buffer[i]);
                    }
                }
                COM2.RX_Cnt = 0;
            }
        }

        os_wait2(K_TMO, 1); // 5ms * 1
    }
}
#endif



/*
 * 巡线测试
 */
#if 1
void Track_Task(void) RTX_TASK(Track_Task_Id) {
    int pos = 0;
    int speed = 15;
    while (1) {
        pos = Track_get_position();
        if(pos <= -64) { // 最左边
            // printf("最左边\n");
            CarMotors_turn(speed + 5, LEFT_M);
        } else if(pos <= -48) { // 最左边的两个灯都压着
            // printf("最左边的两个灯都压着\n");
            CarMotors_turn(speed + 10, LEFT_M);
        } else if(pos <= -32) { // 左边的第一个的灯
            // printf("左边的第一个的灯\n");
            CarMotors_turn(speed + 15, LEFT_M);
        } else if(pos <= -16) { // 中间和左边第二个灯压着
            // printf("中间和左边第二个灯压着\n");
            CarMotors_turn(speed + 20, LEFT_M);
        } else if(pos == 0) { // 中间的灯
            // printf("中间的灯\n");
            CarMotors_forward(speed + 25, MID_M);
        } else if(pos >= 64) { // 最右边的灯
            // printf("最右边的灯\n");
            CarMotors_turn(speed + 5, RIGHT_M);
        } else if(pos >= 48) { // 最右边的两个灯都压着
            // printf("最右边的两个灯都压着\n");
            CarMotors_turn(speed + 10, RIGHT_M);
        } else if(pos >= 32) { // 右边的第1个灯
            // printf("右边的第1个灯\n");
            CarMotors_turn(speed + 15, RIGHT_M);
        } else if(pos >= 16) {  // 中间和右边第二个灯压着线
            // printf("中间和右边第二个灯压着线\n");
            CarMotors_turn(speed + 20, RIGHT_M);
        }
        os_wait2(K_TMO, 2);
    }
}
#endif



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
        // 创建巡线任务
        os_create_task(Track_Task_Id);
        break;
    case 2:
        // 销毁巡线任务
        os_delete_task(Track_Task_Id);
        CarMotors_stop();   // 销毁任务后并不会关闭电机
        break;
    default:  break;
    }
    flag++;
    if (flag > 2) flag = 1;
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

