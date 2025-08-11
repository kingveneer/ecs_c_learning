//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H
#define SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H
#include "entity_manager.h"
#include "storage_manager.h"

typedef struct {
    Entity *entities;  // dynamic array of entity handles
    size_t count;
    size_t capacity;
} DeathQueue;

void death_queue_init(DeathQueue *dq, size_t initial_capacity);
void death_queue_free(DeathQueue *dq);
void death_queue_push(DeathQueue *dq, Entity e);
void death_queue_clear(DeathQueue *dq);
void process_deaths(DeathQueue *dq, StorageManager *sm, EntityManager *em);
#endif //SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H