#include "StepperControl.h"

void StepperControl_Stop(StepperControl *s)
{
    s->enabled = 0;
    s->rpm = 0;
    s->wait_ms = 0;
    s->ramp_ms = 0;
}

void StepperControl_Init(StepperControl *s)
{
    s->target_rpm = STEPPER_MIN_RPM;
    s->direction = 0;
    StepperControl_Stop(s);
}

void StepperControl_Command(StepperControl *s, char command)
{
    switch (command) {
    case 'g':
        if (!s->enabled) {
            s->enabled = 1;
            s->wait_ms = STEPPER_ENABLE_WAIT_MS;
            s->ramp_ms = 0;
        }
        break;
    case '+':
        if (s->target_rpm < STEPPER_MAX_RPM)
            s->target_rpm += STEPPER_RPM_STEP;
        break;
    case '-':
        if (s->target_rpm > STEPPER_MIN_RPM)
            s->target_rpm -= STEPPER_RPM_STEP;
        break;
    case 'd':
        if (s->enabled) StepperControl_Stop(s);
        else s->direction ^= 1u;
        break;
    case '?': case '\r': case '\n':
        break;
    case 's': case ' ': default:
        StepperControl_Stop(s);
        break;
    }
}

void StepperControl_Tick1ms(StepperControl *s)
{
    if (!s->enabled) return;
    if (s->wait_ms != 0) {
        if (--s->wait_ms == 0) s->rpm = STEPPER_MIN_RPM;
        return;
    }
    if (++s->ramp_ms < STEPPER_RAMP_INTERVAL_MS) return;
    s->ramp_ms = 0;
    if (s->rpm < s->target_rpm) s->rpm += STEPPER_RPM_STEP;
    else if (s->rpm > s->target_rpm) s->rpm -= STEPPER_RPM_STEP;
}

uint32_t StepperControl_PeriodUs(uint16_t rpm)
{
    uint32_t divisor;
    if (rpm < STEPPER_MIN_RPM || rpm > STEPPER_MAX_RPM) return 0;
    divisor = STEPPER_PULSES_PER_REV * (uint32_t)rpm;
    return (60000000u + divisor / 2u) / divisor;
}
