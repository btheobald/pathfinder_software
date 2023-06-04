// Hardware Definitions
#include "pathfinder.h"

#include <math.h>
#include <wchar.h>
#include "sys/time.h"

#include "hagl_hal.h"
#include "hagl/bitmap.h"
#include "hagl.h"
#include "tusb.h"
#include "font6x9.h"
#include "font9x14.h"

#include "utility.h"

#include "rgb332.h"

#define EMBEDDED_CLI_IMPL
#include "embedded_cli.h"

#include <mcufont.h>

int seconds = 0;
volatile bool flush_flag = true;
volatile bool update_flag = false;

static hagl_backend_t *display;

const char pathfinder_str[] = "Pathfinder";

const char test_menu_str[] = "Test Menu";
const char test_sensor_str[] = "Sensor Test";
const char test_gps_str[] = "GPS Test";
const char test_sdio_str[] = "SDIO Test";
const char test_map_str[] = "Map Test";

bool flush_timer_callback(struct repeating_timer *t)
{
    flush_flag = true;
    return true;
}

bool second_timer_callback(struct repeating_timer *t)
{
    seconds++;
    update_flag = false;
    return true;
}

wchar_t print[30];

void onCommand(EmbeddedCli *embeddedCli, CliCommand *command) {
    printf("Received command: %s\r\n", command->name);
    embeddedCliTokenizeArgs(command->args);
    for (int i = 1; i <= embeddedCliGetTokenCount(command->args); ++i) {
        printf("\t arg %d : %s", i, embeddedCliGetToken(command->args, i));
    }
}

void writeChar(EmbeddedCli *embeddedCli, char c) {
    putchar(c);
}

typedef struct {
    uint8_t *buffer;
    uint16_t width;
    uint16_t height;
    uint16_t y;
    const struct mf_font_s *font;
} state_t;

bool count_lines(const char *line, uint16_t count, void *state)
{
    int *linecount = (int*)state;
    (*linecount)++;
    return true;
}

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
{
    uint8_t original = get_buffer()[y*HAGL_DISPLAY_WIDTH+x];
    int16_t color;
    uint8_t result;
    if(original & 0x08) { // If current colour is high luminance
        color = original - (alpha & 0x0F & original); // Subtract Alpha
        result = (color < 0x00) ? 0x00 : color;
    } else {
        color = original + (alpha & 0x0F & ~original); // Add Alpha
        result = (color > 0xFF) ? 0xFF : color;
    }

    hagl_draw_hline(display, x, y, count, result);  
}

static uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state)
{
    return mf_render_character(mf_find_font(mf_get_font_list()->font->short_name), x0, y0, character, &pixel_callback, state);
}

void main(void) {  
    gpio_set_function(LCD_BL_GPIO, GPIO_FUNC_PWM);

    gpio_set_pulls(SELECT_BUTTON_GPIO, true, false);

    stdio_init_all();
    while (!tud_cdc_connected() && gpio_get(SELECT_BUTTON_GPIO))
        sleep_ms(100);
    
    printf("\n\n*******************\n");
    printf(    "* %s *\n", pathfinder_str);
    printf(    "*******************\n");

    struct repeating_timer second_timer;
    struct repeating_timer flush_timer;

    hagl_color_t red = 8;
    hagl_color_t green = 56;
    hagl_color_t blue = 192;

    pathfinder_hw_setup();

    display = hagl_init();

    hagl_clear(display);
    hagl_flush(display);

    add_repeating_timer_ms(30, flush_timer_callback, NULL, &flush_timer);
    add_repeating_timer_ms(1000, second_timer_callback, NULL, &second_timer);

    int16_t x0, y0, code;
    int8_t color;

    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->maxBindingCount = 16;
    EmbeddedCli *cli = embeddedCliNew(config);
    cli->writeChar = writeChar;

    CliCommandBinding i2c_scan_binding;
    i2c_scan_binding.name = "i2c_scan";
    i2c_scan_binding.help = "Scan I2C bus for attached devices.";
    i2c_scan_binding.tokenizeArgs = false;
    i2c_scan_binding.context = NULL;
    i2c_scan_binding.binding = bus_scan_app;
    embeddedCliAddBinding(cli, i2c_scan_binding);

    uint8_t bl = 0;

    const struct mf_font_s * font = mf_find_font(mf_get_font_list()->font->short_name);
    printf("Font Loaded: %s", mf_get_font_list()->font->short_name);

    while (1) {
        int c;
        while((c = getchar_timeout_us(0)) > 0) {
            embeddedCliReceiveChar(cli, c);
        }

        if (flush_flag) {
            flush_flag = 0;
            hagl_fill_rectangle(display, 0, 0, 170, 320*(bl/(float)255), bl+16);
            
            mf_render_aligned(
            font,
            (HAGL_DISPLAY_WIDTH/2), (HAGL_DISPLAY_HEIGHT/2),
            MF_ALIGN_CENTER,
            "PATHFINDER", 10,
            &char_callback, NULL);
            
            hagl_flush(display);

            hagl_clear(display);

            if(bl < 255) {
                set_backlight(bl++);
            }
            
            embeddedCliProcess(cli);
        }

        
    }
} 

