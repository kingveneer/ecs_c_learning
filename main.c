#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "entity_manager.h"
#include "sparse_set_storage.h"
#include  "components.h"


int main(void) {
    // entity manager capable of handling 100 entities
    EntityManager em;
    entity_manager_init(&em, 100);

    // one sparse set per component type
    SparseSet names, stats, combats;
    sparse_set_init(&names, 100, sizeof(NameComponent));
    sparse_set_init(&stats, 100, sizeof(StatComponent));
    sparse_set_init(&combats, 100, sizeof(CombatComponent));

    // create goblin
    for (int i = 0; i < 50; i++) {
        Entity goblin = entity_create(&em);

        NameComponent goblin_name = {"Goblin"};
        StatComponent goblin_stats = {30, 5, 2};
        CombatComponent goblin_combat = {false, false};

        sparse_set_add(&names, goblin.id, &goblin_name);
        sparse_set_add(&stats, goblin.id, &goblin_stats);
        sparse_set_add(&combats, goblin.id, &goblin_combat);
    }
    // create gnome


    printf("=== Entity Stats ===\n");
    for (uint32_t i= 0; i < stats.dense_count; i++) {
        uint32_t ent_id = stats.dense_entities[i];
        NameComponent *n = sparse_set_get(&names, ent_id);
        StatComponent *s = sparse_set_get(&stats, ent_id);
        printf("%s -> HP: %d, ATK %d, DEF %d\n",
                n->name, s->health, s->attack_level, s->defense_level);
    }

    for (uint32_t i = stats.dense_count - 1; i >= 0; i--) {
        // Get the entity ID from dense storage
        uint32_t ent_id = stats.dense_entities[i];

        // Get its name component
        NameComponent *n = sparse_set_get(&names, ent_id);

        // Check if this entity is a goblin
        if (strcmp(n->name, "goblin") == 0) {
            // Retrieve the full Entity struct for generation checking
            Entity e = em.living;

            // Destroy the entity (frees ID + increments generation)
            entity_destroy(&em, e);

            // Remove its components from their sparse sets
            sparse_set_remove(&names, ent_id);
            sparse_set_remove(&stats, ent_id);
            sparse_set_remove(&combats, ent_id);

            printf("Removed goblin (Entity ID: %u)\n", ent_id);
        }
    }
    return 0;
}