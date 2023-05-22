#include "utility.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/error.h"
#include "utility.h"
#include "pathfinder.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void bus_scan_app(EmbeddedCli *cli, char *args, void *context) {
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    uint8_t found_addr[4] = {0};
    uint8_t index = 0;

    for (int addr = 0; addr < 128; ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_write_blocking(i2c1, addr, NULL, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
        
        if(ret == 1) {
            found_addr[index] = addr;
            index++;
        }
    }

    for(int i = 0; i < index; i++) {
        printf("Found - 0x%02X = ", found_addr[i]);
        switch(found_addr[i]) {
            case SENSOR_I2C_FXOS8700_ADDR: 
                printf("FXOS8700\t - Accelerometer/Gyro\r\n");
                break;
            case SENSOR_I2C_MAX17048_ADDR:
                printf("MAX17048\t - Battery Monitor\r\n");
                break;
            case SENSOR_I2C_MS5637_ADDR:
                printf("MS5637\t - Altimeter\r\n");
                break;
            default:
                printf("Unknown Device\r\n");
        }
    }
}