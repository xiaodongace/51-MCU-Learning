#include "IIC.h"
#include "GPIO.h"
#include "I2C.h"
#include "NVIC.h"
#include "Switch.h"


#define NUMBER	7

#define DECIMAL_2_HEX(num) (((num / 10) << 4) | (num % 10));
// bit为num右移后&操作的数字
#define BCD_2_DECIMAL(num, bit) ((num >> 4) & bit) * 10 + (num & 0x0F);

/* 配置I2C总线使用的SCL和SDA引脚。 */
static void IIC_GPIO_Config(void) {
    /* I2C只使用P3.2和P3.3，不在这里配置UART的P3.0和P3.1。 */
    P3_MODE_OUT_OD(GPIO_Pin_2 | GPIO_Pin_3);
}

/* 配置I2C主机模式、总线速度、引脚映射和中断状态。 */
static void IIC_Config(void) {
    I2C_InitTypeDef I2C_InitStructure;

    I2C_InitStructure.I2C_Mode = I2C_Mode_Master; //主从选择   I2C_Mode_Master, I2C_Mode_Slave
    I2C_InitStructure.I2C_Enable = ENABLE; //I2C功能使能,   ENABLE, DISABLE
    I2C_InitStructure.I2C_MS_WDTA = DISABLE; //主机使能自动发送,  ENABLE, DISABLE
    I2C_InitStructure.I2C_Speed = 13; //总线速度=Fosc/2/(Speed*2+4),      0~63
    // 400K = 24M / 2 / (Speed * 2 + 4):
    // 400  = 12000 / (Speed * 2 + 4)
    // Speed * 2   = 26
    I2C_Init(&I2C_InitStructure);
    NVIC_I2C_Init(I2C_Mode_Master,DISABLE,Priority_0);
    //主从模式, I2C_Mode_Master, I2C_Mode_Slave; 中断使能, ENABLE/DISABLE; 优先级(低到高) Priority_0,Priority_1,Priority_2,Priority_3

    I2C_SW(I2C_P33_P32); //I2C_P14_P15,I2C_P24_P25,I2C_P33_P32
}


void IIC_Init(void) {
    /* 只初始化I2C总线，不隐式初始化UART、外部中断或显示模块。 */
    EAXSFR();
    IIC_GPIO_Config();
    IIC_Config();
}

/* 输出RTC闹钟事件的测试信息。 */
static void I2C_On_Alarm(void) {
    printf("Alarm...\n");
}

/* 输出RTC定时器事件的测试信息。 */
static void I2C_On_Timer(void) {
    printf("Timer...\n");
}


void I2C_Set_Clock(Clock_t clock) {
    // 读写数据数组
    u8 p[NUMBER] = {0};
    u8 C;

    // 秒: VL 1 1 1 - 0 0 0 0 十进制数 -> BCD  56 -> 0x56
    p[0] = DECIMAL_2_HEX(clock.second); // 秒 (十位 << 4) | 个位
    // 分:  x 1 1 1 - 0 0 0 0 十进制数 -> BCD  59 -> 0x59
    p[1] = DECIMAL_2_HEX(clock.minute); // 分钟
    // 时:  x x 1 1 - 0 0 0 0 十进制数 -> BCD  23 -> 0x23
    p[2] = DECIMAL_2_HEX(clock.hour); // 小时
    // 日:  x x 1 1 - 0 0 0 0 十进制数 -> BCD  31 -> 0x31
    p[3] = DECIMAL_2_HEX(clock.day); // 天
    // 周:  x x x x - x 0 0 0 十进制数 -> BCD  6 -> 0x06
    p[4] = clock.week; // 周 0,1,2,3,4,5,6

    // 世纪
    C = (clock.year >= 2100) ? 1 : 0; // year >= 2100
    // 月:  C x x 1 - 0 0 0 0 十进制数 -> BCD 12 -> 0x12
    p[5] = (C << 7) | DECIMAL_2_HEX(clock.month); // 月
    // 年:  1 1 1 1 - 0 0 0 0 十进制数 -> BCD  26 -> 0x26
    p[6] = DECIMAL_2_HEX((clock.year % 100)); // 年

    // 进行一次性的写入
    I2C_WriteNbyte(PCF8563_ADDR, PCF8563_REG, p, NUMBER);
}


void I2C_Get_Clock(Clock_t* p_clock) {
    // 读写数据数组
    u8 p[NUMBER] = {0};
    u8 C;

    // 读取 秒,分,时,天,周,月,年  世纪
    I2C_ReadNbyte(PCF8563_ADDR, PCF8563_REG, p, NUMBER);
    // 秒: VL 1 1 1 - 0 0 0 0  BCD->十进制数 0x56 -> 56
    p_clock->second = BCD_2_DECIMAL(p[0], 0x07);
    // 分:  x 1 1 1 - 0 0 0 0  BCD->十进制数 0x59 -> 59
    p_clock->minute = BCD_2_DECIMAL(p[1], 0x07);
    // 时:  x x 1 1 - 0 0 0 0  BCD->十进制数 0x23 -> 23
    p_clock->hour = BCD_2_DECIMAL(p[2], 0x03);

    // 日:  x x 1 1 - 0 0 0 0  BCD->十进制数 0x31 -> 31
    p_clock->day = BCD_2_DECIMAL(p[3], 0x03);
    // 周:  x x x x - x 0 0 0  BCD->十进制数 0x06 ->  6
    p_clock->week = p[4] & 0x07;
    // 月:  C x x 1 - 0 0 0 0  BCD->十进制数 0x12 -> 12
    p_clock->month = BCD_2_DECIMAL(p[5], 0x01);

    C = p[5] >> 7; // 0->20xx年  1->21xx年

    // 年:  1 1 1 1 - 0 0 0 0  BCD->十进制数 0x26 -> 26
    p_clock->year = BCD_2_DECIMAL(p[6], 0x0F);
    p_clock->year += ((C == 0) ? 2000 : 2100);
}



