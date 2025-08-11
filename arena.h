//
// Created by jo on 8/10/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_ARENA_H
#define SPARSE_STORAGE_LEARNING_ARENA_H
#include <stdint.h>

typedef struct Arena {
    uint8_t *buffer; // pointer to chunk of memory
    size_t size;  // size of the buffer
    size_t offset; // current position in buffer
    struct Arena *next; //for chaining arenas
} Arena;

Arena* arena_create(size_t size);
void* arena_alloc(Arena * arena, size_t size);
void* arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);
void arena_reset(Arena *arena);
void arena_destroy(Arena *arena);
// for check points
size_t arena_checkpoint(Arena *arena);
void arena_restore(Arena,size_t checkpoint);

#endif //SPARSE_STORAGE_LEARNING_ARENA_H