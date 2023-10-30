/** 
 * @defgroup module_queue Queue
 * 
 * @file queue.h
 * @brief This module implements the datatype Queue
 * 
 * The module provides an interface and generic operations for the 
 * datatype queue, specifically designed to store strings.
 * 
 *  @author Daniel Hylander
 * @date 2023-10-18
 * 
 *  @{
 */


#ifndef QUEUE_H
#define QUEUE_H


#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct node {
    struct node *next;
    char *value;
};

/** @brief The type for the queue.
 */
typedef struct queue {
    struct node *first;
    struct node *last;

    pthread_mutex_t mutex;
} Queue;


/** @brief Create and return an empty queue.
 * 
 * @return Returns the newly created queue.
 * 
 * @note It is the caller's responsible to deallocate the the queue after use 
 * by calling the function `queue_destroy()`.
 * @see queue_destroy()
 */
Queue *queue_create(void* in_use_data);


/** @brief Destroy the queue.
 * 
 *  @param q A pointer to the queue.
 */
void queue_destroy(Queue *q);


/** @brief Add a value to the end of the queue.
 * 
 *  @param q Pointer to the queue.
 *  @param value Pointer to the string value to be enqueued.
 * 
 *  @note The value is copied to dynamically allocated memory.
 */
void queue_enqueue(Queue *q, const char *value, void* in_use_data);


/** @brief Remove the first value in the queue and returns it.
 * 
 *  @param q Pointer to the queue.
 *  @return Returns the value at the beginning of the queue.
 * 
 *  @note The caller is responsible to check that the queue is not empty, 
 *  and for deallocating the returned pointer.
 *  @see queue_is_empty()
 */
char *queue_dequeue(Queue *q);


/** @brief Check if the queue is empty.
 * 
 *  @param q Pointer to the queue.
 *  @return It returns true if the queue is empty; else false.
 */
bool queue_is_empty(Queue *q);


#endif /* QUEUE_H */