// 设置闹钟
void I2C_Set_Alarm(Alarm_t alarm) {
    u8 a[4] = {0x80, 0x80, 0x80, 0x80}; // 默认禁用
    // 设置闹铃时间: 09h分钟, 0Ah小时, 0Bh天, 0Ch周 (最高0: 启动)
    // 分:  M 1 1 1 - 0 0 0 0 十进制数 -> BCD 最高位 启用0x00, 禁用0x80 (1<<7)
    if (alarm.minute >= 0) {
        a[0] = 0x00 | DECIMAL_2_HEX(alarm.minute);
    }

    // 时:  M x 1 1 - 0 0 0 0 十进制数 -> BCD 最高位 启用0x00, 禁用0x80 (1<<7)
    if (alarm.hour >= 0) {
        a[1] = 0x00 | DECIMAL_2_HEX(alarm.hour);
    }

    // 日:  M x 1 1 - 0 0 0 0 十进制数 -> BCD 最高位 启用0x00, 禁用0x80 (1<<7)
    if (alarm.day >= 0) {
        a[2] = 0x00 | DECIMAL_2_HEX(alarm.day);
    }

    // 周:  M x x x - x 0 0 0 十进制数 -> BCD 最高位 启用0x00, 禁用0x80 (1<<7)
    if (alarm.week >= 0) {
        a[3] = 0x00 | alarm.week;
    }

    // 进行一次性的写入
    I2C_WriteNbyte(PCF8563_ADDR, 0x09, a, 4);
}



// 启用闹钟
void I2C_Enable_Alarm(u8 enable) {
    u8 cs2 = 0;
    // 配置控制寄存器2 (CS2)  AF=0, AIE=1 启用闹钟-----------------
    I2C_ReadNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
    // AF -> 清理Alarm中断标记 Alarm Flag Bit3清0, 确保闹铃中断触发
    cs2 &= ~(1 << 3);
    // AIE-> 开启Alarm中断 Bit1置1, Alarm Interrupt Enable
    if (enable) {
        cs2 |= (1 << 1);
    }
    else {
        cs2 &= ~(1 << 1);
    }
    I2C_WriteNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
}



// 清理闹钟标记
void I2C_Clear_Alarm(void) {
    u8 cs2 = 0;
    // 配置控制寄存器2 (CS2)  AF=0, AIE=1 启用闹钟-----------------
    I2C_ReadNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
    // AF -> 清理Alarm中断标记 Alarm Flag Bit3清0, 确保闹铃中断触发
    cs2 &= ~(1 << 3);
    // 把修改后的信息写回0x01寄存器
    I2C_WriteNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
}



// 设置定时器
void I2C_Set_Timer(TimerFreq_t freq, u8 conut) {
    u8 p;
    //    3. 设置Timer运行频率 & 启用Timer
    p = (1 << 7) | freq; // 4096Hz, 64Hz, 1Hz, 1/60Hz
    I2C_WriteNbyte(PCF8563_ADDR, 0x0E, &p, 1);

    //    4. 设置Timer计数值n
    p = conut;
    I2C_WriteNbyte(PCF8563_ADDR, 0x0F, &p, 1);
}



// 启用定时器
void I2C_Enable_Timer(u8 enable) {
    u8 cs2 = 0;
    //    5. 设置cs2, TIE置1启用, 清理TF标记
    // 配置控制寄存器2 (CS2)  AF=0, AIE=1 启用闹钟-----------------
    I2C_ReadNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
    // TF -> 清理Timer中断标记 Timer Flag Bit2清0, 确保定时器中断触发
    cs2 &= ~(1 << 2);
    // TIE-> 开启Timer中断 Bit1置1, Timer Interrupt Enable
    if (enable) {
        cs2 |= (1 << 0);
    }
    else {
        cs2 &= ~(1 << 0);
    }
    I2C_WriteNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
}

// 清理定时器标记
void I2C_Clear_Timer(void) {
    u8 cs2 = 0;
    I2C_ReadNbyte(PCF8563_ADDR, 0x01, &cs2, 1);

    // TF -> 清理Timer中断标记 Timer Flag Bit2清0, 确保定时器中断触发
    cs2 &= ~(1 << 2);
    I2C_WriteNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
}


void on_int3_call(void) {
    u8 cs2 = 0;

    // 读取cs2控制寄存器的值, 查看AF和TF标记
    I2C_ReadNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
    if ((cs2 >> 3) & 0x01) {
        // AF: Bit3
        I2C_On_Alarm();
        // 判断并清理Alarm的AF标记
        cs2 &= ~(1 << 3);
    }
    //  判断并清理Timer的TF标记
    if (cs2 & (1 << 2)) {
        // TF: Bit2
        I2C_On_Timer();
        // TF -> 清理Timer中断标记 Timer Flag Bit2清0, 确保定时器中断触发
        cs2 &= ~(1 << 2);
    }
    // 最后统一写入修改后的信息
    I2C_WriteNbyte(PCF8563_ADDR, 0x01, &cs2, 1);
}
