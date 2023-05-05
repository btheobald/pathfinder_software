#ifndef MS5637_HEADER_GUARD
#define MS5637_HEADER_GUARD

#include "hardware/i2c.h"

#define MS5637_I2C_ADDR 0x76

#define MS5637_I2C_TIMEOUT 1000

// MS5637 device commands
#define MS5637_RESET_COMMAND 0x1E
#define MS5637_START_PRESSURE_ADC_CONVERSION 0x40
#define MS5637_START_TEMPERATURE_ADC_CONVERSION 0x50
#define MS5637_READ_ADC 0x00

#define MS5637_CONVERSION_OSR_MASK 0x0F

// MS5637 commands
#define MS5637_PROM_READ_BASE_COMMAND 0xA0

// Coefficients indexes for temperature and pressure computation
#define MS5637_CRC_INDEX 0
#define MS5637_PRESSURE_SENSITIVITY_INDEX 1
#define MS5637_PRESSURE_OFFSET_INDEX 2
#define MS5637_TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX 3
#define MS5637_TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX 4
#define MS5637_REFERENCE_TEMPERATURE_INDEX 5
#define MS5637_TEMP_COEFF_OF_TEMPERATURE_INDEX 6

#define MS5637_COEFFICIENT_COUNT 7

#define MS5637_CONVERSION_TIME_OSR_256 1
#define MS5637_CONVERSION_TIME_OSR_512 2
#define MS5637_CONVERSION_TIME_OSR_1024 3
#define MS5637_CONVERSION_TIME_OSR_2048 5
#define MS5637_CONVERSION_TIME_OSR_4096 9
#define MS5637_CONVERSION_TIME_OSR_8192 17

#define MS5637_1B_COMMAND 1
#define MS5637_2B_PROM 2
#define MS5637_3B_ADC_RESULT 3

// Enum
typedef enum {
  MS5637_OSR_256 = 0,
  MS5637_OSR_512 = 1,
  MS5637_OSR_1024 = 2,
  MS5637_OSR_2048 = 3,
  MS5637_OSR_4096 = 4,
  MS5637_OSR_8192 = 5

} ms5637_osr_t;

typedef enum {
  MS5637_PRESSURE = 0,
  MS5637_TEMPERATURE = 1

} ms5637_reading_t;

typedef enum {
  MS5637_STATUS_OK = 0,
  MS5637_ERROR_NO_ACKNOWLEDGE = 1,
  MS5637_ERROR_CRC_INVALID = 2,
  MS5637_ERROR_TIMEOUT = 3

} ms5637_status_t;

typedef struct {
  uint16_t pressure_sensitivity;
  uint16_t pressure_offset;
  uint16_t pressure_sensitivity_tempco;
  uint16_t pressure_offset_tempco;
  uint16_t reference_temperature;
  uint16_t reference_temperature_tempco;

} ms5637_prom_data_t;

typedef struct {
  i2c_inst_t *i2c_ref;  // I2C Reference Pointer
  ms5637_prom_data_t prom_data;
  int32_t output_temperature;
  int32_t output_pressure;
  uint8_t conversion_time;
} ms5637_t;

// Initialise sensor on I2C bus
ms5637_status_t ms5637_init(ms5637_t * ms5637_instance, i2c_inst_t *i2c);

// Software reset (Ensure PROM data is loaded).
ms5637_status_t ms5637_reset(ms5637_t * ms5637_instance);

ms5637_status_t ms5637_get_compensated_readings(ms5637_t * ms5637_instance);

#endif // MS5637_HEADER_GUARD