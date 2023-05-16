#include "gps_d.h"
#include <stdio.h>
#include <string.h>

bool gps_data_ready = false;
bool gps_fix = false;

// The buffer size needs to be a power of two and alignment must be the same
static char buffer[1024] __attribute__((aligned(1024)));

void uart_rx_irq();
void gps_gpio_irq(uint gpio, uint32_t events);

void configure_gps(void) {
    printf("Configure UART\n");

    uart_init(GPS_UART_PERIPHERAL, 9600);
    gpio_set_function(GPS_UART_RX, GPIO_FUNC_UART);
    gpio_set_function(GPS_UART_TX, GPIO_FUNC_UART);
    uart_set_hw_flow(GPS_UART_PERIPHERAL, false, false);
    uart_set_format(GPS_UART_PERIPHERAL, 8, 1, 0);
    uart_set_fifo_enabled(GPS_UART_PERIPHERAL, true);

    printf("Configure GPS\n");

    const char pmtk_set_pos_fix_100[] = "$PMTK220,100*2F\r\n";
    const char pmtk_set_baudrate_115200[] = "$PMTK251,115200*1F\r\n";
    const char pmtk_set_1pps_always_50ms[] = "$PMTK285,4,50*0C\r\n";
    const char pmtk_set_1pps_nmea_sync[] = "$PMTK255,1*2D\r\n";
    const char pmtk_set_gps_glonass_galileo[] = "$PMTK353,1,1,1,0,0*2A\r\n";

    // Now enable the UART to send interrupts - RX only
    // And set up and enable the interrupt handlers
    // Useful to get early configuration command responses
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_irq);
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
    gpio_set_irq_enabled_with_callback(GPS_1PPS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gps_gpio_irq);
    gpio_set_irq_enabled(GPS_3DFIX_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_pulls(GPS_1PPS_GPIO, false, false);

    // Disable UART IRQ since we have configured high-rate DMA
    uart_set_irq_enables(GPS_UART_PERIPHERAL, false, false);
    irq_set_enabled(UART0_IRQ, false);
    irq_remove_handler(UART0_IRQ, uart_rx_irq);
    
    printf("Waiting for GPS\n");
}

void configure_dma(int channel) {
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

// This probably needs to move from HAL to higher level
void gps_decode(void) {
    if(!gps_data_ready)
        return;
    
    gps_data_ready = false;

    char line[NMEA_MAX_LEN];

    uint16_t start = 0;
    uint16_t end = 0;

    while (buffer[start] != 0) {
        // Find sentence start
        for(uint16_t ch = end; ch < 1024; ch++) {
            if(buffer[ch] == '$') {
                start = ch;
                break;
            }
        }

        // Find sentence end
        for(uint16_t len = 8; len < NMEA_MAX_LEN; len++) {
            if(buffer[start+len] == '$') {
                buffer[start+len-1] = 0;
                end = start+len;
                break;
            }
        }

        if(start >= end) break;

        strcpy(line, buffer + start);

        // TODO: Proccess GPS Here
    }

}

// Byte-by-byte UART processing (Suitable for low-speed 9600 baud only)
// This is used for status reads from GPS before 115200 baud configured.
void uart_rx_irq() {
    volatile uint8_t ch, idx;
    // Add up to 82 char into interrupt buffer
    volatile static char nmea_buffer[NMEA_MAX_LEN];

    while (uart_is_readable(GPS_UART_PERIPHERAL)) {
        ch = uart_getc(GPS_UART_PERIPHERAL);

        if(ch == '\n') {
            // Valid line
            nmea_buffer[idx] = 0;
            if(nmea_buffer[0]=='$' && nmea_buffer[1]=='P')
                printf("%s\n", nmea_buffer); // Only print PMTK messages for now. TODO: Check ACKs
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

void gps_gpio_irq(uint gpio, uint32_t events) {
    if (gpio == GPS_1PPS_GPIO) {
        if (events == GPIO_IRQ_EDGE_RISE) {
            // Do the buffer contents start with $?            
            if (buffer[0] == '$') {
                gps_data_ready = true;
            }

            // Abort existing DMA
            dma_channel_abort(0);

            // 1PPS onto STATUS
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        } if (events == GPIO_IRQ_EDGE_FALL) {
            // Configure DMA for next NMEA packet
            configure_dma(0);
            dma_channel_start(0);

            // 1PPS onto STATUS
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }
    }

    if (gpio == GPS_3DFIX_GPIO) {
        if (events == GPIO_IRQ_EDGE_RISE) {
            gps_fix = true;
        }
        if (events == GPIO_IRQ_EDGE_FALL) {
            gps_fix = false;
        }
    }
}