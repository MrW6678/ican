#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Motor.h"

/*
 * 接线说明：
 *   控制器 S        → PA8  （TIM1_CH1，推挽直连）
 *   控制器 GND      → STM32 GND
 *   控制器 Power +/- → 电源（电池）
 *   控制器 Motor +/- → 直流电机
 *
 
 * 上电后演示：满反转 → 停止 → 满正转 → 停止，循环
 
 */
int main(void)
{
	Motor_Init();	// 控制器 RC-PWM 输出：PA8

	while (1)
	{
		Motor_SetPulse(1000);	// 1.0ms → 满反转
		Delay_ms(3000);

		Motor_SetPulse(1500);	// 1.5ms → 停止
		Delay_ms(2000);

		Motor_SetPulse(2000);	// 2.0ms → 满正转
		Delay_ms(3000);

		Motor_SetPulse(1500);	// 1.5ms → 停止
		Delay_ms(2000);
	}
}
