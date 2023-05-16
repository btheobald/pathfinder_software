#ifndef PATHFINDER_GPS_D_HEADER_GUARD
#define PATHFINDER_GPS_D_HEADER_GUARD

#include <hardware/uart.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>

#include "pathfinder.h"

#define NMEA_MAX_LEN 82

void configure_gps(void);
void configure_dma(int channel);

#endif // PATHFINDER_GPS_D_HEADER_GUARD