#include "Config.h"
#include "SelfTest.h"

/*
 * 0.所有LED闪烁        Task_LED
 * 1.热敏电阻           Task_
 * 2.电位器+马达        Task_Motor
 * 3.RTC时钟           Task_RTC
 * 4.数码管            Task_NIXIE
 * 5.温湿度            Task_Temperature
 * 6.键盘蜂鸣器         Task_Buzzer
 */

/*
 * RTX51 Tiny 的任务 0 仅做一次性初始化和任务创建。
 *
 * 任务 1（SelfTest_InputTask）：扫描 key1~key4，产生界面事件。
 * 任务 2（SelfTest_ViewTask）：独占初始化并刷新 SPI 菜单和 I2C 详情屏。
 */
void main_start(void) RTX_TASK(0)
{
    SelfTest_Init();

    os_create_task(1);
    os_create_task(2);
    os_delete_task(0);
}
