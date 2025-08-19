//
// Created by jo on 8/18/2025.
//

#include "component_allocator.h"
#include <string.h>

#include "sparse_set_storage.h"

// Predefined size classes optimized for common component patterns
static const size_t DEFAULT_SIZE_THRESHOLDS[MAX_COMPONENT_POOLS] = {
    16,   // Tiny: Position, Velocity, Health, Tags
    32,   // Small: UUID, Timer, Flags, Small state
    64,   // Medium: Transform matrix, Sprite data
    128,  // Large: Animation state, Physics body
    256,  // Very Large: AI state, Complex behaviors
    SIZE_MAX // Overflow: Anything larger
};

// Memory allocation percentages (should sum to ~100)
static const float POOL_ALLOCATION_RATIOS[MAX_COMPONENT_POOLS] = {
    0.35f, // 35% for tiny components (most common)
    0.25f, // 25% for small components
    0.20f, // 20% for medium components
    0.12f, // 12% for large components
    0.06f, // 6% for very large components
    0.02f  // 2% for overflow
};

int component_allocator_init(ComponentAllocator *allocator, size_t total_memory) {
    // Create master arena for all component data
    allocator->master_arena = arena_create(total_memory);
    if (!allocator->master_arena) {
        return 0;
    }

    allocator->pool_count = MAX_COMPONENT_POOLS;

    // Copy size thresholds
    memcpy(allocator->size_thresholds, DEFAULT_SIZE_THRESHOLDS,
           sizeof(size_t) * MAX_COMPONENT_POOLS);

    // Calculate pool sizes based on allocation ratios
    for (size_t i = 0; i < MAX_COMPONENT_POOLS; i++) {
        allocator->pool_sizes[i] = (size_t)(total_memory * POOL_ALLOCATION_RATIOS[i]);
    }

    // Carve out sub-arenas from master arena memory
    for (size_t i = 0; i < MAX_COMPONENT_POOLS; i++) {
        size_t pool_size = allocator->pool_sizes[i];

        // Allocate aligned memory for this pool from master arena
        uint8_t *pool_memory = arena_alloc_aligned(allocator->master_arena, pool_size, 64);
        if (!pool_memory) {
            // Handle partial failure - reduce pool size or skip
            allocator->pool_sizes[i] = 0;
            continue;
        }

        // Initialize sub-arena to use this carved memory
        Arena *sub_pool = &allocator->sub_pools[i];
        sub_pool->buffer = pool_memory;
        sub_pool->size = pool_size;
        sub_pool->offset = 0;
        sub_pool->next = NULL;

        // Zero memory for deterministic behavior
        memset(sub_pool->buffer, 0, pool_size);
    }

    return 1;
}

Arena* component_allocator_get_arena(ComponentAllocator *allocator, size_t component_size) {
    // Find the smallest pool that can fit this component size
    for (size_t i = 0; i < allocator->pool_count; i++) {
        if (component_size <= allocator->size_thresholds[i] &&
            allocator->pool_sizes[i] > 0) {
            return &allocator->sub_pools[i];
        }
    }

    // Fallback to overflow pool (last pool)
    return &allocator->sub_pools[MAX_COMPONENT_POOLS - 1];
}

void component_allocator_reset(ComponentAllocator *allocator) {
    // Reset all sub-pool offsets to make their memory available again
    for (size_t i = 0; i < MAX_COMPONENT_POOLS; i++) {
        if (allocator->pool_sizes[i] > 0) {
            arena_reset(&allocator->sub_pools[i]);
        }
    }

    // Note: Don't reset master arena since sub-pools are using its memory
}

void component_allocator_free(ComponentAllocator *allocator) {
    if (allocator->master_arena) {
        arena_destroy(allocator->master_arena);
        allocator->master_arena = NULL;
    }

    // Clear sub-pool metadata (memory was freed with master arena)
    memset(allocator->sub_pools, 0, sizeof(allocator->sub_pools));
    memset(allocator->pool_sizes, 0, sizeof(allocator->pool_sizes));
    allocator->pool_count = 0;
}

void component_allocator_get_stats(const ComponentAllocator *allocator, ComponentMemoryStats *stats) {
    memset(stats, 0, sizeof(ComponentMemoryStats));

    if (!allocator->master_arena) return;

    stats->total_allocated = allocator->master_arena->size;

    // Sum up usage from all pools
    for (size_t i = 0; i < MAX_COMPONENT_POOLS; i++) {
        if (allocator->pool_sizes[i] > 0) {
            stats->pool_allocated[i] = allocator->pool_sizes[i];
            stats->pool_used[i] = allocator->sub_pools[i].offset;
            stats->total_used += stats->pool_used[i];
        }
    }

    stats->utilization_percent = (stats->total_allocated > 0)
        ? (float)stats->total_used / stats->total_allocated * 100.0f
        : 0.0f;
}

// Convenience function to initialize sparse sets with component allocator
void sparse_set_init_with_allocator(SparseSet *set, uint32_t capacity, size_t comp_size,
                                    ComponentAllocator *allocator) {
    Arena *arena = component_allocator_get_arena(allocator, comp_size);
    sparse_set_init(set, capacity, comp_size, arena);
}