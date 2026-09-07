#include "Config.h"
#include "GPIO.h"

// P5.3 闪烁
void sys_init() {
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
	GPIO_InitStructure.Pin  = GPIO_Pin_5;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P4, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.Pin  = GPIO_Pin_7;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P2, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.Pin  = GPIO_Pin_3;		//指定要初始化的IO,
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P5, &GPIO_InitStructure);//初始化

	EA = 1;
}

// 这里函数名可随意, 建议不要使用start, 会和I2C.h里的Start冲突
void main_start() _task_ 0 {
	sys_init();
	// 创建任务 1
	os_create_task(1);
	os_create_task(2);
	// 结束任务 0
	os_delete_task(0);
}

void task_1() _task_ 1 {
	while(1) {
		P53 = 1;
		os_wait1(K_TMO);
		
		P53 = 0;
		os_wait1(K_TMO);
	}
}


void task_2() _task_ 2 {
	P45 = 0;
	while(1) {
		P27 = 1;
		os_wait1(K_TMO);

		P27 = 0;
		os_wait1(K_TMO);
	}
}

