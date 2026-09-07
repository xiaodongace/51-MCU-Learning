#include "SpiOled.h"
#include "Delay.h"
#include "GPIO.h"

/*
 * 0.96 英寸 SPI OLED 与字库芯片的实际接线，来自提供的移植示例。
 * P1.0 同时连接马达 PWM6，因此马达测试期间菜单必须停止刷新。
 */
#define SPI_OLED_SCLK       P50
#define SPI_OLED_MOSI       P13
#define SPI_OLED_DC         P16
#define SPI_OLED_CS         P47
#define SPI_OLED_FONT_MISO  P11
#define SPI_OLED_FONT_CS    P10

#define SPI_OLED_CMD        0
#define SPI_OLED_DATA       1
#define SPI_OLED_MENU_COUNT 7

/* 菜单文本为 GB2312 字节，供外接字库芯片读取；每行以 0 结束。 */
static u8 code menu_led[]    = {0x30, 0x2e, 0xcb, 0xf9, 0xd3, 0xd0, 'L', 'E', 'D', 0xc9, 0xc1, 0xcb, 0xb8, 0};
static u8 code menu_ntc[]    = {0x31, 0x2e, 0xc8, 0xc8, 0xc3, 0xf4, 0xb5, 0xe7, 0xd7, 0xe8, 0};
static u8 code menu_motor[]  = {0x32, 0x2e, 0xb5, 0xe7, 0xce, 0xbb, 0xc6, 0xf7, '+', 0xc2, 0xed, 0xb4, 0xef, 0};
static u8 code menu_rtc[]    = {0x33, 0x2e, 'R', 'T', 'C', 0xca, 0xb1, 0xd6, 0xd3, 0};
static u8 code menu_nixie[]  = {0x34, 0x2e, 0xca, 0xfd, 0xc2, 0xeb, 0xb9, 0xdc, 0};
static u8 code menu_dht[]    = {0x35, 0x2e, 0xce, 0xc2, 0xca, 0xaa, 0xb6, 0xc8, 0};
static u8 code menu_buzzer[] = {0x36, 0x2e, 0xbc, 0xfc, 0xc5, 0xcc, 0xb7, 0xe4, 0xc3, 0xf9, 0xc6, 0xf7, 0};

static void SpiOled_WriteByte(u8 data_byte, u8 mode)
{
    u8 i;

    SPI_OLED_DC = mode;
    SPI_OLED_CS = 0;

    for (i = 0; i < 8; i++) {
        SPI_OLED_SCLK = 0;
        SPI_OLED_MOSI = (data_byte & 0x80) ? 1 : 0;
        SPI_OLED_SCLK = 1;
        data_byte <<= 1;
    }

    SPI_OLED_CS = 1;
    SPI_OLED_DC = 1;
}

static void SpiOled_SetPos(u8 x, u8 page)
{
    SpiOled_WriteByte(0xb0 + page, SPI_OLED_CMD);
    SpiOled_WriteByte(((x & 0xf0) >> 4) | 0x10, SPI_OLED_CMD);
    SpiOled_WriteByte(x & 0x0f, SPI_OLED_CMD);
}

/* 向外接字库发送一个字节。 */
static void SpiOled_FontSend(u8 data_byte)
{
    u8 i;

    for (i = 0; i < 8; i++) {
        SPI_OLED_SCLK = 0;
        SPI_OLED_MOSI = (data_byte & 0x80) ? 1 : 0;
        SPI_OLED_SCLK = 1;
        data_byte <<= 1;
    }
}

/* 从外接字库读取一个字节。 */
static u8 SpiOled_FontRead(void)
{
    u8 i;
    u8 value = 0;

    for (i = 0; i < 8; i++) {
        SPI_OLED_SCLK = 0;
        value <<= 1;
        if (SPI_OLED_FONT_MISO != 0) {
            value++;
        }
        SPI_OLED_SCLK = 1;
    }

    return value;
}

/* 从字库芯片读取指定地址的连续字节。 */
static void SpiOled_ReadFont(u32 address, u8 *buffer, u8 length)
{
    u8 i;

    SPI_OLED_FONT_CS = 0;
    SpiOled_FontSend(0x03);
    SpiOled_FontSend((u8)(address >> 16));
    SpiOled_FontSend((u8)(address >> 8));
    SpiOled_FontSend((u8)address);

    for (i = 0; i < length; i++) {
        buffer[i] = SpiOled_FontRead();
    }

    SPI_OLED_FONT_CS = 1;
}

static void SpiOled_Draw16x16(u8 x, u8 page, u8 *font)
{
    u8 i;
    u8 column;

    for (i = 0; i < 2; i++) {
        SpiOled_SetPos(x, page + i);
        for (column = 0; column < 16; column++) {
            SpiOled_WriteByte(*font++, SPI_OLED_DATA);
        }
    }
}

static void SpiOled_Draw8x16(u8 x, u8 page, u8 *font)
{
    u8 i;
    u8 column;

    for (i = 0; i < 2; i++) {
        SpiOled_SetPos(x, page + i);
        for (column = 0; column < 8; column++) {
            SpiOled_WriteByte(*font++, SPI_OLED_DATA);
        }
    }
}

