//
// Created by jo on 8/8/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H
#define SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H
/**
 * @file death_queue.h
 * @brief Deferred entity destruction system for safe deletion during iteration
 *
 * The DeathQueue enables safe entity destruction by deferring the actual
 * deletion until a safe point, preventing iterator invalidation and
 * use-after-free bugs during system processing.
 */

#include "entity_manager.h"
#include "storage_manager.h"

/**
 * @brief Queue for deferred entity destruction to prevent iteration invalidation
 *
 * Entities marked for destruction are collected in this queue and processed
 * in batch during a safe point in the frame, ensuring systems can safely
 * iterate over components without worrying about mid-iteration deletions.
 */
typedef struct {
    Entity *entities;  /**< Dynamic array of entity handles pending destruction */
    size_t count;      /**< Number of entities currently queued for destruction */
    size_t capacity;   /**< Allocated capacity for the entities array */
} DeathQueue;

/**
 * @brief Initialize the death queue with the specified initial capacity
 * @param dq Pointer to the DeathQueue to initialize
 * @param initial_capacity Initial capacity for the queue (minimum 64 if 0)
 */
void death_queue_init(DeathQueue *dq, size_t initial_capacity);

/**
 * @brief Free the death queue's allocated memory
 * @param dq Pointer to the DeathQueue to free
 */
void death_queue_free(DeathQueue *dq);

/**
 * @brief Add an entity to the death queue for deferred destruction
 * @param dq Pointer to the DeathQueue
 * @param e Entity handle to queue for destruction
 * @note The queue will automatically grow if needed
 */
void death_queue_push(DeathQueue *dq, Entity e);

/**
 * @brief Clear all entities from the death queue without processing them
 * @param dq Pointer to the DeathQueue
 * @note This resets the count to 0 but doesn't actually destroy the entities
 */
void death_queue_clear(DeathQueue *dq);

/**
 * @brief Process all queued entities for destruction and clear the queue
 * @param dq Pointer to the DeathQueue containing entities to destroy
 * @param sm Pointer to the StorageManager to remove components from
 * @param em Pointer to the EntityManager to destroy entity handles
 * @param arena Pointer to the Arena (currently unused but kept for future use)
 *
 * @note This function destroys entities in the EntityManager and removes
 *       all their components from registered storage systems, then clears the queue
 */
void process_deaths(DeathQueue *dq, StorageManager *sm, EntityManager *em, Arena *arena);

#endif //SPARSE_STORAGE_LEARNING_DEATH_QUEUE_H