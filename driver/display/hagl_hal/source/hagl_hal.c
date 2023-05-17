#include "hagl_hal.h"

#include <hagl/bitmap.h>
#include <hagl/backend.h>
#include <hagl.h>
#include <string.h>

#include "fb_dma_pio_lut.h"

#include "hardware/dma.h"
#include "pathfinder.h"

uint8_t fb[(HAGL_DISPLAY_WIDTH*HAGL_DISPLAY_HEIGHT)/FB_X_SCALE] __attribute__((aligned(FB_ALIGN)));

static st7735_t lcd_handle;

static dma_channel_config gdma_ctrl_config;
static dma_channel_config gdma_data_config_blit;
static dma_channel_config gdma_data_config_hline;
static dma_channel_config gdma_data_config_vline;

static hagl_color_t gdma_colour_copy;

typedef struct {
    uint8_t * data;
    uint8_t * target;
    uint32_t len;
    uint32_t trigger;
} dma_ctrl_blk;

static dma_ctrl_blk * current_dma_blk ;

static size_t hagl_hal_flush(void *self) {
    st7735_setup_fill(&lcd_handle);
    trigger_framebuffer_dma();
}

static void hagl_hal_put_pixel(void *self, int16_t x0, int16_t y0, hagl_color_t color) {
    #ifdef LUT_4B
    if(!(x0 % 2)) {
        fb[y0*(HAGL_DISPLAY_WIDTH/2) + (x0/2)] |= (color & 0x0F)      | (fb[y0*(HAGL_DISPLAY_WIDTH/2) + (x0/2)] & 0xF0);
    } else {
        fb[y0*(HAGL_DISPLAY_WIDTH/2) + (x0/2)] |= (color & 0x0F) << 4 | (fb[y0*(HAGL_DISPLAY_WIDTH/2) + (x0/2)] & 0x0F);
    }
    #else
        fb[y0*(HAGL_DISPLAY_WIDTH) + (x0)] = color;
    #endif
}

static void hagl_hal_blit(void *self, int16_t x0, int16_t y0, hagl_bitmap_t *src) {
    //dma_channel_wait_for_finish_blocking(FBLUT_DMA_CH_A);
    
    if(current_dma_blk != NULL) {
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    }
    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    if(current_dma_blk != NULL)
        free(current_dma_blk);

    current_dma_blk = malloc(sizeof(dma_ctrl_blk) * (src->height+1));

    for(int line = 0; line < src->height; line++) {
        current_dma_blk[line].len = src->width;
        current_dma_blk[line].data = &(src->buffer[src->width * line]);
        current_dma_blk[line].target = &(fb[((line+y0)*(HAGL_DISPLAY_WIDTH))+x0]);
        current_dma_blk[line].trigger = (gdma_data_config_blit.ctrl | 0x01);
    }

    current_dma_blk[src->height].len = 0;
    current_dma_blk[src->height].data = NULL;
    current_dma_blk[src->height].target = NULL;
    current_dma_blk[src->height].trigger = 0;

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].read_addr,          // Initial write address
        &current_dma_blk[0],                                // Initial read address
        4,                                                  // Halt after each control block
        true                                                // Start Now
    );
}

static void hagl_hal_vline(void *self, int16_t x0, int16_t y0, uint16_t height, hagl_color_t color) {
   // dma_channel_wait_for_finish_blocking(FBLUT_DMA_CH_A);
    
    if(current_dma_blk != NULL) {
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    }
    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    if(current_dma_blk != NULL)
        free(current_dma_blk);

    current_dma_blk = malloc(sizeof(dma_ctrl_blk) * (height+1));

    channel_config_set_read_increment(&gdma_data_config_vline, false);
    gdma_colour_copy = color;

    for(int line = 0; line < height; line++) {
        current_dma_blk[line].len = 1;
        current_dma_blk[line].data = &gdma_colour_copy;
        current_dma_blk[line].target = &(fb[((line+y0)*(HAGL_DISPLAY_WIDTH))+x0]);
        current_dma_blk[line].trigger = (gdma_data_config_vline.ctrl | 0x01);
    }

    current_dma_blk[height].len = 0;
    current_dma_blk[height].data = NULL;
    current_dma_blk[height].target = NULL;
    current_dma_blk[height].trigger = 0;

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].read_addr,          // Initial write address
        &current_dma_blk[0],                                // Initial read address
        4,                                                  // Halt after each control block
        true                                                // Start Now
    );
}

