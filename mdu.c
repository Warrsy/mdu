/*
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


#include "mdu.h"


/*-----------------------INTERNAL FUCTIONS-----------------------*/

/*
 * @brief Returns the length of the path from `file_path` and `directory_path`.
 * 
 * @param file_path String of a file path.
 * @param directory_path String of a directory path.
 * @return Returns the length of combined strings including the terminating 
 * character `\0`.
*/
static int get_path_length(const char* file_path, const char* directory_path) {
    return strlen(file_path) + strlen(directory_path) + 2;
}

/*
 * @brief Checks if a path is the `.` or `..` directory's.
 * 
 * @param path String of a path.
 * @return Returns true if the path is either the `.` or `..` directory; 
 * else false.
*/
static bool is_dot_or_dot_dot(char* path) {
    return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

/*
 * @brief Checks if a file is a directory.
 * 
 * @param file_info A stat strcut containing the information on a file.
 * @return Returns true if the file is a directory; else false.
*/
static bool is_dictionary(struct stat file_info) {
    return (file_info.st_mode & S_IFMT) == S_IFDIR;
}

/*
 * @brief Checks if the index in `argv` is the flag or its argument.
 * input arguments.
 * 
 * @param i Index for argv
 * @return Returns true if it is the flag argument; else false.
*/
static bool is_flag_arg(int i) {
    return i == (optind - 2) || i == optind - 1;
}

/*
 * @brief Checks that the the user entered atleast one argument.
 *
 * If no argument was given, the function will print out a message to stderr 
 * and exit the program.
 * 
 * @param argc The amount of arguments entered.
*/
static void check_arguments(int argc) {
    if (argc < 2) {
        fprintf(stderr, "mdu -j {antal trådar} {fil} [filer ...]\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * @brief Processes an argument from `argv`.
 * 
 * If the argument is a directory, it will create a new coordinator for the 
 * directory. In the new coordinator it will add the directory to its queue 
 * and update the total size of the directory.
 * 
 * @param index The Index for the argument in argv.
 * @param arg The argument to process.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
static void process_argument(int index , char* arg, 
                                ThreadContext* thread_context) {
    struct stat file_info;

    if (!is_flag_arg(index)) {
        safe_lstat(arg, &file_info, thread_context);

        if (is_dictionary(file_info)) {
            thread_context->dir_num++;
            int dir_num = thread_context->dir_num;

            expand_and_create_coordinator(thread_context);
            queue_enqueue(thread_context->coordinator[dir_num - 1]->dir_queue, 
                                arg, thread_context);

            update_total_sum(file_info.st_blocks, 
                                thread_context->coordinator[dir_num - 1]);
        }
    }
}

/*
 * @brief Check if there are any threads active or available work in a 
 * coordinator.
 * 
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
 * @return Returns true if there are threads active, or there is work available 
 * in a coordinator.
*/
static bool are_threads_active(ThreadContext *thread_context) {
    for (int i = 0 ; i < thread_context->dir_num ; i++) {
        Coordinator* coordinator = thread_context->coordinator[i];
        int threads_num = get_counter_value(coordinator->active_threads);

        if (threads_num > 0 || !queue_is_empty(coordinator->dir_queue)) {
            return true;
        }
    }

    return false;
}

/*
 * @brief Finds a coordinator in a thread context struct that has an 
 * non-empty queue.
 * 
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
 * @param dir_path A pointer to a string containing the directory path.
 * @return Returns a non-empty coordinator if found; else `NULL` is returned.
*/
static Coordinator* get_non_empty_coordinator(ThreadContext *thread_context, 
                                                char** dir_path) {
    for (int i = 0 ; i < thread_context->dir_num ; i++) {
        Coordinator* coordinator = thread_context->coordinator[i];

        if (!queue_is_empty(coordinator->dir_queue)) {
            *dir_path = queue_dequeue(coordinator->dir_queue);
            increment_counter(coordinator->active_threads);

            return coordinator;
        }
    }

    return NULL;
}

/*
 * @brief Creates the full path to a file.
 * 
 * @param dir_name String of a directory path.
 * @param file_name String of a file path.
 * @return Returns the full path of the file.
*/
static char* create_full_path(const char* dir_name, const char *file_name, 
                                ThreadContext* thread_context) {
    int path_length = get_path_length(file_name, dir_name);
    char* full_path = safe_calloc(path_length, sizeof(char), thread_context);
    snprintf(full_path, path_length, "%s/%s", dir_name, file_name);

    return full_path;
}

/*
 * @brief Processes a directory entry.
 * 
 * If the entry is a directory, add the directory to its queue and add the 
 * size of the directory to `sum`; else it will only add the files size to `sum`.
 * 
 * @param full_path String of the files full path.
 * @param sum The current sum of the traversed directory.
 * @param coordinator A pointer to the coordinator for the directory.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
static void process_directory_entry(char* full_path, int *sum, 
                                Coordinator* coordinator, ThreadContext *thread_context) {
    struct stat file_info;
    safe_lstat(full_path ,&file_info, thread_context);
    
    if (is_dictionary(file_info)) {
        queue_enqueue(coordinator->dir_queue, full_path, thread_context);
    }

    *sum += file_info.st_blocks;


}

/*
 * @brief Processes all entry's in a directory.
 * 
 * Adds each entry's files size into sum.
 * 
 * @param dir_name String of the directory's path.
 * @param directory The DIR object of the directory.
 * @param sum The current sum of the traversed directory.
 * @param coordinator A pointer to the coordinator for the directory.
 * @param thread_context A pointer to the thread context struct containing the 
 * coordinators.
*/
static void process_directory_entries(char* dir_name, DIR* directory, int* sum, 
                            Coordinator* coordinator, ThreadContext* thread_context) {
    struct dirent* file;

    while((file = readdir(directory)) != NULL) {

        if (!is_dot_or_dot_dot(file->d_name)) {
            char* full_path = create_full_path(dir_name, file->d_name, thread_context);
            process_directory_entry(full_path, sum, coordinator, thread_context);

            free(full_path);
        }
    }
}


/*-----------------------EXTERNAL FUCTIONS-----------------------*/


int get_thread_amount(int argc, char* argv[]) {
    int opt;

    while((opt = getopt(argc, argv, "j:")) != -1) {
        switch (opt) {
            case 'j':
                if (*optarg < 1) {
                    return 0;

                } else {
                    return atoi(optarg) - 1;
                }
            case '?':
                break;
        }
    }

    return 0;
}


void traverse_input_arguments(int argc ,char* argv[], ThreadContext* thread_context) {
    for (int i = 1 ; i < argc ; i++) {
        process_argument(i, argv[i], thread_context);
    }
}


void* thread_handler(void* arg) {
    ThreadContext* thread_context = (ThreadContext*) arg;

    char* dir_path = NULL;
    Coordinator* coordinator;

    while((coordinator = get_coordinator_with_work(thread_context, &dir_path)) != NULL) {
        traverse_directory(dir_path, coordinator, thread_context);

        pthread_mutex_lock(&thread_context->mutex_work);
        decrement_counter(coordinator->active_threads);
        pthread_cond_broadcast(&thread_context->cond_work);
        pthread_mutex_unlock(&thread_context->mutex_work);
    }

    int *exit_status = safe_malloc(sizeof(int), thread_context);

    if (errno != 0) {
        *exit_status = 1;
        return exit_status;
    }

    *exit_status = 0;
    return exit_status;
}


Coordinator* get_coordinator_with_work(ThreadContext* thread_context, char** dir_path) {
    pthread_mutex_lock(&thread_context->mutex_work);

    while(are_threads_active(thread_context)) {
       Coordinator* coordinator;
       coordinator = get_non_empty_coordinator(thread_context, dir_path);

        if (coordinator == NULL) {
            pthread_cond_wait(&thread_context->cond_work, &thread_context->mutex_work);

        } else {
            pthread_mutex_unlock(&thread_context->mutex_work);
            return coordinator;
       }
    }

    pthread_mutex_unlock(&thread_context->mutex_work);

    return NULL;
}


void traverse_directory(char* dir_name, Coordinator* coordinator, ThreadContext* thread_context) {
    DIR* directory = safe_opendir(dir_name, thread_context);
    int sum = 0;
    
    if (directory != NULL) {
        process_directory_entries(dir_name ,directory, &sum, coordinator, thread_context);

        update_total_sum(sum, coordinator);
        closedir(directory);
    }

    free(dir_name);
}


void print_results(int argc, char* argv[], ThreadContext* thread_context) {
    int file_size;
    int dir_num = 0;
    struct stat file_info;

    for (int i = 1 ; i < argc ; i++) {
        if (!is_flag_arg(i)) {
            safe_lstat(argv[i], &file_info, thread_context);

            if (is_dictionary(file_info)) {
                Coordinator* coordinator = thread_context->coordinator[dir_num];
                dir_num++;

                file_size = coordinator->tot_sum;

            } else {
                file_size = file_info.st_blocks;
            }

        char* file_path = argv[i];
        printf("%d	%s\n", file_size, file_path);
        }
    }
}


void collect_thread_exit_statuses(pthread_t *threads, int thread_num, int* exit_status) {
    void* value;

    for (int i = 0 ; i < thread_num ; i++) {
        pthread_join(threads[i], &value);

        if (*(int*) value == EXIT_FAILURE) {
            *exit_status = EXIT_FAILURE;
        }

        free(value);
    }
}


int main(int argc, char* argv[]) {
    check_arguments(argc);

    int thread_num = get_thread_amount(argc, argv);
    pthread_t threads[thread_num];

    ThreadContext* thread_context = create_thread_context();
    traverse_input_arguments(argc, argv, thread_context);
    
    for (int i = 0 ; i < thread_num ; i++) {
        pthread_create(&threads[i], NULL, &thread_handler, thread_context);
    }

    void* arg = thread_handler(thread_context);

    int exit_status = *(int*) arg;
    free(arg);
    collect_thread_exit_statuses(threads, thread_num, &exit_status);

    print_results(argc, argv, thread_context);

    thread_context_destroy(thread_context);

    if (exit_status != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}