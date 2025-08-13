//
// Created by jo on 8/8/2025.
//
#include <stdint.h>
#include <stdlib.h>
#include "sparse_set_storage.h"

#include <string.h>



void sparse_set_init(SparseSet *set, const uint32_t capacity, const size_t comp_size, Arena *arena) {
    set->capacity = capacity;
    set->comp_size = comp_size;
    set->dense_count = 0;
    set->arena = arena;

    // Allocate from arena
    set->sparse = arena_alloc(arena, sizeof(uint32_t) * capacity);

    // Initialize sparse array
    for (uint32_t i = 0; i < capacity; i++) {
        set->sparse[i] = UINT32_MAX;
    }

    set->dense_entities = arena_alloc(arena, sizeof(uint32_t) * capacity);

    // Align component data for better cache performance
    set->dense_data = arena_alloc_aligned(arena, comp_size * capacity, 64);
}

void sparse_set_add(SparseSet *set, const uint32_t entity, const void *component_data) {
    const uint32_t index = set->sparse[entity];
    // if has component, overwrite
    if (index != UINT32_MAX) {
        // char* 1 bit offset by index * comp_size used to pick the start of the memory we want to change
        void *dest = (char*)set->dense_data + (index * set->comp_size);
        //copy data to destination, from component data, with comp_size bits
        memcpy(dest, component_data, set->comp_size);
        return;
    }
    // create new component
    const uint32_t dense_index = set->dense_count++;
    set->dense_entities[dense_index] = entity;
    set->sparse[entity] = dense_index;

    void *dest = (char*)set->dense_data + (dense_index * set->comp_size);
    memcpy(dest, component_data, set->comp_size);
}

void sparse_set_remove(SparseSet *set, const uint32_t entity) {
    const uint32_t index = set->sparse[entity];
    if (index == UINT32_MAX) {
        return; // if no component, return stupid
    }
    // get the last entity index, arrays are 0 indexed so -1 from dense_count
    const uint32_t last_index = set->dense_count-1;
    const uint32_t last_entity = set->dense_entities[last_index];

    //copy last entity into removed slot from provided entity
    set->dense_entities[index] = last_entity;
    // set component data of last entity into the removed slot
    void *dest = (char*)set->dense_data + (index * set->comp_size);
    const void *src = (char*)set->dense_data + (last_index * set->comp_size);
    memcpy(dest, src, set->comp_size); // from, to, size

    set->sparse[last_entity] = index;
    set->sparse[entity] = UINT32_MAX;
    set->dense_count--;
}

void* sparse_set_get(const SparseSet *set, const uint32_t entity) {
    const uint32_t index = set->sparse[entity];
    if (index == UINT32_MAX) return NULL;
    return (char*)set->dense_data + (index * set->comp_size);
}