#include "DHT11.h"
#include "Delay.h"
#include "GPIO.h"

#define DHT P46

/*************************************************************************************
DHT11 数字温度传感器

使用P46(单总线, 准双向): 发起开始信号, 读取5字节数据, 主机拉高释放总线

串口触发等组合行为由业务Lab负责，本模块只负责DHT11通信和数据解析。

*************************************************************************************/
static void DHT11_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Mode = GPIO_PullUp;
    GPIO_InitStructure.Pin = GPIO_Pin_6;
    GPIO_Inilize(GPIO_P4, &GPIO_InitStructure);

}

/* 初始化DHT11单总线引脚。 */
void DHT11_Init(void) {
    /* DHT11只使用P4.6，不在传感器模块中初始化UART引脚或串口外设。 */
    DHT11_GPIO_Init();
}


static void DHT11_Delay1us(void) {
    NOP7(); // 1us -> 1000ns
    //    NOP6();        // 1us -> 1000ns 多加最大范围判定时, 使用此NOP
    // 41.67ns * 12 = 500ns
}

// 等待电平变换
#define wait_level_change(level, min, max, desc)                                                         \
do{                                                                                                  \
    cnt = 0; /*确保开始是0*/                                                                          \
    while(DHT == level){                                                                             \
        /*每循环一次,代表过去了1us,通过cnt记录时间*/                                                   \
        DHT11_Delay1us();                                                                           \
        cnt++;                                                                                       \
    };                                                                                               \
    \
    /*不符合目标范围, 及时短路返回, 避免代码嵌套*/                                                     \
    if(cnt < min || cnt > max){                                                                      \
        printf("err: 时间[%dus], 不满足 %s[%dus, %dus]\n", cnt, desc, (int)min, (int)max);           \
        return -2;                                                                                      \
    }                                                                                                \
}while(0)


// 读取DHT11温湿度
static int8 DHT11_ReadRaw(u8* dat) {
    u16 cnt = 0; // 计数器, 每+1, 代表时间过了1us
    int8 i, j;


    // 1. 主机发起起始信号: 拉低 18ms, 30ms
    DHT = 0;
    delay_ms(20);
    DHT = 1;

    // 2. 主机释放总线 (13us, 35us)
    cnt = 0; // 确保开始是0
    while (DHT == 1 && cnt < 45) {
        // 每循环一次,代表过去了1us,通过cnt记录时间
        DHT11_Delay1us();
        cnt++;
    };
    // 如果不符合目标范围, 及时短路返回, 避免代码嵌套
    if (cnt < 6 || cnt > 35) {
        printf("err: 时间[%dus], 不满足 主机释放总线时间[%dus, %dus]\n", cnt, (int)6, (int)35);
        return -1;
    }

    // 不要在此过程中随意打日志, 因为会消耗时间, 影响cnt计数

    // 3. 响应低电平时间 83us, [78, 88]us, 当前0, 直到1, 结束循环
    wait_level_change(0, 78, 88, "响应信号低电平时间");

    // 4. 响应高电平时间 87us, [78, 88]us, 当前1, 直到0, 结束循环
    wait_level_change(1, 77, 95, "响应信号高电平时间");

    // 5. 解析40bit的数据(5Byte * 8bit)
    // 外循环: 1次, 接收处理1个byte字节(一共5个字节)
    for (i = 0; i < 5; i++) {
        // 0,1,2,3,4

        // 内循环: 1次, 接收处理1个bit位(每个字节8bit)
        for (j = 7; j >= 0; j--) {
            // 7,6,5,4,2,1,0 先收到高位
            // 一个bit信号由一低一高的电平组成: 低电平一样长(54us), 区别在于高电平

            // 数据信号: 低电平时间 54us [50, 58]us 当前0, 直到1
            wait_level_change(0, 46, 62, "Data信号低电平时间");

            // 数据信号: 高电平时间 [23, 74]us 当前1, 直到0
            wait_level_change(1, 23, 74, "Data信号高电平时间");

            // 信号0: cnt 24us左右 [23, 27]
            // 信号1: cnt 71us左右 [68, 74]
            // 假如收到的数据  0b 1001 1010 -> dat[i]
            // 0b 0 0 0 0 - 0 0 0 0 默认值
            // 0b 1 0 0 0 - 0 0 0 0 j = 7
            // 0b 1 0 0 0 - 0 0 0 0 j = 6
            // 0b 1 0 0 0 - 0 0 0 0 j = 5
            // 0b 1 0 0 1 - 0 0 0 0 j = 4
            // 0b 1 0 0 1 - 1 0 0 0 j = 3
            // 0b 1 0 0 1 - 1 0 0 0 j = 2
            // 0b 1 0 0 1 - 1 0 1 0 j = 1
            // 0b 1 0 0 1 - 1 0 1 0 j = 0

            // 通过高电平时长cnt, 区分是0还是1 (是0就不管, 默认dat存的都是0)
            // (24 + 71) / 2 = 47.5
            if (cnt > 47) {
                // 信号1: 指定置1
                dat[i] |= (1 << j);
            }
        }
    }
    // 主机拉高释放总线(可选)
    DHT = 1;

    // printf("cnt -> %d us\n", cnt);
    // // 打印5个字节的数据
    // printf("dat-> ");
    // for(i = 0; i < 5; i++){
    //     printf("%d ", (int)dat[i]);
    // }
    // printf("\n");
    if (((dat[0] + dat[1] + dat[2] + dat[3]) & 0xFF) != dat[4]) {
        printf("温度校验失败: %d!\n", (int)__LINE__);
        return -3;
    }

    printf("温度校验成功: %d\n", (int)__LINE__);

    return 0;
}


// 获取温湿度测试
void DHT11_Task(void) {
    float humidity; // 湿度
    float temperature; // 温度
    getHumidityAndTemperature(&humidity, &temperature);
}


// 获取温度和湿度
int8 getHumidityAndTemperature(float* p_humidity, float* p_temperature) {
    float humidity; // 湿度
    float temperature; // 温度
    u8 dat[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    int8 res = DHT11_ReadRaw(dat);

    if (res != SUCCESS) {
        printf("获取温湿度失败,错误码为 -> %d\n", (int)res);
        return res;
    }

    /*****************************************************
     温湿度获取数据格式：
          湿度值    忽略    温度值  温度后面小数
            41      0       26      70
    对应dat索引：
            0       1       2       3
     *****************************************************/
    // 获取温湿度成功 开始解析数据
    humidity = dat[0];

    // 温度高8位 整数部分, 低8位 小数部分
    // 整数部分 + 小数部分(低7位) * 0.1
    temperature = dat[2] + (dat[3] & 0x7F) * 0.1f;

    // 如果温度最高位是1, 表示温度为负
    if (dat[3] >> 7 & 0x01) {
        temperature = -temperature;
    }
    *p_humidity = humidity;
    *p_temperature = temperature;

    printf("湿度: %.2f%%, 温度: %.2f℃ \n", humidity, temperature);

    return res;
}
