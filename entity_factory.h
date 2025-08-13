//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_ENTITY_FACTORY_H
#define SPARSE_STORAGE_LEARNING_ENTITY_FACTORY_H
#include "world.h"
#include <stdint.h>
#include "ecs_core/entity_manager.h"  // For Entity type

// Forward declaration of World
typedef struct World World;

Entity spawn_soldier(World *world, uint8_t team_id, uint32_t unit_number);
void spawn_army(World *world, uint8_t team_id, uint32_t count);

#endif //SPARSE_STORAGE_LEARNING_ENTITY_FACTORY_H