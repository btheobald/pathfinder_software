#ifndef __ST7735_H__
#define __ST7735_H__

#include "hardware/spi.h"
#include "hardware/gpio.h"

// Success / Error
// -----------------------------------
#define ST7735_SUCCESS        0
#define ST7735_ERROR          1

// Command definition
// -----------------------------------
#define DELAY                 0x80

#define NOP                   0x00
#define SWRESET               0x01
#define RDDID                 0x04
#define RDDST                 0x09

#define SLPIN                 0x10
#define SLPOUT                0x11
#define PTLON                 0x12
#define NORON                 0x13

#define INVOFF                0x20
#define INVON                 0x21
#define DISPOFF               0x28
#define DISPON                0x29
#define RAMRD                 0x2E
#define CASET                 0x2A
#define RASET                 0x2B
#define RAMWR                 0x2C

#define PTLAR                 0x30
#define MADCTL                0x36
#define COLMOD                0x3A

#define FRMCTR1               0xB1
#define FRMCTR2               0xB2
#define FRMCTR3               0xB3
#define INVCTR                0xB4
#define DISSET5               0xB6

#define PWCTR1                0xC0
#define PWCTR2                0xC1
#define PWCTR3                0xC2
#define PWCTR4                0xC3
#define PWCTR5                0xC4
#define VMCTR1                0xC5

#define RDID1                 0xDA
#define RDID2                 0xDB
#define RDID3                 0xDC
#define RDID4                 0xDD

#define GMCTRP1               0xE0
#define GMCTRN1               0xE1

#define PWCTR6                0xFC

// Colors
// -----------------------------------
#define BLACK                 0x0000
#define WHITE                 0xFFFF
#define RED                   0xF000

// AREA definition
// -----------------------------------
#define MAX_X                 161               // max columns / MV = 0 in MADCTL
#define MAX_Y                 130               // max rows / MV = 0 in MADCTL
#define SIZE_X                MAX_X - 1         // columns max counter
#define SIZE_Y                MAX_Y - 1         // rows max counter
#define CACHE_SIZE_MEM        (MAX_X * MAX_Y)   // whole pixels
#define CHARS_COLS_LEN        5                 // number of columns for chars
#define CHARS_ROWS_LEN        8                 // number of rows for chars

typedef struct {
    spi_inst_t *spi_ref;  // SPI Reference Pointer
    uint8_t dc_gpio;
    uint8_t reset_gpio;
    uint8_t backlight_gpio;
} st7735_t;

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc);
void st7735_cmd_send (st7735_t * lcd, uint8_t * cmd);
void st7735_data_8b_send (st7735_t * lcd, uint8_t * data);
void st7735_data_16b_send (st7735_t * lcd, uint16_t * data);
void st7735_draw_pixel (st7735_t * lcd, uint8_t x, uint8_t y, uint16_t color);

#endif