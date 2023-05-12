#include "fb_dma_pio_lut.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "pathfinder.h"

#include "addr_lut.pio.h"

#define FBLUT_DMA_CH_A 1
#define FBLUT_DMA_CH_B 2
#define FBLUT_DMA_CH_C 3

#define FBLUT_PIO pio0
#define FBLUT_PIO_SM 0

#ifdef LUT_4B
#define FBLUT_4B_PICO8 lut_pico8
#define FBLUT_USE_LUT FBLUT_4B_PICO8

#define FBLUT_PIO_PROGRAM addr_lut_4b_program
#define FBLUT_PIO_PROG_DEFAULT_CONF addr_lut_4b_program_get_default_config
#else
//#define FBLUT_8B_DUEL lut_duel
//#define FBLUT_8B_SONIC lut_sonic
#define FBLUT_8B_ATARI lut_atari
//#define FBLUT_8B_AURORA lut_aurora
#define FBLUT_USE_LUT FBLUT_8B_ATARI

#define FBLUT_PIO_PROGRAM addr_lut_8b_program
#define FBLUT_PIO_PROG_DEFAULT_CONF addr_lut_8b_program_get_default_config
#endif

#ifdef FBLUT_8B_DUEL
const uint16_t lut_duel[256] __attribute__((aligned(512), section(".scratch_x.lut"))) = {
    0x0000, 0x2124, 0x4229, 0x634e, 0x8452, 0xa577, 0xce9b, 0xf7be, 0x62ea, 0x83ac, 0x9c6f, 0xad11, 0xbd74, 0xce16, 0xe6d8, 0xff9a, 
    0x5985, 0x71e7, 0x8a88, 0x9b09, 0xab6a, 0xd46d, 0xfd50, 0xfe6f, 0x0146, 0x01ca, 0x026b, 0x0b2f, 0x0371, 0x3474, 0x257a, 0x8ebf, 
    0x6165, 0x91a7, 0xb269, 0xcae9, 0xe3c8, 0xf4c9, 0xfdc9, 0xff49, 0x2969, 0x3a2d, 0x62f0, 0x7bb3, 0x8496, 0x959a, 0xc6bf, 0xc75f, 
    0x0103, 0x0184, 0x1a43, 0x22c3, 0x3341, 0x5444, 0x7d25, 0xa646, 0x1906, 0x2189, 0x222d, 0x3b51, 0x3476, 0x459c, 0x569f, 0x77bf, 
    0x19a5, 0x3207, 0x3a88, 0x32e8, 0x43aa, 0x4c4c, 0x55af, 0x96d4, 0x5842, 0x8104, 0xb1e6, 0xe2ec, 0xfbae, 0xfcd4, 0xfdd8, 0xfedf, 
    0x2987, 0x4a49, 0x5aed, 0x738f, 0x8432, 0xad77, 0xbe3b, 0xef7e, 0x3987, 0x59e8, 0x8a8b, 0xab4c, 0xc40d, 0xd4ee, 0xee30, 0xffd5, 
    0x3105, 0x49a7, 0x5a29, 0x72ca, 0x7b6a, 0x9c4d, 0xbd31, 0xddf3, 0x3085, 0x4947, 0x61ab, 0x92ae, 0xb372, 0xbbd5, 0xdcd7, 0xf63b, 
    0x0169, 0x020a, 0x028c, 0x034d, 0x040f, 0x0510, 0x05f4, 0x06fb, 0x4184, 0x6247, 0x7b08, 0x9bca, 0xb48c, 0xcd4d, 0xe650, 0xff54, 
    0x6045, 0x6926, 0x9229, 0xa30b, 0xbbec, 0xccae, 0xedaf, 0xee92, 0x31ab, 0x4a90, 0x6333, 0x7bb7, 0x8c7b, 0x9cdd, 0xb57f, 0xde9f, 
    0x40c5, 0x7167, 0x99ea, 0xd24d, 0xf2f0, 0xfbf5, 0xfd38, 0xfe7f, 0x4923, 0x61a6, 0x7a69, 0x92cb, 0xab6d, 0xbbef, 0xd46f, 0xe4cf, 
    0x2140, 0x3281, 0x4ae0, 0x6381, 0x7c04, 0x94c5, 0xb546, 0xce46, 0x6140, 0x71e1, 0x8282, 0x9b24, 0xbc46, 0xcd47, 0xe689, 0xffaa, 
    0x2927, 0x39ca, 0x528d, 0x734d, 0x93cf, 0xb4b0, 0xcd71, 0xfef6, 0x2168, 0x31e9, 0x3a67, 0x4ae6, 0x5b85, 0x6c25, 0x7ce4, 0x7de7, 
    0x3924, 0x51c7, 0x7a69, 0x92ea, 0xab6b, 0xbbec, 0xd48e, 0xf510, 0x2a69, 0x4b2b, 0x5bcd, 0x74af, 0x8571, 0x8e12, 0xae97, 0xdfdd, 
    0x00e8, 0x018c, 0x0a4f, 0x12f4, 0x13b7, 0x44bc, 0x559d, 0x6e5f, 0x524d, 0x72ee, 0x93b1, 0xb472, 0xd513, 0xedf3, 0xfeb3, 0xffb0, 
    0x20e4, 0x398a, 0x5a4f, 0x7b37, 0x943d, 0xacbd, 0xbd5e, 0xcdff, 0x292a, 0x29ab, 0x29ee, 0x3a90, 0x5335, 0x53b8, 0x6c18, 0x8498, 
    0x4905, 0x5a09, 0x72ab, 0x930d, 0xabd0, 0xb452, 0xd575, 0xff19, 0x70e0, 0x99a5, 0xbac8, 0xe425, 0xfd81, 0xfe61, 0xff65, 0xf797
};
#endif

