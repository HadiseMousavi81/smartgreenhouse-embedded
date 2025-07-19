#include "sensor_temphum.h"
#include "i2c.h"

#define SHT3X_I2C_ADDR 0x44
#define SHT3X_CMD_MEAS_HIGHREP 0x2400
#define MAX_RETRY 4

static uint8_t retry_count = 0;
static uint8_t data_is_valid = 0;

static sht3x_raw_data_t sht3x_data;

// Forward declarations
static STATUS check_crc(void);
static void read_again(void);
static void parse_data(void);

//================ CRC ===================
static uint8_t calculate_crc(uint8_t data1, uint8_t data2, uint8_t crc_in) {
uint8_t crc = 0xFF;

crc ^= data1;
for (int j = 0; j < 8; j++)
crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);

crc ^= data2;
for (int j = 0; j < 8; j++)
crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);

return (crc == crc_in);

}

//================ I2C CALLBACK ===================
static void parse_data(void) {
sht3x_data.temp_msb = sht3x_data.buffer[0];
sht3x_data.temp_lsb = sht3x_data.buffer[1];
sht3x_data.temp_crc = sht3x_data.buffer[2];
sht3x_data.hum_msb  = sht3x_data.buffer[3];
sht3x_data.hum_lsb  = sht3x_data.buffer[4];
sht3x_data.hum_crc  = sht3x_data.buffer[5];

STATUS status = check_crc();
handle_status(status);

}

static void read_again(void) {
sht3x_start_measurement();
}

//================ API ===================
void sht3x_start_measurement(void) {
uint8_t cmd[2] = {
(uint8_t)(SHT3X_CMD_MEAS_HIGHREP >> 8),
(uint8_t)(SHT3X_CMD_MEAS_HIGHREP & 0xFF)
};

I2C_EnqueueWrite(SHT3X_I2C_ADDR, cmd, 2);
I2C_EnqueueRead(SHT3X_I2C_ADDR, sht3x_data.buffer, 6);
I2C_SetCallback(parse_data);
I2C_StartQueue();

}

//================ CRC CHECK ===================
static STATUS check_crc(void) {
uint8_t temp_ok = calculate_crc(sht3x_data.temp_msb, sht3x_data.temp_lsb, sht3x_data.temp_crc);
uint8_t hum_ok  = calculate_crc(sht3x_data.hum_msb, sht3x_data.hum_lsb, sht3x_data.hum_crc);

if (temp_ok && hum_ok) {
data_is_valid = 1;
return DATA_Received;
} else {
data_is_valid = 0;
if (retry_count < MAX_RETRY) {
retry_count++;
I2C_SetCallback(read_again);
I2C_StartQueue();
return CRC_RETRY;
} else {
return CRC_ERROR_LIMIT;
}
}

}

//================= STATUS HANDLER ===================
void handle_status(STATUS status) {
switch (status) {
case DATA_Received:
retry_count = 0;
break;

case CRC_RETRY:
// Already handled by check_crc
break;

case CRC_ERROR_LIMIT:    
reset_i2c1_via_registers();    
break;

}

}

//================= UTILS ===================
float sht3x_get_temperature(void) {
if (data_is_valid) {
uint16_t raw = (sht3x_data.temp_msb << 8) | sht3x_data.temp_lsb;
return -45.0f + 175.0f * (float)raw / 65535.0f;
} else {
return -999.0f;
}
}

float sht3x_get_humidity(void) {
if (data_is_valid) {
uint16_t raw = (sht3x_data.hum_msb << 8) | sht3x_data.hum_lsb;
return 100.0f * (float)raw / 65535.0f;
} else {
return -1.0f;
}
}

void reset_i2c1_via_registers(void) {
RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
for (volatile int i = 0; i < 1000; i++);
RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
}