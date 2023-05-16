#include "fb_dma_pio_lut.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static struct {
    // LCD Framebuffer
    uint16_t fb_len;
    volatile uint8_t *fb_ptr;

    // PIO configurations
    uint32_t pio_offset_fblut;
    pio_sm_config pio_cfg_fblut;
    
    // DMA configurations for write
    dma_channel_config dmacfg_write_chA; // Copy bytes from RAM Framebuffer to PIO
    dma_channel_config dmacfg_write_chB; // Copy address from PIO to lookup table DMA (C) READ_ADDR register
    dma_channel_config dmacfg_write_chC; // Copy from lookup table into LCD write buffer (direct to 16*8 SPI fifo?)

    // Destination SPI Instance
    spi_inst_t * dest_spi_inst;
} cfg_fb_lut;

//static uint16_t colour_lut[FBLUT_COLORS] __attribute__((aligned(FBLUT_COLORS*2), section(".scratch_x.lut")));

/*void lut_select(const uint16_t * selected_lut) {
    for(uint16_t c = 0; c < FBLUT_COLORS; c++) {
        colour_lut[c] = selected_lut[c];
    }
}*/

void framebuffer_pio_init() {
    // Claim PIO with SDK
    pio_sm_claim(FBLUT_PIO, FBLUT_PIO_SM);

    // Load base address to state machine register X
    uint32_t addrbase = (uint32_t)&(FB_USE_LUT[0]);
    assert((addrbase & 0x1FF) == 0);

    uint8_t offset = pio_add_program(FBLUT_PIO, &FBLUT_PIO_PROGRAM);

    pio_sm_config c = FBLUT_PIO_PROG_DEFAULT_CONF(offset);
    //sm_config_set_clkdiv(&c, 1);

    sm_config_set_out_shift(&c, true, false, 8);
    sm_config_set_in_shift(&c, true, true, 32);

    pio_sm_init(FBLUT_PIO, FBLUT_PIO_SM, offset, &c);
    pio_sm_put(FBLUT_PIO, FBLUT_PIO_SM, addrbase >> 9);
    pio_sm_exec(FBLUT_PIO, FBLUT_PIO_SM, pio_encode_pull(false, false));
    pio_sm_exec(FBLUT_PIO, FBLUT_PIO_SM, pio_encode_mov(pio_x, pio_osr));
}

void framebuffer_dma_init() {
    // Claim DMA with SDK
    dma_channel_claim(FBLUT_DMA_CH_A);
    dma_channel_claim(FBLUT_DMA_CH_B);
    dma_channel_claim(FBLUT_DMA_CH_C);

    // Create DMA channel configurations so they can be applied quickly later
    
    // Channel A: Copy bytes from RAM Framebuffer to PIO
    dma_channel_config cfg = dma_channel_get_default_config(FBLUT_DMA_CH_A);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(FBLUT_PIO, FBLUT_PIO_SM, true));
    cfg_fb_lut.dmacfg_write_chA = cfg;

    // Channel B: Copy address from PIO to lookup table DMA (C) READ_ADDR register
    cfg = dma_channel_get_default_config(FBLUT_DMA_CH_B);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(FBLUT_PIO, FBLUT_PIO_SM, false));
    cfg_fb_lut.dmacfg_write_chB = cfg;

    // Channel C: Copy from lookup table into SPI FIFO
    cfg = dma_channel_get_default_config(FBLUT_DMA_CH_C);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, spi_get_dreq(cfg_fb_lut.dest_spi_inst, true));
    channel_config_set_chain_to(&cfg, FBLUT_DMA_CH_B);
    cfg_fb_lut.dmacfg_write_chC = cfg;


    // Configure DMA B Immediately
    dma_channel_configure(FBLUT_DMA_CH_B,
        &cfg_fb_lut.dmacfg_write_chB,
        &dma_hw->ch[FBLUT_DMA_CH_C].al3_read_addr_trig,
        &FBLUT_PIO->rxf[FBLUT_PIO_SM],
        1, true);

    // Configure DMA C Immediately
    dma_channel_configure(FBLUT_DMA_CH_C,
        &cfg_fb_lut.dmacfg_write_chC,
        &spi_get_hw(cfg_fb_lut.dest_spi_inst)->dr,
        NULL,
        1, false);
}

void setup_framebuffer(uint16_t fb_x, uint16_t fb_y, volatile uint8_t *ptr, spi_inst_t * dest_spi_inst) {
    cfg_fb_lut.fb_len = fb_x * fb_y / FB_X_SCALE;
    cfg_fb_lut.fb_ptr = ptr;

    cfg_fb_lut.dest_spi_inst = dest_spi_inst;

    framebuffer_pio_init();
    framebuffer_dma_init();
}

void trigger_framebuffer_dma() {
    // Enable LUT PIO State machine
    pio_sm_set_enabled(FBLUT_PIO, FBLUT_PIO_SM, true);

    // Start DMA from current buffer to parity generator
    dma_channel_configure(FBLUT_DMA_CH_A,
        &cfg_fb_lut.dmacfg_write_chA,
        &FBLUT_PIO->txf[FBLUT_PIO_SM],
        cfg_fb_lut.fb_ptr,
        cfg_fb_lut.fb_len,
        true
    );
}