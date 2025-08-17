//
// Created by jo on 8/12/2025.
//

#include "combat_system.h"
#include "components.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>


// Use team-specific sets to find weakest, not full scan
static Entity find_weakest_in_team_optimized(World *world, uint8_t team_id, int *out_health) {
    SparseSet *team_set = (team_id == 0) ? world->team_a_storage : world->team_b_storage;
    Entity weakest = {UINT32_MAX, 0};
    int lowest_health = INT_MAX;

    uint32_t *team_entities = team_set->dense_entities;
    uint32_t team_count = team_set->dense_count;

    // Only iterate through the specific team's entities
    for (uint32_t i = 0; i < team_count; i++) {
        uint32_t entity_id = team_entities[i];

        // Get combatant data from main storage
        uint32_t combatant_idx = world->combatant_storage->sparse[entity_id];
        if (combatant_idx >= world->combatant_storage->dense_count) continue;

        CombatantBundle *combatant = &((CombatantBundle*)world->combatant_storage->dense_data)[combatant_idx];

        if (combatant->health > 0 && combatant->health < lowest_health) {
            lowest_health = combatant->health;
            weakest.id = entity_id;
            weakest.generation = world->entity_manager->generation[entity_id];
        }
    }

    if (out_health) *out_health = lowest_health;
    return weakest;
}

//  Cache multiple weak targets per team
static void update_weakest_cache_multi(World *world) {
    if (!world->needs_target_update) return;

    // Reset caches
    world->weakest_cache_a.count = 0;
    world->weakest_cache_b.count = 0;

    // Team A weakest targets
    SparseSet *team_a = world->team_a_storage;
    for (uint32_t i = 0; i < team_a->dense_count && world->weakest_cache_a.count < WEAKEST_CACHE_SIZE; i++) {
        uint32_t entity_id = team_a->dense_entities[i];
        uint32_t idx = world->combatant_storage->sparse[entity_id];
        if (idx >= world->combatant_storage->dense_count) continue;

        CombatantBundle *combatant = &((CombatantBundle*)world->combatant_storage->dense_data)[idx];
        if (combatant->health > 0) {
            // Insert into sorted cache
            uint32_t insert_pos = world->weakest_cache_a.count;
            for (uint32_t j = 0; j < world->weakest_cache_a.count; j++) {
                if (combatant->health < world->weakest_cache_a.healths[j]) {
                    insert_pos = j;
                    // Shift others down
                    for (uint32_t k = world->weakest_cache_a.count; k > j; k--) {
                        if (k < WEAKEST_CACHE_SIZE) {
                            world->weakest_cache_a.targets[k] = world->weakest_cache_a.targets[k-1];
                            world->weakest_cache_a.healths[k] = world->weakest_cache_a.healths[k-1];
                        }
                    }
                    break;
                }
            }
            if (insert_pos < WEAKEST_CACHE_SIZE) {
                world->weakest_cache_a.targets[insert_pos].id = entity_id;
                world->weakest_cache_a.targets[insert_pos].generation = world->entity_manager->generation[entity_id];
                world->weakest_cache_a.healths[insert_pos] = combatant->health;
                if (world->weakest_cache_a.count < WEAKEST_CACHE_SIZE) {
                    world->weakest_cache_a.count++;
                }
            }
        }
    }

    // Team B weakest targets (same logic)
    SparseSet *team_b = world->team_b_storage;
    for (uint32_t i = 0; i < team_b->dense_count && world->weakest_cache_b.count < WEAKEST_CACHE_SIZE; i++) {
        uint32_t entity_id = team_b->dense_entities[i];
        uint32_t idx = world->combatant_storage->sparse[entity_id];
        if (idx >= world->combatant_storage->dense_count) continue;

        CombatantBundle *combatant = &((CombatantBundle*)world->combatant_storage->dense_data)[idx];
        if (combatant->health > 0) {
            uint32_t insert_pos = world->weakest_cache_b.count;
            for (uint32_t j = 0; j < world->weakest_cache_b.count; j++) {
                if (combatant->health < world->weakest_cache_b.healths[j]) {
                    insert_pos = j;
                    for (uint32_t k = world->weakest_cache_b.count; k > j; k--) {
                        if (k < WEAKEST_CACHE_SIZE) {
                            world->weakest_cache_b.targets[k] = world->weakest_cache_b.targets[k-1];
                            world->weakest_cache_b.healths[k] = world->weakest_cache_b.healths[k-1];
                        }
                    }
                    break;
                }
            }
            if (insert_pos < WEAKEST_CACHE_SIZE) {
                world->weakest_cache_b.targets[insert_pos].id = entity_id;
                world->weakest_cache_b.targets[insert_pos].generation = world->entity_manager->generation[entity_id];
                world->weakest_cache_b.healths[insert_pos] = combatant->health;
                if (world->weakest_cache_b.count < WEAKEST_CACHE_SIZE) {
                    world->weakest_cache_b.count++;
                }
            }
        }
    }

    // Set primary weakest for backward compatibility
    if (world->weakest_cache_a.count > 0) {
        world->weakest_team_a = world->weakest_cache_a.targets[0];
        world->weakest_team_a_health = world->weakest_cache_a.healths[0];
    } else {
        world->weakest_team_a = (Entity){UINT32_MAX, 0};
        world->weakest_team_a_health = INT_MAX;
    }

    if (world->weakest_cache_b.count > 0) {
        world->weakest_team_b = world->weakest_cache_b.targets[0];
        world->weakest_team_b_health = world->weakest_cache_b.healths[0];
    } else {
        world->weakest_team_b = (Entity){UINT32_MAX, 0};
        world->weakest_team_b_health = INT_MAX;
    }

    world->needs_target_update = false;
}