#ifdef FBLUT_8B_SONIC
const uint16_t lut_sonic[256] __attribute__((aligned(512), section(".scratch_x.lut"))) = {
    0x0000, 0x1082, 0x2104, 0x31a6, 0x4228, 0x52aa, 0x632c, 0x73ae, 0x8c51, 0x9cd3, 0xad55, 0xbdd7, 0xce59, 0xdefb, 0xef7d, 0xffff, 
    0x1840, 0x28c0, 0x3940, 0x49c0, 0x5a60, 0x6ae0, 0x7b60, 0x93e1, 0xa463, 0xb505, 0xc587, 0xd609, 0xe68b, 0xf70d, 0xffb0, 0xfff2, 
    0x3000, 0x4000, 0x50c0, 0x6140, 0x71c0, 0x8240, 0x92c1, 0xa363, 0xb3e5, 0xcc67, 0xdce9, 0xed6c, 0xfe0e, 0xfe90, 0xff13, 0xff96, 
    0x4020, 0x5000, 0x6040, 0x70c0, 0x8142, 0x91e4, 0xa266, 0xb2e8, 0xc36a, 0xd3ec, 0xe48e, 0xf510, 0xfd92, 0xfe15, 0xfe98, 0xff3b, 
    0x4000, 0x5002, 0x6003, 0x7085, 0x8107, 0x9189, 0xa20b, 0xb2ad, 0xc330, 0xd3b2, 0xe434, 0xf4b6, 0xfd58, 0xfddb, 0xfe5d, 0xfede, 
    0x3006, 0x4008, 0x5009, 0x606b, 0x70ed, 0x896f, 0x9a11, 0xaa93, 0xbb16, 0xcb98, 0xdc1a, 0xecbc, 0xfd3c, 0xfdbc, 0xfe3d, 0xfebe, 
    0x200b, 0x300d, 0x400e, 0x5090, 0x6112, 0x7194, 0x8236, 0x92b8, 0xa33a, 0xb3bc, 0xc43d, 0xd4dd, 0xe55d, 0xfddd, 0xfe5d, 0xfefe, 
    0x000e, 0x100f, 0x2071, 0x30f3, 0x4975, 0x59f7, 0x6a99, 0x7b1b, 0x8b9d, 0x9c1e, 0xac9e, 0xbd3e, 0xcdbe, 0xde3e, 0xeebe, 0xff3e, 
    0x000d, 0x004f, 0x08f2, 0x1974, 0x29f6, 0x3a78, 0x4afa, 0x5b9c, 0x6c1e, 0x7c9f, 0x951f, 0xa59f, 0xb63f, 0xc6bf, 0xd73f, 0xe7bf, 
    0x0049, 0x00ec, 0x016f, 0x01f1, 0x1273, 0x22f6, 0x3398, 0x4c1a, 0x5c9c, 0x6d1e, 0x7d9f, 0x8e3f, 0x9ebf, 0xaf3f, 0xbfbf, 0xcfff, 
    0x00c5, 0x0167, 0x01ea, 0x026d, 0x0aef, 0x1b71, 0x2c13, 0x3c95, 0x4d17, 0x5d99, 0x6e1b, 0x7ebe, 0x8f3f, 0x9fbf, 0xafff, 0xbfff, 
    0x0121, 0x01a2, 0x0224, 0x02a7, 0x0b49, 0x1bcb, 0x2c4d, 0x3cd0, 0x4d52, 0x5df4, 0x6e76, 0x7ef8, 0x8f7a, 0x9ffc, 0xaffd, 0xbfff, 
    0x0122, 0x01c2, 0x0a42, 0x02c3, 0x1343, 0x23e5, 0x3467, 0x44e9, 0x556c, 0x65ee, 0x7690, 0x8712, 0x9794, 0xaff6, 0xbff7, 0xcff9, 
    0x0121, 0x01a2, 0x0222, 0x1aa2, 0x2b22, 0x3bc2, 0x4c42, 0x5cc4, 0x6d47, 0x7dc9, 0x8e6b, 0x9eed, 0xaf6f, 0xbff1, 0xcff2, 0xdff4, 
    0x00c1, 0x1141, 0x21c1, 0x3241, 0x42c1, 0x5361, 0x63e1, 0x7462, 0x84e4, 0x9566, 0xa608, 0xb68a, 0xc70c, 0xdf8e, 0xeff0, 0xfff1, 
    0x1840, 0x28c0, 0x3940, 0x49e0, 0x6260, 0x72e0, 0x8360, 0x9401, 0xa483, 0xb505, 0xc587, 0xd629, 0xe6ac, 0xf70d, 0xffd0, 0xfff3,
};
#endif

