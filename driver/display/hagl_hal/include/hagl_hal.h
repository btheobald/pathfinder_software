#include <hagl/backend.h>
#include <hagl.h>
#include <string.h>

#include "st7735.h"

#define HAGL_HAS_HAL_BACK_BUFFER

#define HAGL_DISPLAY_WIDTH     (170)
#define HAGL_DISPLAY_HEIGHT    (320)
#define HAGL_DISPLAY_DEPTH     (8)
#define HAGL_CHAR_BUFFER_SIZE  (16 * 16 * HAGL_DISPLAY_DEPTH / 8)

void hagl_hal_init(hagl_backend_t *backend);

uint8_t * get_buffer();