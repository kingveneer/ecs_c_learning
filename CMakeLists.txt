cmake_minimum_required(VERSION 4.0)
project(sparse_storage_learning C)

set(CMAKE_C_STANDARD 99)

add_executable(sparse_storage_learning main.c
        entity.h
        components.h
        entity_manager.c
        entity_manager.h
        sparse_set_storage.c
        sparse_set_storage.h
        entity_factory.c
        entity_factory.h
        storage_manager.c
        storage_manager.h
        death_queue.c
        death_queue.h
        arena.c
        arena.h)
