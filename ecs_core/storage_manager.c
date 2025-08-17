//
// Created by jo on 8/8/2025.
//

#include "storage_manager.h"

void storage_manager_init(StorageManager *sm, size_t initial_capacity) {
    sm->count = 0;
    sm->capacity = (initial_capacity > 0) ? initial_capacity : 8;
    sm->sets = malloc(sizeof(SparseSet*) * sm->capacity);
}

void storage_manager_free(StorageManager *sm) {
    free(sm->sets);
    sm->sets = NULL;
    sm->count = sm->capacity = 0;
}

static void storage_manager_grow_if_needed(StorageManager *sm) {
    if (sm->count >= sm->capacity) {
        size_t new_cap = sm->capacity * 2;
        sm->sets = realloc(sm->sets, sizeof(SparseSet*) * new_cap);
        sm->capacity = new_cap;
    }
}

void storage_manager_register(StorageManager *sm, SparseSet *set) {
    storage_manager_grow_if_needed(sm);
    sm->sets[sm->count++] = set;
}

// Remove a single entity from every registered sparse set.
void storage_manager_remove_entity(StorageManager *sm, uint32_t entity_id) {
    for (size_t i = 0; i < sm->count; ++i) {
        SparseSet *s = sm->sets[i];
        // guard against invalid ID vs set capacity — avoids OOB in sparse array
        if (entity_id < s->capacity) {
            sparse_set_remove(s, entity_id);
        }
    }
}

// True batch removal - iterate each set once, remove all entities
void storage_manager_remove_entities(StorageManager *sm, const uint32_t *entity_ids, size_t n) {
    for (size_t i = 0; i < sm->count; ++i) {
        SparseSet *s = sm->sets[i];
        for (size_t j = 0; j < n; ++j) {
            uint32_t entity_id = entity_ids[j];
            if (entity_id < s->capacity) {
                sparse_set_remove(s, entity_id);
            }
        }
    }
}