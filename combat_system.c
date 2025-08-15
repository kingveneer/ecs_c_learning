//
// Created by jo on 8/12/2025.
//

#include "combat_system.h"
#include "components.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // Required for INT_MAX

/**
 * @brief Finds the weakest unit in a specific team.
 * This is called once per team per turn when the cache needs updating.
 */
static Entity find_weakest_in_team(World *world, uint8_t team_id) {
    Entity weakest = {UINT32_MAX, 0};
    int lowest_health = INT_MAX;

    CombatantBundle *all_combatants = world->combatant_storage->dense_data;

    for (uint32_t i = 0; i < world->combatant_storage->dense_count; i++) {
        CombatantBundle *combatant = &all_combatants[i];

        if (combatant->team.team_id == team_id && combatant->stats.health > 0) {
            if (combatant->stats.health < lowest_health) {
                lowest_health = combatant->stats.health;
                uint32_t entity_id = world->combatant_storage->dense_entities[i];
                weakest.id = entity_id;
                weakest.generation = world->entity_manager->generation[entity_id];
            }
        }
    }

    return weakest;
}

/**
 * @brief Updates the cached weakest enemies for both teams.
 * Only runs when needs_target_update flag is set.
 */
static void update_weakest_cache(World *world) {
    if (!world->needs_target_update) return;

    // Find weakest once for each team
    world->weakest_team_a = find_weakest_in_team(world, 0);
    world->weakest_team_b = find_weakest_in_team(world, 1);
    world->needs_target_update = false;
}

/**
 * @brief Assigns a target to any combatant that doesn't have a valid one.
 * Now uses cached weakest enemies instead of searching every time.
 */
void combat_system_target_acquisition(World *world) {
    // Update cache ONCE at the start of targeting phase
    update_weakest_cache(world);

    CombatantBundle *all_combatants = world->combatant_storage->dense_data;

    // Iterate directly over the data
    for (uint32_t i = 0; i < world->combatant_storage->dense_count; i++) {
        CombatantBundle *bundle = &all_combatants[i];

        // Only retarget if current target is invalid or dead
        if (bundle->combat.target.id == UINT32_MAX ||
            !entity_is_alive(world->entity_manager, bundle->combat.target)) {

            // Use cached weakest enemy instead of searching
            if (bundle->team.team_id == 0) {
                bundle->combat.target = world->weakest_team_b;
            } else {
                bundle->combat.target = world->weakest_team_a;
            }

            bundle->combat.is_attacking = (bundle->combat.target.id != UINT32_MAX);
        }
    }
}

/**
 * @brief Executes all attacks for the current turn with optimized data access.
 * Now includes caching of dense indices to reduce lookups.
 */
void combat_system_execute_attacks(World *world) {
    size_t checkpoint = arena_checkpoint(world->battle_arena);

    typedef struct {
        Entity attacker;
        Entity target;
        uint32_t attacker_dense_idx;  // Cache dense indices
        uint32_t target_dense_idx;
        int damage;
    } AttackEvent;

    AttackEvent *events = arena_alloc(world->battle_arena,
                                      sizeof(AttackEvent) * world->combatant_storage->dense_count);
    int event_count = 0;

    CombatantBundle *all_combatants = world->combatant_storage->dense_data;

    // --- GATHER PHASE (Optimized with cached indices) ---
    for (uint32_t i = 0; i < world->combatant_storage->dense_count; i++) {
        CombatantBundle *attacker_bundle = &all_combatants[i];

        if (!attacker_bundle->combat.is_attacking) continue;

        if (attacker_bundle->combat.target.id != UINT32_MAX &&
            entity_is_alive(world->entity_manager, attacker_bundle->combat.target)) {

            // Get target's dense index from sparse array
            uint32_t target_dense_idx = world->combatant_storage->sparse[attacker_bundle->combat.target.id];

            // Validate the dense index
            if (target_dense_idx != UINT32_MAX && target_dense_idx < world->combatant_storage->dense_count) {
                CombatantBundle *target_bundle = &all_combatants[target_dense_idx];

                if (target_bundle->stats.health > 0) {
                    int damage = attacker_bundle->stats.attack - target_bundle->stats.defense;
                    if (damage < 1) damage = 1;

                    Entity attacker_entity = {
                        .id = world->combatant_storage->dense_entities[i],
                        .generation = world->entity_manager->generation[world->combatant_storage->dense_entities[i]]
                    };

                    events[event_count++] = (AttackEvent){
                        .attacker = attacker_entity,
                        .target = attacker_bundle->combat.target,
                        .attacker_dense_idx = i,
                        .target_dense_idx = target_dense_idx,
                        .damage = damage
                    };
                }
            }
        }
    }

    // --- APPLY PHASE (Using cached indices) ---
    bool any_damage_dealt = false;

    for (int i = 0; i < event_count; i++) {
        AttackEvent *event = &events[i];

        // Direct access using cached dense index
        if (event->target_dense_idx < world->combatant_storage->dense_count) {
            CombatantBundle *target_bundle = &all_combatants[event->target_dense_idx];
            CombatantBundle *attacker_bundle = &all_combatants[event->attacker_dense_idx];

            if (target_bundle->stats.health > 0) {
                target_bundle->stats.health -= event->damage;
                any_damage_dealt = true;

                printf("%s attacks %s for %d damage! (%d hp remaining)\n",
                       attacker_bundle->name.name, target_bundle->name.name,
                       event->damage, target_bundle->stats.health);

                if (target_bundle->stats.health <= 0) {
                    printf("  >> %s has been defeated!\n", target_bundle->name.name);
                    death_queue_push(world->death_queue, event->target);

                    // Clear attacker's target
                    attacker_bundle->combat.target = (Entity){UINT32_MAX, 0};
                    attacker_bundle->combat.is_attacking = false;

                    // Mark that we need to update weakest cache
                    world->needs_target_update = true;
                }
            }
        }
    }

    // Only mark cache dirty if health values actually changed
    if (any_damage_dealt) {
        world->needs_target_update = true;
    }

    arena_restore(world->battle_arena, checkpoint);
}

/**
 * @brief Process all queued deaths.
 */
void combat_system_process_deaths(World *world) {
    if (world->death_queue->count > 0) {
        // Deaths mean the weakest might have changed
        world->needs_target_update = true;
    }
    process_deaths(world->death_queue, world->storage_manager, world->entity_manager);
}

/**
 * @brief Check if either team has won.
 */
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