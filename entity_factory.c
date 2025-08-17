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

    // Hot data
    bundle.health = 100 + (rand() % 21); // 100 - 120
    bundle.attack = 15 + (rand() % 11); // 15 - 25
    bundle.defense = 5 + (rand() % 6); // 5 - 10
    bundle.team_id = team_id;
    bundle.is_attacking = false;
    bundle.target = (Entity){UINT32_MAX, 0}; // invalid initially

    // Cold data
    bundle.max_health = bundle.health;
    bundle.speed = 1.0f + (rand() % 100) / 100.0f; // 1.0 - 2.0
    bundle.attack_cooldown = 0.0f;
    bundle.unit_number = unit_number;

    snprintf(bundle.name, sizeof(bundle.name), "Team %c Soldier #%u",
            team_id == 0 ? 'A' : 'B', unit_number);

    // Add the entire bundle to storage
    sparse_set_add(world->combatant_storage, soldier.id, &bundle);

    // Add to team-specific index sets
    if (team_id == 0) {
        sparse_set_add(world->team_a_storage, soldier.id, NULL);
        world->team_a_count++;
    } else {
        sparse_set_add(world->team_b_storage, soldier.id, NULL);
        world->team_b_count++;
    }

    return soldier;
}

void spawn_army(World *world, uint8_t team_id, uint32_t count) {
    // Pre-allocate in batches for better cache locality
    for (uint32_t i = 0; i < count; i++) {
        spawn_soldier(world, team_id, i + 1);
    }

    // Force cache update after spawning
    world->needs_target_update = true;
}