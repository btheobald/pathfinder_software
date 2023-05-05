#include "pathfinder.h"

#include "pico/binary_info.h"

#include "ms5637.h"

#include "utility.h"

#include <stdio.h>

// The buffer size needs to be a power of two and alignment must be the same
__attribute__((aligned(32))) static char buffer[1024];

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

#define KHz *1000

void configure_i2c(void)
{
    printf("Configure I2C Hardware:");
    i2c_init(i2c1, 400 KHz);
    gpio_set_function(SENSOR_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SENSOR_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SENSOR_SDA_GPIO);
    gpio_pull_up(SENSOR_SCL_GPIO);
    gpio_set_drive_strength(SENSOR_SDA_GPIO, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(SENSOR_SCL_GPIO, GPIO_DRIVE_STRENGTH_12MA);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(SENSOR_SDA_GPIO, SENSOR_SCL_GPIO, GPIO_FUNC_I2C));

    printf("OK\n");

    bus_scan_app();
}

#define NMEA_MAX_LEN 82

// RX interrupt handler
/*void on_uart_rx() {
    volatile uint8_t ch, idx;
    // Add up to 82 char into interrupt buffer
    volatile static char nmea_buffer[NMEA_MAX_LEN];

    if(uart_get_hw(GPS_UART_PERIPHERAL)->ris & (1 << UART_UARTMIS_RTMIS_LSB)) {
        printf("\n  RT  \n");
    }

    /*while (uart_is_readable(GPS_UART_PERIPHERAL)) {
        ch = uart_getc(GPS_UART_PERIPHERAL);

        if(ch == '\n' || ch == '\r') {
            if(idx != 0) {
                // Valid line
                printf("%s\n", nmea_buffer);
                idx = 0;
                return;
            }

        } else {
            if((ch == '$') || idx == NMEA_MAX_LEN) {
                idx = 0;
            }

            nmea_buffer[idx++] = ch;
        }
    }
}*/

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
                printf("Buffer = %04d/1024\r", bfill);
            }

            dma_channel_abort(0);
            configure_dma(0);
        }
        if (events == GPIO_IRQ_EDGE_FALL)
        {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            // Configure DMA for incoming NMEA packet
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
    uart_set_fifo_enabled(GPS_UART_PERIPHERAL, false);

    printf("Configure GPS\n");

    char pmtk_set_pos_fix_100[] = "$PMTK220,100*2F\r\n";
    char pmtk_set_baudrate_115200[] = "$PMTK251,115200*1F\r\n";
    char pmtk_set_1pps_always_50ms[] = "$PMTK285,4,50*0C\r\n";
    char pmtk_set_1pps_nmea_sync[] = "$PMTK255,1*2D\r\n";

    // Now enable the UART to send interrupts - RX only
    // And set up and enable the interrupt handlers
    // irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    // irq_set_enabled(UART0_IRQ, true);
    // uart_set_irq_enables(GPS_UART_PERIPHERAL, true, false);

    sleep_ms(500);
    printf("Increase GPS baudrate: ");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_baudrate_115200, strlen(pmtk_set_baudrate_115200));
    sleep_ms(500);
    printf("%d OK\n\r", uart_set_baudrate(GPS_UART_PERIPHERAL, 115200));
    sleep_ms(500);
    printf("Enable 1PPS Always, 50ms pulse\n");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_1pps_always_50ms, strlen(pmtk_set_1pps_always_50ms));
    sleep_ms(500);
    printf("Enable 1PPS/NMEA Sync\n");
    uart_write_blocking(GPS_UART_PERIPHERAL, pmtk_set_1pps_nmea_sync, strlen(pmtk_set_1pps_nmea_sync));
    sleep_ms(500);

    printf("Enable 1PPS IRQ\n");
    gpio_set_irq_enabled_with_callback(GPS_1PPS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(GPS_3DFIX_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_pulls(GPS_1PPS_GPIO, false, false);

    printf("Waiting for GPS\n");
}

void pathfinder_hw_setup(void)
{
    printf("\nConfigure Hardware\n");
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    configure_i2c();
    configure_gps();

    uint8_t str_start = 0;
    uint8_t str_end = 0;
    char nmea_line[NMEA_MAX_LEN];
}
