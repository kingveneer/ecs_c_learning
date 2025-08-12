//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMPONENTS_H
#define SPARSE_STORAGE_LEARNING_COMPONENTS_H
#include <stdbool.h>

typedef struct {
    char name[32];  // "Team A Soldier #1"
} NameComponent;

typedef struct {
    uint8_t team_id;  // 0 = team A, 1 = team B
    uint32_t unit_number;  // for display "Soldier #3"
} TeamComponent;

typedef  struct {
    int health;
    int attack_level;
    int defense_level;
} StatComponent;

typedef struct {
    bool is_attacking;
    Entity target;
    float attack_cooldown;
} CombatComponent;


#endif //SPARSE_STORAGE_LEARNING_COMPONENTS_H