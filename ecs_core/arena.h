//
// Created by jo on 8/10/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_ARENA_H
#define SPARSE_STORAGE_LEARNING_ARENA_H
/**
 * @file arena.h
 * @brief Fast linear memory allocator with automatic alignment and checkpoints
 *
 * Provides extremely fast allocation by simply bumping a pointer through
 * a pre-allocated buffer. Supports alignment requirements, checkpoints for
 * temporary allocations, and potential chaining for large allocation needs.
 */

#include <stdint.h>

/**
 * @brief Linear memory allocator with checkpoint/restore functionality
 *
 * Arena allocators provide very fast allocation by maintaining a simple
 * offset into a pre-allocated buffer. Memory cannot be individually freed,
 * but the entire arena can be reset or restored to a previous checkpoint.
 *
 * @note Memory is automatically zeroed on creation for deterministic behavior
 */
typedef struct Arena {
    uint8_t *buffer;      /**< Pointer to the allocated memory buffer */
    size_t size;          /**< Total size of the buffer in bytes */
    size_t offset;        /**< Current allocation offset within the buffer */
    struct Arena *next;   /**< Pointer to next arena in chain (for future chaining support) */
} Arena;

/**
 * @brief Create a new arena with the specified size
 * @param size Size of the memory buffer to allocate in bytes
 * @return Pointer to the created Arena, or NULL on allocation failure
 * @note The allocated buffer is automatically zeroed for deterministic behavior
 */
Arena* arena_create(size_t size);

/**
 * @brief Allocate memory from the arena with default 8-byte alignment
 * @param arena Pointer to the Arena to allocate from
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL if insufficient space
 * @note Uses 8-byte alignment which is suitable for most data types
 */
void* arena_alloc(Arena * arena, size_t size);

/**
 * @brief Allocate aligned memory from the arena with custom alignment
 * @param arena Pointer to the Arena to allocate from
 * @param size Number of bytes to allocate
 * @param alignment Required alignment in bytes (must be power of 2)
 * @return Pointer to allocated memory aligned to the specified boundary, or NULL if insufficient space
 * @note Commonly used alignments: 8 (general), 16 (SIMD), 64 (cache line)
 */
void* arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);

/**
 * @brief Reset the arena to empty state, making all memory available for reuse
 * @param arena Pointer to the Arena to reset
 * @note This invalidates all previously allocated pointers from this arena
 * @note Memory is not cleared - use with caution if deterministic state is required
 */
void arena_reset(Arena *arena);

/**
 * @brief Destroy the arena and free all associated memory
 * @param arena Pointer to the Arena to destroy
 * @note This frees both the arena structure and its buffer
 */
void arena_destroy(Arena *arena);

/**
 * @brief Create a checkpoint representing the current allocation state
 * @param arena Pointer to the Arena to checkpoint
 * @return Checkpoint value that can be used with arena_restore()
 * @note Useful for temporary allocations that can be bulk-freed later
 */
size_t arena_checkpoint(const Arena *arena);

/**
 * @brief Restore the arena to a previous checkpoint state
 * @param arena Pointer to the Arena to restore
 * @param checkpoint Checkpoint value from arena_checkpoint()
 * @note This effectively frees all allocations made after the checkpoint
 * @note Checkpoint must be valid (not greater than current offset)
 */
void arena_restore(Arena *arena, size_t checkpoint);
#endif //SPARSE_STORAGE_LEARNING_ARENA_H