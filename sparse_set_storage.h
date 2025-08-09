//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H
#define SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t *sparse;         // entity id -> dense index
    uint32_t *dense_entities; // dense index -> entity id
    void *dense_data;         // dense array of components
    uint32_t dense_count;
    uint32_t capacity;
    size_t comp_size;
} SparseSet;

// Initialize sparse set for a given capacity and component size
void sparse_set_init(SparseSet *set, uint32_t capacity, size_t comp_size);

// Add or update a component for an entity
void sparse_set_add(SparseSet *set, uint32_t entity, const void *component_data);

// Get a pointer to an entity's component, or NULL if it doesn't exist
void* sparse_set_get(const SparseSet *set, uint32_t entity);

// Remove a component from an entity
void sparse_set_remove(SparseSet *set, uint32_t entity);

// Free sparse set memory
void sparse_set_free(SparseSet *set);

#endif //SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H