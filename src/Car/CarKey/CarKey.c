#include "CarKey.h"
#include "Battery.h"
#include "CarBuzzer.h"

#define DOWN  0
#define UP    1

static u8 last_state = 1;

// 初始化
void CarKey_init() {
    // 配置开漏
    P0_MODE_OUT_OD(5);
}

// 扫描按键
void CarKey_scan() {
#if 0
    if (last_state == UP && KEY == DOWN) {
        last_state = DOWN;
        #if USE_KEYDOWN
        CarKey_on_keydown();
        #endif
    } else if (last_state == DOWN && KEY == UP) {
        last_state = UP;
        #if USE_KEYUP
        CarKey_on_keyup();
        #endif
    }
#else
    if (last_state != KEY) {
        last_state = KEY;
        if (KEY == DOWN) {
            #if USE_KEYDOWN
            CarKey_on_keydown();
            #endif
        } else {
            #if USE_KEYUP
            CarKey_on_keyup();
            #endif
        }
    }
#endif
}

// 函数指针版本
void CarKey_scan2(void (*down)(), void (*up)()) {
    if (last_state != KEY) {
        last_state = KEY;
        if (KEY == DOWN) {
            if (down != NULL) down();
        } else {
            if (up != NULL) up();
        }
    }
}


