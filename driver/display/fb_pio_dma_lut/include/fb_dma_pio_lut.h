#include <stdint.h>

#include "lut.h"
#include "addr_lut.pio.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/spi.h"

#define FBLUT_DMA_CH_A 1
#define FBLUT_DMA_CH_B 2
#define FBLUT_DMA_CH_C 3

#define FBLUT_PIO pio0
#define FBLUT_PIO_SM 0

#ifdef LUT_4B
#define FBLUT_PIO_PROGRAM addr_lut_4b_program
#define FBLUT_PIO_PROG_DEFAULT_CONF addr_lut_4b_program_get_default_config
#else
#define FBLUT_PIO_PROGRAM addr_lut_8b_program
#define FBLUT_PIO_PROG_DEFAULT_CONF addr_lut_8b_program_get_default_config
#endif

void lut_select(const uint16_t * selected_lut);

void framebuffer_pio_init();

void framebuffer_dma_init();

void setup_framebuffer(uint16_t fb_x, uint16_t fb_y, volatile uint8_t *ptr, spi_inst_t * dest_spi_inst);

void trigger_framebuffer_dma();