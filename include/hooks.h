#ifndef PATHFINDER_HOOKS_HEADER_GUARD
#define PATHFINDER_HOOKS_HEADER_GUARD

// Scheduler include files.
#include "FreeRTOS.h"
#include "task.h"

// Prototypes for the standard FreeRTOS callback/hook functions
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);

#endif // PATHFINDER_HOOKS_HEADER_GUARD