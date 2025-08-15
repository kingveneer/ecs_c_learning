//
// Created by jo on 8/8/2025.
//

#include "entity_factory.h"
#include "world.h"
#include "components.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Entity spawn_soldier(World *world, uint8_t team_id, uint32_t unit_number) {
    Entity soldier = entity_create(world->entity_manager);

    CombatantBundle bundle;

    bundle.stats.health = 100 + (rand() % 21); // 100 - 120
    bundle.stats.max_health = bundle.stats.health;
    bundle.stats.attack = 15 + (rand() % 11); // 15 - 25
    bundle.stats.defense = 5 + (rand() % 6); // 5 - 10
    bundle.stats.speed = 1.0f  + (rand() % 100) / 100.0f; // 1.0 - 2.0

    bundle.team.team_id = team_id;
    bundle.team.unit_number = unit_number;

    bundle.combat.is_attacking = false;
    bundle.combat.target = (Entity){UINT32_MAX, 0}; // invalid initially
    bundle.combat.attack_cooldown = 0.0f;

    snprintf(bundle.name.name, sizeof(bundle.name.name), "Team %c Soldier #%u",
            team_id == 0 ? 'A' : 'B', unit_number);
    // Add the entire bundle to the new storage in ONE operation.
    // This is far more cache-friendly than adding components separately.
    sparse_set_add(world->combatant_storage, soldier.id, &bundle);

    // add NULL because these are component-less index sets.
    if (team_id == 0) {
        sparse_set_add(world->team_a_storage, soldier.id, NULL);
    } else {
        sparse_set_add(world->team_b_storage, soldier.id, NULL);
    }
    return soldier;
}

void spawn_army(World *world, uint8_t team_id, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        spawn_soldier(world, team_id, i+1);
        }
}