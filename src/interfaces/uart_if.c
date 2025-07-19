#include "uart_if.h"
#include <string.h>

// === بافر چرخشی ===

#define BUF_SIZE 128

typedef struct {
    uint8_t data[BUF_SIZE];
    uint8_t head, tail, count;
} CircularBuffer;

static CircularBuffer rx_buf, tx_buf;

static void cb_init(CircularBuffer *cb) {
    cb->head = cb->tail = cb->count = 0;
}

static int cb_push(CircularBuffer *cb, uint8_t byte) {
    if(cb->count >= BUF_SIZE) return -1;
    cb->data[cb->head] = byte;
    cb->head = (cb->head + 1) % BUF_SIZE;
    cb->count++;
    return 0;
}

static int cb_pop(CircularBuffer *cb, uint8_t *byte) {
    if(cb->count == 0) return -1;
    *byte = cb->data[cb->tail];
    cb->tail = (cb->tail + 1) % BUF_SIZE;
    cb->count--;
    return 0;
}

// === SysTick ===

volatile uint32_t msTicks = 0;
void SysTick_Handler(void) { msTicks++; }

// === GPIO و USART Config ===

UART_ErrorStatus_t uart1_error_status = {0};

static void UART1_GPIO_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    GPIOA->CRH &= ~((0xF<<4)|(0xF<<8));
    GPIOA->CRH |= (0xB<<4)|(0x4<<8);
}

static void UART1_Config(UART_Config_t cfg) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->CR1 &= ~USART_CR1_UE;

    USART1->BRR = SystemCoreClock / cfg.baudrate;

    if(cfg.parity == UART_PARITY_NONE) {
        USART1->CR1 &= ~USART_CR1_PCE;
    } else {
        USART1->CR1 |= USART_CR1_PCE;
        USART1->CR1 = (cfg.parity == UART_PARITY_ODD) ?
                      (USART1->CR1 | USART_CR1_PS) :
                      (USART1->CR1 & ~USART_CR1_PS);
    }

    if(cfg.stop_bits == UART_STOPBITS_2)
        USART1->CR2 |= USART_CR2_STOP_1;
    else
        USART1->CR2 &= ~USART_CR2_STOP;

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_UE;

    NVIC_EnableIRQ(USART1_IRQn);

    cb_init(&rx_buf);
    cb_init(&tx_buf);
}

// === وقفه USART1 ===

void USART1_IRQHandler(void) {
    UART_CheckErrors(USART1, &uart1_error_status);

    if(USART1->SR & USART_SR_RXNE) {
        uint8_t b = USART1->DR;
        if(cb_push(&rx_buf, b) < 0) {
            uart1_error_status.rx_overflow_count++;
        }
    }

    if(USART1->SR & USART_SR_TXE) {
        uint8_t b;
        if(cb_pop(&tx_buf, &b) == 0) {
            USART1->DR = b;
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

void UART_CheckErrors(USART_TypeDef* U, UART_ErrorStatus_t* s) {
    uint32_t sr = U->SR;
    if(sr & USART_SR_ORE){ volatile uint8_t d=U->DR; s->overrun_count++; s->last_error=1; U->CR1 &= ~USART_CR1_RE; U->CR1 |= USART_CR1_RE; }
    if(sr & USART_SR_FE){ volatile uint8_t d=U->DR; s->framing_count++; s->last_error=2; }
    if(sr & USART_SR_PE){ volatile uint8_t d=U->DR; s->parity_count++; s->last_error=3; }
    if(sr & USART_SR_NE){ volatile uint8_t d=U->DR; s->noise_count++; s->last_error=4; }
}

// === ارسال داده ===

void UART1_SendBuffer(uint8_t *data, uint16_t len) {
    for(uint16_t i=0;i<len;i++) {
        if(cb_push(&tx_buf, data[i])==0)
            USART1->CR1 |= USART_CR1_TXEIE;
    }
}

// === دریافت با timeout ===

uint16_t UART1_ReadBuffer(uint8_t *dest, uint16_t len, uint32_t timeout_ms) {
    uint32_t t0 = msTicks;
    uint16_t cnt = 0;

    while(cnt < len) {
        uint8_t b;
        if(cb_pop(&rx_buf, &b) == 0) {
            dest[cnt++] = b;
            t0 = msTicks;
        } else {
            if(msTicks - t0 >= timeout_ms) break;
        }
    }

    return cnt;
}

// === راه‌اندازی اولیه ===

void UART1_Init(uint32_t baud) {
    UART_Config_t cfg = {baud, UART_PARITY_NONE, UART_STOPBITS_1};
    UART1_GPIO_Config();
    SysTick_Config(SystemCoreClock/1000);
    UART1_Config(cfg);
}
