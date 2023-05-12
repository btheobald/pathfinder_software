#ifndef PATHFINDER_HEADER_GUARD
#define PATHFINDER_HEADER_GUARD

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

// GPIO Pin Allocation

// PWR
#define POWER_SHUTDOWN_GPIO 	26
#define POWER_SHUTDOWN_MODE 	GPIO_OUT
#define CHARGER_STATUS_GPIO     22
#define CHARGER_STATUS_MODE 	GPIO_IN

// Device Buttons
#define POWER_BUTTON_GPIO       26
#define POWER_BUTTON_MODE   	GPIO_IN
#define DOWN_BUTTON_GPIO        27
#define DOWN_BUTTON_MODE 		GPIO_IN
#define LEFT_BUTTON_GPIO        1
#define LEFT_BUTTON_MODE 		GPIO_IN
#define SELECT_BUTTON_GPIO      0
#define SELECT_BUTTON_MODE 	    GPIO_IN
#define RIGHT_BUTTON_GPIO       28
#define RIGHT_BUTTON_MODE 	    GPIO_IN

// LCD
#define LCD_SCLK_GPIO           2
#define LCD_MOSI_GPIO           3
#define LCD_DC_GPIO             4
#define LCD_CS_GPIO             5
#define LCD_BL_GPIO             9
#define LCD_MISO_GPIO           16
#define LCD_CS2_GPIO            17

// SDIO
#define SDIO_CMD_GPIO           10
#define SDIO_CLK_GPIO           11
#define SDIO_DAT0_GPIO          18
#define SDIO_DAT1_GPIO          19
#define SDIO_DAT2_GPIO          20
#define SDIO_DAT3_GPIO          21

// GPS
#define GPS_UART_TX             12
#define GPS_UART_RX             13
#define GPS_3DFIX_GPIO          14
#define GPS_3DFIX_MODE 	        GPIO_IN
#define GPS_1PPS_GPIO           15
#define GPS_1PPS_MODE 	        GPIO_IN

// I2C
#define SENSOR_SDA_GPIO         6
#define SENSOR_SCL_GPIO         7
#define SENSOR_IMU_INT_GPIO     8
#define LCD_OPT_SDA_GPIO        16
#define LCD_OPT_SCL_GPIO        17

// Peripheral Allocation
#define LCD_SPI_PERIPHERAL      spi0
#define SDIO_PIO_PERIPHERAL     pio1
#define GPS_UART_PERIPHERAL     uart0
#define SENSOR_I2C_PERIPHERAL   i2c1
#define LCD_OPT_I2C_PERIPHERAL  i2c0

// I2C ADDRESS
#define SENSOR_I2C_FXOS8700_ADDR    0x1E
#define SENSOR_I2C_MAX17048_ADDR    0x36
#define SENSOR_I2C_MS5637_ADDR      0x76

void pathfinder_hw_setup(void);

void gps_decode();

// Use 4 Bit colour mode to half framebuffer size
//#define LUT_4B

#ifdef LUT_4B
#define FB_ALIGN 16384
#define FB_X_SCALE 2
#else
#define FB_ALIGN 32768
#define FB_X_SCALE 1
#endif

#define YRES 130
#define XRES 160

#endif // PATHFINDER_HEADER_GUARD
