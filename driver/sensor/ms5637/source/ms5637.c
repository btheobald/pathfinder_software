#include "ms5637.h"
#include <stdint.h>
#include <stdio.h>
#include "pico/error.h"

//
// HAL TRANSLATION FUNCTIONS
//

// Write Command
ms5637_status_t ms5637_write_command(i2c_inst_t *i2c, uint8_t command) {
    int8_t ret;
    ret = i2c_write_timeout_per_char_us(i2c, MS5637_I2C_ADDR, &command, MS5637_1B_COMMAND, false, MS5637_I2C_TIMEOUT);

    if(ret == PICO_ERROR_GENERIC) return MS5637_ERROR_NO_ACKNOWLEDGE; // I2C device not acknowledged
    if(ret == PICO_ERROR_TIMEOUT) return MS5637_ERROR_TIMEOUT; // I2C transfer did not complete (May be partial)

    return MS5637_STATUS_OK; // Command writte OK
}

// Read data (n-bytes)
ms5637_status_t ms5637_read(i2c_inst_t *i2c, uint8_t command, uint8_t length, uint8_t * destination) {
    int8_t ret;
    i2c_write_timeout_per_char_us(i2c, MS5637_I2C_ADDR, &command, MS5637_1B_COMMAND, true, MS5637_I2C_TIMEOUT);
    ret = i2c_read_timeout_per_char_us(i2c, MS5637_I2C_ADDR, destination, length, false, MS5637_I2C_TIMEOUT);

    if(ret == PICO_ERROR_GENERIC) return MS5637_ERROR_NO_ACKNOWLEDGE; // I2C device not acknowledged
    if(ret == PICO_ERROR_TIMEOUT) return MS5637_ERROR_TIMEOUT; // I2C transfer did not complete (May be partial)

    return MS5637_STATUS_OK; // Command writte OK
}

//
// PRIVATE FUNCTIONS
//

// Calculate CRC4
ms5637_status_t ms5637_prom_crc(uint16_t *prom) {
    uint16_t n_rem = 0;

    uint8_t crc = (prom[0] & 0xF000) >> 12;
    prom[0] = ((prom[0]) & 0x0FFF); // Mask away CRC4
    prom[7] = 0;

    for(uint8_t cnt = 0; cnt < (MS5637_COEFFICIENT_COUNT + 1) * 2; cnt++) {
        if(cnt % 2 == 1) 
            n_rem ^= (uint16_t) ((prom[cnt >> 1]) & 0x00FF);
        else
            n_rem ^= (uint16_t) (prom[cnt >> 1] >> 8);

        for(int n_bit = 8; n_bit > 0; n_bit--) {
            if(n_rem & (0x8000))
                n_rem = (n_rem << 1) ^ 0x3000;
            else
                n_rem = (n_rem << 1);
        }
    }
    
    n_rem = ((n_rem >> 12) & 0x000F);
    prom[0] |= crc << 12;

    printf("CRC4 Value: %X\r\n", n_rem);

    return (n_rem == crc);
}

// Read PROM data and check CRC4
ms5637_status_t ms5637_read_prom(ms5637_t * ms5637_instance) {
    ms5637_status_t ret;

    uint16_t prom_raw[MS5637_COEFFICIENT_COUNT+1];

    // Read in CRC values
    for(int offset = 0; offset < MS5637_COEFFICIENT_COUNT; offset++) {
        printf("%02X - ",  (MS5637_PROM_READ_BASE_COMMAND+(offset<<1)));
        ret = ms5637_read(ms5637_instance->i2c_ref, (MS5637_PROM_READ_BASE_COMMAND+(offset<<1)), MS5637_2B_PROM, (uint8_t *)(&prom_raw[offset]));
        
        prom_raw[offset] = __builtin_bswap16(prom_raw[offset]); // Reads are MSB first

        if(ret != MS5637_STATUS_OK) return ret;
        printf("%04X\r\n", prom_raw[offset]);
    }

    // Validate PROM with CRC
    if(ms5637_prom_crc(prom_raw))
        printf("CRC4 OK\r\n");
    else
        printf("CRC4 Invalid\r\n");

    // Save Coeffcients
    ms5637_instance->prom_data.pressure_offset = prom_raw[MS5637_PRESSURE_OFFSET_INDEX];
    ms5637_instance->prom_data.pressure_offset_tempco = prom_raw[MS5637_TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX];
    
    ms5637_instance->prom_data.pressure_sensitivity = prom_raw[MS5637_PRESSURE_SENSITIVITY_INDEX];
    ms5637_instance->prom_data.pressure_sensitivity_tempco = prom_raw[MS5637_TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX];
  
    ms5637_instance->prom_data.reference_temperature = prom_raw[MS5637_REFERENCE_TEMPERATURE_INDEX];
    ms5637_instance->prom_data.reference_temperature_tempco = prom_raw[MS5637_TEMP_COEFF_OF_TEMPERATURE_INDEX];

    return ret;
}

