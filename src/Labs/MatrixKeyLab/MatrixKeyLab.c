#include "MatrixKeyLab.h"
#include "MatrixKey.h"
#include "UARTS.h"

/* 将一次矩阵按键状态变化格式化后输出到串口。 */
void MK_Callback(u8 row, u8 col, u8 is_keyup) {
    /* 回调只负责演示业务，矩阵键盘驱动本身不再依赖串口模块。 */
    if (is_keyup != 0) {
        printf("Key %d行%d列 -> UP!!!\n", (int)(row + 1), (int)(col + 1));
    }
    else {
        printf("Key %d行%d列 -> DOWN!!!\n", (int)(row + 1), (int)(col + 1));
    }
}

/* 初始化矩阵键盘串口打印测试。 */
void MatrixKeyLab_Init(void) {
    /* UART和矩阵键盘分别初始化，再由Lab把打印回调连接到按键驱动。 */
    UART_Init();
    MatrixKey_Init();
    MatrixKey_SetCallback(MK_Callback);
}

/* 扫描矩阵键盘并打印本轮产生的状态变化。 */
void MatrixKeyLab_Task(void) {
    /* 扫描函数发现边沿后会调用初始化时注册的MK_Callback()。 */
    MK_Scan();
}
