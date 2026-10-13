#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

// 电机（电调）编号
#define MOTOR1   1
#define MOTOR2   2
#define MOTOR3   3
#define MOTOR4   4

// 电调 RC 舵机信号脉宽（μs）
#define MOTOR_PULSE_MIN   1000    // 1.0ms 满反转
#define MOTOR_PULSE_STOP  1500    // 1.5ms 停止（中位）
#define MOTOR_PULSE_MAX   2000    // 2.0ms 满正转

void Motor_Init(void);                                   // 初始化 4 路 RC-PWM（TIM1_CH1~CH4 → PE9/PE11/PE13/PE14）
void Motor_SetPulse(uint8_t Motor, uint16_t Pulse);      // 直接设脉宽 1000~2000（μs）
void Motor_SetSpeed(uint8_t Motor, int16_t Speed);       // -100（满反转）~ 0（停）~ +100（满正转）

#endif
