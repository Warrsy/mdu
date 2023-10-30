/**
 * @file mdu.h
 * @brief The program estimates and reports the disk usage by a certain 
 * directory tree or file.
 * 
 * All figures are reported with a block size of 512 bytes. It works 
 * the same as the command `du -s -l -B512 {fil} [filer ...]`.
 *
 * @author Daniel Hylander
 * @date 2023-10-18
 * @{
 */

#ifndef MDU_H
#define MDU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>

#include "queue.h"
#include "safe_functions.h"
#include "thread_context.h"

#define BLOCK_SIZE 512

/**
 * @brief Returns the users requested thread amount.
 * 
 * @param argc Number of input arguments.
 * @param argv Input arguments.
 * @return Returns the users requested thread amount.
*/
int get_thread_amount(int argc, char* argv[]);

/**
 * @brief Handles the logic for what work each thread will preform.
 * 
 * @param arg A thread context struct.
 * @return Returns the threads exit status.
*/
void* thread_handler(void* arg);

/**
 * @brief Traverses and processes all arguments from `argv`.
 * 
 * If the argument is a directory, it will create a new coordinator for the 
 * directory. In the new coordinator it will add the directory to its queue 
 * and update the total size of the directory.
 * 
 * @param argc The amount of input arguments.
 * @param argv The input arguments.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
void traverse_input_arguments(int argc ,char* argv[], 
                                ThreadContext* thread_context);

/**
 * @brief Traverses and processes all entry's in a directory.
 * 
 * Adds each entry's files size into the coordinators `tot_sum`.
 * 
 * @param dir_name String of the directory's path.
 * @param coordinator A pointer to the coordinator for the directory.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
void traverse_directory(char* dir_name, Coordinator* coordinator, 
                            ThreadContext* thread_context);

/**
 * @brief Finds a coordinator struct that has work available.
 * 
 * Tries to find a coordinator with a non-empty queue, if non exist 
 * it will wait till a thread is done traversing a directory. If no thread is 
 * active it will return `NULL`.
 * 
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
 * @param dir_path String of the directory's path.
 * @return Returns a coordinator with work available; else if none exists and
 * no thread is active NULL is returned.
*/
Coordinator* get_coordinator_with_work(ThreadContext* thread_context, 
                                        char** dir_path);

/**
 * @brief Prints the disk usage of each file file entered as an argument.
 * 
 * @param argc The amount of input arguments.
 * @param argv The input arguments.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
void print_results(int argc, char* argv[], ThreadContext* thread_context);

/**
 * @brief Checks if any thread exited with the status `EXIT_FAILURE`.
 * 
 * If any thread exited with the status `EXIT_FAILURE`, it is saved in 
 * `exit_status`.
 * 
 * @param threads Array of threads.
 * @param thread_num Number of threads in the array.
 * @param exit_status Pointer to an int.
*/
void collect_thread_exit_statuses(pthread_t *threads, int thread_num, 
                                    int* exit_status);

#endif

/**
 * }
*/