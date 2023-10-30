/**
 * @defgroup safe_functions Safe functions
 * 
 * @file safe_functions.h
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


#ifndef SAFE_FUNCTIONS_H
#define SAFE_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include "thread_context.h"

/**
 * @brief Safely allocates memory using malloc.
 * 
 * This function allocates memory of the specified size using the `malloc` function.
 * If the allocation fails, an error message is printed to the standard error stream,
 * and any data specified by the `in_use_data` parameter is destroyed to prevent memory leaks.
 * The function then exits the program with a failure status code.
 * 
 * @param size The size of the memory block to allocate.
 * @param in_use_data A pointer to data that should be destroyed if 
 *                    memory allocation fails.
 * 
 * @return A void pointer to the alloacted memory.
 * 
 * @note It is the caller's responsibility to handle the returned pointer and 
 * free the memory when no longer needed.
*/
void* safe_malloc(size_t size, void* in_use_data);

/**
 * @brief Safely allocates memory using calloc.
 * 
 * This function allocates memory of the specified size using the `calloc` function.
 * If the allocation fails, an error message is printed to the standard error stream,
 * and any data specified by the `in_use_data` parameter is destroyed to prevent memory leaks.
 * The function then exits the program with a failure status code.
 * 
 * @param __nmemb The number of elements to allocate.
 * @param size The size of the memory contained in the desired datatype to allocate.
 * @param in_use_data A pointer to data that should be destroyed if 
 * memory allocation fails.
 * 
 * @return A void pointer to the alloacted memory.
 * 
 * @note It is the caller's responsibility to handle the returned pointer and 
 * free the memory when no longer needed.
*/
void* safe_calloc(size_t __nmemb, size_t size, void* in_use_data);

/**
 * @brief Safely re-allocates memory using realloc.
 * 
 * This function re-allocates memory of an array to the specified size using the `realloc` function.
 * If the allocation fails, an error message is printed to the standard error stream,
 * and any data specified by the `in_use_data` parameter is destroyed to prevent memory leaks.
 * The function then exits the program with a failure status code.
 * 
 * @param __ptr Pointer to an array to re-allocate.
 * @param size The size of the memory block to allocate.
 * @param in_use_data A pointer to data that should be destroyed if 
 * memory allocation fails.
 * 
 * @return A void pointer to the re-alloacted memory.
 * 
 * @note It is the caller's responsibility to handle the returned pointer and 
 *       free the memory when no longer needed.
*/
void* safe_realloc(void *__ptr, size_t size, void *in_use_data);


/**
 * @brief Safely creates a stat struct using lstat.
 * 
 * This function creates a stat struct from the file specified by the 
 * file name `path_name` and stores it in `buff` using the function `lstat`. If the 
 * execution of stat fails, an error message is printed to the standard error 
 * stream. The function then exits he program with a failure status code.
 * 
 * @param path_name String of the file name.
 * @param buff The the struct to store the created stat struct in.
 * @param in_use_data A pointer to data that should be destroyed if 
 * memory allocation fails.
*/
void safe_lstat(char* path_name, struct stat* buff, void* in_use_data);


/**
 * @brief Safely opens a directory using opendir.
 * 
 * This function opens a directory specified by the directory name `dir_name` using the 
 * `opendir` function. If it fails to open the file, an error message is printed 
 * to the standard error stream. The function then returns NULL.
 * 
 * @param dir_name String of the directory name to be opened.
 * 
 * @return On success it returns the DIR pointer of the opened 
 * directory; else NULL.
*/
DIR* safe_opendir(const char* dir_name, void* in_use_data);

#endif

/**
 * }
*/