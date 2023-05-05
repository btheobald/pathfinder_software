// Hardware Definitions
#include "pathfinder.h"
#include "utility.h"
#include "ms5637.h"

void vLaunch(void)
{
    // Main App Launch
    ms5637_t baro0;
    ms5637_status_t ret = ms5637_init(&baro0, i2c1);

    if(ret != MS5637_STATUS_OK) {
        printf("MS5637 Init Failed");
    }
}

void main(void)
{
    tusb_cdc_wait();
 
    printf("\n*******************\n");
    printf(  "* PATHFINDER DEMO *\n");
    printf("\n*******************\n\n");
    
    pathfinder_hw_setup();

    while(1) {
        
    }
    //vLaunch();
} 

