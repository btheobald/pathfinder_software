#include "parse.h"

static uint8_t byte_buffer[16];
static uint32_t valvar;

void get_bytes(fb_handler * fbh, uint8_t bytes) {
    // Check Highwater, reload buffer if needed
    if(bytes == 1) {
        if(!get_remaining_bytes(fbh)) {
            load_buffer(fbh);
        }
    } else if(fbh->buffer_pos + bytes > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    memcpy(byte_buffer, fbh->buffer_ptr + fbh->buffer_pos, bytes);
    fbh->buffer_pos += bytes;
}

uint8_t get_uint8(fb_handler * fbh) {
    get_bytes(fbh, 1);
    return byte_buffer[0];
}

uint16_t get_uint16(fb_handler * fbh) {
    get_bytes(fbh, 2);
    return __builtin_bswap16(((uint16_t*)byte_buffer)[0]);
}

uint32_t get_uint32(fb_handler * fbh) {
    get_bytes(fbh, 4);
    return __builtin_bswap32(((uint32_t*)byte_buffer)[0]);
}

uint64_t get_uint64(fb_handler * fbh) {
    get_bytes(fbh, 8);
    return __builtin_bswap64(((uint64_t*)byte_buffer)[0]);
}

int_least8_t get_int8(fb_handler * fbh) {
    get_bytes(fbh, 1);
    return ((int_least8_t*)byte_buffer)[0];
}

int16_t get_int16(fb_handler * fbh) {
    get_bytes(fbh, 2);
    return __builtin_bswap16(((int16_t*)byte_buffer)[0]);
}

int32_t get_int32(fb_handler * fbh) {
    get_bytes(fbh, 4);
    return __builtin_bswap32(((int32_t*)byte_buffer)[0]);
}

int64_t get_int64(fb_handler * fbh) {
    get_bytes(fbh, 8);
    return __builtin_bswap64(((int64_t*)byte_buffer)[0]);
}

uint64_t get_varint(fb_handler * fbh, uint8_t len) {
    //printf("\n\rVARINT %d ", len);
    get_bytes(fbh, len);
    
    valvar = 0;
    for(int i = 0; i < len; i++) {
        valvar |= ((byte_buffer[i] &0xffL) << 8*(len-i-1)); 
    }

    return valvar;
}

// LEB128 Decode
uint32_t get_vbe_uint(fb_handler * fbh) {
    uint8_t shift = 0;

    valvar = 0;
    while(shift <= 25) {
        get_uint8(fbh);
        valvar |= ((uint32_t)(byte_buffer[0] & 0x7F)) << shift;
        if(!(byte_buffer[0] & 0x80))
            break;
        shift += 7;
    }
    
    return valvar;
}

// LEB128 Signed Decode
int32_t get_vbe_int(fb_handler * fbh) {
    uint8_t shift = 0;
    
    get_uint8(fbh);

    valvar = 0;
    while((byte_buffer[0] & 0x80) && (shift <= 25)) {
        valvar |= ((uint32_t)(byte_buffer[0] & 0x7f)) << shift;
        shift += 7;
        get_uint8(fbh);
    }

    // Add sign bit
    if ((byte_buffer[0] & 0x40)) {
        return -(valvar | ((uint32_t)(byte_buffer[0] & 0x3f) << shift));
    }

    return valvar | ((uint32_t)(byte_buffer[0] & 0x3f) << shift);
}

void get_string(fb_handler * fbh, char * ptr, uint8_t len) {
    // Check Highwater, reload buffer if needed
    if(fbh->buffer_pos + len > FILE_READ_BUFFER_SIZE) {
        relative_reset_buffer(fbh, FILE_READ_BUFFER_SIZE - fbh->buffer_pos);
    }

    memcpy(ptr, (fbh->buffer_ptr + fbh->buffer_pos), len);
    fbh->buffer_pos+=len;    
    ptr[len] = '\0';
}