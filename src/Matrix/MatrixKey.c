#include "MatrixKey.h"
#include "GPIO.h"

#define UP   1
#define DOWN 0
#define ROW_NUM 4
#define COL_NUM 4

// 行
#define ROW1    P34
#define ROW2    P35
#define ROW3    P40
#define ROW4    P41

// 列
#define COL1    P03
#define COL2    P06
#define COL3    P07
#define COL4    P17

// u8 states[] = {
//     1, 1, 1, 1, // 0, 1, 2, 3
//     1, 1, 1, 1, // 4, 5, 6, 7
//     1, 1, 1, 1, // 8, 9,10,11
//     1, 1, 1, 1, //12,13,14,15
// };

/* 保存16个矩阵按键的上一次电平，1表示松开、0表示按下。 */
static u16 states = 0xFFFF;

/* 按键状态变化后的业务回调，由外部Lab按需设置。 */
static MK_func_cb matrix_key_callback = NULL;

// 获取指定按键的状态
#define GET_STATUS(pos) ((states >> pos) & 1)

// 设置指定按键的状态
#define SET_STATUS_UP(pos)      (states |= (1 << pos))
#define SET_STATUS_DOWN(pos)    (states &= ~(1 << pos))

// #define SET_STATUS(pos, new_status) (new_status ? SET_STATUS_UP(pos) : SET_STATUS_DOWN(pos))
// 将原来的位置清零 根据最后pos位是0是1决定新状态
#define SET_STATUS(pos, new_status) states = states & (~(1 << pos)) | (new_status << pos)

void MatrixKey_Init(void) {
    /* 只配置矩阵键盘使用的行列GPIO，串口调试由调用者自行初始化。 */
    P0_MODE_IO_PU(GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7);
    P1_MODE_IO_PU(GPIO_Pin_7);
    P3_MODE_IO_PU(GPIO_Pin_4 | GPIO_Pin_5);
    P4_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1);

    /* 初始化为全部松开，避免上电后产生虚假的状态变化。 */
    states = 0xFFFF;
}

/* 将指定扫描行拉低，并将其余行保持为高电平。 */
static void ROW_OUT(u8 row) {
    ROW1 = (row == 0) ? 0 : 1;
    ROW2 = (row == 1) ? 0 : 1;
    ROW3 = (row == 2) ? 0 : 1;
    ROW4 = (row == 3) ? 0 : 1;
}


/* 读取指定矩阵键盘列的当前电平。 */
static u8 COL_IN(u8 col) {
    if (col == 0) return COL1;
    if (col == 1) return COL2;
    if (col == 2) return COL3;
    if (col == 3) return COL4;
    return 0;
}

/* 设置矩阵按键状态变化后的回调函数。 */
void MatrixKey_SetCallback(MK_func_cb callback) {
    /* 只保存函数地址；如何处理按键事件由Lab决定。 */
    matrix_key_callback = callback;
}

/* 扫描16个矩阵按键，并在状态变化时通知已注册的回调函数。 */
void MK_Scan(void) {
    // row=行    col=列   pos=按键在数组中的位置
    u8 row = 0, col = 0, pos = 0;
    // 外循环得到所有行
    for (row = 0; row < ROW_NUM; row++) {
        // 把第row行拉低 其他拉高
        ROW_OUT(row);
        // 等待电平稳定
        NOP2();
        // 内循环得到所有列
        for (col = 0; col < COL_NUM; col++) {
            // 计算行列坐标 查看当前按键在数组中的位置
            // 计算公式 (行号 * 每行列数) + 列号
            pos = (row * COL_NUM) + col;

            if (GET_STATUS(pos) != COL_IN(col)) {
                SET_STATUS(pos, COL_IN(col));

                if (COL_IN(col) == DOWN) {
                    /* 低电平表示按下，第三个参数0表示不是抬起事件。 */
                    if (matrix_key_callback != NULL) {
                        matrix_key_callback(row, col, 0);
                    }
                }
                else {
                    /* 高电平表示松开，第三个参数1表示抬起事件。 */
                    if (matrix_key_callback != NULL) {
                        matrix_key_callback(row, col, 1);
                    }
                }
            }
        }
    }
}
