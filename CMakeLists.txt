cmake_minimum_required(VERSION 4.0)
project(sparse_storage_learning C)

set(CMAKE_C_STANDARD 99)

add_executable(sparse_storage_learning main.c
        sparse_set_storage.c
        entity.h
        components.h)
