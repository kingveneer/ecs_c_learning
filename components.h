//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMPONENTS_H
#define SPARSE_STORAGE_LEARNING_COMPONENTS_H
#include <stdbool.h>
#include "ecs_core/entity_manager.h"

// Reordered for better cache alignment and access patterns
typedef struct {
    // Hot data - accessed every frame (32 bytes)
    int health;           // 4 bytes
    int attack;           // 4 bytes
    int defense;          // 4 bytes
    uint8_t team_id;      // 1 byte
    bool is_attacking;    // 1 byte
    uint8_t _padding[2];  // 2 bytes padding for alignment
    Entity target;        // 8 bytes
    // Total: 24 bytes (fits well in cache)

    // Cold data - rarely accessed during combat
    int max_health;       // 4 bytes
    float speed;          // 4 bytes
    float attack_cooldown;// 4 bytes
    uint32_t unit_number; // 4 bytes
    char name[32];        // 32 bytes
} CombatantBundle;

// Simplified component structures for clarity
typedef struct {
    char name[32];
} NameComponent;

typedef struct {
    uint8_t team_id;
    uint32_t unit_number;
} TeamComponent;

typedef struct {
    int health;
    int max_health;
    int attack;
    int defense;
    float speed;
} StatComponent;

typedef struct {
    bool is_attacking;
    Entity target;
    float attack_cooldown;
} CombatComponent;
#endif //SPARSE_STORAGE_LEARNING_COMPONENTS_H