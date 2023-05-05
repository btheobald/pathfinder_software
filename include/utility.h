#ifndef PATHFINDER_HOOKS_HEADER_GUARD
#define PATHFINDER_HOOKS_HEADER_GUARD

#include "tusb.h"

int bus_scan_app(void);
void tusb_cdc_wait(void);

#endif // PATHFINDER_HOOKS_HEADER_GUARD