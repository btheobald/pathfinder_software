#include "pathfinder.h"

#include "pico/binary_info.h"

#include "utility.h"

#include "minmea.h"

#include <stdio.h>

#include "hardware/sync.h"

#define KHz *1000
#define MHz *1000000
#define NMEA_MAX_LEN 82
#define INDENT_SPACES "  "

// The buffer size needs to be a power of two and alignment must be the same
__attribute__((aligned(1024))) static char buffer[1024];

static void configure_dma(int channel)
{
    dma_channel_config config = dma_channel_get_default_config(0);

    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);

    // The read address is the address of the UART data register which is constant
    channel_config_set_read_increment(&config, false);

    // Write into a ringbuffer with '2^5=32' elements
    channel_config_set_write_increment(&config, true);
    channel_config_set_ring(&config, true, 10);

    // The UART signals when data is avaliable
    channel_config_set_dreq(&config, DREQ_UART0_RX);

    dma_channel_configure(channel, &config, buffer, &uart0_hw->dr, 1024, false);
}

void configure_i2c(void)
{
    printf("Configure I2C Hardware:");
    
    gpio_set_function(SENSOR_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SENSOR_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SENSOR_SDA_GPIO);
    gpio_pull_up(SENSOR_SCL_GPIO);
    gpio_set_drive_strength(SENSOR_SDA_GPIO, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(SENSOR_SCL_GPIO, GPIO_DRIVE_STRENGTH_12MA);

    i2c_init(SENSOR_I2C_PERIPHERAL, 400 KHz);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(SENSOR_SDA_GPIO, SENSOR_SCL_GPIO, GPIO_FUNC_I2C));

    printf("OK\n");

    bus_scan_app();
}

void on_uart_rx() {
    volatile uint8_t ch, idx;
    // Add up to 82 char into interrupt buffer
    volatile static char nmea_buffer[NMEA_MAX_LEN];

    while (uart_is_readable(GPS_UART_PERIPHERAL)) {
        ch = uart_getc(GPS_UART_PERIPHERAL);

        if(ch == '\n') {
            // Valid line
            nmea_buffer[idx] = 0;
            if(nmea_buffer[0]=='$' && nmea_buffer[1]=='P')
                printf("%s\n", nmea_buffer); // Only print PMTK messages for now.
            idx = 0;
            return;

        } else {
            if((ch == '$') || idx == NMEA_MAX_LEN) {
                idx = 0;
            }

            nmea_buffer[idx++] = ch;
        }
    }
}

bool gps_data_ready;

