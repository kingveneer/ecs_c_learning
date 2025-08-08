//
// Created by jo on 8/7/2025.
//

#include <stdint.h>
#include <stdlib.h>
/*
 *note for stupid(me)*
uint32 is exactly 32 bits on all platforms, with a max value of 4,294,967,295.
*/

typedef struct {
    uint32_t id; // for indexing into component arrays to find its specific components
    uint32_t generation; // version number to check component validity, prevents stale values
} Entity;


typedef struct {
    uint32_t capacity; // Max allowed enemies
    uint32_t living_count; // number of entities currently "alive"
    uint32_t *generation; // stores how many times an ID has been reused
    uint32_t *free_ids; // stack of available and recycled IDs for new entities
    uint32_t free_count; // how many IDs are in the free_ids stack
} EntityManager;

// init with given capacity, allocates memory for
// generation tracking and free id stack.
void entity_manager_init(EntityManager *em, uint32_t capacity) {
    em->capacity = capacity;
    em->living_count = 0;

    em->generation = calloc(capacity, sizeof(uint32_t));
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
    uint32_t id = em->free_ids[--em->free_count];

    em->living_count++;

    // return handle with ID and the current generation for that ID
    return (Entity){id, em->generation[id] };
}

//TODO add entity_destroy and entity validation