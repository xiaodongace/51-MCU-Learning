#include "Config.h"
#include "SelfTest.h"
#include "AdcNixieLab.h"

/*
 * Demo练习main函数入口
 */
void main_start(void) RTX_TASK(0)
{
    AdcNixieLab_Init();
    while (1) {
        AdcNixieLab_Task();
    }
}


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
