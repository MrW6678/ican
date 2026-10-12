#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Motor.h"

/*
 * 接线说明（4 路 HC480A/HC960A 双向有刷电调，STM32F103ZET6）：
 *   电调1 信号线 S → PE9   (TIM1_CH1)
 *   电调2 信号线 S → PE11  (TIM1_CH2)
 *   电调3 信号线 S → PE13  (TIM1_CH3)
 *   电调4 信号线 S → PE14  (TIM1_CH4)
 *   所有电调 GND   → STM32 GND
 *   电调电源        → 12~24V 电池
 *   电调电机线      → 有刷电机
 *
 * 信号：50Hz RC 舵机 PWM，1.0ms 满反转 / 1.5ms 停 / 2.0ms 满正转
 *
 * 上电演示：4 个电机同时 满反转 → 停 → 满正转 → 停，循环
 */
int main(void)
{
	Motor_Init();

	while (1)
	{
		// 满反转
		Motor_SetSpeed(MOTOR1, -100);
		Motor_SetSpeed(MOTOR2, -100);
		Motor_SetSpeed(MOTOR3, -100);
		Motor_SetSpeed(MOTOR4, -100);
		Delay_ms(3000);

		// 停止
		Motor_SetSpeed(MOTOR1, 0);
		Motor_SetSpeed(MOTOR2, 0);
		Motor_SetSpeed(MOTOR3, 0);
		Motor_SetSpeed(MOTOR4, 0);
		Delay_ms(2000);

		// 满正转
		Motor_SetSpeed(MOTOR1, 100);
		Motor_SetSpeed(MOTOR2, 100);
		Motor_SetSpeed(MOTOR3, 100);
		Motor_SetSpeed(MOTOR4, 100);
		Delay_ms(3000);

		// 停止
		Motor_SetSpeed(MOTOR1, 0);
		Motor_SetSpeed(MOTOR2, 0);
		Motor_SetSpeed(MOTOR3, 0);
		Motor_SetSpeed(MOTOR4, 0);
		Delay_ms(2000);
	}
}

/*
 * 差速转向示例（坦克/履带，左右各两个电调）：
 *   左履带 = MOTOR1、MOTOR2，右履带 = MOTOR3、MOTOR4
 *
 *   前进：左右同速正转
 *   后退：左右同速反转
 *   左转：左反转 + 右正转（原地左转）
 *   右转：左正转 + 右反转（原地右转）
 *
 *   Motor_SetSpeed(MOTOR1,  50); Motor_SetSpeed(MOTOR2,  50);   // 左履带前进
 *   Motor_SetSpeed(MOTOR3, -50); Motor_SetSpeed(MOTOR4, -50);   // 右履带反转 → 左转
 */
