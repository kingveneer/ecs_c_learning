#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "world.h"
#include "entity_factory.h"
#include "combat_system.h"

void run_battle(World *world) {
    clock_t start_time = clock();

    printf("\n=== BATTLE BEGINS ===\n");
    world->battle_active = true;

    while (world->battle_active && world->turn_number < 100000) {
        //printf("\n--- Turn %u ---\n", ++world->turn_number);

        // 1. Target acquisition
        combat_system_target_acquisition(world);

        // 2. Execute attacks
        combat_system_execute_attacks(world);

        // 3. Process deaths
        combat_system_process_deaths(world);

        // 4. Check victory
        if (combat_system_check_victory(world)) {
            world->battle_active = false;
            break;
        }

        // Optional: pause for dramatic effect
        // getchar();  // Press enter to continue
    }

    if (world->turn_number >= 100000) {
        printf("\nBattle timeout - draw!\n");
    }

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("\nBattle simulation finished in %.4f seconds.\n", time_taken);
}

int main() {
    srand(time(NULL));

    // Create world
    World *world = world_create(100000);

    printf("=== ECS BATTLE SIMULATOR ===\n");

    while (1) {
        printf("\nEnter number of units per team (0 to exit): ");
        int count;
        scanf("%d", &count);

        if (count <= 0) break;

        // Reset for new battle
        world_reset_battle(world);

        // Spawn armies
        printf("Spawning %d units per team...\n", count);
        spawn_army(world, 0, count);  // Team A
        spawn_army(world, 1, count);  // Team B

        // Run the battle
        run_battle(world);
    }

    printf("Thanks for playing!\n");
    world_destroy(world);


    return 0;
}
