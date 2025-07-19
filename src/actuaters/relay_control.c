#include "relay_control.h"
#include "stm32f1xx.h"

void RELAY_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &= ~(0xF << (0 * 4));
    GPIOB->CRL |=  (0x1 << (0 * 4));  // PB0 output
    RELAY_Off();
}

void RELAY_On(void) {
    GPIOB->ODR |= GPIO_ODR_ODR0;
}

void RELAY_Off(void) {
    GPIOB->ODR &= ~GPIO_ODR_ODR0;
}
