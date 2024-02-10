#include <stdint.h>
#include "stdio.h"

#include "ff.h"

#define FILE_READ_BUFFER_SIZE 2048  

typedef struct _file_buffer_handler {
    FIL fil;
    uint16_t buffer_pos;
    unsigned int bytes_read;
    uint8_t buffer_ptr[FILE_READ_BUFFER_SIZE];
} fb_handler;

int init_buffer(fb_handler * fbh, char * filename);
void load_buffer(fb_handler * fbh);
void relative_reset_buffer(fb_handler * fbh, int16_t seek);
uint16_t get_remaining_bytes(fb_handler * fbh);

static inline void file_seek(fb_handler * fbh, int32_t offset) { 
    printf("IO_POSIX: Seek %d\n\r", offset);
    f_lseek(&fbh->fil, offset);
    load_buffer(fbh);
};

static inline void file_seek_rel(fb_handler * fbh, int32_t offset) { 
    printf("IO_POSIX: RelSeek %d\n\r", f_tell(&fbh->fil) + offset);
    f_lseek(&fbh->fil, f_tell(&fbh->fil) + offset);
    load_buffer(fbh);
};

static inline void file_close(fb_handler * fbh) { 
    f_close(&fbh->fil); 
};