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

/* ======== OPTIONS ======== */
// Enum for option flags
enum Option {
    IGNORE_CASE = 1 << 0,
    INVERT_MATCH = 1 << 1,
};

// Function to print help message
void print_help();

// Function to parse command line options and return a bitmask of options
int parse_opts(int argc, char** argv);

// Structure to hold file paths
typedef struct FileList {
    char** files;
    int count;
    int capacity;
} FileList;

/* ======== LIST FILES ======== */
// Function to recursively list files in the current directory and
// subdirectories
void list_files_recursively(FileList* file_list, const char* base_path);

// Initialize a file list
void init_file_list(FileList* list);

// Free a file list
void free_file_list(FileList* list);

#endif // FRONTEND_H