#ifdef FBLUT_8B_ATARI
const uint16_t lut_atari[256] __attribute__((aligned(512), section(".scratch_x.lut"))) = {
    0xffff, 0xf7be, 0xef7d, 0xe71c, 0xdedb, 0xce99, 0xc638, 0xbdf7, 0xb5b6, 0xad75, 0xa534, 0x9cf3, 0x94b2, 0x8c71, 0x8430, 0x7bef, 
    0x73ae, 0x6b6d, 0x6b2d, 0x62ec, 0x5aab, 0x528a, 0x4a49, 0x4208, 0x39c7, 0x3186, 0x2945, 0x2104, 0x18c3, 0x1082, 0x0841, 0x0000, 
    0xfbef, 0xfaec, 0xfa08, 0xf800, 0xe800, 0xd800, 0xc800, 0xb800, 0xa800, 0x9800, 0x8800, 0x7800, 0x6800, 0x6000, 0x4800, 0x3800, 
    0xfed7, 0xfe52, 0xfdce, 0xfd4a, 0xfca7, 0xfc23, 0xfba0, 0xeb40, 0xdae0, 0xcac0, 0xc280, 0xb220, 0x99c0, 0x8940, 0x8100, 0x78c0, 
    0xeecb, 0xd5c8, 0xc4c6, 0xabc4, 0x9ac2, 0x8221, 0x7140, 0x5000, 0xffea, 0xffe0, 0xe6c2, 0xc5c2, 0xacc1, 0x8bc1, 0x7301, 0x5a41, 
    0xfff9, 0xfff5, 0xfff1, 0xffee, 0xeef0, 0xce10, 0xb54e, 0x940b, 0xdff4, 0xc712, 0xae30, 0x956d, 0x7c8b, 0x63a8, 0x4ac6, 0x3204, 
    0x77ea, 0x7769, 0x6ee9, 0x6668, 0x5de7, 0x5567, 0x4d06, 0x4485, 0x3c05, 0x3384, 0x2b04, 0x2a83, 0x2202, 0x1982, 0x1101, 0x0880, 
    0x07e0, 0x06e0, 0x05e0, 0x04e0, 0x03e0, 0x02e0, 0x0200, 0x0160, 0xb7dc, 0x67b9, 0x1f95, 0x0e92, 0x0590, 0x048d, 0x038a, 0x02a8, 
    0xcfdf, 0xa79f, 0x773f, 0x5ebf, 0x563f, 0x4ddf, 0x3d3f, 0x245b, 0x1b76, 0x1ab0, 0x11aa, 0x08e6, 0x7698, 0x4596, 0x1c51, 0x02ec, 
    0xe73f, 0xc63f, 0xad7f, 0x8c7f, 0x739f, 0x529f, 0x319f, 0x18df, 0x001f, 0x001b, 0x0018, 0x0015, 0x0012, 0x0010, 0x000c, 0x000a, 
    0xd5bf, 0xc4df, 0xab5f, 0x923f, 0x78bf, 0x681d, 0x5818, 0x4813, 0x300e, 0x200a, 0x94ba, 0x73b7, 0x52b4, 0x4210, 0x316b, 0x2109, 
    0xfe5f, 0xfd5f, 0xfc5f, 0xfb5f, 0xfa5f, 0xf81f, 0xd81b, 0xb817, 0xa014, 0x780f, 0x500a, 0x3006, 0xc720, 0xa640, 0x8d41, 0x6be2, 
    0xcbf9, 0xb376, 0x9af3, 0x8290, 0x6a0d, 0x598b, 0x4108, 0x28a5, 0xfe1b, 0xfcd7, 0xf394, 0xdab1, 0xc1ee, 0xb1ac, 0x998b, 0x8149, 
    0xff3b, 0xfdf7, 0xfcf3, 0xdc30, 0xcb8e, 0xc30c, 0xb2aa, 0xa208, 0xfe76, 0xfe13, 0xfdb1, 0xf54f, 0xed0e, 0xe4ad, 0xd44b, 0xcc0a, 
    0xbbc9, 0xb389, 0xab68, 0xa348, 0x9b07, 0x8ae7, 0x82a6, 0x7aa6, 0x7285, 0x6a45, 0x6224, 0x5204, 0x49c3, 0x4183, 0x3162, 0x2922, 
    0xbd31, 0xacd0, 0x9c4e, 0x93ec, 0x838b, 0x7b4a, 0x7309, 0x6ac8, 0x62a7, 0x5a66, 0x5a25, 0x49e4, 0xff3e, 0x0008, 0x0004, 0x07ff,
};
#endif

