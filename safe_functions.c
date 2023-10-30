/*
 * @brief Safe Error-Handling Functions for Resource Management.
 * 
 * This module provides a collection of safe error-handling functions that 
 * can be used to manage resources such as memory allocation, file operations, 
 * and process management. These functions are designed to handle errors, 
 * print informative error messages to the standard error stream, and exit 
 * the program with a failure status code when errors occur.
 *
 * @author  Daniel Hylander
 * @date 2023-10-18
 * @{
 */

#include "safe_functions.h"

void* safe_malloc(size_t size, void* in_use_data) {
    char *ptr;

    if ((ptr = malloc(size)) == NULL) {
        fprintf(stderr, "malloc() failed to allocate memory\n");
        thread_context_destroy(in_use_data);
        exit(EXIT_FAILURE);
    }

    return ptr;
}


void* safe_calloc(size_t __nmemb, size_t size, void* in_use_data) {
    char *ptr;
    
    if ((ptr = calloc(__nmemb, size)) == NULL) {
        fprintf(stderr, "calloc() failed to allocate memory\n");
        thread_context_destroy(in_use_data);
        exit(EXIT_FAILURE);
    }

    return ptr;
}


void* safe_realloc(void *__ptr, size_t size, void *in_use_data) {
    void *ptr;
    
    if ((ptr = realloc(__ptr, size)) == NULL) {
        fprintf(stderr, "realloc() failed to allocate memory\n");
        thread_context_destroy(in_use_data);
        exit(EXIT_FAILURE);
    }

    return ptr;
}


void safe_lstat(char* path_name, struct stat* buff, void *in_use_data) {
    if (lstat(path_name, buff) == -1) {
        perror("lstat");
        thread_context_destroy(in_use_data);
        exit(EXIT_FAILURE);
    }
}


DIR* safe_opendir(const char* dir_name, void* in_use_data) {
    ThreadContext* thread_context = (ThreadContext*) in_use_data;

    pthread_mutex_lock(&thread_context->mutex_error);
    DIR* dir;

    if ((dir = opendir(dir_name)) == NULL) {
        fprintf(stderr, "du: cannot read directory '%s': %s\n", 
            dir_name, strerror(errno));
        pthread_mutex_unlock(&thread_context->mutex_error);

        return NULL;
    }

    pthread_mutex_unlock(&thread_context->mutex_error);
    return dir;
}