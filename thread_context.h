/**
 * @defgroup module_thread_context ThreadContext
 * 
 * @file thread_context.h
 * @brief This module is an implementation of the datatype ThreadContext.
 * 
 * This module provides an implementation of the ThreadContext data type, which
 * allows you to store and manipulate a collection of coordinator objects and their
 * synchronisation mechanisms. It is designed to be a dynamic data structure that can
 * accommodate an arbitrary number of coordinators, with the first coordinators at 
 * index of zero.
 *
 * @author  Daniel Hylander
 * @date 2023-10-18
 * @{
 */


#ifndef THREAD_CONTEXT_H
#define THREAD_CONTEXT_H

#include <pthread.h>

#include "queue.h"
#include "safe_functions.h"

/**
 * @struct Counter
 * 
 * @brief Type for a counter with a mutex lock.
*/
typedef struct {
    pthread_mutex_t mutex;
    int counter;
} Counter;

/**
 * @struct Coordinator
 * 
 * @brief Type for a thread coordinator.
 * 
 * Contains the information for traversing a directory tree given as an 
 * argument. Each sub directory is stored in the queue and a counter for 
 * each thread currently traversing sub-directory's. The total size of the 
 * directory's is also stored.
*/
typedef struct {
    pthread_mutex_t mutex_sum;

    Counter* active_threads;

    int tot_sum;

    Queue* dir_queue;
    pthread_mutex_t mutex_queue;
} Coordinator;

/**
 * @struct ThreadContext
 * 
 * @brief Type for the thread context.
 * 
 * Contains an array of coordinators, and a mutex lock and condition.
*/
typedef struct {
    Coordinator** coordinator;

    pthread_mutex_t mutex_error;
    pthread_mutex_t mutex_work;
    pthread_cond_t cond_work;

    int dir_num;
    int size;
} ThreadContext;


/**
 * @brief Constructs an initializes a new Commands object.
 * 
 * @return A pointer to the newly created object.
 * 
 * @note The returned object must be freed using the `thread_context_destroy` function
 *       when it is no longer needed to prevent memory leaks.
 * @see thread_context_destroy()
*/
ThreadContext* create_thread_context(void);

/**
 * @brief Creates a new coordinator object in an array of coordinators.
 * 
 * @param thread_context A pointer to the thread_context storing 
 * the coordinators.
 * @param dir_num Number of coordinators in the array.
 *
 * @note The returned object must be freed using the `thread_context_destroy` function
 *       when it is no longer needed to prevent memory leaks.
 * @see thread_context_destroy()
*/
void expand_and_create_coordinator(ThreadContext* thread_context);

/**
 * @brief Increments the counter plus one.
 * 
 * @param counter Pointer to the counter.
*/
void increment_counter(Counter* counter);

/**
 * @brief Decrements the counter minus one.
 * 
 * @param counter Pointer to the counter.
*/
void decrement_counter(Counter* counter);

/**
 * @brief Returns the value of the counter.
 * 
 * @param counter Pointer to the counter.
 * @return Returns the value of the counter.
*/
int get_counter_value(Counter* counter);

/**
 * @brief Updates the total sum stored in the coordinator.
 * 
 * @param value The value to add to the total sum.
 * @param coordinator Pointer to the coordinator.
*/
void update_total_sum(int value, Coordinator* coordinator);

/**
 * @brief Deallocates the ThreadContext object.
 * 
 * @param A pointer to the ThreadContext object.
*/
void thread_context_destroy(ThreadContext* thread_context);


#endif

/**
 * }
*/