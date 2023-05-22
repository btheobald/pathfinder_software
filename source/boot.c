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

#define EMBEDDED_CLI_IMPL
#include "embedded_cli.h"

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

void main(void) {  
    gpio_set_function(LCD_BL_GPIO, GPIO_FUNC_PWM);

    stdio_init_all();
    while (!tud_cdc_connected())
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

    swprintf(print, sizeof(print), L"%s", pathfinder_str);
    hagl_put_text(display, print, (HAGL_DISPLAY_WIDTH/2)-45, (HAGL_DISPLAY_HEIGHT/2)-25, 0xff, IBM_MDA_9x14);

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

    while (1) {
        int c;
        while((c = getchar_timeout_us(0)) > 0) {
            embeddedCliReceiveChar(cli, c);
        }

        if(seconds > 5 & !update_flag) {
            update_flag = true;
            hagl_clear(display);
        }

        if (flush_flag) {
            flush_flag = 0;
            hagl_flush(display);

            if(seconds < 5) {
                set_backlight(bl++);
                hagl_draw_rectangle(display, 0, 80, bl, 85, bl);
            }
            
            embeddedCliProcess(cli);
        }
    }
} 

