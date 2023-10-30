/* 
 * @brief The module provides an interface and generic operations for the 
 * datatype queue, specifically designed to store strings.
 *
 * @author  Daniel Hylander
 * @date 2023-10-18
 */


#include "queue.h"
#include "safe_functions.h"

/*-----------------------INTERNAL FUCTIONS-----------------------*/

/*
 * @brief Copies a string 'in' to dynamically allocated memory and 
 * returns it.
 * 
 * @param in String to copy.
 * @return Returns the copied string.
 */
static char *clone_string(const char *in) {
    size_t len = strlen(in);
    char *out = safe_calloc(len + 1, sizeof(char), NULL);

    strncpy(out, in, len);

    return out;
}

/*
 * @brief Makes a new node.
 *
 * Given a value specified by `value`, this function will make a node with `value`
 * and return it. 
 *
 * @param value Pointer to a value.
 * @return Pointer to the newly made node.
 * 
 * @note It's the callers responsibility to deallocate the node with 
 * node_destroy.
 */
static struct node *make_node(const char *value, void* in_use_data)
{
    struct node *node = safe_malloc(sizeof(struct node), in_use_data);
    node->value = clone_string(value);

    return node;
}

static void node_destroy(struct node* node) {
    free(node->value);
    free(node);
}

/*-----------------------EXTERNAL FUCTIONS-----------------------*/

Queue *queue_create(void* in_use_data) {
    Queue *q = safe_malloc(sizeof(Queue), in_use_data);

    pthread_mutex_init(&q->mutex, NULL);

    q->first = NULL;
    q->last = NULL;

    return q;
}


void queue_destroy(Queue *q) {
    char* value;

    while (q->first != NULL) {
        value = queue_dequeue(q);
        free(value);
    }
    pthread_mutex_destroy(&q->mutex);
    free(q);
}


void queue_enqueue(Queue *q, const char *value, void* in_use_data) {
    pthread_mutex_lock(&q->mutex);

    struct node *node = make_node(value, in_use_data);


    struct node *last_in_queue = q->last;
    node->next = NULL;

    if (!(q->first == NULL)) {
        last_in_queue->next = node;
        q->last = node;

    } else {
        q->first = node;
        q->last = node;
    }

    pthread_mutex_unlock(&q->mutex);
}


char *queue_dequeue(Queue *q) {
    pthread_mutex_lock(&q->mutex);

    char* value = q->first->value;
    char* copy_value = clone_string(value);

    struct node* first = q->first;
    q->first = q->first->next;

    node_destroy(first);

    pthread_mutex_unlock(&q->mutex);
    
    return copy_value;
}


bool queue_is_empty(Queue *q) {
    pthread_mutex_lock(&q->mutex);
    bool result = q->first == NULL;
    pthread_mutex_unlock(&q->mutex);

    return result;
}