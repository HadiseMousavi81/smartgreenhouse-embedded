#ifndef SENSOR_LIGHT_H_
#define SENSOR_LIGHT_H_

#include <stdint.h>

void TSL_Init(void);
void tsl_start_read(void);
void TSL_SetGainIntegration(uint8_t gain, uint8_t integration);
float get_auto_lux(void);

#endif /* SENSOR_LIGHT_H_ */
