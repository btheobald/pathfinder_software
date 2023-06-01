#include "st7735.h"
#include "pico/stdlib.h"

/** @array Init command */
const uint8_t st7735B_Init[] = {
    SWRESET,  0,
    COLMOD,   1,  0x05,
    MADCTL,   1,  0x00,
    PORCTRL,  5,  0x0C, 0x0C, 0x00, 0x33, 0x33,
    GCTRL,    1,  0x35,
    VCOMS,    1,  0x1A,
    LCMCTRL,  1,  0x2C,
    VRHEN,    1,  0x01,
    VRHS,     1,  0x03,
    0xC4,     1,  0x20,
    FRCTRL2,  1,  0x0F,
    PWCTRL1,  2,  0xA4, 0xA1,
    INVON,    0,
    GMCTRP1, 14, 0xF0, 0x00, 0x04, 0x04, 0x04, 0x05, 0x29, 0x33, 0x3E, 0x38, 0x12, 0x12, 0x28, 0x30,
    GMCTRN1, 14, 0xF0, 0x07, 0x0A, 0x0D, 0x0B, 0x07, 0x28, 0x33, 0x3E, 0x36, 0x14, 0x14, 0x29, 0x32,
    SLPOUT,   0,
    DISPON,   0
};

uint8_t st7735B_Fill[] = {
    CASET,  4,  0, 35, ((34+170) >> 8), (34+170) & 0x00FF,  
    RASET,  4,  0, 0, ((320) >> 8), (320) & 0x00FF,
    RAMWR,  0
};

void exec_cmd_array(st7735_t * lcd, const uint8_t * cmdset, uint8_t size, uint8_t us_sleep) {
    for(int cmd = 0; cmd < size; cmd+=2) {
        st7735_cmd_send(lcd, (uint8_t *)&(cmdset[cmd]));
        if(cmdset[cmd+1] > 0) {
            st7735_data_send(lcd, (uint8_t *)&cmdset[cmd+2], cmdset[cmd+1]);  
            cmd+=cmdset[cmd+1];
        }
        if(us_sleep > 0) sleep_us(us_sleep);
    }
}

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc, uint16_t xres, uint16_t yres) {
    lcd->spi_ref = spi;
    lcd->dc_gpio = dc;

    gpio_init(lcd->dc_gpio);
    gpio_set_dir(lcd->dc_gpio, 1);
    gpio_put(lcd->dc_gpio, 0); // Default - Command

    //st7735B_Fill[5] = yres;
    //st7735B_Fill[11] = xres;

    exec_cmd_array(lcd, st7735B_Init, sizeof(st7735B_Init), 100);
}

void st7735_cmd_send (st7735_t * lcd, uint8_t * cmd) {
    gpio_put(lcd->dc_gpio, 0);
    spi_write_blocking(lcd->spi_ref, cmd, 1);
}

void st7735_data_send (st7735_t * lcd, uint8_t * data, uint8_t len) {
    gpio_put(lcd->dc_gpio, 1);
    spi_write_blocking(lcd->spi_ref, data, len);
}

void st7735_setup_fill(st7735_t * lcd) {
    spi_set_format(lcd->spi_ref, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame
    exec_cmd_array(lcd, st7735B_Fill, sizeof(st7735B_Fill), 0);
    gpio_put(lcd->dc_gpio, 1); // Data burst will follow
    spi_set_format(lcd->spi_ref, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame
}