#ifndef SENSOR_TEMPHUM_H_
#define SENSOR_TEMPHUM_H_

void sht3x_start_measurement(void);
float sht3x_get_temperature(void);
float sht3x_get_humidity(void);
void handle_status(STATUS status);
void reset_i2c1_via_registers(void);

#endif /* SENSOR_TEMPHUM_H_ */
