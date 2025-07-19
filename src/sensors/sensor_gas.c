#include "sensor_gas.h"
#include "stm32f103.h"
#include <stdio.h>
#include <stdbool.h>

#define THRESHOLD_High  700.0f
#define THRESHOLD_Low   50.0f

volatile bool adc_done = false;
volatile uint16_t adc_result = 0;

typedef enum {
ADC_timeout = 0,
ADC_out_of_range = 1,
ADC_timeout_ok = 2,
ADC_range_ok = 3
} ErrorCode;

typedef struct {
float gas_concentration2;
float gas_concentration;
float threshold_low;
} GasSensordata;

GasSensordata sensor;

void TIM2_Init(void) {
RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
TIM2->PSC = 71;
TIM2->ARR = 1000;
TIM2->CR1 |= TIM_CR1_CEN;
}

void ADC1_Init(void) {
RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN;
GPIOA->CRL &= ~(0xF << 0);  // PA0 analog
ADC1->SQR3 = 0;
ADC1->CR2 |= ADC_CR2_ADON;
for (volatile int i = 0; i < 1000; i++);
ADC1->CR2 |= ADC_CR2_ADON;

ADC1->HTR = (uint16_t)(THRESHOLD_High * 4095.0f / 1000.0f);
ADC1->LTR = 0;
ADC1->CR1 |= ADC_CR1_AWDEN | ADC_CR1_AWDIE;
NVIC_EnableIRQ(ADC1_2_IRQn);

}

uint16_t ADC_Read(void) {
return adc_result;
}

float calculate_gas_ppm_from_adc(uint16_t data_gas) {
sensor.gas_concentration = ((float)data_gas / 4095.0f);
sensor.gas_concentration2 = sensor.gas_concentration * 1000.0f;
return sensor.gas_concentration2;
}

void fan_on(void) {
GPIOA->ODR |= GPIO_ODR_ODR3;
}

void fan_off(void) {
GPIOA->ODR &= ~GPIO_ODR_ODR3;
}

void restart_ADC(void) {
ADC1->CR2 &= ~ADC_CR2_ADON;
for (volatile int i = 0; i < 1000; i++);
ADC1->CR2 |= ADC_CR2_ADON;
}

void ADC1_2_IRQHandler(void) {
if (ADC1->SR & ADC_SR_AWD) {
ADC1->SR &= ~ADC_SR_AWD;
fan_on();
}

if (ADC1->SR & ADC_SR_EOC) {
adc_result = ADC1->DR;
adc_done = true;
ADC1->SR &= ~ADC_SR_EOC;
}

}

bool readinr_with_timeout(uint16_t *val, uint32_t timeout_max) {
adc_done = false;
uint32_t timeout = timeout_max;
while (!adc_done && timeout--) {}
if (!adc_done) return false;
*val = adc_result;
return true;
}

bool reading_with_range(void) {
if (sensor.gas_concentration2 > THRESHOLD_Low &&
sensor.gas_concentration2 < THRESHOLD_High) {
return true;
}
return false;
}

ErrorCode Safe_reading_ADC(void) {
if (!readinr_with_timeout(&adc_result, 100000)) {
return ADC_timeout;
}
if (!reading_with_range()) {
return ADC_out_of_range;
}
return ADC_range_ok;
}

void lcd_print(const char* msg) {
// ???? ??????? ?? ???? ????
}

void error_actions(void) {
ErrorCode error_check = Safe_reading_ADC();
switch (error_check) {
case ADC_timeout:
restart_ADC();
break;
case ADC_out_of_range:
lcd_print("unnormal situation");
break;
case ADC_timeout_ok:
case ADC_range_ok:
lcd_print("normal situation");
break;
}
}

/*int main(void) {
ADC1_Init();
TIM2_Init();

while (1) {
error_actions();
uint16_t gas = ADC_Read();
float ppm = calculate_gas_ppm_from_adc(gas);
printf("Gas Concentration: %.2f ppm\n", ppm);}}*/