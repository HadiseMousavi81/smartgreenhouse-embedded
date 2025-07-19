#ifndef HEATER_CONTROL_H
#define HEATER_CONTROL_H

void HEATER_Init(void);
void HEATER_SetPWM(int duty_cycle);  // duty: 0-1000

#endif
