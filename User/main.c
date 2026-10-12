#include "stm32f10x.h"
#include "Serial.h"
#include "Stepper.h"

volatile uint32_t App_Milliseconds;

static void PrintStatus(void)
{
    const StepperControl *s = Stepper_GetState();
    Serial_Printf("%s target=%u cmd=%u RPM dir=%u PPR=%u\r\n",
        s->enabled ? "RUN" : "STOP", (unsigned)s->target_rpm,
        (unsigned)s->rpm, (unsigned)s->direction,
        (unsigned)STEPPER_PULSES_PER_REV);
}

int main(void)
{
    uint32_t last_tick, now, elapsed;
    int input;

    /* Only the seeding stepper and debug UART are initialized.
     * No ESC / OLED / key / GNSS / IMU initialization in this test. */
    Stepper_Init();
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Serial_Init();
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

    Serial_SendString("HBS86H stepper ONLY. Boot STOP. 115200 8N1 ASCII\r\n");
    Serial_SendString("g=start s/space=stop +=6RPM -=6RPM d=direction(when stopped) ?=status\r\n");
    Serial_SendString("SW123 ON, SW45678 OFF: 1000 PPR, OPEN LOOP 6A.\r\n");
    PrintStatus();
    last_tick = App_Milliseconds;

    while (1) {
        now = App_Milliseconds;
        elapsed = now - last_tick;
        last_tick = now;
        if (elapsed > 100u) {
            Stepper_Stop(); /* Do not catch up a stalled acceleration ramp. */
        } else {
            while (elapsed != 0u) {
                Stepper_Tick1ms();
                --elapsed;
            }
        }
        input = Serial_ReadByte();
        if (input == -2) {
            Stepper_Stop();
            Serial_SendString("STOP: UART error/overflow; send commands one at a time.\r\n");
        } else if (input >= 0 && input != '\r' && input != '\n') {
            Stepper_Command((char)input);
            PrintStatus();
        }
        IWDG_ReloadCounter();
    }
}
