#include "heater_control.h"
#include "stm32f1xx.h"

void HEATER_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    GPIOA->CRL &= ~(0xF << (6 * 4));
    GPIOA->CRL |=  (0xB << (6 * 4));  // PA6: AF Push-pull

    TIM3->PSC = 72 - 1;     // 1 MHz
    TIM3->ARR = 1000 - 1;   // 1 KHz
    TIM3->CCR1 = 0;
    TIM3->CCMR1 |= (6 << 4);  // PWM mode 1
    TIM3->CCER |= TIM_CCER_CC1E;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void HEATER_SetPWM(uint16_t duty) {
    if (duty > 1000) duty = 1000;
    TIM3->CCR1 = duty;
}