/* 显示混合 GB2312/ASCII 的一行 16 像素高文字。 */
static void SpiOled_ShowGb2312(u8 x, u8 page, u8 code *text)
{
    u8 font[32];
    u32 address;

    while (*text != 0) {
        if ((*text >= 0xb0) && (*text <= 0xf7) && (*(text + 1) >= 0xa1)) {
            address = ((u32)(*text - 0xb0) * 94UL + (u32)(*(text + 1) - 0xa1) + 846UL) * 32UL;
            SpiOled_ReadFont(address, font, 32);
            SpiOled_Draw16x16(x, page, font);
            x += 16;
            text += 2;
        }
        else if ((*text >= 0x20) && (*text <= 0x7e)) {
            address = ((u32)(*text - 0x20) * 16UL) + 0x3cf80UL;
            SpiOled_ReadFont(address, font, 16);
            SpiOled_Draw8x16(x, page, font);
            x += 8;
            text++;
        }
        else {
            text++;
        }
    }
}

static u8 code *SpiOled_MenuText(u8 index)
{
    switch (index) {
    case 0: return menu_led;
    case 1: return menu_ntc;
    case 2: return menu_motor;
    case 3: return menu_rtc;
    case 4: return menu_nixie;
    case 5: return menu_dht;
    default: return menu_buzzer;
    }
}

void SpiOled_Clear(void)
{
    u8 page;
    u8 column;

    for (page = 0; page < 8; page++) {
        SpiOled_SetPos(0, page);
        for (column = 0; column < 128; column++) {
            SpiOled_WriteByte(0x00, SPI_OLED_DATA);
        }
    }
}

void SpiOled_Init(void)
{
    /* OLED、字库共用软件 SPI；均使用普通 GPIO 推挽输出。 */
    P1_MODE_IO_PU(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_6);
    P4_MODE_IO_PU(GPIO_Pin_7);
    P5_MODE_IO_PU(GPIO_Pin_0);

    SPI_OLED_SCLK = 1;
    SPI_OLED_MOSI = 1;
    SPI_OLED_DC = 1;
    SPI_OLED_CS = 1;
    SPI_OLED_FONT_CS = 1;
    delay_ms(200);

    SpiOled_WriteByte(0xae, SPI_OLED_CMD);
    SpiOled_WriteByte(0x00, SPI_OLED_CMD);
    SpiOled_WriteByte(0x10, SPI_OLED_CMD);
    SpiOled_WriteByte(0x40, SPI_OLED_CMD);
    SpiOled_WriteByte(0x81, SPI_OLED_CMD);
    SpiOled_WriteByte(0xcf, SPI_OLED_CMD);
    SpiOled_WriteByte(0xa1, SPI_OLED_CMD);
    SpiOled_WriteByte(0xc8, SPI_OLED_CMD);
    SpiOled_WriteByte(0xa6, SPI_OLED_CMD);
    SpiOled_WriteByte(0xa8, SPI_OLED_CMD);
    SpiOled_WriteByte(0x3f, SPI_OLED_CMD);
    SpiOled_WriteByte(0xd3, SPI_OLED_CMD);
    SpiOled_WriteByte(0x00, SPI_OLED_CMD);
    SpiOled_WriteByte(0xd5, SPI_OLED_CMD);
    SpiOled_WriteByte(0x80, SPI_OLED_CMD);
    SpiOled_WriteByte(0xd9, SPI_OLED_CMD);
    SpiOled_WriteByte(0xf1, SPI_OLED_CMD);
    SpiOled_WriteByte(0xda, SPI_OLED_CMD);
    SpiOled_WriteByte(0x12, SPI_OLED_CMD);
    SpiOled_WriteByte(0xdb, SPI_OLED_CMD);
    SpiOled_WriteByte(0x40, SPI_OLED_CMD);
    SpiOled_WriteByte(0x20, SPI_OLED_CMD);
    SpiOled_WriteByte(0x02, SPI_OLED_CMD);
    SpiOled_WriteByte(0x8d, SPI_OLED_CMD);
    SpiOled_WriteByte(0x14, SPI_OLED_CMD);
    SpiOled_WriteByte(0xa4, SPI_OLED_CMD);
    SpiOled_WriteByte(0xa6, SPI_OLED_CMD);
    SpiOled_Clear();
    SpiOled_WriteByte(0xaf, SPI_OLED_CMD);
}

void SpiOled_ShowMenu(u8 selected_item)
{
    u8 first_item;
    u8 row;
    u8 index;

    if (selected_item >= SPI_OLED_MENU_COUNT) {
        selected_item = 0;
    }

    /* 中文为 16 像素高，屏幕一页只能显示四项；选择后半段时滚动。 */
    first_item = (selected_item <= 3) ? 0 : 3;
    SpiOled_Clear();

    for (row = 0; row < 4; row++) {
        index = first_item + row;
        if (index >= SPI_OLED_MENU_COUNT) {
            break;
        }

        /* 使用 ASCII 箭头标识当前选项，文字从 x=8 开始。 */
        if (index == selected_item) {
            static u8 code mark[] = {'>', 0};
            SpiOled_ShowGb2312(0, row * 2, mark);
        }
        SpiOled_ShowGb2312(8, row * 2, SpiOled_MenuText(index));
    }
}
