// Hardware Definitions
#include "pathfinder.h"
#include "utility.h"
#include "ms5637.h"
#include "st7735.h"

void vLaunch(void)
{
    // Main App Launch
    ms5637_t baro0;
    ms5637_status_t ret = ms5637_init(&baro0, SENSOR_I2C_PERIPHERAL);

    if(ret != MS5637_STATUS_OK) {
        printf("MS5637 Init Failed");
    }
}

void main(void)
{
    tusb_cdc_wait();
 
    printf("\n\n*******************\n");
    printf(    "* PATHFINDER DEMO *\n");
    printf(    "*******************\n");

    pathfinder_hw_setup();

    st7735_t lcd0;
    st7735_init(&lcd0, LCD_SPI_PERIPHERAL, LCD_DC_GPIO);
    printf("Configure LCD - ST7735 - OK\n");

    while(1) {
        sleep_ms(1000);
        //gps_decode();
    }

    //vLaunch();
} 

