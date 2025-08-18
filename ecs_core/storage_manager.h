//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H
#define SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H
/**
 * @file storage_manager.h
 * @brief Central management system for component storage across multiple sparse sets
 *
 * The StorageManager coordinates operations across all registered component storage
 * systems, enabling efficient batch operations and unified entity management.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h> // strcmp used in example tallying
#include <stdio.h>  // printf for debug/tally - optional

#include "entity_manager.h"
#include "sparse_set_storage.h"

/**
 * @brief Manages a collection of sparse sets for coordinated component operations
 *
 * The StorageManager maintains references to all component storage systems,
 * allowing for efficient batch operations like removing an entity from all
 * component types simultaneously.
 */
typedef struct {
    SparseSet **sets;   /**< Dynamic array of pointers to registered SparseSet instances */
    size_t count;       /**< Number of currently registered sets */
    size_t capacity;    /**< Allocated capacity for the sets array */
} StorageManager;

/**
 * @brief Initialize a storage manager with the specified initial capacity
 * @param sm Pointer to the StorageManager to initialize
 * @param initial_capacity Initial capacity for the sets array (minimum 8 if 0)
 */
void storage_manager_init(StorageManager *sm, size_t initial_capacity);

/**
 * @brief Free the storage manager's internal arrays
 * @param sm Pointer to the StorageManager to free
 * @note This only frees the pointer array, not the sparse sets themselves
 */
void storage_manager_free(StorageManager *sm);

/**
 * @brief Register a sparse set with the storage manager for coordinated operations
 * @param sm Pointer to the StorageManager
 * @param set Pointer to the SparseSet to register
 * @note The storage manager does not take ownership of the sparse set
 */
void storage_manager_register(StorageManager *sm, SparseSet *set);

/**
 * @brief Remove an entity from all registered sparse sets
 * @param sm Pointer to the StorageManager
 * @param entity_id The entity ID to remove from all component storage
 * @note Safely handles entity IDs that exceed individual set capacities
 */
void storage_manager_remove_entity(StorageManager *sm, uint32_t entity_id);

/**
 * @brief Batch remove multiple entities from all registered sparse sets
 * @param sm Pointer to the StorageManager
 * @param entity_ids Array of entity IDs to remove
 * @param n Number of entities in the array
 * @note More efficient than calling storage_manager_remove_entity() in a loop
 */
void storage_manager_remove_entities(StorageManager *sm, const uint32_t *entity_ids, size_t n);

#endif //SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H

