//
// Created by jo on 8/7/2025.
//

#include <stdint.h>
#include <stdlib.h>
#include "entity_manager.h"
/*
 *note for stupid(me)*
uint32 adds consistency, as it's exactly 32 bits on all platforms, with a max value of 4,294,967,295.
*/



// init with given capacity, allocates memory for
// generation tracking and free id stack.
void entity_manager_init(EntityManager *em, const uint32_t capacity) {
    em->capacity = capacity;
    em->living_count = 0;
    // using calloc because we want initial values to be 0
    em->generation = calloc(capacity, sizeof(uint32_t));
    // malloc is faster, we only access index 0 to free_count
    em->free_ids = malloc(sizeof(uint32_t) * capacity);

    // fill the stack with all ids, starting with highest down to 0,
    // this makes popping from the end easier(stack)
    for (uint32_t i = 0; i < capacity; i++) {
        em->free_ids[i] = capacity -1 - i;
    }
    // all IDs are free on init
    em->free_count = capacity;
}

Entity entity_create(EntityManager *em) {
    if (em->free_count == 0) {
        return (Entity){ UINT32_MAX, 0}; // invalid entity
    }
    // Pop the last free ID from the stack.
    // --em->free_count both decrements and returns the new index.
    const uint32_t id = em->free_ids[--em->free_count];

    em->living_count++;

    // return handle with ID and the current generation for that ID
    return (Entity){id, em->generation[id] };
}

// Destroy entity associated with given ID
void entity_destroy(EntityManager *em, Entity e) {
    const uint32_t id = e.id;

    // stale reference check
    if (em->generation[id] != e.generation) {
        return;
    }
    // increase generation number, invalidates older gens
    em->generation[id]++;
    // push ID back into free stack for reuse using free count, decrement living.
    // ex: free_count = 3 means IDs are in slot 0, 1, 2 so the next empty slot is 3, then increment++
    em->free_ids[em->free_count++] = id;
    em->living_count--;
}

void entity_destroy_batch(EntityManager *em, const Entity *entities, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        entity_destroy(em, entities[i]);
    }
}

int entity_is_alive(const EntityManager *em,  Entity e) {
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
