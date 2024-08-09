#ifndef FRONTEND_H
#define FRONTEND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ======== OPTIONS ======== */
// Enum for option flags
enum Option {
    IGNORE_CASE = 1 << 0,
    INVERT_MATCH = 1 << 1,
};

// Function to print help message
void print_help();

// Function to parse command line options and return a bitmask of options
int parse_opts(int argc, char **argv);

// Structure to hold file paths
typedef struct FileList {
    char **files;
    int count;
    int capacity;
} FileList;

/* ======== LIST FILES ======== */
// Global variable to hold the list of file paths
extern FileList file_list;

// Function to recursively list files in the current directory and subdirectories
void list_files_recursively(const char *base_path);

#endif // FRONTEND_H
