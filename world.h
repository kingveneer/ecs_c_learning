//
// Created by jo on 8/12/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_WORLD_H
#define SPARSE_STORAGE_LEARNING_WORLD_H

#include "ecs_core/arena.h"
#include "ecs_core/entity_manager.h"
#include "ecs_core/storage_manager.h"
#include "ecs_core/death_queue.h"
#include "ecs_core/sparse_set_storage.h"
#include "components.h"
#include "entity_factory.h"

#define WEAKEST_CACHE_SIZE 8

//  Cache multiple weak targets per team
typedef struct {
    Entity targets[WEAKEST_CACHE_SIZE];
    int healths[WEAKEST_CACHE_SIZE];
    uint32_t count;
} WeakestCache;

typedef struct World {
    // ECS Core
    EntityManager *entity_manager;
    StorageManager *storage_manager;
    DeathQueue *death_queue;

    // Memory
    Arena *persistent_arena;
    Arena *battle_arena; // reset per battle

    // Component Storages
    SparseSet *combatant_storage;
    // index-only sets for fast team-based lookups
    SparseSet *team_a_storage;
    SparseSet *team_b_storage;

    // Battle State
    uint32_t team_a_count;
    uint32_t team_b_count;
    bool battle_active;
    uint32_t turn_number;

    // Targeting Cache (Enhanced)
    Entity weakest_team_a;
    Entity weakest_team_b;
    int weakest_team_a_health;
    int weakest_team_b_health;
    bool needs_target_update;

    //  Multiple target caching
    WeakestCache weakest_cache_a;
    WeakestCache weakest_cache_b;
} World;

World* world_create(size_t max_entities);
void world_destroy(World *world);
void world_reset_battle(World *world);

#endif //SPARSE_STORAGE_LEARNING_WORLD_H