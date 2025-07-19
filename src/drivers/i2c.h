#ifndef I2C_H #define I2C_H

#include <stdint.h>

typedef enum { I2C_OP_WRITE, I2C_OP_READ } I2C_Operation;

typedef struct { uint8_t address; uint8_t reg; I2C_Operation op; uint8_t* buffer; uint8_t  data; uint8_t length; } I2C_Request;

typedef void (*I2C_Callback)(void);

// صف درخواست‌های I2C extern I2C_Request i2c_queue[10]; extern int i2c_queue_head; extern int i2c_queue_tail;

// تابع callback که پس از پایان اجرا می‌شود extern I2C_Callback i2c_done_callback;

// شروع اجرای صف void I2C_StartQueue(void);

// تابع تنظیم callback void I2C_SetCallback(I2C_Callback cb);

// ثبت درخواست خواندن void I2C_EnqueueRead(uint8_t address, uint8_t reg, uint8_t* buffer);

// ثبت درخواست نوشتن void I2C_EnqueueWrite(uint8_t address, uint8_t reg, uint8_t data);

#endif