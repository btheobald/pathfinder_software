#ifndef LUT_HEADER_GUARD
#define LUT_HEADER_GUARD

#include <stdint.h>

// Use 4 Bit colour mode to half framebuffer size
//#define LUT_4B

#ifdef LUT_4B
#define FB_ALIGN 16384
#define FB_X_SCALE 2
#else
#define FB_ALIGN 32768
#define FB_X_SCALE 1
#endif

#ifdef LUT_4B
#define FBLUT_COLORS 16
#define FBLUT_4B_PICO8 lut_pico8
#define FBLUT_4B_RGB4 lut_rgb4b
#define FBLUT_4B_COLORBLIND lut_colorblind
#define FBLUT_4B_CYBERWAV lut_cyberwav
#define FBLUT_4B_EUROPA lut_europa
#define FBLUT_4B_FANTASY lut_fantasy
#define FBLUT_4B_FUN lut_fun
#define FBLUT_4B_GOLINE lut_goline
#define FBLUT_4B_SWEETIE lut_sweetie
#define FBLUT_4B_ZXARNE lut_zxarne
#define FB_USE_LUT FBLUT_4B_SWEETIE
#else

#define FBLUT_COLORS 256
//#define FBLUT_8B_DUEL lut_duel
//#define FBLUT_8B_SONIC lut_sonic
#define FBLUT_8B_ATARI lut_atari
//#define FBLUT_8B_AURORA lut_aurora
//#define FBLUT_8B_UZEBOX lut_uzebox
//#define FBLUT_8B_RGBM lut_rgbm
#define FB_USE_LUT FBLUT_8B_ATARI
#endif

// 8-BIT Color Palletes

#ifdef FBLUT_8B_DUEL
extern const uint16_t lut_duel[256];
#endif

#ifdef FBLUT_8B_SONIC
extern const uint16_t lut_sonic[256];
#endif

#ifdef FBLUT_8B_ATARI
extern const uint16_t lut_atari[256] __attribute__((aligned(FBLUT_COLORS*2), section(".scratch_x.lut")));
#endif

#ifdef FBLUT_8B_AURORA
extern const uint16_t lut_aurora[256];
#endif

#ifdef FBLUT_8B_UZEBOX
extern const uint16_t lut_uzebox[256] __attribute__((aligned(FBLUT_COLORS*2), section(".scratch_x.lut")));
#endif

#ifdef FBLUT_8B_RGBM
extern const uint16_t lut_rgbm[256] __attribute__((aligned(FBLUT_COLORS*2), section(".scratch_x.lut")));
#endif

// 4-BIT Color Palletes

#ifdef FBLUT_4B_PICO8
extern const uint16_t lut_pico8[16]; 
#endif

#ifdef FBLUT_4B_RGB4
extern const uint16_t lut_rgb4b[16];
#endif

#ifdef FBLUT_4B_COLORBLIND
extern const uint16_t lut_colorblind[16];
#endif

#ifdef FBLUT_4B_CYBERWAV
extern const uint16_t lut_cyberwav[16];
#endif

#ifdef FBLUT_4B_EUROPA
extern const uint16_t lut_europa[16];
#endif

#ifdef FBLUT_4B_FANTASY
extern const uint16_t lut_fantasy[16];
#endif

#ifdef FBLUT_4B_FUN
extern const uint16_t lut_fun[16];
#endif

#ifdef FBLUT_4B_GOLINE
extern const uint16_t lut_goline[16];
#endif

#ifdef FBLUT_4B_SWEETIE
extern const uint16_t lut_sweetie[16];
#endif

#ifdef FBLUT_4B_ZXARNE
extern const uint16_t lut_zxarne[16];
#endif

#endif //LUT_HEADER_GUARD