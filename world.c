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
    World *world = arena_alloc(persistent, sizeof(world));
    world->persistent_arena = persistent;
    world->battle_arena = battle;

    // initialize permanent arenas
    world->entity_manager = arena_alloc(persistent, sizeof(EntityManager));
    entity_manager_init(world->entity_manager, 16);

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

    sparse_set_init(world->stats_storage, world->entity_manager->capacity,
            sizeof(StatComponent), world->battle_arena);
    sparse_set_init(world->team_storage, world->entity_manager->capacity,
            sizeof(TeamComponent), world->battle_arena);
    sparse_set_init(world->combat_storage, world->entity_manager->capacity,
            sizeof(CombatComponent), world->battle_arena);
    sparse_set_init(world->name_storage, world->entity_manager->capacity,
            sizeof(NameComponent), world->battle_arena);

    entity_manager_free(world->entity_manager);

    world->team_a_count = 0;
    world->team_b_count = 0;
    world->turn_number = 0;
}