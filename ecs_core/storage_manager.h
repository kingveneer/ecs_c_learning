//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H
#define SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h> // strcmp used in example tallying
#include <stdio.h>  // printf for debug/tally - optional

#include "entity_manager.h"
#include "sparse_set_storage.h"

typedef struct {
    SparseSet **sets;   // dynamic array of pointers to SparseSet
    size_t count;       // how many sets registered
    size_t capacity;    // allocated capacity for sets[]
} StorageManager;

void storage_manager_init(StorageManager *sm, size_t initial_capacity);
void storage_manager_free(StorageManager *sm); // frees the pointer array (not the sets themselves)
void storage_manager_register(StorageManager *sm, SparseSet *set);
void storage_manager_remove_entity(StorageManager *sm, uint32_t entity_id);
void storage_manager_remove_entities(StorageManager *sm, const uint32_t *entity_ids, size_t n);

#endif //SPARSE_STORAGE_LEARNING_STORAGE_MANAGER_H