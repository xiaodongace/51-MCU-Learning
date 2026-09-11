#include "Light.h"

// 初始化
void Light_Init() {
    // 准双向
    P0_MODE_IO_PU(GPIO_Pin_7);
    P5_MODE_IO_PU(GPIO_Pin_2);

    // 推挽输出
    // P0_MODE_OUT_PP(GPIO_Pin_7);
    // P5_MODE_OUT_PP(GPIO_Pin_2);
}

// 开灯
void Light_On(Light temp) {
    if (temp == LEFT) {
        LED_LEFT = 1;
    } else if (temp == RIGHT) {
        LED_RIGHT = 1;
    } else {
        LED_LEFT = 1;
        LED_RIGHT = 1;
    }
}

// 关灯
void Light_Off(Light temp) {
    if (temp == LEFT) {
        LED_LEFT = 0;
    } else if (temp == RIGHT) {
        LED_RIGHT = 0;
    } else {
        LED_LEFT = 0;
        LED_RIGHT = 0;
    }
}
