#include "st7735.h"
#include "pico/stdlib.h"

/** @array Init command */
const uint8_t st7735B_Init[] = {
    SWRESET,  0,  0,  
    SLPOUT,   0,  0,
    COLMOD,   1,  0x05,
    MADCTL,   1,  0xA0,
    DISPON,   0,  0,
};

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc) {
    lcd->spi_ref = spi;
    lcd->dc_gpio = dc;

    gpio_init(lcd->dc_gpio);
    gpio_set_dir(lcd->dc_gpio, 1);
    gpio_put(lcd->dc_gpio, 0); // Default - Command

    for(int cmd = 0; cmd < sizeof(st7735B_Init); cmd+=3) {
        st7735_cmd_send(lcd, (uint8_t *)&(st7735B_Init[cmd]));
        if(st7735B_Init[cmd+1])
            st7735_data_8b_send(lcd, (uint8_t *)&st7735B_Init[cmd+2]);  
        sleep_ms(100);
    }

    
}

void st7735_cmd_send (st7735_t * lcd, uint8_t * cmd) {
    gpio_put(lcd->dc_gpio, 0); // Default - Command
    spi_write_blocking(lcd->spi_ref, cmd, 1);
}

void st7735_data_8b_send (st7735_t * lcd, uint8_t * data) {
    gpio_put(lcd->dc_gpio, 1); // Default - Command
    spi_write_blocking(lcd->spi_ref, data, 1);
}

void st7735_data_16b_send (st7735_t * lcd, uint16_t * data) {
    gpio_put(lcd->dc_gpio, 1); // Default - Command
    spi_write_blocking(lcd->spi_ref, (uint8_t *) data, 2);
}

void st7735_draw_pixel (st7735_t * lcd, uint8_t x, uint8_t y, uint16_t color) {

}