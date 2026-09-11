#include "Ultrasonic.h"

void Ultrasonic_init() {
    // TRIG设置为推挽输出
    P4_MODE_OUT_PP(GPIO_Pin_7);

    TRIG = 0; // 拉低 方便触发起始信号
    // ECHO设置为高阻输入
    P3_MODE_IN_HIZ(GPIO_Pin_3);
}

void Delay10us(void)	//@24.000MHz
{
    unsigned char data i;

    i = 78;
    while (--i);
}


// 返回值为char，因为有负数，代表不同的状态，返回0，才代表成功获取距离
char Ultrasonic_get_distance(float *distance) {
    u16 cnt = 0;
    // 拉高起始信号
    TRIG = 1;
    Delay10us(); Delay10us();
    TRIG = 0;

    // 计算echo低电平的时间，(当echo变高电平时，退出循环)
    while (ECHO == 0 && cnt < 500) {
        cnt++;
        Delay10us();
    }
    if (cnt >= 500) return -1;

    // 收到高电平信号 计算高电平时间
    while (ECHO == 1 && cnt < 3000) {
        cnt++;
        Delay10us();
    }
    if (cnt >= 3000) return -2;

    // 计算距离：测试距离= (高电平时间*声速(340M/S))/2 要除以2，因为声音有来回
    // cnt 是高电平的时间  1个cnt 为 10 us 为0.01ms
    // dis = ((cnt * 0.01)ms * 340m/s) / 2
    // dis = ((cnt * 0.01)ms * 34000cm/1000ms) / 2
    // dis = ((cnt * 0.01)ms * 34cm/ms) / 2
    *distance = ((cnt * 0.01) * 34) / 2;

    if (*distance < 2 || *distance > 400) return -3;

    return 0;
}