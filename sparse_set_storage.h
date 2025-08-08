//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H
#define SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H

typedef struct {
    int* sparse_storage;
    int* dense_storage;
    int active_components;
} sparse_storage;

#endif //SPARSE_STORAGE_LEARNING_SPARSE_SET_STORAGE_H