//
// Created by jo on 8/8/2025.
//

#include "entity_factory.h"

#include "components.h"
#include "entity_manager.h"
#include "sparse_set_storage.h"

void spawn_goblin(EntityManager *em) {
    Entity e = entity_create(em);
    // Attach components here
    NameComponent goblin_name = {"Goblin"};
    StatComponent goblin_stats = {30, 5, 2};
    CombatComponent goblin_combat = {false, false};

    sparse_set_add(&names, goblin.id, &goblin_name);
    sparse_set_add(&stats, goblin.id, &goblin_stats);
    sparse_set_add(&combats, goblin.id, &goblin_combat);
    return e;
}
