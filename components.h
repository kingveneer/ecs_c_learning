//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMPONENTS_H
#define SPARSE_STORAGE_LEARNING_COMPONENTS_H
#include <stdbool.h>

typedef  struct StatComponent {
    int health;
    int attack_level;
    int defense_level;
} stat_component;

typedef struct CombatComponent {
    bool is_attacking;
    bool is_defending;
} combat_component;

#endif //SPARSE_STORAGE_LEARNING_COMPONENTS_H