void gps_decode(void) {
    if(!gps_data_ready)
        return;
    
    gps_data_ready = false;

    char line[MINMEA_MAX_SENTENCE_LENGTH];

    uint16_t start = 0;
    uint16_t end = 0;
    
    //uint32_t stat = save_and_disable_interrupts();
    //dma_channel_abort(0);

    //printf("%c[2J", 27);
    //printf("%c[H", 27);

    printf("\n%s\n", buffer);

    while (buffer[start] != 0) {
        // Find sentence start
        for(uint16_t ch = end; ch < 1024; ch++) {
            if(buffer[ch] == '$') {
                start = ch;
                break;
            }
        }

        // Find sentence end
        for(uint16_t len = 8; len < MINMEA_MAX_SENTENCE_LENGTH; len++) {
            if(buffer[start+len] == '$') {
                buffer[start+len-1] = 0;
                end = start+len;
                break;
            }
        }

        //printf("\nS:%d=%c E:%d=%c\n\r", start, buffer[start], end, buffer[end]);        
        if(start >= end) break;

        strcpy(line, buffer + start);

        //printf("%s\n\r", line);
        switch (minmea_sentence_id(line, false)) {
            case MINMEA_SENTENCE_RMC: {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, line)) {
                    printf("%02d:%02d:%02d:%06d\n", frame.time.hours, frame.time.minutes, frame.time.seconds, frame.time.microseconds);
                    printf(INDENT_SPACES "$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                            frame.latitude.value, frame.latitude.scale,
                            frame.longitude.value, frame.longitude.scale,
                            frame.speed.value, frame.speed.scale);
                    printf(INDENT_SPACES "$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                            minmea_rescale(&frame.latitude, 1000),
                            minmea_rescale(&frame.longitude, 1000),
                            minmea_rescale(&frame.speed, 1000));
                    printf(INDENT_SPACES "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
                            minmea_tocoord(&frame.latitude),
                            minmea_tocoord(&frame.longitude),
                            minmea_tofloat(&frame.speed));
                }
                else {
                    printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GSA: {
                struct minmea_sentence_gsa frame;
                if (minmea_parse_gsa(&frame, line)) {
                    printf(INDENT_SPACES "$xxGSA: fix mode: %c\n", frame.mode);
                    printf(INDENT_SPACES "        fix type: %d\n", frame.fix_type);
                    printf(INDENT_SPACES "        pdop: %01.2f\n", minmea_tofloat(&frame.pdop));
                    printf(INDENT_SPACES "        hdop: %01.2f\n", minmea_tofloat(&frame.hdop));
                    printf(INDENT_SPACES "        vdop: %01.2f\n", minmea_tofloat(&frame.vdop));
                }
                else {
                    printf(INDENT_SPACES "$xxGSA sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GGA: {
                struct minmea_sentence_gga frame;
                if (minmea_parse_gga(&frame, line)) {
                    printf(INDENT_SPACES "$xxGGA: fix quality: %d\n", frame.fix_quality);
                }
                else {
                    printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GST: {
                struct minmea_sentence_gst frame;
                if (minmea_parse_gst(&frame, line)) {
                    printf(INDENT_SPACES "$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
                            frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
                            frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
                            frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
                    printf(INDENT_SPACES "$xxGST fixed point latitude,longitude and altitude error deviation"
                           " scaled to one decimal place: (%d,%d,%d)\n",
                            minmea_rescale(&frame.latitude_error_deviation, 10),
                            minmea_rescale(&frame.longitude_error_deviation, 10),
                            minmea_rescale(&frame.altitude_error_deviation, 10));
                    printf(INDENT_SPACES "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
                            minmea_tofloat(&frame.latitude_error_deviation),
                            minmea_tofloat(&frame.longitude_error_deviation),
                            minmea_tofloat(&frame.altitude_error_deviation));
                }
                else {
                    printf(INDENT_SPACES "$xxGST sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_GSV: {
                struct minmea_sentence_gsv frame;
                if (minmea_parse_gsv(&frame, line)) {
                    printf(INDENT_SPACES "$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                    printf(INDENT_SPACES "$xxGSV: satellites in view: %d\n", frame.total_sats);
                    for (int i = 0; i < 4; i++)
                        printf(INDENT_SPACES "$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                            frame.sats[i].nr,
                            frame.sats[i].elevation,
                            frame.sats[i].azimuth,
                            frame.sats[i].snr);
                }
                else {
                    printf(INDENT_SPACES "$xxGSV sentence is not parsed\n");
                }
            } break;

            case MINMEA_SENTENCE_VTG: {
               struct minmea_sentence_vtg frame;
               if (minmea_parse_vtg(&frame, line)) {
                    printf(INDENT_SPACES "$xxVTG: true track degrees = %f\n",
                           minmea_tofloat(&frame.true_track_degrees));
                    printf(INDENT_SPACES "        magnetic track degrees = %f\n",
                           minmea_tofloat(&frame.magnetic_track_degrees));
                    printf(INDENT_SPACES "        speed knots = %f\n",
                            minmea_tofloat(&frame.speed_knots));
                    printf(INDENT_SPACES "        speed kph = %f\n",
                            minmea_tofloat(&frame.speed_kph));
               }
               else {
                    printf(INDENT_SPACES "$xxVTG sentence is not parsed\n");
               }
            } break;

            case MINMEA_SENTENCE_ZDA: {
                struct minmea_sentence_zda frame;
                if (minmea_parse_zda(&frame, line)) {
                    printf(INDENT_SPACES "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
                           frame.time.hours,
                           frame.time.minutes,
                           frame.time.seconds,
                           frame.date.day,
                           frame.date.month,
                           frame.date.year,
                           frame.hour_offset,
                           frame.minute_offset);
                }
                else {
                    printf(INDENT_SPACES "$xxZDA sentence is not parsed\n");
                }
            } break;

            case MINMEA_INVALID: {
                printf(INDENT_SPACES "$xxxxx sentence is not valid\n");
            } break;

            default: {
                printf(INDENT_SPACES "$xxxxx sentence is not parsed\n");
            } break;
        }
    }

    printf("\n\n");

    //restore_interrupts(stat);
}

void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == GPS_1PPS_GPIO)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            // Clear existing DMA and reconfigure

            if (buffer[0] == '$')
            {
                uint16_t bfill = 1024 - dma_channel_hw_addr(0)->transfer_count;
                buffer[bfill] = 0;
                printf("Buffer = %04d/1024\n\r", bfill);
                gps_data_ready = true;
            }

            dma_channel_abort(0);
            
        }
        if (events == GPIO_IRQ_EDGE_FALL)
        {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            // Configure DMA for incoming NMEA packet
            configure_dma(0);
            dma_channel_start(0);
        }
    }

    if (gpio == GPS_3DFIX_GPIO)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("------TRACK OK------\n");
        }
        if (events == GPIO_IRQ_EDGE_FALL)
        {
            printf("-----TRACK LOST-----\n");
        }
    }
}

void configure_gps(void)
{
    printf("Configure UART\n");

    uart_init(GPS_UART_PERIPHERAL, 9600);
    gpio_set_function(GPS_UART_RX, GPIO_FUNC_UART);
    gpio_set_function(GPS_UART_TX, GPIO_FUNC_UART);
    uart_set_hw_flow(GPS_UART_PERIPHERAL, false, false);
    uart_set_format(GPS_UART_PERIPHERAL, 8, 1, 0);
    uart_set_fifo_enabled(GPS_UART_PERIPHERAL, true);

    printf("Configure GPS\n");

    char pmtk_set_pos_fix_100[] = "$PMTK220,100*2F\r\n";
    char pmtk_set_baudrate_115200[] = "$PMTK251,115200*1F\r\n";
    char pmtk_set_1pps_always_50ms[] = "$PMTK285,4,50*0C\r\n";
    char pmtk_set_1pps_nmea_sync[] = "$PMTK255,1*2D\r\n";
    char pmtk_set_gps_glonass_galileo[] = "$PMTK353,1,1,1,0,0*2A\r\n";

    // Now enable the UART to send interrupts - RX only
    // And set up and enable the interrupt handlers
    // Useful to get early configuration command responses
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(GPS_UART_PERIPHERAL, true, false);

    sleep_ms(2000);
    printf("Enable Search GPS/GLONASS/GALILEO\n");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_gps_glonass_galileo, strlen(pmtk_set_gps_glonass_galileo));
    sleep_ms(2000);
    printf("Enable 1PPS Always, 50ms pulse\n");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_1pps_always_50ms, strlen(pmtk_set_1pps_always_50ms));
    sleep_ms(2000);
    printf("Enable 1PPS/NMEA Sync\n");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_1pps_nmea_sync, strlen(pmtk_set_1pps_nmea_sync));
    sleep_ms(2000);

    printf("Increase GPS baudrate: ");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_baudrate_115200, strlen(pmtk_set_baudrate_115200));
    sleep_ms(2000);
    printf("%d OK\n\r", uart_set_baudrate(GPS_UART_PERIPHERAL, 115200));

    // Triggers DMA to deal with high-rate NMEA.
    printf("Enable 1PPS IRQ\n");
    gpio_set_irq_enabled_with_callback(GPS_1PPS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(GPS_3DFIX_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_pulls(GPS_1PPS_GPIO, false, false);

    printf("Waiting for GPS\n");
}

void configure_spi(void) {
    printf("Configure SPI Hardware:");

    gpio_set_function(LCD_SCLK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(LCD_CS_GPIO, GPIO_FUNC_SPI);

    spi_init(LCD_SPI_PERIPHERAL, 62.5 MHz);
    
    // Default - 6.36ms per frame
    spi_set_format(LCD_SPI_PERIPHERAL, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // SPI MODE 3 - 5.36ms per Frame

    bi_decl(bi_3pins_with_func(LCD_SCLK_GPIO, LCD_MOSI_GPIO, LCD_CS_GPIO, GPIO_FUNC_SPI));

    printf("OK\n");
}

void pathfinder_hw_setup(void)
{
    printf("\nConfigure Hardware\n");
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    configure_spi();    
    configure_i2c();

    #ifdef ENABLE_GPS
        configure_gps();
    #endif

    uint8_t str_start = 0;
    uint8_t str_end = 0;
    char nmea_line[NMEA_MAX_LEN];
}
