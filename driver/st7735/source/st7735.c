#include "st7735.h"
#include "pico/stdlib.h"

/** @array Init command */
const uint8_t st7735B_Init[] = {
    SWRESET,  0, 
    SLPOUT,   0,
    COLMOD,   1,  0x05,
    MADCTL,   1,  0xA0,
    DISPON,   0
};

const uint8_t st7735B_Fill[] = {
    CASET,  4,  0, 0, 0, 130,  
    RASET,  4,  0, 0, 0, 160,
    RAMWR,  0
};

void exec_cmd_array(st7735_t * lcd, const uint8_t * cmdset, uint8_t size) {
    for(int cmd = 0; cmd < size; cmd+=2) {
        st7735_cmd_send(lcd, (uint8_t *)&(cmdset[cmd]));
        if(cmdset[cmd+1] > 0) {
            st7735_data_send(lcd, (uint8_t *)&cmdset[cmd+2], cmdset[cmd+1]);  
            cmd+=cmdset[cmd+1];
        }
        sleep_ms(100);
    }
}

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc) {
    lcd->spi_ref = spi;
    lcd->dc_gpio = dc;

    gpio_init(lcd->dc_gpio);
    gpio_set_dir(lcd->dc_gpio, 1);
    gpio_put(lcd->dc_gpio, 0); // Default - Command

    exec_cmd_array(lcd, st7735B_Init, sizeof(st7735B_Init));

    exec_cmd_array(lcd, st7735B_Fill, sizeof(st7735B_Fill));
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
    exec_cmd_array(lcd, st7735B_Fill, sizeof(st7735B_Fill));
    gpio_put(lcd->dc_gpio, 1); // Data burst will follow
}