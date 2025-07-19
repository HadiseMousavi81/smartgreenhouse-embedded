#include "stm32f10x.h"
 #include "i2c.h"

I2C_Request i2c_queue[10]; int i2c_queue_head = 0; int i2c_queue_tail = 0;

I2C_Callback i2c_done_callback = 0;

static I2C_Request current;

void I2C1_GPIO_Init(void) { // فعال‌سازی کلاک GPIOB و 
I2C1 RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

// پیکربندی پایه‌های PB6 و PB7 به حالت Alternate Function Open-Drain
 GPIOB->CRL &= ~((0xF << (6 * 4)) | (0xF << (7 * 4))); GPIOB->CRL |=  ((0xB << (6 * 4)) | (0xB << (7 * 4)));  // AF Open-drain, 50MHz 
 }

void I2C1_Init(void) { // ریست نرم‌افزاری 
I2C I2C1->CR1 |= I2C_CR1_SWRST; I2C1->CR1 &= ~I2C_CR1_SWRST;

// تنظیم فرکانس کاری 
I2C I2C1->CR2 = 8;

// تنظیم نرخ 100kHz
 I2C1->CCR = 40; I2C1->TRISE = 9;

// فعال‌سازی وقفه‌ها 
I2C1->CR2 |= I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN;

// فعال‌سازی ماژول 
I2C I2C1->CR1 |= I2C_CR1_PE;

// فعال‌سازی وقفه در 
NVIC NVIC_EnableIRQ(I2C1_EV_IRQn); NVIC_EnableIRQ(I2C1_ER_IRQn); }

static enum { STATE_IDLE, STATE_SEND_START, STATE_SEND_ADDRESS, STATE_SEND_REG, STATE_SEND_DATA, STATE_REPEATED_START, STATE_RECEIVE_DATA, STATE_SEND_STOP, STATE_DONE } i2c_state = STATE_IDLE;

void I2C_StartQueue(void) { if (i2c_queue_head < i2c_queue_tail) { current = i2c_queue[i2c_queue_head]; i2c_state = STATE_SEND_START; I2C1->CR1 |= I2C_CR1_START; } }

void I2C1_EV_IRQHandler(void) { switch(i2c_state) { case STATE_SEND_START: I2C1->DR = (current.address << 1) | (current.op == I2C_OP_READ ? 1 : 0); i2c_state = (current.op == I2C_OP_WRITE) ? STATE_SEND_REG : STATE_RECEIVE_DATA; break;

case STATE_SEND_REG:
I2C1->DR = current.reg;
i2c_state = (current.op == I2C_OP_WRITE) ? STATE_SEND_DATA : STATE_REPEATED_START;
break;

case STATE_SEND_DATA:
I2C1->DR = current.data;
i2c_state = STATE_SEND_STOP;
break;

case STATE_REPEATED_START:
I2C1->CR1 |= I2C_CR1_START;
i2c_state = STATE_RECEIVE_DATA;
break;

case STATE_RECEIVE_DATA:
*(current.buffer) = I2C1->DR;
i2c_state = STATE_SEND_STOP;
break;

case STATE_SEND_STOP:
I2C1->CR1 |= I2C_CR1_STOP;
i2c_state = STATE_DONE;
break;

case STATE_DONE:
i2c_queue_head++;
if (i2c_queue_head < i2c_queue_tail) {
current = i2c_queue[i2c_queue_head];
i2c_state = STATE_SEND_START;
I2C1->CR1 |= I2C_CR1_START;
} else {
i2c_state = STATE_IDLE;
if (i2c_done_callback) {
i2c_done_callback();
}
}
break;

default:
break;

} }

void I2C1_ER_IRQHandler(void) { if (I2C1->SR1 & I2C_SR1_AF) { I2C1->SR1 &= ~I2C_SR1_AF; I2C1->CR1 |= I2C_CR1_STOP; }

if (I2C1->SR1 & I2C_SR1_ARLO) { I2C1->SR1 &= ~I2C_SR1_ARLO; I2C1->CR1 |= I2C_CR1_STOP; }

if (I2C1->SR1 & I2C_SR1_BERR) { I2C1->SR1 &= ~I2C_SR1_BERR; I2C1->CR1 |= I2C_CR1_STOP; }

if (I2C1->SR1 & I2C_SR1_TIMEOUT) { I2C1->SR1 &= ~I2C_SR1_TIMEOUT; I2C1->CR1 |= I2C_CR1_STOP; } }

// --- New API Implementations ---

void I2C_SetCallback(I2C_Callback cb) { i2c_done_callback = cb; }

void I2C_EnqueueRead(uint8_t address, uint8_t reg, uint8_t* buffer) { if (i2c_queue_tail < 10) { i2c_queue[i2c_queue_tail].address = address; i2c_queue[i2c_queue_tail].reg = reg; i2c_queue[i2c_queue_tail].op = I2C_OP_READ; i2c_queue[i2c_queue_tail].buffer = buffer; i2c_queue[i2c_queue_tail].length = 1; i2c_queue_tail++; } }

void I2C_EnqueueWrite(uint8_t address, uint8_t reg, uint8_t data) { if (i2c_queue_tail < 10) { i2c_queue[i2c_queue_tail].address = address; i2c_queue[i2c_queue_tail].reg = reg; i2c_queue[i2c_queue_tail].op = I2C_OP_WRITE; i2c_queue[i2c_queue_tail].data = data; i2c_queue[i2c_queue_tail].buffer = 0; i2c_queue[i2c_queue_tail].length = 1; i2c_queue_tail++; } }                                                     