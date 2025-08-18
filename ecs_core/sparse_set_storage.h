//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H
#define SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H

/**
 * @file sparse_set_storage.h
 * @brief High-performance sparse set implementation for ECS component storage
 *
 * Provides O(1) insertion, removal, and lookup operations with cache-friendly
 * dense iteration. Supports both component data storage and index-only sets.
 * Uses arena allocation for memory management.
 */

#include <stdint.h>
#include <stddef.h>

#include "arena.h"

/**
 * @brief Cache-friendly sparse set for component storage with O(1) operations
 *
 * The sparse set maintains two arrays: a sparse array for O(1) lookup and
 * a dense array for cache-friendly iteration. Components are stored in a
 * separate dense array aligned to the sparse entities.
 *
 * @note Uses UINT32_MAX as a sentinel value to indicate empty sparse slots
 */
typedef struct {
    uint32_t *sparse;         /**< Entity ID to dense index mapping (size: capacity) */
    uint32_t *dense_entities; /**< Dense array of entity IDs (size: capacity) */
    void *dense_data;         /**< Dense array of component data, NULL for index-only sets */
    uint32_t dense_count;     /**< Number of entities currently stored */
    uint32_t capacity;        /**< Maximum number of entities this set can hold */
    size_t comp_size;         /**< Size of each component in bytes, 0 for index-only sets */
    Arena *arena;             /**< Arena allocator used for memory management */
} SparseSet;

/**
 * @brief Initialize a sparse set with the given capacity and component size
 * @param set Pointer to the SparseSet to initialize
 * @param capacity Maximum number of entities this set can store
 * @param comp_size Size of each component in bytes (0 for index-only sets)
 * @param arena Arena allocator to use for memory allocation
 * @note Component data is aligned to 64 bytes for optimal cache performance
 */
void sparse_set_init(SparseSet *set, uint32_t capacity, size_t comp_size, Arena *arena);

/**
 * @brief Add or update a component for an entity
 * @param set Pointer to the SparseSet
 * @param entity Entity ID to add/update
 * @param component_data Pointer to component data to copy (ignored for index-only sets)
 * @note If entity already has a component, the data is updated in-place
 */
void sparse_set_add(SparseSet *set, uint32_t entity, const void *component_data);

/**
 * @brief Get a pointer to an entity's component data
 * @param set Pointer to the SparseSet
 * @param entity Entity ID to lookup
 * @return Pointer to component data, or NULL if entity doesn't exist or set is index-only
 * @note For index-only sets, this always returns NULL
 */
void* sparse_set_get(const SparseSet *set, uint32_t entity);

/**
 * @brief Remove a component from an entity using swap-and-pop
 * @param set Pointer to the SparseSet
 * @param entity Entity ID to remove
 * @note Uses swap-and-pop technique to maintain dense array compactness in O(1) time
 */
void sparse_set_remove(SparseSet *set, uint32_t entity);

/**
 * @brief Free sparse set memory (currently unimplemented - uses arena allocation)
 * @param set Pointer to the SparseSet to free
 * @note Memory is managed by the arena allocator, so this is typically unnecessary
 */
void sparse_set_free(SparseSet *set);

#endif //SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H