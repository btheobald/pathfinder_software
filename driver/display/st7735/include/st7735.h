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

#define RGBCTRL               0xB1
#define PORCTRL               0xB2
#define FRMCTR1               0xB3
#define INVCTR                0xB4
#define DISSET5               0xB6
#define GCTRL                 0xB7
#define VCOMS                 0xBB

#define LCMCTRL               0xC0
#define IDSET                 0xC1
#define VRHEN                 0xC2
#define VRHS                  0xC3
#define VCMOFSET              0xC5
#define FRCTRL2               0xC6

#define PWCTRL1               0xD0

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

typedef struct {
    spi_inst_t *spi_ref;  // SPI Reference Pointer
    uint8_t dc_gpio;
    uint8_t reset_gpio;
    uint8_t backlight_gpio;
} st7735_t;

void st7735_init (st7735_t * lcd, spi_inst_t * spi, uint8_t dc, uint16_t xres, uint16_t yres);
void st7735_cmd_send (st7735_t * lcd, uint8_t * cmd);
void st7735_data_send (st7735_t * lcd, uint8_t * data, uint8_t len);
void st7735_setup_fill(st7735_t * lcd);

#endif