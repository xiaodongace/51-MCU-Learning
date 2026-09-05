#include "Nixie.h"
#include "Delay.h"

#define GET_BIT_VAL(byte, pos)	(byte & (1 << pos))

//#define NOP_TIME() NOP40()	// 用于看logic分析仪
#define NOP_TIME() NOP2()

// 锁存操作 - 多行宏定义
#define RCK_ACTION() 		\
NIXIE_RCK = 0;		\
NOP_TIME();			\
NIXIE_RCK = 1;		\
NOP_TIME();


// 初始化数码管模块。
void Nixie_Init(void) {
    NIXIE_PIN_INIT(); // 将数码管通信引脚配置为普通推挽输出。
}


static void NIXIE_out(u8 dat) {
    char i;
    // 8bit，先发出去的会作为高位
    for (i = 7; i >= 0; i--) {
        NIXIE_DI = GET_BIT_VAL(dat, i);
        // 寄存器的移位操作
        NIXIE_SCK = 0;
        NOP_TIME(); // 休眠一会儿
        NIXIE_SCK = 1;
        NOP_TIME(); // 休眠一会儿
    }
}

/* 向数码管驱动芯片发送段码和位选码，并锁存显示数据。 */
void NIXIE_show(u8 a_dat, u8 b_idx) {
    NIXIE_out(a_dat); // 发送段码，控制a~g和小数点的亮灭。
    NIXIE_out(b_idx); // 发送位选码，控制哪一位或哪几位显示。
    RCK_ACTION(); // 产生锁存脉冲，使移位数据正式输出。
}


static u8 code LED_TABLE[] =
{
    // 0 	1	 2	-> 9	(索引012...9)
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
    // 0. 1. 2. -> 9.	(索引10,11,12....19)
    0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10,
    // . -						(索引20,21)
    0x7F, 0xBF,
    // AbCdEFHJLPqU		(索引22,23,24....33)
    0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E, 0x89, 0xF1, 0xC7, 0x8C, 0x98, 0xC1
};

/* 在指定的一个数码管位置显示LED_TABLE中的字符。 */
void NIXIE_display(u8 num, u8 id) {
    u8 a_dat = LED_TABLE[num]; // 根据字符索引取得对应的段码。
    u8 b_idx = 1 << id; // 生成位选码，id范围为0~7。

    NIXIE_show(a_dat, b_idx);
}

/* 让8个数码管同时按照a、b、c、d、e、f顺序点亮外围段。 */
void Nixie_Running_Test(void) {
    u8 i;
    static u8 code perimeter_segments[] = {
        0xFE, // a段：顶部
        0xFD, // b段：右上
        0xFB, // c段：右下
        0xF7, // d段：底部
        0xEF, // e段：左下
        0xDF // f段：左上
    };

    Nixie_Init(); // 初始化数码管的串行数据、时钟和锁存引脚。

    while (1) {
        for (i = 0; i < 6; i++) {
            NIXIE_show(perimeter_segments[i], 0xFF); // 选中8位，并让每一位显示当前外围段。
            delay_ms(100); // 控制跑马灯速度，每段保持100ms。
        }
    }
}
