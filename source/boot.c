// Hardware Definitions
#include "pathfinder.h"
#include "utility.h"
#include "ms5637.h"
#include "st7735.h"

#include "fb_dma_pio_lut.h"

void vLaunch(void)
{
    // Main App Launch
    ms5637_t baro0;
    ms5637_status_t ret = ms5637_init(&baro0, SENSOR_I2C_PERIPHERAL);

    if(ret != MS5637_STATUS_OK) {
        printf("MS5637 Init Failed");
    }
}

uint8_t fb[(XRES*YRES)/FB_X_SCALE];// __attribute__((aligned(FB_ALIGN))) ;

void main(void) {
    tusb_cdc_wait();
 
    printf("\n\n*******************\n");
    printf(    "* PATHFINDER DEMO *\n");
    printf(    "*******************\n");

    pathfinder_hw_setup();

    st7735_t lcd0;
    st7735_init(&lcd0, LCD_SPI_PERIPHERAL, LCD_DC_GPIO, XRES, YRES);
    printf("Configure LCD - ST7735 - OK\n");

    setup_framebuffer(XRES, YRES, fb);
    st7735_setup_fill(&lcd0);
    trigger_framebuffer_dma();

    uint16_t t = 0;
    uint16_t gps = 0;

    while(1) {
        st7735_setup_fill(&lcd0);
        trigger_framebuffer_dma();

        sleep_us(5000);

        uint16_t p = 0;
        uint8_t xb = 0;
        uint8_t yb = 0;
        uint8_t cb = 0;
                
        for(uint16_t p = 0; p < (YRES*XRES/FB_X_SCALE); p++) { 
            

            xb = (p % XRES)/8; 
            yb = (p / XRES)/8;
            cb = yb * (XRES/16) + xb;

            #ifdef LUT_4B
            fb[p] = (((x)/20) + ((y+t)/32)*4)%16;
            fb[p] |= (((((x)/20) + ((y+t)/32)*4))%16 << 4);
            #else
            fb[p] = cb+t;
            #endif
        }

        t++;

        #ifdef ENABLE_GPS
        gps+=30;
        if(gps > 1000) {
            gps_decode();
            gps = 0;
        }
        #endif
    }

    //vLaunch();
} 

