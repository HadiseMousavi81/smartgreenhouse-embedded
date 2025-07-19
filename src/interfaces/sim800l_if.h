#ifndef SIM800L_IF_H
#define SIM800L_IF_H

#include <stdint.h>
#include <stdbool.h>

// -------------------- تعریف اندازه فیلتر --------------------
#define FILTER_SIZE 10  // مقدار پیش‌فرض، قابل تغییر

// -------------------- تعریف Enums --------------------
typedef enum {
    COMMUNICATION_TEST_error,
    MODULE_ACTIVATION_error,
    NETWORK_CKECK_error,
    SIGNAL_CHECK_error,
    SMS_CONFIG_error
} SIM800L_Error;

typedef enum {
    STATE_COMMUNICATION_TEST,
    STATE_MODULE_ACTIVATION,
    STATE_NETWORK_CKECK,
    STATE_SIGNAL_CHECK,
    STATE_SMS_CONFIG,
    STATE_DONE
} SIM800L_ConfigState;

typedef enum {
    Temperature_error,
    Hum_error,
    Light_error
} DataError;

// -------------------- ساختار داده‌ها --------------------
typedef struct {
    float Temp_data;
    float finalTemp_data;
    float Hum_data;
    float finalHum_data;
    float Light_data;
    float finalLight_data;
} Sensor_data;

// -------------------- متغیرهای خارجی --------------------
extern volatile uint8_t flag;
extern Sensor_data sensor;

// -------------------- توابع راه‌اندازی --------------------
SIM800L_Error Sim800l_Init(void);
bool Sim800l_sendcommand(const char* command, const char* respond, bool checktype);
bool Error_check(SIM800L_Error error);

// -------------------- توابع تحلیل داده --------------------
void DATA_Analize(void);
float Filter_Data(float new_data);

// -------------------- توابع UART --------------------
void sim800l_Uart_sendcallback(void);
void sim800l_Uart_recievecallback(void);

// -------------------- توابع SMS --------------------
void Send_SMS(const char* message);
void Send_DataSMS(DataError error);
void Recive_SMS(void);

// -------------------- توابع Timer --------------------
void timer2_init(void);
void TIM2_IRQHandler(void);

// -------------------- توابع UART فرض‌شده --------------------
char* uart_read_buff(void);
void uart_send_buff(const char* data, int len);

// -------------------- GPIO فرضی برای Reset --------------------
#define GPIOX GPIOA        // جایگزین کنید با پورت واقعی
#define GPIO_PIN_Y GPIO_PIN_0  // جایگزین کنید با پین واقعی

#endif // SIM800L_IF_H

