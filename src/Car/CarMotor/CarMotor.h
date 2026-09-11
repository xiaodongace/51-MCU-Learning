#ifndef __CAR_MOTOR_H
#define __CAR_MOTOR_H



// 左前轮 left  forward
#define 	LF_P		P16
#define 	LF_N		P17

// 右前轮 right forward
#define 	RF_P		P14
#define 	RF_N		P15

// 左后轮 left backward
#define 	LB_P		P22
#define 	LB_N		P23

// 右后轮 right backward
#define 	RB_P		P20
#define 	RB_N		P21

// 初始化
void CarMotors_init();
// 前进
void CarMotors_forward(char speed);
// 后退
void CarMotors_backward(char speed);
// 停止
void CarMotors_stop();

#endif /* __CAR_MOTOR_H */
