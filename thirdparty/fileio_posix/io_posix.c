#include "io_posix.h"

int init_buffer(fb_handler * fbh, char * filename) {
    fbh->buffer_pos = 0;
    fbh->bytes_read = 0;

    printf("Opening: %s\n\r", filename);
    f_open(&fbh->fil, filename, FA_READ);
    if(&fbh->fil == NULL) {
        printf("Failed\n\r");
        return 1;
    }

    load_buffer(fbh); 

    return 0;
}

void load_buffer(fb_handler * fbh) {
    fbh->buffer_pos = 0; // Reset buffer offset
    f_read(&fbh->fil, fbh->buffer_ptr, FILE_READ_BUFFER_SIZE, &fbh->bytes_read);
}

uint16_t get_remaining_bytes(fb_handler * fbh) {
    return (fbh->bytes_read - fbh->buffer_pos);
}

void relative_reset_buffer(fb_handler * fbh, int16_t seek) {
    fbh->buffer_pos = 0;
    file_seek_rel(fbh, -seek);
}