#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

void Motor_Init(void);               // 初始化 RC-PWM 输出（TIM1_CH1 = PA8，推挽直连）
void Motor_SetPulse(uint16_t Pulse); // 设置脉宽（μs，自动限幅到 1000~2000）

#endif
