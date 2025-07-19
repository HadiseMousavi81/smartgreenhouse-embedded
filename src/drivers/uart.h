#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f10x.h"

// ================== تنظیمات ===================

typedef enum {
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} UART_Parity_t;

typedef enum {
    UART_STOPBITS_1,
    UART_STOPBITS_2
} UART_StopBits_t;

typedef struct {
    uint32_t baudrate;
    UART_Parity_t parity;
    UART_StopBits_t stop_bits;
} UART_Config_t;

typedef struct {
    uint32_t overrun_count;
    uint32_t framing_count;
    uint32_t parity_count;
    uint32_t noise_count;
    uint32_t rx_overflow_count;
    uint8_t  last_error;
} UART_ErrorStatus_t;

// Circular Buffer
#define BUF_SIZE 128
typedef struct {
    uint8_t data[BUF_SIZE];
    uint8_t head, tail, count;
} CircularBuffer;

void cb_init(CircularBuffer *cb);
int cb_push(CircularBuffer *cb, uint8_t byte);
int cb_pop(CircularBuffer *cb, uint8_t *byte);

// UART API
void UART1_GPIO_Config(void);
void UART1_Config(UART_Config_t cfg);
void UART_CheckErrors(USART_TypeDef* U, UART_ErrorStatus_t* s);
void UART1_SendBuffer(uint8_t *data, uint16_t len);
uint16_t UART1_ReadBuffer(uint8_t *dest, uint16_t len, uint32_t timeout_ms);
void UART1_Init(uint32_t baud);

// متغیر خطاهای UART1
extern UART_ErrorStatus_t uart1_error_status;

#endif
