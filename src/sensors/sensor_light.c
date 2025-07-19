#include "sensor_light.h"
#include "i2c.h"
#include "tsl.h"

// --- DEFINES for TSL2561 Registers ---
#define TSL2561_COMMAND_BIT     0x80 #define TSL2561_REG_CONTROL     0x00 #define TSL2561_REG_TIMING      0x01 #define TSL2561_REG_DATA0LOW    0x0C #define TSL2561_REG_DATA0HIGH   0x0D #define TSL2561_REG_DATA1LOW    0x0E #define TSL2561_REG_DATA1HIGH   0x0F

#define TSL2561_POWER_ON        0x03 #define TSL2561_ADD             0x39

static uint8_t ch0_low, ch0_high, ch1_low, ch1_high; static uint8_t tsl_gain = 0, tsl_integration = 0; static uint8_t config_done = 0;

uint8_t tsl_data_ready = 0; uint16_t ch0 = 0, ch1 = 0;

/ ---- CALLBACKS ---- static void tsl_read_callback(void) { ch0 = (ch0_high << 8) | ch0_low; ch1 = (ch1_high << 8) | ch1_low; tsl_data_ready = 1; }

static void tsl_config_callback(void) {
config_done = 1; tsl_start_read(); // پس از پیکربندی، خواندن را آغاز کن }

/---- INIT ---- void TSL_Init(void) { I2C_EnqueueWrite(TSL2561_ADD,TSL2561_COMMAND_BIT | TSL2561_REG_CONTROL, TSL2561_POWER_ON); // Power ON I2C_SetCallback(tsl_config_callback); I2C_StartQueue(); }

void tsl_start_read(void) { tsl_data_ready = 0;

I2C_EnqueueRead(TSL2561_ADD, TSL2561_COMMAND_BIT | TSL2561_REG_DATA0LOW, &ch0_low);
I2C_EnqueueRead(TSL2561_ADD, TSL2561_COMMAND_BIT | TSL2561_REG_DATA0HIGH, &ch0_high);
I2C_EnqueueRead(TSL2561_ADD, TSL2561_COMMAND_BIT | TSL2561_REG_DATA1LOW, &ch1_low);
I2C_EnqueueRead(TSL2561_ADD, TSL2561_COMMAND_BIT | TSL2561_REG_DATA1HIGH, &ch1_high);

I2C_SetCallback(tsl_read_callback);
I2C_StartQueue();

}

void TSL_SetGainIntegration(uint8_t gain, uint8_t integration) { tsl_gain = gain; tsl_integration = integration; uint8_t timing = (gain << 4) | (integration & 0x03); I2C_EnqueueWrite(TSL2561_ADD, 0x81, timing); I2C_SetCallback(tsl_config_callback); I2C_StartQueue(); }

// ---- AUTO LUX ----

static void auto_lux_decision_callback(void); static float lux_temp = 0; static float final_lux_result = 0; static uint8_t retry_stage = 0;

static void auto_lux_read_done(void) { float ratio = (float)ch1 / (float)ch0; lux_temp = calculate_lux(ch0, ch1);

// بررسی شرط و تصمیم به بازپیکربندی
if (lux_temp < 10.0f && retry_stage == 0) {
retry_stage = 1;
TSL_SetGainIntegration(1, tsl_integration); // Gain = High
} else if (lux_temp > 30000.0f && retry_stage == 0) {
retry_stage = 1;
TSL_SetGainIntegration(0, tsl_integration); // Gain = Low
} else {
final_lux_result = lux_temp; retry_stage = 0; if (user_lux_callback) { user_lux_callback(final_lux_result);  // نتیجه نهایی را به کاربر بده user_lux_callback = 0; // Reset after call }
}

}

float get_auto_lux(void) { tsl_start_read(); I2C_SetCallback(auto_lux_read_done); I2C_StartQueue(); return final_lux_result; // مقدار موقتی تا callback نهایی شود }