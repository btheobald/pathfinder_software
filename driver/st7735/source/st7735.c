#include "st7735.h"
#include "pico/stdlib.h"

/** @array Init command */
const uint8_t st7735B_Init[] = {
    SWRESET,  0, 
    SLPOUT,   0,
    COLMOD,   1,  0x05,
    FRMCTR1,  3,  0x00, 0x06, 0x03,
    MADCTL,   1,  0xE0,
    DISSET5,  2,  0x15, 0x02,
    INVCTR,   1,  0x0,
    PWCTR1,   2,  0x02, 0x70,
    PWCTR2,   1,  0x05,
    //PWCTR3,   2,  0x01, 0x02,
    //VMCTR1,   2,  0x3C, 0x38,
    //PWCTR6,   2,  0x11, 0x15,*/
    GMCTRP1,  16,
      0x09, 0x16, 0x09, 0x20,
      0x21, 0x1B, 0x13, 0x19,
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    GMCTRN1,  16,
      0x0B, 0x14, 0x08, 0x1E,
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
    NORON,    0,
    DISPON,   0
};

uint8_t st7735B_Fill[] = {
    CASET,  4,  0, 1, 0, 0,  
    RASET,  4,  0, 1, 0, 0,
    RAMWR,  0
};

void exec_cmd_array(st7735_t * lcd, const uint8_t * cmdset, uint8_t size, uint8_t us_sleep) {
    for(int cmd = 0; cmd < size; cmd+=2) {
        st7735_cmd_send(lcd, (uint8_t *)&(cmdset[cmd]));
        if(cmdset[cmd+1] > 0) {
            st7735_data_send(lcd, (uint8_t *)&cmdset[cmd+2], cmdset[cmd+1]);  
            cmd+=cmdset[cmd+1];
        }
        sleep_us(us_sleep);
    }
}

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc, uint8_t xres, uint8_t yres) {
    lcd->spi_ref = spi;
    lcd->dc_gpio = dc;

    gpio_init(lcd->dc_gpio);
    gpio_set_dir(lcd->dc_gpio, 1);
    gpio_put(lcd->dc_gpio, 0); // Default - Command

    st7735B_Fill[5] = xres;
    st7735B_Fill[11] = yres;

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
    exec_cmd_array(lcd, st7735B_Fill, sizeof(st7735B_Fill), 10);
    gpio_put(lcd->dc_gpio, 1); // Data burst will follow
    spi_set_format(lcd->spi_ref, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame
}