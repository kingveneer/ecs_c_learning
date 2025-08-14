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

typedef struct World {
    // ECS Core
    EntityManager *entity_manager;
    StorageManager *storage_manager;
    DeathQueue *death_queue;

    // Memory
    Arena *persistent_arena;
    Arena *battle_arena; // reset per battle

    // Component Storages
    SparseSet *stats_storage;
    SparseSet *team_storage;
    SparseSet *team_a_storage;
    SparseSet *team_b_storage;
    SparseSet *combat_storage;
    SparseSet *name_storage;

    // Battle State
    uint32_t team_a_count;
    uint32_t team_b_count;
    bool battle_active;
    uint32_t turn_number;
} World;

World* world_create(size_t max_entities);
void world_destroy(World *world);
void world_reset_battle(World *world);

#endif //SPARSE_STORAGE_LEARNING_WORLD_H