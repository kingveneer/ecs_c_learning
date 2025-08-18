//
// Created by jo on 8/7/2025.
//

#include <stdint.h>
#include <stdlib.h>
#include "entity_manager.h"
/**
 * @note uint32_t provides consistency across platforms with exactly 32 bits
 * and a maximum value of 4,294,967,295 entities.
 */

void entity_manager_init(EntityManager *em, const uint32_t capacity) {
    em->capacity = capacity;
    em->living_count = 0;

    // Use calloc for generation array to ensure initial values are 0
    em->generation = calloc(capacity, sizeof(uint32_t));

    // Use malloc for free_ids as we only access indices 0 to free_count-1
    em->free_ids = malloc(sizeof(uint32_t) * capacity);

    // Fill the free stack with all IDs in reverse order (highest to lowest)
    // This makes popping from the end easier (stack behavior)
    for (uint32_t i = 0; i < capacity; i++) {
        em->free_ids[i] = capacity - 1 - i;
    }

    // All IDs are initially free
    em->free_count = capacity;
}

Entity entity_create(EntityManager *em) {
    if (em->free_count == 0) {
        return (Entity){ UINT32_MAX, 0}; // Return invalid entity handle
    }

    // Pop the last free ID from the stack
    // --em->free_count both decrements and returns the new index
    const uint32_t id = em->free_ids[--em->free_count];

    em->living_count++;

    // Return handle with ID and current generation for that ID
    return (Entity){id, em->generation[id]};
}

void entity_destroy(EntityManager *em, Entity e) {
    const uint32_t id = e.id;

    // Validate handle against current generation (prevents double-destroy)
    if (em->generation[id] != e.generation) {
        return; // Stale or invalid handle
    }

    // Increment generation to invalidate existing handles with this ID
    em->generation[id]++;

    // Push ID back onto free stack for reuse
    // free_count points to the next available slot, then increment
    em->free_ids[em->free_count++] = id;
    em->living_count--;
}

void entity_destroy_batch(EntityManager *em, const Entity *entities, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        entity_destroy(em, entities[i]);
    }
}

int entity_is_alive(const EntityManager *em, Entity e) {
    return em->generation[e.id] == e.generation;
}

void entity_manager_free(EntityManager *em) {
    if (em) {
        free(em->generation);
        free(em->free_ids);
        em->generation = NULL;
        em->free_ids = NULL;
        em->capacity = 0;
        em->living_count = 0;
        em->free_count = 0;
    }
}
