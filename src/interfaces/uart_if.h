#ifndef UART_IF_H
#define UART_IF_H

#include "stm32f10x.h"
#include <stdint.h>

// ==== تنظیمات پیکربندی ====

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

// ==== توابع عمومی ====

void UART1_Init(uint32_t baud);
void UART1_SendBuffer(uint8_t *data, uint16_t len);
uint16_t UART1_ReadBuffer(uint8_t *dest, uint16_t len, uint32_t timeout_ms);
void UART_CheckErrors(USART_TypeDef* U, UART_ErrorStatus_t* s);

extern UART_ErrorStatus_t uart1_error_status;

#endif
