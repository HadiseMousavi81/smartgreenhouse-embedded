#ifndef SENSOR_GAS_H_
#define SENSOR_GAS_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ADC_timeout = 0,
    ADC_out_of_range,
    ADC_timeout_ok,
    ADC_range_ok
} ErrorCode;

void TIM2_Init(void);
void ADC1_Init(void);
uint16_t ADC_Read(void);
float calculate_gas_ppm_from_adc(uint16_t data_gas);
void fan_on(void);
void fan_off(void);
bool readinr_with_timeout(uint16_t *val, uint32_t timeout_max);
bool reading_with_range(void);
ErrorCode Safe_reading_ADC(void);
void lcd_print(const char* msg);
void error_actions(void);
void restart_ADC(void);

#endif /* SENSOR_GAS_H_ */
