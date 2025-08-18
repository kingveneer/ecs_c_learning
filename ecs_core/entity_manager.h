//
// Created by jo on 8/7/2025.
//

#ifndef SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H
#define SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H


#include <stdint.h>

/**
 * @file entity_manager.h
 * @brief Robust entity management with handle validation and ID recycling
 *
 * Provides generation-based entity handles to prevent use-after-free bugs
 * and implements efficient ID recycling for memory conservation. Entities
 * are represented as handles containing both ID and generation components.
 */

#include <stdint.h>

/**
 * @brief Legacy handle structure (kept for compatibility)
 * @deprecated Use Entity instead
 */
typedef struct {
    uint32_t id;         /**< Entity identifier */
    uint32_t generation; /**< Generation counter for validation */
} Handle;

/**
 * @brief Entity handle with generation-based validation
 *
 * Combines an entity ID with a generation counter to prevent accessing
 * stale or recycled entities. The generation is incremented each time
 * an ID is recycled, invalidating old handles.
 */
typedef struct {
    uint32_t id;         /**< Unique entity identifier within the current generation */
    uint32_t generation; /**< Version number to detect stale handles */
} Entity;

/**
 * @brief Manages entity creation, destruction, and validation with ID recycling
 *
 * The EntityManager prevents use-after-free bugs through generation counters
 * and efficiently recycles entity IDs to minimize memory usage. Each entity
 * ID maintains a generation counter that increments on destruction.
 */
typedef struct {
    uint32_t *generation; /**< Generation counter for each entity ID */
    uint32_t *free_ids;   /**< Stack of available/recycled entity IDs */
    uint32_t capacity;    /**< Maximum number of entities supported */
    uint32_t living_count; /**< Number of currently active entities */
    uint32_t free_count;  /**< Number of IDs available in the free stack */
} EntityManager;

/**
 * @brief Initialize the entity manager with the specified maximum capacity
 * @param em Pointer to the EntityManager to initialize
 * @param capacity Maximum number of entities that can exist simultaneously
 * @note All entity IDs are initially available in the free stack
 */
void entity_manager_init(EntityManager *em, uint32_t capacity);

/**
 * @brief Create a new entity and return its handle
 * @param em Pointer to the EntityManager
 * @return Entity handle with valid ID and generation, or invalid handle if at capacity
 * @retval Entity with id=UINT32_MAX if no free IDs available
 */
Entity entity_create(EntityManager *em);

/**
 * @brief Destroy an entity and invalidate its handle
 * @param em Pointer to the EntityManager
 * @param e Entity handle to destroy
 * @note Increments the generation counter and returns the ID to the free stack
 * @note Safely handles already-destroyed or invalid entities
 */
void entity_destroy(EntityManager *em, Entity e);

/**
 * @brief Destroy multiple entities in a single batch operation
 * @param em Pointer to the EntityManager
 * @param entities Array of Entity handles to destroy
 * @param count Number of entities in the array
 * @note More efficient than calling entity_destroy() in a loop for large batches
 */
void entity_destroy_batch(EntityManager *em, const Entity *entities, uint32_t count);

/**
 * @brief Check if an entity handle is still valid and alive
 * @param em Pointer to the EntityManager
 * @param e Entity handle to validate
 * @return 1 if entity is alive, 0 if destroyed or invalid
 * @note Compares the handle's generation against the current generation for that ID
 */
int entity_is_alive(const EntityManager *em, Entity e);

/**
 * @brief Free all allocated memory and reset the entity manager
 * @param em Pointer to the EntityManager to free
 * @note Sets all pointers to NULL and counters to 0 for safety
 */
void entity_manager_free(EntityManager *em);

#endif //SPARSE_STORAGE_LEARNING_ENTITY_MANAGER_H