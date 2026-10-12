#include <assert.h>
#include <stdio.h>
#include "StepperControl.h"

int main(void)
{
    StepperControl s;
    unsigned int i;
    StepperControl_Init(&s);
    assert(!s.enabled && s.rpm == 0 && s.target_rpm == 6);
    assert(StepperControl_PeriodUs(0) == 0);
    assert(StepperControl_PeriodUs(6) == 10000);
    assert(StepperControl_PeriodUs(60) == 1000);
    assert(StepperControl_PeriodUs(61) == 0);
    /* Setting speed cannot start a stopped motor. */
    for (i = 0; i < 50; ++i) StepperControl_Command(&s, '+');
    assert(s.target_rpm == 60 && !s.enabled && s.rpm == 0);
    StepperControl_Command(&s, 'g');
    for (i = 0; i < 19; ++i) StepperControl_Tick1ms(&s);
    assert(s.enabled && s.rpm == 0);
    StepperControl_Tick1ms(&s);
    assert(s.rpm == 6);
    /* Repeated start commands must not restart the ramp or settle delay. */
    StepperControl_Command(&s, 'g');
    assert(s.rpm == 6);
    for (i = 0; i < 900; ++i) StepperControl_Tick1ms(&s);
    assert(s.rpm == 60);
    for (i = 0; i < 50; ++i) StepperControl_Command(&s, '-');
    assert(s.target_rpm == 6 && s.rpm == 60);
    for (i = 0; i < 100; ++i) StepperControl_Tick1ms(&s);
    assert(s.rpm == 54);
    /* Running direction change is rejected by stopping, never instant reversal. */
    StepperControl_Command(&s, 'd');
    assert(!s.enabled && s.rpm == 0 && s.direction == 0);
    StepperControl_Command(&s, 'd');
    assert(s.direction == 1 && !s.enabled);
    StepperControl_Command(&s, 'g');
    StepperControl_Command(&s, 's');
    for (i = 0; i < 1000; ++i) StepperControl_Tick1ms(&s);
    assert(!s.enabled && s.rpm == 0);
    StepperControl_Command(&s, 'g');
    StepperControl_Command(&s, '\r');
    assert(s.enabled);
    StepperControl_Command(&s, '!');
    assert(!s.enabled && s.rpm == 0);
    puts("PASS: startup, DIP conversion, limits, ramp, stop and direction interlock");
    return 0;
}
