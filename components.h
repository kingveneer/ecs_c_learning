//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMPONENTS_H
#define SPARSE_STORAGE_LEARNING_COMPONENTS_H
#include <stdbool.h>

typedef  struct {
    char *name;
} NameComponent;

typedef  struct {
    int health;
    int attack_level;
    int defense_level;
} StatComponent;

typedef struct {
    bool is_attacking;
    bool is_defending;
} CombatComponent;

#endif //SPARSE_STORAGE_LEARNING_COMPONENTS_H