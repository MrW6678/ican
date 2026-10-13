#include "stm32f10x.h"
#include "Stepper.h"

volatile uint32_t App_Milliseconds;

int main(void)
{
    uint32_t last_tick, now, elapsed;
    uint8_t auto_started = 0;

    /* Only the seeding stepper is initialized. No UART, ESC, OLED, key,
     * GNSS or IMU initialization is needed for this wiring test. */
    Stepper_Init();
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    App_Milliseconds = 0;
    if (SysTick_Config(SystemCoreClock / 1000u) != 0) {
        Stepper_EmergencyDisable();
        while (1) { }
    }
    /* Approx. 1 second at nominal 40 kHz LSI, not a precision timeout.
     * A hung main loop resets into the stopped startup state. */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(1249);
    IWDG_ReloadCounter();
    IWDG_Enable();

    last_tick = App_Milliseconds;

    while (1) {
        now = App_Milliseconds;
        elapsed = now - last_tick;
        last_tick = now;
        if (!auto_started && now >= 2000u) {
            Stepper_Command('g');
            auto_started = 1;
        }
        if (elapsed > 100u) {
            Stepper_Stop(); /* Do not catch up a stalled acceleration ramp. */
        } else {
            while (elapsed != 0u) {
                Stepper_Tick1ms();
                --elapsed;
            }
        }
        IWDG_ReloadCounter();
    }
}
