//
// Created by jo on 8/12/2025.
//

#include "combat_system.h"
#include "components.h"
#include <stdio.h>
#include <stdlib.h>

static Entity find_weakest_enemy(World *world, Entity attacker) {
    TeamComponent *attacker_team = sparse_set_get(world->team_storage, attacker.id);
    if (!attacker_team) return (Entity){UINT32_MAX, 0};

    // CHOOSE THE CORRECT ENEMY INDEX TO SEARCH
    SparseSet* enemy_team_storage = (attacker_team->team_id == 0)
                                    ? world->team_b_storage
                                    : world->team_a_storage;

    Entity weakest = {UINT32_MAX, 0};
    int lowest_health = INT32_MAX;

    // ITERATE ONLY OVER THE ENEMY TEAM
    for (uint32_t i = 0; i < enemy_team_storage->dense_count; i++) {
        uint32_t enemy_id = enemy_team_storage->dense_entities[i];

        // We already know this entity is on the enemy team, so no team check is needed.
        // We still need to get its stats.
        StatComponent *enemy_stats = sparse_set_get(world->stats_storage, enemy_id);

        // The entity is guaranteed to be alive because it's removed from this
        // list by process_deaths. We just check stats.
        if (enemy_stats && enemy_stats->health < lowest_health) {
            lowest_health = enemy_stats->health;
            weakest.id = enemy_id;
            // We must get the current generation for the handle to be valid.
            weakest.generation = world->entity_manager->generation[enemy_id];
        }
    }
    return weakest;
}

void combat_system_target_acquisition(World *world) {
    // For each entity with combat component
    for (uint32_t i = 0; i < world->combat_storage->dense_count; i++) {
        uint32_t entity_id = world->combat_storage->dense_entities[i];
        Entity attacker = {entity_id, world->entity_manager->generation[entity_id]};

        if (!entity_is_alive(world->entity_manager, attacker)) continue;

        CombatComponent *combat = sparse_set_get(world->combat_storage, entity_id);
        StatComponent *stats = sparse_set_get(world->stats_storage, entity_id);

        if (!stats || stats->health <= 0) continue;

        // If no valid target, find one
        if (combat->target.id == UINT32_MAX || !entity_is_alive(world->entity_manager, combat->target)) {
            combat->target = find_weakest_enemy(world, attacker);
            combat->is_attacking = (combat->target.id != UINT32_MAX);
        }
    }
}

void combat_system_execute_attacks(World *world) {
    // Collect all attacks first (using frame arena for temp storage)
    // save current memory arena to "rewind" and free all temp allocations at once
    size_t checkpoint = arena_checkpoint(world->battle_arena);

    typedef struct {
        Entity attacker;
        Entity target;
        int damage;
    } AttackEvent;

    AttackEvent *events = arena_alloc(world->battle_arena,
                                      sizeof(AttackEvent) * 1000);
    int event_count = 0;

    // Gather attacks
    for (uint32_t i = 0; i < world->combat_storage->dense_count; i++) {
        uint32_t entity_id = world->combat_storage->dense_entities[i];
        Entity attacker = {entity_id, world->entity_manager->generation[entity_id]};
        // if alive
        if (!entity_is_alive(world->entity_manager, attacker)) continue;
        // if has combat component
        CombatComponent *combat = sparse_set_get(world->combat_storage, entity_id);
        if (!combat->is_attacking) continue;
        // skip dead entities
        StatComponent *attacker_stats = sparse_set_get(world->stats_storage, entity_id);
        if (!attacker_stats || attacker_stats->health <= 0) continue;
        // check attacker has stats and isn't dead
        if (combat->target.id != UINT32_MAX && entity_is_alive(world->entity_manager, combat->target)) {
            StatComponent *target_stats = sparse_set_get(world->stats_storage, combat->target.id);
            if (target_stats && target_stats->health > 0) {
                // Calculate damage
                int damage = attacker_stats->attack - target_stats->defense;
                if (damage < 1) damage = 1;  // Minimum 1 damage

                events[event_count++] = (AttackEvent){
                    .attacker = attacker,
                    .target = combat->target,
                    .damage = damage
                };
            }
        }
    }
    // Apply damage
    for (int i = 0; i < event_count; i++) {
        AttackEvent *event = &events[i];
        StatComponent *target_stats = sparse_set_get(world->stats_storage, event->target.id);
        if (target_stats) {
            if (target_stats && target_stats->health > 0){
                target_stats->health -= event->damage;

                NameComponent *attacker_name = sparse_set_get(world->name_storage, event->attacker.id);
                NameComponent *target_name = sparse_set_get(world->name_storage, event->target.id);

                printf("%s attacks %s for %d damage! (%d hp remaining)\n",
                       attacker_name->name, target_name->name,
                       event->damage, target_stats->health);

                if (target_stats->health <= 0) {
                    printf("  >> %s has been defeated!\n", target_name->name);
                    death_queue_push(world->death_queue, event->target);

                    // Clear attacker's target
                    CombatComponent *attacker_combat = sparse_set_get(world->combat_storage, event->attacker.id);
                    if (attacker_combat) {
                        attacker_combat->target = (Entity){UINT32_MAX, 0};
                        attacker_combat->is_attacking = false;
                    }
                }
            }
        }
    }
    // Restore arena (frees the events array)
    arena_restore(world->battle_arena, checkpoint);
}

void combat_system_process_deaths(World *world) {
    process_deaths(world->death_queue, world->storage_manager, world->entity_manager);
}

bool combat_system_check_victory(World *world) {
    uint32_t team_a_alive = world->team_a_storage->dense_count;
    uint32_t team_b_alive = world->team_b_storage->dense_count;

    if (team_a_alive == 0 || team_b_alive == 0) {
        printf("\n=== BATTLE COMPLETE ===\n");
        if (team_a_alive > 0) {
            printf("Team A wins with %u survivors!\n", team_a_alive);
        } else if (team_b_alive > 0) {
            printf("Team B wins with %u survivors!\n", team_b_alive);
        } else {
            printf("It's a draw - mutual destruction!\n");
        }
        return true;
    }
    return false;
}