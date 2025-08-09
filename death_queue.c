//
// Created by jo on 8/8/2025.
//

#include "death_queue.h"
#include <stdlib.h>

#include "storage_manager.h"

void death_queue_init(DeathQueue *dq, size_t initial_capacity) {
    dq->count = 0;
    dq->capacity = (initial_capacity > 0) ? initial_capacity : 64;
    dq->entities = malloc(sizeof(Entity) * dq->capacity);
}

void death_queue_free(DeathQueue *dq) {
    free(dq->entities);
    dq->entities = NULL;
    dq->count = dq->capacity = 0;
}

void death_queue_push(DeathQueue *dq, Entity e) {
    if (dq->count >= dq->capacity) {
        dq->capacity *= 2;
        dq->entities = realloc(dq->entities, sizeof(Entity) * dq->capacity);
    }
    dq->entities[dq->count++] = e;
}

void death_queue_clear(DeathQueue *dq) {
    dq->count = 0;
}

void process_deaths(DeathQueue *dq, StorageManager *sm, EntityManager *em) {
    for (uint32_t i = 0; i < dq->count; i++) {
        Entity e = dq->entities[i];
        storage_manager_remove_all(sm, e.id);
        entity_destroy(em, e);
    }
    dq->count = 0; // Clear queue
}