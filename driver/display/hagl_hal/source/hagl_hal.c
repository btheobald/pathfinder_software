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
} dma_ctrl_blk_blit;

static dma_ctrl_blk_blit current_dma_blk[HAGL_DISPLAY_HEIGHT];

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
    if(dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_write_addr_trig != 0)
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    for(int line = 0; line < src->height; line++) {
        current_dma_blk[line].data = &(src->buffer[src->width * line]);
        current_dma_blk[line].target = &(fb[((line+y0)*(HAGL_DISPLAY_WIDTH))+x0]);
    }

    current_dma_blk[src->height].data = 0;
    current_dma_blk[src->height].target = 0;

    dma_channel_configure(
        HAGL_HAL_GDMA_DATA,
        &gdma_data_config_blit,
        0,
        0,
        src->width,
        false
    );

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_read_addr,      // Initial write address
        &current_dma_blk[0],                                // Initial read address
        2,                                                  // Halt after each control block
        true                                                // Start Now
    );
}

static void hagl_hal_vline(void *self, int16_t x0, int16_t y0, uint16_t height, hagl_color_t color) {  
    if(dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_write_addr_trig != 0)
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();

    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    gdma_colour_copy = color;

    for(int line = 0; line < height; line++) {
        current_dma_blk[line].data = &gdma_colour_copy;
        current_dma_blk[line].target = &(fb[((line+y0)*(HAGL_DISPLAY_WIDTH))+x0]);
    }

    current_dma_blk[height].data = 0;
    current_dma_blk[height].target = 0;

    dma_channel_configure( 
        HAGL_HAL_GDMA_DATA,
        &gdma_data_config_vline,
        0,
        0,
        1,
        false
    );

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_read_addr,          // Initial write address
        &(current_dma_blk[0]),                                  // Initial read address
        2,                                                      // Halt after each control block
        true                                                    // Start Now
    );
}

static void hagl_hal_hline(void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color) {   
    if(dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_write_addr_trig != 0)
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();

    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    gdma_colour_copy = color;

    current_dma_blk[0].data = &gdma_colour_copy;
    current_dma_blk[0].target = &(fb[((y0)*(HAGL_DISPLAY_WIDTH))+x0]);
    current_dma_blk[1].data = 0;
    current_dma_blk[1].target = 0;

    dma_channel_configure(
        HAGL_HAL_GDMA_DATA,
        &gdma_data_config_hline,
        0,
        0,
        width,
        false
    );

    dma_channel_configure(
        HAGL_HAL_GDMA_CTRL,
        &gdma_ctrl_config,
        &dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_read_addr,      // Initial write address
        &(current_dma_blk[0]),                              // Initial read address
        2,                                                  // Halt after each control block
        true                                                // Start Now
    );
}

static void hagl_hal_hline_direct(void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color) {
    if(dma_hw->ch[HAGL_HAL_GDMA_DATA].al2_write_addr_trig != 0) {
        while (!(dma_hw->intr & 1u << HAGL_HAL_GDMA_DATA))
            tight_loop_contents();
    }

    dma_hw->ints0 = 1u << HAGL_HAL_GDMA_DATA;

    gdma_colour_copy = color;

    dma_channel_configure(
        HAGL_HAL_GDMA_DATA,
        &gdma_data_config_hline,
        &(fb[((y0)*(HAGL_DISPLAY_WIDTH))+x0]),
        &gdma_colour_copy,
        width,
        true
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
    channel_config_set_ring(&gdma_ctrl_config, true, 3); // 8 byte boundary on write ptr
    
    gdma_data_config_blit = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_blit, DMA_SIZE_8);
    // Increment across source / destination column
    channel_config_set_read_increment(&gdma_data_config_blit, true);
    channel_config_set_write_increment(&gdma_data_config_blit, true);
    // Raise the IRQ flag when 0 is written to a trigger register (end of chain):
    channel_config_set_irq_quiet(&gdma_data_config_blit, true);
    // Trigger ctrl_chan when data_chan completes
    channel_config_set_chain_to(&gdma_data_config_blit, HAGL_HAL_GDMA_CTRL);

    gdma_data_config_vline = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_vline, DMA_SIZE_8);
    // No need to increment source or destination column
    channel_config_set_read_increment(&gdma_data_config_vline, false);
    channel_config_set_write_increment(&gdma_data_config_vline, false);
    // Raise the IRQ flag when 0 is written to a trigger register (end of chain):
    channel_config_set_irq_quiet(&gdma_data_config_vline, true);
    // Trigger ctrl_chan when data_chan completes
    channel_config_set_chain_to(&gdma_data_config_vline, HAGL_HAL_GDMA_CTRL);
    
    gdma_data_config_hline = dma_channel_get_default_config(HAGL_HAL_GDMA_DATA);
    channel_config_set_transfer_data_size(&gdma_data_config_hline, DMA_SIZE_8);
    // Increment across destination column
    channel_config_set_read_increment(&gdma_data_config_hline, false);
    channel_config_set_write_increment(&gdma_data_config_hline, true);
    channel_config_set_irq_quiet(&gdma_data_config_hline, false);

    dma_hw->ints0 = 0u << HAGL_HAL_GDMA_DATA;
}