static void hagl_hal_hline(void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color) {
    //dma_channel_wait_for_finish_blocking(FBLUT_DMA_CH_A);
    
    if(current_dma_blk != NULL) {
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    }
    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    if(current_dma_blk != NULL)
        free(current_dma_blk);

    current_dma_blk = malloc(sizeof(dma_ctrl_blk)*2);
    gdma_colour_copy = color;

    current_dma_blk[0].len = width;
    current_dma_blk[0].data = &gdma_colour_copy;
    current_dma_blk[0].target = &(fb[((y0)*(HAGL_DISPLAY_WIDTH))+x0]);
    current_dma_blk[0].trigger = (gdma_data_config_hline.ctrl | 0x01);

    current_dma_blk[1].len = 0;
    current_dma_blk[1].data = NULL;
    current_dma_blk[1].target = NULL;
    current_dma_blk[1].trigger = 0;

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].read_addr,          // Initial write address
        &current_dma_blk[0],                                // Initial read address
        4,                                                  // Halt after each control block
        true                                                // Start Now
    );
}

static void hagl_hal_hline_direct(void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color) {
    //dma_channel_wait_for_finish_blocking(FBLUT_DMA_CH_A);

    if(current_dma_blk != NULL) {
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    }

    if(current_dma_blk != NULL) { 
        free(current_dma_blk);
        current_dma_blk = NULL;
    }

    channel_config_set_read_increment(&gdma_data_config_hline, false);
    gdma_colour_copy = color;

    dma_channel_configure(
        HAGL_HAL_GDMA_DATA,
        &gdma_data_config_hline,
        &(fb[((y0)*(HAGL_DISPLAY_WIDTH))+x0]),          // Initial write address
        &gdma_colour_copy,                              // Initial read address
        width,                                          // Halt after each control block
        true                                            // Start Now
    );
}

void hagl_hal_init(hagl_backend_t *backend) {
    backend->width = HAGL_DISPLAY_WIDTH;
    backend->height = HAGL_DISPLAY_HEIGHT;
    backend->depth = HAGL_DISPLAY_DEPTH;
    backend->hline = hagl_hal_hline_direct;
    backend->vline = hagl_hal_vline;
    backend->blit = hagl_hal_blit;
    backend->put_pixel = hagl_hal_put_pixel;
    backend->flush = hagl_hal_flush;

    st7735_init(&lcd_handle, LCD_SPI_PERIPHERAL, LCD_DC_GPIO, HAGL_DISPLAY_WIDTH, HAGL_DISPLAY_HEIGHT);
    setup_framebuffer(HAGL_DISPLAY_WIDTH, HAGL_DISPLAY_HEIGHT, fb, LCD_SPI_PERIPHERAL);
    
    gdma_ctrl_config = dma_channel_get_default_config(HAGL_HAL_GDMA_CTRL);
    channel_config_set_transfer_data_size(&gdma_ctrl_config, DMA_SIZE_32);
    channel_config_set_read_increment(&gdma_ctrl_config, true);
    channel_config_set_write_increment(&gdma_ctrl_config, true);
    channel_config_set_ring(&gdma_ctrl_config, true, 4); // 16 byte boundary on write ptr
    
    gdma_data_config_blit = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_blit, DMA_SIZE_8);
    channel_config_set_read_increment(&gdma_data_config_blit, true);
    channel_config_set_write_increment(&gdma_data_config_blit, true);
    // Raise the IRQ flag when 0 is written to a trigger register (end of chain):
    channel_config_set_irq_quiet(&gdma_data_config_blit, true);
    // Trigger ctrl_chan when data_chan completes
    channel_config_set_chain_to(&gdma_data_config_blit, HAGL_HAL_GDMA_CTRL);

    gdma_data_config_vline = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_vline, DMA_SIZE_8);
    channel_config_set_read_increment(&gdma_data_config_vline, false);
    channel_config_set_write_increment(&gdma_data_config_vline, false);
    // Raise the IRQ flag when 0 is written to a trigger register (end of chain):
    channel_config_set_irq_quiet(&gdma_data_config_vline, true);
    // Trigger ctrl_chan when data_chan completes
    channel_config_set_chain_to(&gdma_data_config_vline, HAGL_HAL_GDMA_CTRL);
    
    gdma_data_config_hline = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_hline, DMA_SIZE_8);
    channel_config_set_read_increment(&gdma_data_config_hline, false);
    channel_config_set_write_increment(&gdma_data_config_hline, true);

    current_dma_blk = NULL;
}