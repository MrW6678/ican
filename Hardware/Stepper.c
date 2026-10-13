#include "stm32f10x.h"
#include "Stepper.h"

/* Expansion board: PC6 -> Q1 -> PUL-, PC4 -> Q2 -> DIR-,
 * PC5 -> Q3 -> ENA-. All '+' terminals connect to board +5V.
 * HBS86H: conducting ENA optocoupler DISABLES the driver.
 * Thus PC5 HIGH = disabled; PC5 LOW = enabled. */
#define PULSE_PIN GPIO_Pin_6
#define DIR_PIN GPIO_Pin_4
#define DISABLE_PIN GPIO_Pin_5

static StepperControl state;
static uint16_t applied_rpm;
static uint8_t clock_ok;

static void PulsePinMode(GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = PULSE_PIN;
    gpio.GPIO_Mode = mode;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
}

/* May be called from a fault handler; no delays or interrupt dependencies. */
void Stepper_EmergencyDisable(void)
{
    RCC->APB2ENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM8;
    GPIOC->BSRR = DISABLE_PIN;
    GPIOC->CRL = (GPIOC->CRL & ~(0xFu << 20)) | (0x3u << 20);
    TIM8->BDTR &= ~TIM_BDTR_MOE;
    TIM8->CR1 &= ~TIM_CR1_CEN;
    GPIOC->BRR = PULSE_PIN;
    GPIOC->CRL = (GPIOC->CRL & ~(0xFu << 24)) | (0x3u << 24);
}

static void ApplyState(void)
{
    uint32_t period;
    if (!state.enabled || !clock_ok) {
        Stepper_EmergencyDisable();
        applied_rpm = 0;
        GPIO_WriteBit(GPIOC, DIR_PIN, state.direction ? Bit_SET : Bit_RESET);
        return;
    }

    GPIO_WriteBit(GPIOC, DIR_PIN, state.direction ? Bit_SET : Bit_RESET);
    GPIO_ResetBits(GPIOC, DISABLE_PIN);
    /* The 20 ms controller wait guarantees enable and direction setup. */
    if (state.rpm == 0 || state.rpm == applied_rpm) return;
    period = StepperControl_PeriodUs(state.rpm);
    if (period < 4u || period > 65536u) {
        Stepper_Stop();
        return;
    }
    /* UDIS defers update transfers until BOTH preload registers are written.
     * Never restart a running counter when changing frequency. */
    TIM_UpdateDisableConfig(TIM8, ENABLE);
    TIM_SetAutoreload(TIM8, period - 1u);
    TIM_SetCompare1(TIM8, period / 2u);
    TIM_UpdateDisableConfig(TIM8, DISABLE);
    if (applied_rpm == 0) {
        TIM_GenerateEvent(TIM8, TIM_EventSource_Update);
        TIM_SetCounter(TIM8, 0);
        /* PWM2 begins LOW, so enabling the pin does not emit a short pulse. */
        PulsePinMode(GPIO_Mode_AF_PP);
        TIM_CtrlPWMOutputs(TIM8, ENABLE);
        TIM_Cmd(TIM8, ENABLE);
    }
    applied_rpm = state.rpm;
}

void Stepper_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef oc;
    RCC_ClocksTypeDef clocks;
    uint32_t timer_clock;

    StepperControl_Init(&state);
    applied_rpm = 0;
    clock_ok = 0;
    Stepper_EmergencyDisable();
    GPIO_ResetBits(GPIOC, DIR_PIN);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = DIR_PIN | DISABLE_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);

    RCC_GetClocksFreq(&clocks);
    timer_clock = clocks.PCLK2_Frequency;
    if (clocks.PCLK2_Frequency != clocks.HCLK_Frequency) timer_clock *= 2u;
    if (timer_clock < 1000000u || timer_clock % 1000000u != 0) return;

    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = (uint16_t)(timer_clock / 1000000u - 1u);
    timer.TIM_Period = 9999u;
    TIM_TimeBaseInit(TIM8, &timer);
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM2;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 5000u;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OC1Init(TIM8, &oc);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM8, ENABLE);
    /* Freeze PWM during debugger halt; normal operation uses an IWDG too. */
    DBGMCU_Config(DBGMCU_TIM8_STOP | DBGMCU_IWDG_STOP, ENABLE);
    clock_ok = 1;
}

void Stepper_Command(char command)
{
    StepperControl_Command(&state, command);
    if (!clock_ok) StepperControl_Stop(&state);
    ApplyState();
}

void Stepper_Tick1ms(void)
{
    StepperControl_Tick1ms(&state);
    ApplyState();
}

void Stepper_Stop(void)
{
    StepperControl_Stop(&state);
    ApplyState();
}

const StepperControl *Stepper_GetState(void)
{
    return &state;
}
