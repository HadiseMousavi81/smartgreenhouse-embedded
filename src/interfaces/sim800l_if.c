#include "sim800l_if.h"
#include  "uart_if.h"
#include <string.h> 
#include <stdio.h>

const char* AT = "AT\r\n"; const char* AT_CFUN_1 = "AT+CFUN=1\r\n"; const char* AT_CREG = "AT+CREG?\r\n"; const char* AT_CMGF_1 = "AT+CMGF=1\r\n";

static SIM800L_ConfigState config_state; static SIM800L_Error Error;

static float data_buffer[FILTER_SIZE]; static int data_index = 0;

Sensor_data sensor;

bool Sim800l_sendcommand(const char* command, const char* respond, bool checktype) { uart_send_buff((char*)command, strlen(command)); char* data1 = uart_read_buff();

if (checktype) {
    if (strncmp(data1, respond, strlen(respond)) == 0)
        return true;
} else {
    if (strstr(data1, respond)) {
        int num1, num2;
        sscanf(data1, "%s %d,%d", respond, &num1, &num2);
        return true;
    }
}
return false;

}

SIM800L_Error Sim800l_Init() { uint8_t retry_count = 0; config_state = STATE_COMMUNICATION_TEST;

while (config_state != STATE_DONE) {
    switch (config_state) {
        case STATE_COMMUNICATION_TEST:
            if (Sim800l_sendcommand(AT, "OK", 1)) {
                config_state = STATE_MODULE_ACTIVATION;
            } else {
                retry_count++;
                if (retry_count >= 3) return COMMUNICATION_TEST_error;
            }
            break;

        case STATE_MODULE_ACTIVATION:
            if (Sim800l_sendcommand(AT_CFUN_1, "OK", 1)) {
                config_state = STATE_NETWORK_CKECK;
            } else {
                retry_count++;
                if (retry_count >= 3) return MODULE_ACTIVATION_error;
            }
            break;

        case STATE_NETWORK_CKECK:
            if (Sim800l_sendcommand(AT_CREG, "+CREG:", 0)) {
                int num2 = 1; // فرضی، باید مقدار واقعی از sscanf گرفته شود
                if (num2 == 1 || num2 == 5) {
                    config_state = STATE_SIGNAL_CHECK;
                }
            } else {
                retry_count++;
                if (retry_count >= 3) return NETWORK_CKECK_error;
            }
            break;

        case STATE_SIGNAL_CHECK:
            if (Sim800l_sendcommand("AT+CSQ", "+CSQ:", 0)) {
                int num1 = 31, num2 = 220; // فرضی برای تست
                if (num1 <= 31 && num2 >= 220) {
                    config_state = STATE_SMS_CONFIG;
                }
            } else {
                retry_count++;
                if (retry_count >= 3) return SIGNAL_CHECK_error;
            }
            break;

        case STATE_SMS_CONFIG:
            if (Sim800l_sendcommand(AT_CMGF_1, "OK", 1)) {
                config_state = STATE_DONE;
                Uart_SetCallback(sim800l_Uart_sendcallback);
            } else {
                retry_count++;
                if (retry_count >= 3) return SMS_CONFIG_error;
            }
            break;

        default:
            break;
    }
}

return SMS_CONFIG_error;

}

void sim800l_Uart_sendcallback(void) { DATA_Analize(); }

void sim800l_Uart_recievecallback(void) { Recive_SMS(); }

float Filter_Data(float new_data) { data_buffer[data_index] = new_data; data_index = (data_index + 1) % FILTER_SIZE;

float sum = 0;
for (int i = 0; i < FILTER_SIZE; i++) {
    sum += data_buffer[i];
}
return sum / FILTER_SIZE;

}

void DATA_Analize() { sensor.Temp_data = Calculate_Tempreture(); sensor.finalTemp_data = Filter_Data(sensor.Temp_data); if (sensor.finalTemp_data > 50) Send_DataSMS(Temperature_error);

sensor.Hum_data = Calculate_Hum();
sensor.finalHum_data = Filter_Data(sensor.Hum_data);
if (sensor.finalHum_data > 34)
    Send_DataSMS(Hum_error);

sensor.Light_data = Calculate_Light();
sensor.finalLight_data = Filter_Data(sensor.Light_data);
if (sensor.finalLight_data > 56)
    Send_DataSMS(Light_error);

}

void Send_SMS(const char* message) { uart_send_buff("AT+CMGS="+98xxxxxxxx"\r\n", strlen("AT+CMGS="+98xxxxxxxx"\r\n")); char* data = uart_read_buff();

if (strncmp(data, ">", 1) == 0) {
    uart_send_buff((char*)message, strlen(message));
    uart_send_buff("\x1A", 1);
}

}

void Send_DataSMS(DataError error) { switch (error) { case Temperature_error: Send_SMS("Alert temp"); break; case Hum_error: Send_SMS("Alert Hum"); break; case Light_error: Send_SMS("Alert Light"); break; default: break; } }


volatile uint8_t flag = 0;

void timer2_init(void) { RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; TIM2->PSC = 35999; TIM2->ARR = 999; TIM2->DIER |= TIM_DIER_UIE; TIM2->CR1 |= TIM_CR1_CEN; NVIC_EnableIRQ(TIM2_IRQn); }

void TIM2_IRQHandler(void) { if (TIM2->SR & TIM_SR_UIF) { TIM2->SR &= ~TIM_SR_UIF; flag = 1; } }

void Recive_SMS() { if (flag == 1) { char* data3 = uart_read_buff(); if (strstr(data3, "+CMT")) { if (strstr(data3, "splet")) { // فن روشن 
} else if (strstr(data3, "buzzer")) { 
    // بازر روشن 
    } } } }

bool Error_check(SIM800L_Error error) { switch (error) { case COMMUNICATION_TEST_error: printf("Check wiring and power source\n"); return false; case MODULE_ACTIVATION_error: HAL_GPIO_WritePin(GPIOX, GPIO_PIN_Y, GPIO_PIN_RESET); HAL_Delay(200); HAL_GPIO_WritePin(GPIOX, GPIO_PIN_Y, GPIO_PIN_SET); HAL_Delay(2000); return false; case NETWORK_CKECK_error: printf("Check SIM and Antenna\n"); return false; case SIGNAL_CHECK_error: uart_send_buff("AT+CFUN=1", strlen("AT+CFUN=1")); config_state = STATE_NETWORK_CKECK; return false; default: break; } return true; }