void combat_system_target_acquisition(World *world) {
    update_weakest_cache_multi(world);

    CombatantBundle *all_combatants = world->combatant_storage->dense_data;
    uint32_t count = world->combatant_storage->dense_count;

    //  Distribute targets across multiple weak enemies
    uint32_t team_a_target_idx = 0;
    uint32_t team_b_target_idx = 0;

    for (uint32_t i = 0; i < count; i++) {
        CombatantBundle *bundle = &all_combatants[i];

        if (bundle->target.id == UINT32_MAX ||
            !entity_is_alive(world->entity_manager, bundle->target)) {

            if (bundle->team_id == 0) {
                // Team A attacks Team B
                if (world->weakest_cache_b.count > 0) {
                    bundle->target = world->weakest_cache_b.targets[team_b_target_idx % world->weakest_cache_b.count];
                    team_b_target_idx++;
                } else {
                    bundle->target = (Entity){UINT32_MAX, 0};
                }
            } else {
                // Team B attacks Team A
                if (world->weakest_cache_a.count > 0) {
                    bundle->target = world->weakest_cache_a.targets[team_a_target_idx % world->weakest_cache_a.count];
                    team_a_target_idx++;
                } else {
                    bundle->target = (Entity){UINT32_MAX, 0};
                }
            }

            bundle->is_attacking = (bundle->target.id != UINT32_MAX);
        }
    }
}

// Batch damage application with prefetching
void combat_system_execute_attacks(World *world) {
    CombatantBundle *all_combatants = world->combatant_storage->dense_data;
    uint32_t *sparse = world->combatant_storage->sparse;
    uint32_t count = world->combatant_storage->dense_count;

    // Use damage accumulator to reduce random memory access
    size_t checkpoint = arena_checkpoint(world->battle_arena);
    int32_t *damage_accumulator = arena_alloc(world->battle_arena, sizeof(int32_t) * count);
    memset(damage_accumulator, 0, sizeof(int32_t) * count);

    // First pass: Calculate all damage (read-only, cache-friendly)
    for (uint32_t i = 0; i < count; i++) {
        CombatantBundle *attacker = &all_combatants[i];

        if (!attacker->is_attacking || attacker->target.id == UINT32_MAX)
            continue;

        if (!entity_is_alive(world->entity_manager, attacker->target))
            continue;

        uint32_t target_idx = sparse[attacker->target.id];
        if (target_idx >= count) continue;

        CombatantBundle *target = &all_combatants[target_idx];
        if (target->health <= 0) continue;

        // Calculate damage
        int damage = attacker->attack - target->defense;
        if (damage < 1) damage = 1;

        // Accumulate damage for this target
        damage_accumulator[target_idx] += damage;
    }

    // Second pass: Apply damage and process deaths (single write pass)
    bool needs_cache_update = false;

    for (uint32_t i = 0; i < count; i++) {
        if (damage_accumulator[i] > 0) {
            CombatantBundle *target = &all_combatants[i];
            target->health -= damage_accumulator[i];

            if (target->health <= 0) {
                // Get entity ID from dense array
                uint32_t entity_id = world->combatant_storage->dense_entities[i];
                Entity dead_entity = {entity_id, world->entity_manager->generation[entity_id]};
                death_queue_push(world->death_queue, dead_entity);

                // Clear attackers targeting this entity
                for (uint32_t j = 0; j < count; j++) {
                    if (all_combatants[j].target.id == entity_id) {
                        all_combatants[j].target = (Entity){UINT32_MAX, 0};
                        all_combatants[j].is_attacking = false;
                    }
                }

                needs_cache_update = true;
            }
        }
    }

    world->needs_target_update = needs_cache_update;
    arena_restore(world->battle_arena, checkpoint);
}

// Batch process deaths more efficiently
void combat_system_process_deaths(World *world) {
    if (world->death_queue->count == 0) return;

    // Pre-allocate batch removal array
    size_t checkpoint = arena_checkpoint(world->battle_arena);
    uint32_t *dead_ids = arena_alloc(world->battle_arena, sizeof(uint32_t) * world->death_queue->count);

    // Collect all dead entity IDs
    for (size_t i = 0; i < world->death_queue->count; i++) {
        dead_ids[i] = world->death_queue->entities[i].id;
    }

    // Batch destroy entities
    entity_destroy_batch(world->entity_manager, world->death_queue->entities, world->death_queue->count);

    // Remove from all storages in a single pass
    // This is more cache-friendly than multiple passes
    for (size_t i = 0; i < world->death_queue->count; i++) {
        uint32_t entity_id = dead_ids[i];

        // Remove from combatant storage
        if (entity_id < world->combatant_storage->capacity) {
            sparse_set_remove(world->combatant_storage, entity_id);
        }

        // Remove from team storages
        if (entity_id < world->team_a_storage->capacity) {
            sparse_set_remove(world->team_a_storage, entity_id);
        }
        if (entity_id < world->team_b_storage->capacity) {
            sparse_set_remove(world->team_b_storage, entity_id);
        }
    }

    arena_restore(world->battle_arena, checkpoint);
    death_queue_clear(world->death_queue);
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