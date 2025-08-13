//
// Created by jo on 8/12/2025.
//

#include "world.h"
#include <stdio.h>

World* world_create(size_t max_entities) {
    // create arenas
    Arena *persistent = arena_create(8 * 1024 * 1024); // 8MB
    Arena *battle = arena_create(4 * 1024 * 1024); // 4MB

    // allocate world
    World *world = arena_alloc(persistent, sizeof(World));
    world->persistent_arena = persistent;
    world->battle_arena = battle;

    // initialize permanent arenas
    world->entity_manager = arena_alloc(persistent, sizeof(EntityManager));
    entity_manager_init(world->entity_manager, max_entities);

    world->storage_manager = arena_alloc(persistent, sizeof(StorageManager));
    storage_manager_init(world->storage_manager, 16);

    world->death_queue = arena_alloc(persistent, sizeof(DeathQueue));
    death_queue_init(world->death_queue, 16);

    // initialize temporary battle storage
    world->stats_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->stats_storage, max_entities, sizeof(StatComponent), battle);

    world->team_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->team_storage, max_entities, sizeof(TeamComponent), battle);

    world->combat_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->combat_storage, max_entities, sizeof(CombatComponent), battle);

    world->name_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->name_storage, max_entities, sizeof(NameComponent), battle);

    // register temp storages
    storage_manager_register(world->storage_manager, world->stats_storage);
    storage_manager_register(world->storage_manager, world->team_storage);
    storage_manager_register(world->storage_manager, world->combat_storage);
    storage_manager_register(world->storage_manager, world->name_storage);

    world->battle_active = false;
    world->turn_number = 0;

    return world;
}

void world_reset_battle(World *world) {
    arena_reset(world->battle_arena);

    // Save the capacity before freeing
    uint32_t entity_capacity = world->entity_manager->capacity;

    // Re-initialize sparse sets with the battle arena
    sparse_set_init(world->stats_storage, entity_capacity,
            sizeof(StatComponent), world->battle_arena);
    sparse_set_init(world->team_storage, entity_capacity,
            sizeof(TeamComponent), world->battle_arena);
    sparse_set_init(world->combat_storage, entity_capacity,
            sizeof(CombatComponent), world->battle_arena);
    sparse_set_init(world->name_storage, entity_capacity,
            sizeof(NameComponent), world->battle_arena);

    // Reset entity manager
    entity_manager_free(world->entity_manager);
    entity_manager_init(world->entity_manager, entity_capacity);

    // Clear death queue
    death_queue_clear(world->death_queue);

    world->team_a_count = 0;
    world->team_b_count = 0;
    world->turn_number = 0;
}

void world_destroy(World *world) {
    // Note: We don't free individual components since they're allocated from arenas
    // We just destroy the arenas which will free everything at once

    if (world) {
        // Storage manager cleanup (just frees the pointer array)
        storage_manager_free(world->storage_manager);

        // Death queue cleanup
        death_queue_free(world->death_queue);

        // Entity manager cleanup
        entity_manager_free(world->entity_manager);

        // Destroy arenas (this frees all memory allocated from them)
        arena_destroy(world->battle_arena);
        arena_destroy(world->persistent_arena);  // This also frees the World struct itself
    }
}