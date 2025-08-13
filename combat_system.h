//
// Created by jo on 8/12/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_COMBAT_SYSTEM_H
#define SPARSE_STORAGE_LEARNING_COMBAT_SYSTEM_H

#include "world.h"

void combat_system_target_acquisition(World *world);
void combat_system_execute_attacks(World *world);
void combat_system_process_deaths(World *world);
bool combat_system_check_victory(World *world);

#endif //SPARSE_STORAGE_LEARNING_COMBAT_SYSTEM_H