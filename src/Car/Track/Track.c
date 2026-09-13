#include "Track.h"


// 初始化
void Track_init() {
    // 配置准双向
    P0_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
}


static u8 last_pos = 0; // 上一次状态

// 获取寻迹坐标： 高电平-不亮-压到黑线  低电平-亮起-正常反射面
int Track_get_position() {
    int pos = 0;    // 当前坐标
    u8 cnt = 0;     // 压到黑线个数

    if (LED1 == 1) {
        pos += -64;
        cnt++;
    }

    if (LED2 == 1) {
        pos += -32;
        cnt++;
    }

    if (LED3 == 1) {
        // pos += 0;
        cnt++;
    }

    if (LED4 == 1) {
        pos += 32;
        cnt++;
    }

    if (LED5 == 1) {
        pos += 64;
        cnt++;
    }

    if (cnt == 0) {
        last_pos = pos;
    }

    return pos / cnt;
}


