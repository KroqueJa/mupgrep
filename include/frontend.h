#ifndef FRONTEND_H
#define FRONTEND_H

#include <ftw.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* ======== OPTIONS ======== */
// Enum for option flags
enum Option {
    IGNORE_CASE = 1 << 0,
    INVERT_MATCH = 1 << 1,
};

typedef struct Input {
    char* pattern;
    int options;
} Input;

// Function to print help message
void print_help();

// Function to parse command line options and return a bitmask of options
Input parse_opts(int argc, char** argv);

// Structure to hold file paths
typedef struct FileList {
    char** files;
    int count;
    int capacity;
    int next_idx;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} FileList;

/* ======== LIST FILES ======== */
// Function to recursively list files in the current directory and
// subdirectories
void list_files_recursively(FileList* file_list, const char* base_path);

// Initialize a file list
void init_file_list(FileList* list, int capacity);

// Free a file list
void free_file_list(FileList* list);

// Get the next file to process
char* get_next_file(FileList* list);

#endif // FRONTEND_H
