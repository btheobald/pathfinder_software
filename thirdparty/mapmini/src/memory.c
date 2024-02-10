#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void arena_init(arena_t * arena, size_t size) {
    arena->region = malloc(size);
    arena->size = sizeof(uint8_t)*size;
    arena->current = 0;
}

void* arena_malloc(arena_t * arena, size_t size) {
    if(arena->current+size > arena->size) {
        printf ("ARENA: NULL RETURN\r\n");
        return NULL;
    } 
    
    size_t tmp = (size_t) arena->region + arena->current;
    arena->current += size;

    //printf("ARENA: REQUEST %d BYTES -> %d\n\r", size, arena->current);\

    return (void *) (tmp);
}

size_t arena_free(arena_t * arena) { // Invalidates existing pointers
    size_t oldsize = arena->current;
    arena->current = 0;
    free(arena->region);
    return oldsize;
}