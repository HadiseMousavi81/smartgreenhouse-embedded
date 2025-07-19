
#include "fan_control.h"
#include "stm32f1xx.h"

void FAN_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xF << (3 * 4));
    GPIOA->CRL |=  (0x1 << (3 * 4));  // PA3 output push-pull
    FAN_Off();
}

void FAN_On(void) {
    GPIOA->ODR |= GPIO_ODR_ODR3;
}

void FAN_Off(void) {
    GPIOA->ODR &= ~GPIO_ODR_ODR3;
}
