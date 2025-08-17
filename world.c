//
// Created by jo on 8/12/2025.
//

#include "world.h"
#include <stdio.h>

World* world_create(size_t max_entities) {
    // Create arenas - Increase size for better performance
    Arena *persistent = arena_create(32 * 1024 * 1024); // 32MB
    Arena *battle = arena_create(32 * 1024 * 1024); // 32MB

    // Allocate the world struct itself from the persistent arena
    World *world = arena_alloc(persistent, sizeof(World));
    world->persistent_arena = persistent;
    world->battle_arena = battle;
    world->weakest_team_a_health = INT_MAX;
    world->weakest_team_b_health = INT_MAX;

    // Initialize core ECS managers, allocating them in the persistent arena
    world->entity_manager = arena_alloc(persistent, sizeof(EntityManager));
    entity_manager_init(world->entity_manager, max_entities);

    world->storage_manager = arena_alloc(persistent, sizeof(StorageManager));
    storage_manager_init(world->storage_manager, 16);

    // Larger initial death queue
    world->death_queue = arena_alloc(persistent, sizeof(DeathQueue));
    death_queue_init(world->death_queue, max_entities / 10); // Expect ~10% deaths per turn

    world->combatant_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->combatant_storage, max_entities, sizeof(CombatantBundle), battle);

    world->team_a_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->team_a_storage, max_entities, 0, battle);

    world->team_b_storage = arena_alloc(persistent, sizeof(SparseSet));
    sparse_set_init(world->team_b_storage, max_entities, 0, battle);

    // Register all temporary storages with the storage manager
    storage_manager_register(world->storage_manager, world->combatant_storage);
    storage_manager_register(world->storage_manager, world->team_a_storage);
    storage_manager_register(world->storage_manager, world->team_b_storage);

    world->battle_active = false;
    world->turn_number = 0;

    // Initialize cache
    world->weakest_team_a = (Entity){UINT32_MAX, 0};
    world->weakest_team_b = (Entity){UINT32_MAX, 0};
    world->needs_target_update = true;

    // Initialize multi-target caches
    world->weakest_cache_a.count = 0;
    world->weakest_cache_b.count = 0;
    for (int i = 0; i < WEAKEST_CACHE_SIZE; i++) {
        world->weakest_cache_a.targets[i] = (Entity){UINT32_MAX, 0};
        world->weakest_cache_b.targets[i] = (Entity){UINT32_MAX, 0};
        world->weakest_cache_a.healths[i] = INT_MAX;
        world->weakest_cache_b.healths[i] = INT_MAX;
    }

    return world;
}

void world_reset_battle(World *world) {
    arena_reset(world->battle_arena);
    world->weakest_team_a_health = INT_MAX;
    world->weakest_team_b_health = INT_MAX;

    // Save the capacity before freeing
    uint32_t entity_capacity = world->entity_manager->capacity;

    // Re-initialize sparse sets with the battle arena
    sparse_set_init(world->combatant_storage, entity_capacity,
            sizeof(CombatantBundle), world->battle_arena);
    sparse_set_init(world->team_a_storage, entity_capacity,
            0, world->battle_arena);
    sparse_set_init(world->team_b_storage, entity_capacity,
            0, world->battle_arena);

    // Reset entity manager
    entity_manager_free(world->entity_manager);
    entity_manager_init(world->entity_manager, entity_capacity);

    // Clear death queue
    death_queue_clear(world->death_queue);

    world->team_a_count = 0;
    world->team_b_count = 0;
    world->turn_number = 0;

    // Reset cache
    world->weakest_team_a = (Entity){UINT32_MAX, 0};
    world->weakest_team_b = (Entity){UINT32_MAX, 0};
    world->needs_target_update = true;

    // Reset multi-target caches
    world->weakest_cache_a.count = 0;
    world->weakest_cache_b.count = 0;
    for (int i = 0; i < WEAKEST_CACHE_SIZE; i++) {
        world->weakest_cache_a.targets[i] = (Entity){UINT32_MAX, 0};
        world->weakest_cache_b.targets[i] = (Entity){UINT32_MAX, 0};
        world->weakest_cache_a.healths[i] = INT_MAX;
        world->weakest_cache_b.healths[i] = INT_MAX;
    }
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