#include "stm32f10x.h"                  // Device header

/*
 * 4 路有刷电调（ESC）RC-PWM 输出（STM32F103ZET6，TIM1 高级定时器）
 * 适用于 HC480A / HC960A 双向有刷电调（12~24V，差速履带/坦克/船）
 *
 * 引脚（TIM1 全复用 GPIO_FullRemap_TIM1）：
 *   电调1 信号线 S → PE9   (TIM1_CH1)
 *   电调2 信号线 S → PE11  (TIM1_CH2)
 *   电调3 信号线 S → PE13  (TIM1_CH3)
 *   电调4 信号线 S → PE14  (TIM1_CH4)
 *
 * 信号：周期 20ms（50Hz），高电平脉宽 1.0~2.0ms
 *        1.0ms 满反转 / 1.5ms 停止（中位）/ 2.0ms 满正转
 * 时基：72MHz / 72 = 1MHz，ARR = 20000 → 20ms；CCR 1000~2000 → 1.0~2.0ms
 */

void Motor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// 1. 开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	// PE9/PE11/PE13/PE14
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);	// 复用功能（引脚重映射）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,  ENABLE);	// TIM1

	// 2. TIM1 全复用：CH1~CH4 映射到 PE9/PE11/PE13/PE14
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);

	// 3. PE9/PE11/PE13/PE14 复用推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// 4. 时基：72MHz / 72 = 1MHz，周期 20ms（50Hz）
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 20000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// 5. 4 路 PWM 输出（PWM1，高电平有效，初始 1.5ms = 停止）
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = MOTOR_PULSE_STOP;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	// 6. 高级定时器打开主输出
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	// 7. 启动定时器
	TIM_Cmd(TIM1, ENABLE);
}

void Motor_SetPulse(uint8_t Motor, uint16_t Pulse)
{
	if (Pulse < MOTOR_PULSE_MIN) Pulse = MOTOR_PULSE_MIN;	// 限幅，防止超程
	if (Pulse > MOTOR_PULSE_MAX) Pulse = MOTOR_PULSE_MAX;

	switch (Motor)
	{
		case MOTOR1: TIM_SetCompare1(TIM1, Pulse); break;
		case MOTOR2: TIM_SetCompare2(TIM1, Pulse); break;
		case MOTOR3: TIM_SetCompare3(TIM1, Pulse); break;
		case MOTOR4: TIM_SetCompare4(TIM1, Pulse); break;
		default: break;
	}
}

// Speed：-100（满反转）~ 0（停）~ +100（满正转），线性映射到 1000~2000μs
void Motor_SetSpeed(uint8_t Motor, int16_t Speed)
{
	uint16_t pulse;

	if (Speed < -100) Speed = -100;
	if (Speed >  100) Speed =  100;

	pulse = (uint16_t)(MOTOR_PULSE_STOP + Speed * 5);	// 1500 ± 500

	Motor_SetPulse(Motor, pulse);
}
