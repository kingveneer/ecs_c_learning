cmake_minimum_required(VERSION 4.0)
project(sparse_storage_learning C)

set(CMAKE_C_STANDARD 99)

add_executable(sparse_storage_learning main.c
        components.h
        ecs_core/entity_manager.c
        ecs_core/entity_manager.h
        ecs_core/sparse_set_storage.c
        ecs_core/sparse_set_storage.h
        entity_factory.c
        entity_factory.h
        ecs_core/storage_manager.c
        ecs_core/storage_manager.h
        ecs_core/death_queue.c
        ecs_core/death_queue.h
        ecs_core/arena.c
        ecs_core/arena.h
        world.c
        world.h
        combat_system.c
        combat_system.h
        ecs_core/component_allocator.c
        ecs_core/component_allocator.h)
