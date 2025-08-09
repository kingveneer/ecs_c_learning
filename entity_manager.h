//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H
#define SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <stdint.h>

typedef struct {
    uint32_t id;
    uint32_t generation;
} Handle;

typedef struct {
    uint32_t id; // for indexing into component arrays to find its specific components
    uint32_t generation; // version number to check component validity, prevents stale values
} Entity;

typedef struct {
    uint32_t *generation; // stores how many times an ID has been reused
    uint32_t *free_ids; // stack of available and recycled IDs for new entities

    uint32_t capacity; // Max allowed enemies
    uint32_t living_count; // number of entities currently "alive"
    uint32_t free_count; // how many IDs are in the free_ids stack
} EntityManager;

// Initialize the entity manager with a maximum capacity
void entity_manager_init(EntityManager *em, uint32_t capacity);

// Create a new entity, returning its handle
Entity entity_create(EntityManager *em);

// Destroy an entity, invalidating its handle
void entity_destroy(EntityManager *em, Entity e);

// Destroy an array of entities at once
void entity_destroy_batch(EntityManager *em, const Entity *entities, uint32_t count);

// Check if an entity handle is still valid
int entity_is_alive(const EntityManager *em, Entity e);

// Free allocated memory (optional cleanup)
void entity_manager_free(EntityManager *em);

#endif // ENTITY_MANAGER_H

#endif //SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H