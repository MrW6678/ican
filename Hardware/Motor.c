#include "stm32f10x.h"                  // Device header

// 控制器 RC-PWM 输出（推挽直连，电平 0~3.3V）
// 引脚：PA8（TIM1_CH1，5V 容忍 FT）
// 信号：周期 20ms，高电平脉宽 1~2ms（1ms 满反转 / 1.5ms 停止 / 2ms 满正转）

void Motor_Init(void)
{
	// 1. 开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// PA8
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	// TIM1

	// 2. PA8 复用推挽（直连控制器 S，无需上拉电阻）
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			// 复用推挽
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 3. 时基：72MHz / 72 = 1MHz，周期 20ms = 20000
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 20000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// 4. PWM 输出：CH1，PWM1，极性高，初始脉宽 1.5ms（停止）
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 1500;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	// 5. 高级定时器打开主输出
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	// 6. 启动定时器
	TIM_Cmd(TIM1, ENABLE);
}

void Motor_SetPulse(uint16_t Pulse)
{
	if (Pulse < 1000) Pulse = 1000;		// 限幅，防止超程
	if (Pulse > 2000) Pulse = 2000;
	TIM_SetCompare1(TIM1, Pulse);
}
