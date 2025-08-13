//
// Created by jo on 8/8/2025.
//

#include "entity_factory.h"
#include "world.h"
#include "components.h"
#include <stdio.h>
#include <stdlib.h>

Entity spawn_soldier(World *world, uint8_t team_id, uint32_t unit_number) {
    Entity soldier = entity_create(world->entity_manager);

    // stats with randomness
    StatComponent stats = {
        .health = 100 + (rand() % 20), // 100 - 120
        .max_health = 100 + (rand() % 20), // 15 - 25
        .attack = 15 + (rand() % 10), // 5 - 10
        .defense = 1.0f  + (rand() % 100) / 100.0f // 1.0 - 2.0
    };
    stats.max_health = stats.health;

    TeamComponent team = {
        .team_id = team_id,
        .unit_number = unit_number
    };

    CombatComponent combat = {
        .is_attacking = false,
        .target = {UINT32_MAX, 0}, // invalid initially
        .attack_cooldown = 0.0f
    };

    NameComponent name;
    snprintf(name.name, sizeof(name.name), "Team %c Soldier #%u",
            team_id == 0 ? 'A' : 'B', unit_number);

    // Add components to entity
    sparse_set_add(world->stats_storage, soldier.id, &stats);
    sparse_set_add(world->team_storage, soldier.id, &team);
    sparse_set_add(world->combat_storage, soldier.id, &combat);
    sparse_set_add(world->name_storage, soldier.id, &name);

    return soldier;
}

void spawn_army(World *world, uint8_t team_id, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        spawn_soldier(world, team_id, i+1);
        if (team_id == 0) {
            world->team_a_count++;
        } else {
            world->team_b_count++;
        }
    }
}