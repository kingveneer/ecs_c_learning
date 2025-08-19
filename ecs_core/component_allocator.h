//
// Created by jo on 8/18/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMPONENT_ALLOCATOR_H
#define SPARSE_STORAGE_LEARNING_COMPONENT_ALLOCATOR_H

#include "arena.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_COMPONENT_POOLS 6  // Reasonable number for most games

/**
 * @brief Component allocator using size-segregated sub-arenas
 *
 * Creates multiple logical pools within a single master arena for optimal
 * memory layout and cache performance. Each component size class gets its
 * own sub-arena carved from the master arena's memory.
 */
typedef struct {
    Arena *master_arena;              /**< Single backing memory store */
    Arena sub_pools[MAX_COMPONENT_POOLS]; /**< Size-segregated sub-arenas */
    size_t size_thresholds[MAX_COMPONENT_POOLS]; /**< Max size for each pool */
    size_t pool_sizes[MAX_COMPONENT_POOLS];      /**< Allocated size per pool */
    size_t pool_count;                /**< Number of active pools */
} ComponentAllocator;

/**
 * @brief Initialize component allocator with total memory budget
 * @param allocator Pointer to ComponentAllocator to initialize
 * @param total_memory Total memory to allocate for all components
 * @return 1 on success, 0 on failure
 */
int component_allocator_init(ComponentAllocator *allocator, size_t total_memory);

/**
 * @brief Get the appropriate arena for a component size
 * @param allocator Pointer to ComponentAllocator
 * @param component_size Size of component in bytes
 * @return Arena pointer for allocation, or NULL on failure
 */
Arena* component_allocator_get_arena(ComponentAllocator *allocator, size_t component_size);

/**
 * @brief Reset all component pools for new level/scene
 * @param allocator Pointer to ComponentAllocator
 * @note Keeps the master arena intact but resets all sub-pool offsets
 */
void component_allocator_reset(ComponentAllocator *allocator);

/**
 * @brief Free all allocated memory
 * @param allocator Pointer to ComponentAllocator
 */
void component_allocator_free(ComponentAllocator *allocator);

/**
 * @brief Memory usage statistics for debugging and profiling
 */
typedef struct {
    size_t total_allocated;     /**< Total memory allocated from system */
    size_t total_used;         /**< Total memory used by components */
    size_t pool_used[MAX_COMPONENT_POOLS]; /**< Memory used per pool */
    size_t pool_allocated[MAX_COMPONENT_POOLS]; /**< Memory allocated per pool */
    float utilization_percent; /**< Overall memory utilization */
} ComponentMemoryStats;

/**
 * @brief Get detailed memory statistics
 * @param allocator Pointer to ComponentAllocator
 * @param stats Pointer to stats structure to fill
 */
void component_allocator_get_stats(const ComponentAllocator *allocator, ComponentMemoryStats *stats);


#endif //SPARSE_STORAGE_LEARNING_COMPONENT_ALLOCATOR_H