#include "app.h"
#include "uart.h"
#include "fan_control.h"
#include  "heater_control.h"
#include  "relay_control.h"

// بافر برای دریافت داده UART
static uint8_t uart_rx_buf[64];

void App_Init(void) {
    // راه‌اندازی UART
    UART1_Init(115200);}



void App_Run(void) {
    // 1. دریافت پیام از UART (مثلاً از SIM800L یا ESP32)
    uint16_t len = UART1_ReadBuffer(uart_rx_buf, sizeof(uart_rx_buf), 1000);

    if (len > 0) {
        uart_rx_buf[len] = '\0'; // پایان‌دهی رشته برای strcmp
        process_command((char*)uart_rx_buf);
    }
}


static void process_command(const char* cmd) {
    if (strstr(cmd, "SHADE ON")) {
        RELAY_On();
    } else if (strstr(cmd, "SHADE OFF")) {
        RELAY_Off();
    } else if (strstr(cmd, "HEATER ON")) {
        HEATER_SetPWM(500);
    } else if (strstr(cmd, "HEATER OFF")) {
        HEATER_SetPWM(0);
    } else if (strstr(cmd, "COOLER ON")) {
        FAN_On();
    } else if (strstr(cmd, "COOLER OFF")) {
        FAN_Off();
    } else {
        UART1_SendString("Unknown command\r\n");
    }
}
