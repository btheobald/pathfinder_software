#include "pathfinder.h"
#include "gps_d.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define KHz *1000
#define MHz *1000000

static uint32_t pwm_slice;

void configure_pwm(void) {
    //gpio_set_function(LCD_BL_GPIO, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 2.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);
}

void set_backlight(uint16_t level) {
    pwm_set_gpio_level(LCD_BL_GPIO, level*level);
}

void configure_i2c(void) {
    printf("\tConfigure I2C Peripheral - ");
    
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
}

void configure_spi(void) {
    printf("\tConfigure SPI Peripheral - ");

    gpio_set_function(LCD_SCLK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_CS_GPIO, GPIO_FUNC_SPI);

    printf("%u Hz - ", spi_init(LCD_SPI_PERIPHERAL, 50 MHz)/1000000);
    
    // Default - 6.36ms per frame
    spi_set_format(LCD_SPI_PERIPHERAL, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame

    bi_decl(bi_3pins_with_func(LCD_SCLK_GPIO, LCD_MOSI_GPIO, LCD_CS_GPIO, GPIO_FUNC_SPI));

    printf("OK\n");
}

void pathfinder_hw_setup(void) {
    printf("Configure Clocks:\n");

    const uint8_t clock_mhz = 100;

    set_sys_clock_khz(clock_mhz KHz, true);
    clock_configure(
            clk_peri,
            0,                                                // No glitchless mux
            CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
            clock_mhz MHz,                                    // Input frequency
            clock_mhz MHz                                     // Output (must be same as no divider)
        );

    printf("\tSystem - %d MHz\n", clock_get_hz(clk_sys)/1000000);
    printf("\tPeripheral - %d MHz\n", clock_get_hz(clk_peri)/1000000);

    printf("OK\n");

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    printf("Configure Peripherals:\n");

    configure_pwm();
    set_backlight(0);

    configure_spi();    
    configure_i2c();
    //configure_gps();
}
