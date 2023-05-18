// Hardware Definitions
#include "pathfinder.h"

#include <math.h>
#include <wchar.h>
#include "sys/time.h"

#include "hagl_hal.h"
#include "hagl/bitmap.h"
#include "hagl.h"
#include "tusb.h"
#include "font9x14.h"

volatile bool fps_flag = false;
volatile bool switch_flag = false;
volatile bool flush_flag = true;

static hagl_backend_t *display;

bool
flush_timer_callback(struct repeating_timer *t)
{
    flush_flag = true;
    return true;
}

wchar_t print[30];

void main(void) {  
    stdio_init_all();
    while (!tud_cdc_connected())
        sleep_ms(100);
    
    const char pathfinder[16] = "PATHFINDER DEMO\0";

    printf("\n\n*******************\n");
    printf(    "* %s *\n", pathfinder);
    printf(    "*******************\n");

    struct repeating_timer switch_timer;
    struct repeating_timer fps_timer;
    struct repeating_timer flush_timer;

    hagl_color_t red = 8;
    hagl_color_t green = 56;
    hagl_color_t blue = 192;

    pathfinder_hw_setup();

    display = hagl_init();

    hagl_clear(display);

    hagl_flush(display);

    add_repeating_timer_ms(33, flush_timer_callback, NULL, &flush_timer);

    int16_t x0, y0, code;
    int8_t color;

    swprintf(print, sizeof(print), L"PATHFINDER");
    hagl_put_text(display, print, (HAGL_DISPLAY_WIDTH/2)-45, (HAGL_DISPLAY_HEIGHT/2)-20, green, IBM_MDA_9x14);

    while (1) {
    
        if (flush_flag) {
            flush_flag = 0;
            hagl_flush(display);
        }
    }
} 