// Configure sensor Oversampling Rate and measurement type and start conversion.
ms5637_status_t ms5637_start_conversion(ms5637_t * ms5637_instance, ms5637_reading_t set_reading, ms5637_osr_t set_osr) {
    ms5637_status_t ret;

    uint8_t base_command;

    // Configure base address
    if(set_reading == MS5637_PRESSURE)
        base_command = MS5637_START_PRESSURE_ADC_CONVERSION;
    else // MS5637_TEMPERATURE
        base_command = MS5637_START_TEMPERATURE_ADC_CONVERSION;

    // Start conversion
    ret = ms5637_write_command(ms5637_instance->i2c_ref, base_command+(set_osr<<1));
    if(ret != MS5637_STATUS_OK) return ret;

    // Provide indication of conversion time
    switch(set_osr) {
        case MS5637_OSR_256:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_256;
            break;
        case MS5637_OSR_512:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_512;
            break;
        case MS5637_OSR_1024:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_1024;
            break;
        case MS5637_OSR_2048:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_2048;
            break;
        case MS5637_OSR_4096:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_4096;
            break;
        case MS5637_OSR_8192:
            ms5637_instance->conversion_time = MS5637_CONVERSION_TIME_OSR_8192;
            break;
    }

    return ret;
}

// Read the result
ms5637_status_t ms5637_read_adc(ms5637_t * ms5637_instance, uint32_t * raw_adc) {
    ms5637_status_t ret;
    *raw_adc = 0;
    ret = ms5637_read(ms5637_instance->i2c_ref, MS5637_READ_ADC, MS5637_3B_ADC_RESULT, (uint8_t *)raw_adc);
    *raw_adc = __builtin_bswap32(*raw_adc) >> 8;
    //printf("ADC - %04X ", *raw_adc);
    return ret;
}

//
// USER FUNCTIONS
//

// Software reset (Ensure PROM data is loaded).
ms5637_status_t ms5637_reset(ms5637_t * ms5637_instance) {
    return ms5637_write_command(ms5637_instance->i2c_ref, MS5637_RESET_COMMAND);
}

// Initialise sensor on I2C bus
ms5637_status_t ms5637_init(ms5637_t * ms5637_instance, i2c_inst_t *i2c) {
    ms5637_status_t ret;

    // Set bus reference
    ms5637_instance->i2c_ref = i2c;

    // Reset device to test ACK
    ret = ms5637_reset(ms5637_instance);
    if(ret != MS5637_STATUS_OK) return ret;

    // Read PROM and verify
    ret = ms5637_read_prom(ms5637_instance);
    
    return ret;
}

ms5637_status_t ms5637_get_compensated_readings(ms5637_t * ms5637_instance) {
    ms5637_status_t ret;
    
    uint32_t raw_temperature, raw_pressure;

    ms5637_start_conversion(ms5637_instance, MS5637_TEMPERATURE, MS5637_OSR_8192);
    sleep_ms(ms5637_instance->conversion_time);
    ms5637_read_adc(ms5637_instance, &raw_temperature);

    ms5637_start_conversion(ms5637_instance, MS5637_PRESSURE, MS5637_OSR_8192);
    sleep_ms(ms5637_instance->conversion_time);
    ms5637_read_adc(ms5637_instance, &raw_pressure);

    int64_t dT = raw_temperature - (ms5637_instance->prom_data.reference_temperature << 8);
    ms5637_instance->output_temperature = 2000 + ((dT * ms5637_instance->prom_data.reference_temperature_tempco) >> 23);

    int64_t actual_temp_offset = ((int64_t)ms5637_instance->prom_data.pressure_offset << 17) + (((int64_t)ms5637_instance->prom_data.pressure_offset_tempco * dT) >> 6);
    int64_t actual_temp_sensitivity = ((int64_t)ms5637_instance->prom_data.pressure_sensitivity << 16) + (((int64_t)ms5637_instance->prom_data.pressure_sensitivity_tempco * dT) >> 7);
    ms5637_instance->output_pressure = (((raw_pressure * actual_temp_sensitivity) >> 21) - actual_temp_offset) >> 15;

    printf("%f, %f\n\r", ms5637_instance->output_temperature/100.0f, ms5637_instance->output_pressure/100.0f);
}