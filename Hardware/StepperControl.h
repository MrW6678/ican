#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <stdint.h>

/* HBS86H SW1/2/3 ON, SW4 OFF. Change this when changing microsteps. */
#define STEPPER_PULSES_PER_REV 1000u
#define STEPPER_MIN_RPM 6u
#define STEPPER_MAX_RPM 60u
#define STEPPER_RPM_STEP 6u
#define STEPPER_ENABLE_WAIT_MS 20u
#define STEPPER_RAMP_INTERVAL_MS 100u

typedef struct {
    uint16_t target_rpm;
    uint16_t rpm;           /* Commanded pulse speed, NOT encoder feedback. */
    uint16_t wait_ms;
    uint16_t ramp_ms;
    uint8_t enabled;
    uint8_t direction;
} StepperControl;

void StepperControl_Init(StepperControl *s);
void StepperControl_Stop(StepperControl *s);
void StepperControl_Command(StepperControl *s, char command);
void StepperControl_Tick1ms(StepperControl *s);
uint32_t StepperControl_PeriodUs(uint16_t rpm);
#endif
