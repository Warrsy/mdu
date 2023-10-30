/*
 * @brief This module is an implementation of the datatype ThreadContext.
 * 
 * This module provides an implementation of the ThreadContext data type, which
 * allows you to store and manipulate a collection of coordinator objects and their
 * synchronisation mechanisms. It is designed to be a dynamic data structure that can
 * accommodate an arbitrary number of coordinators, with the first coordinators at 
 * index of zero.
 *
 * @author Daniel Hylander
 * @date 2023-10-18
 * @{
 */

#include "thread_context.h"

/*-----------------------INTERNAL FUCTIONS-----------------------*/

/*
 * @brief Creates a counter.
 *
 * @return A pointer to the newly created object.
 * 
 * @note The returned object must be freed using the `destroy_counter` function
 *       when it is no longer needed to prevent memory leaks.
 * @see destroy_counter()
*/
static Counter* create_counter(void) {
    Counter* counter = safe_malloc(sizeof(Counter), NULL);

    pthread_mutex_init(&counter->mutex, NULL);
    counter->counter = 0;

    return counter;
}

/*
 * @brief Constructs an initializes a new Coordinator object.
 * 
 * @return A pointer to the newly created object.
 * 
 * @note The returned object must be freed using the `destroy_coordinator` function
 *       when it is no longer needed to prevent memory leaks.
 * @see destroy_coordinator()
*/
static Coordinator* create_coordinator(ThreadContext* thread_context) {
    Coordinator* coordinator = safe_malloc(sizeof(Coordinator), thread_context);

    pthread_mutex_init(&coordinator->mutex_sum, NULL);
    pthread_mutex_init(&coordinator->mutex_queue, NULL);

    coordinator->active_threads = create_counter();
    coordinator->dir_queue = queue_create(thread_context);
    coordinator->tot_sum = 0;

    return coordinator;
}

/*
 * @brief Deallocates the counter.
 *
 * @param Pointer to the counter.
*/
static void destroy_counter(Counter* counter) {
    pthread_mutex_destroy(&counter->mutex);
    free(counter);
}

/*
 * @brief Deallocates the coordinator.
 *
 * @param Pointer to the coordinator.
*/
static void destroy_coordinator(Coordinator* coordinator) {
    pthread_mutex_destroy(&coordinator->mutex_sum);
    pthread_mutex_destroy(&coordinator->mutex_queue);
    destroy_counter(coordinator->active_threads);

    queue_destroy(coordinator->dir_queue);

    free(coordinator);
}


/*-----------------------EXTERNAL FUCTIONS-----------------------*/


ThreadContext* create_thread_context(void) {
    ThreadContext* thread_context = safe_malloc(sizeof(ThreadContext), NULL);

    thread_context->coordinator = safe_calloc(2, sizeof(Coordinator*), NULL);
    thread_context->size = 2;
    thread_context->dir_num = 0;

    pthread_mutex_init(&thread_context->mutex_error, NULL);
    pthread_mutex_init(&thread_context->mutex_work, NULL);
    pthread_cond_init(&thread_context->cond_work, NULL);

    return thread_context;
}


void expand_and_create_coordinator(ThreadContext* thread_context) {
    Coordinator** coordinator = thread_context->coordinator;
    int size = thread_context->size;
    int dir_num = thread_context->dir_num;

    if (size <= dir_num) {
        coordinator = safe_realloc(coordinator, sizeof(Coordinator*) * (size + 2), 
                        thread_context);
        thread_context->size *= 2;
    }
    coordinator[dir_num - 1] = create_coordinator(thread_context);

    thread_context->coordinator = coordinator;
}


void increment_counter(Counter* counter) {
    pthread_mutex_lock(&counter->mutex);
    counter->counter++;
    pthread_mutex_unlock(&counter->mutex);
}


void decrement_counter(Counter* counter) {
    pthread_mutex_lock(&counter->mutex);
    if (counter->counter > 0) {
        counter->counter--;
    }
    pthread_mutex_unlock(&counter->mutex);
}


int get_counter_value(Counter* counter) {
    pthread_mutex_lock(&counter->mutex);
    int value = counter->counter;
    pthread_mutex_unlock(&counter->mutex);
    
    return value;
}


void update_total_sum(int value, Coordinator* coordinator) {
    pthread_mutex_lock(&coordinator->mutex_sum);
    coordinator->tot_sum += value;
    pthread_mutex_unlock(&coordinator->mutex_sum);
}


void thread_context_destroy(ThreadContext* thread_context) {
    int dir_num = thread_context->dir_num;

    pthread_mutex_destroy(&thread_context->mutex_error);
    pthread_mutex_destroy(&thread_context->mutex_work);
    pthread_cond_destroy(&thread_context->cond_work);

    for (int i = 0 ; i < dir_num ; i++) {
        destroy_coordinator(thread_context->coordinator[i]);
    }

    free(thread_context->coordinator);
    free(thread_context);
}

/**
 * }
*/