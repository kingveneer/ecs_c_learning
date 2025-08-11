//
// Created by jo on 8/10/2025.
//

#include "arena.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

Arena* arena_create(size_t size) {
    Arena *arena = malloc(sizeof(Arena));
    if (!arena) return NULL;

    arena->buffer = malloc(size);
    if (!arena->buffer) {
        free(arena);
        return NULL;
    }
    arena->size = size;
    arena->offset = 0;
    arena->next = NULL;

    // zero the memory
    memset(arena->buffer, 0, size);

    return arena;
    }



void* arena_alloc(Arena *arena, size_t size) {
    // Default to 8-byte alignment for most platforms
    return arena_alloc_aligned(arena, size, 8);
}

void* arena_alloc_aligned(Arena *arena, size_t size, size_t alignment) {
    // Calculate aligned offset
    size_t aligned_offset = (arena->offset + alignment - 1) & ~(alignment - 1);

    // Check if we have enough space
    if (aligned_offset + size > arena->size) {
        // Could implement arena chaining here
        // For now, NULL
        return NULL;
    }
    void *ptr = arena->buffer + aligned_offset;
    arena->offset = aligned_offset + size;

    return ptr;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
    // Optionally clear memory
    // memset(arena->buffer, 0, arena->size);
}

void arena_destroy(Arena *arena) {
    if (arena) {
        free(arena->buffer);
        free(arena);
    }
}

size_t arena_checkpoint(const Arena *arena) {
    return arena->offset;
}

void arena_restore(Arena *arena, size_t checkpoint) {
    assert(checkpoint <= arena->offset);
    arena->offset = checkpoint;
}