cmake_minimum_required(VERSION 4.0)
project(sparse_storage_learning C)

set(CMAKE_C_STANDARD 99)

add_executable(sparse_storage_learning main.c
        entity.h
        components.h
        sparse_set_storage.h
        entity_manager.c
        entity_manager.h)
