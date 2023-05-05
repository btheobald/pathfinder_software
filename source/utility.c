#include "utility.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/error.h"
#include "pathfinder.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int bus_scan_app(void) {
    printf("\nI2C Bus Scan\n");
    //printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < 128; ++addr) {
        //if (addr % 16 == 0) {
        //    printf("%02x ", addr);
        //}

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        gpio_put(PICO_DEFAULT_LED_PIN, 1);

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_write_blocking(i2c1, addr, NULL, 1, false);

        //printf(ret < 0 ? "." : "@");
        //printf(addr % 16 == 15 ? "\n" : "  ");
        
        //printf("0x%02X - 0x%02X\n", addr, ret);

        if(ret == 1) {
            printf("Found - 0x%02X = ", addr);
            switch(addr) {
                case SENSOR_I2C_FXOS8700_ADDR: 
                    printf("FXOS8700\t - Accelerometer/Gyro\n");
                    break;
                case SENSOR_I2C_MAX17048_ADDR:
                    printf("MAX17048\t - Battery Monitor\n");
                    break;
                case SENSOR_I2C_MS5637_ADDR:
                    printf("MS5637\t - Altimeter\n");
                    break;
                default:
                    printf("\n");
            }
        }

        gpio_put(PICO_DEFAULT_LED_PIN, 0);
    }

    printf("Done.\n\n\r");
}

void tusb_cdc_wait() {
    // Configure STDIO and delay for ACM to be ready.
    stdio_init_all();
    while (!tud_cdc_connected())
        sleep_ms(100);
}