#ifdef FBLUT_8B_AURORA
const uint16_t lut_aurora[256] __attribute__((aligned(512), section(".scratch_x.lut"))) = {
    0x0000, 0x1082, 0x2104, 0x31a6, 0x4228, 0x52aa, 0x632c, 0x73ae, 0x8c51, 0x9cd3, 0xad55, 0xbdd7, 0xce59, 0xdefb, 0xef7d, 0xffff, 
    0x03ef, 0x45f7, 0x07ff, 0xbfff, 0x841f, 0x001f, 0x4217, 0x000f, 0x108a, 0x780f, 0xba17, 0xf01e, 0xfc1f, 0xfdf9, 0xfc10, 0xf800, 
    0xba08, 0x7800, 0x50a2, 0x7a00, 0xbbe8, 0xfbe0, 0xfdf0, 0xfff7, 0xffe0, 0xbde8, 0x7be0, 0x03e0, 0x45e8, 0x07e0, 0xaff5, 0x05ff, 
    0x03ff, 0x4bf8, 0xbd77, 0xcd51, 0xa512, 0x7cb2, 0x6c10, 0x7b6c, 0x9b4c, 0xbbce, 0xcc4e, 0xdccf, 0xed51, 0xf5d3, 0xf635, 0xf71a, 
    0x781f, 0x59e7, 0x7207, 0x8aaa, 0xab8e, 0xc471, 0xe555, 0xf69b, 0xe635, 0xc4ee, 0x8b8b, 0x72a7, 0x3964, 0x4204, 0x7387, 0x8c6b, 
    0xa50a, 0xb5ae, 0xc631, 0xded5, 0xef78, 0xc715, 0xae31, 0x8dea, 0x746b, 0x5be8, 0x4a86, 0x18e2, 0x2287, 0x3aa7, 0x532a, 0x3b89, 
    0x5c6b, 0x754e, 0x65f0, 0x8e31, 0xa6b4, 0xdfbe, 0xb779, 0xaf18, 0x8591, 0x53ec, 0x1349, 0x2164, 0x2209, 0x3b8e, 0x6555, 0x8e38, 
    0xaf1c, 0xc79d, 0xbe9d, 0xae3c, 0xa5db, 0x8d58, 0x5c78, 0x5b91, 0x3aae, 0x10c5, 0x2107, 0x39eb, 0x4a4e, 0x5ab1, 0x7375, 0x73b9, 
    0x8c78, 0xad5c, 0xcede, 0xe71f, 0xac78, 0x8ab8, 0x72b1, 0x59ee, 0x3927, 0x4989, 0x720e, 0x8ab1, 0xaab5, 0xab95, 0xed5b, 0xfede, 
    0xe63c, 0xddda, 0xd517, 0xc476, 0xc3f3, 0xc2d2, 0x4947, 0x30a4, 0x2844, 0x40c2, 0x60c0, 0xa0a1, 0xd902, 0xd289, 0xf9e1, 0xf2c6, 
    0xfb0c, 0xf5e6, 0xfd27, 0xd4c2, 0xdb61, 0xb2c0, 0x9a61, 0x6182, 0x5281, 0x6300, 0x8c0b, 0xaca0, 0xb581, 0xe6ab, 0xfea2, 0xff49, 
    0xc7e8, 0x9f69, 0x96c3, 0x7621, 0x6d41, 0x3b62, 0x29a1, 0x2221, 0x0ae1, 0x14a1, 0x0ea1, 0x1721, 0x7fee, 0x4f6b, 0x0622, 0x0d8a, 
    0x1c69, 0x11c6, 0x14d0, 0x0e12, 0x06ed, 0x2f54, 0x3ff4, 0x6ff9, 0x975f, 0x573f, 0x7ebd, 0x0efa, 0x14fb, 0x0acb, 0x196a, 0x11cf, 
    0x0253, 0x3332, 0x029e, 0x1b57, 0x23db, 0x6cf8, 0x4d3f, 0x957f, 0x5e3f, 0xbdde, 0x7b7d, 0x4adf, 0x621e, 0x39fe, 0x10fb, 0x0097, 
    0x2092, 0x0909, 0x5095, 0x6099, 0x819a, 0x9a1f, 0xbb1f, 0xb49f, 0xd53f, 0xd61e, 0xf63f, 0xe39f, 0xfa9f, 0xd91b, 0xb95f, 0xb898, 
    0x88b7, 0x58cf, 0x60ac, 0x400c, 0x3049, 0x50c7, 0x98d0, 0xc00f, 0xfa97, 0xfb58, 0xf516, 0xf9d1, 0xe0ef, 0xb887, 0x91a9, 0x90a7,
};
#endif

#ifdef FBLUT_4B_PICO8
const uint16_t lut_pico8[16] __attribute__((aligned(32), section(".scratch_x.lut"))) = {
    0x0000, 0x216a, 0x792a, 0x042a, 0xaa87, 0x62aa, 0xc618, 0xff9c, 0xf809, 0xfd00, 0xff45, 0x0707, 0x2d7f, 0x83b3, 0xfbb4, 0xfe55,
};
#endif

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
} cfg_fb_lut;

void framebuffer_pio_init() {
    // Claim PIO with SDK
    pio_sm_claim(FBLUT_PIO, FBLUT_PIO_SM);

    // Load base address to state machine register X
    uint32_t addrbase = (uint32_t)&FBLUT_USE_LUT[0];
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
    channel_config_set_dreq(&cfg, spi_get_dreq(LCD_SPI_PERIPHERAL, true));
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
        &spi0_hw->dr,
        NULL,
        1, false);
}

void setup_framebuffer(uint16_t fb_x, uint16_t fb_y, volatile uint8_t *ptr) {
    cfg_fb_lut.fb_len = fb_x * fb_y / FB_X_SCALE;
    cfg_fb_lut.fb_ptr = ptr;

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