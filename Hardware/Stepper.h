#ifndef STEPPER_H
#define STEPPER_H
#include "StepperControl.h"

void Stepper_Init(void);
void Stepper_Command(char command);
void Stepper_Tick1ms(void);
void Stepper_Stop(void);
void Stepper_EmergencyDisable(void);
const StepperControl *Stepper_GetState(void);
#endif
