#include <stdint.h>

void framebuffer_pio_init();

void framebuffer_dma_init();

void setup_framebuffer(uint16_t fb_x, uint16_t fb_y, uint8_t *ptr);

void trigger_framebuffer_dma();