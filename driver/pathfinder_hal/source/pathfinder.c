#include "pathfinder.h"
#include "gps_d.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define KHz *1000
#define MHz *1000000

void configure_i2c(void)
{
    printf("Configure I2C Peripheral:");
    
    gpio_set_function(SENSOR_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SENSOR_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SENSOR_SDA_GPIO);
    gpio_pull_up(SENSOR_SCL_GPIO);
    gpio_set_drive_strength(SENSOR_SDA_GPIO, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(SENSOR_SCL_GPIO, GPIO_DRIVE_STRENGTH_12MA);

    i2c_init(SENSOR_I2C_PERIPHERAL, 400 KHz);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(SENSOR_SDA_GPIO, SENSOR_SCL_GPIO, GPIO_FUNC_I2C));

    printf("OK\n");

    bus_scan_app();
}

void configure_spi(void) {
    printf("Configure SPI Peripheral:");

    gpio_set_function(LCD_SCLK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_CS_GPIO, GPIO_FUNC_SPI);

    spi_init(LCD_SPI_PERIPHERAL, 62.5 MHz);
    
    // Default - 6.36ms per frame
    spi_set_format(LCD_SPI_PERIPHERAL, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame

    bi_decl(bi_3pins_with_func(LCD_SCLK_GPIO, LCD_MOSI_GPIO, LCD_CS_GPIO, GPIO_FUNC_SPI));

    printf("OK\n");
}

void pathfinder_hw_setup(void)
{
    //set_sys_clock_khz(270000, true);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    configure_spi();    
    //configure_i2c();
    //configure_gps();
}
