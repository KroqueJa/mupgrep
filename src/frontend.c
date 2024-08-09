#include "frontend.h"

// Global variable to hold the list of file paths
FileList file_list;

// Function to print help message
void print_help() {
    printf("Usage: mupgrep [options] pattern [file...]\n");
    printf("Options:\n");
    printf("  -i, --ignore-case   Ignore case distinctions\n");
    printf("  -v, --invert-match  Invert match (select non-matching lines)\n");
    printf("  -h, --help          Display this help and exit\n");
}

// Function to parse command line options and return a bitmask of options
int parse_opts(int argc, char **argv) {
    int option;
    int option_flags = 0; // Variable to hold the bitmask of options

    struct option long_options[] = {
        {"ignore-case", no_argument, 0, 'i'},
        {"invert-match", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command line options
    while ((option = getopt_long(argc, argv, "ivh", long_options, NULL)) != -1) {
        switch (option) {
            case 'i':
                option_flags |= IGNORE_CASE; // Set IGNORE_CASE flag
                break;
            case 'v':
                option_flags |= INVERT_MATCH; // Set INVERT_MATCH flag
                break;
            case 'h':
                print_help();
                return 0; // Return 0 if help is requested
            default:
                print_help();
                return -1; // Return -1 on invalid option
        }
    }

    // Check for required pattern argument
    if (optind < argc) {
        char *pattern = argv[optind++];
        printf("Pattern: %s\n", pattern);
        for (int i = optind; i < argc; i++) {
            printf("File: %s\n", argv[i]);
        }
    } else {
        fprintf(stderr, "Error: No pattern provided\n");
        return -1;
    }

    return option_flags;
}

// Initialize a FileList
void init_file_list(FileList *list) {
    list->count = 0;
    list->capacity = 10;
    list->files = malloc(list->capacity * sizeof(char *));
    if (list->files == NULL) {
        perror("Failed to allocate memory for file list");
        exit(EXIT_FAILURE);
    }
}

// Add a file path to the FileList
void add_file(FileList *list, const char *file_path) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->files = realloc(list->files, list->capacity * sizeof(char *));
        if (list->files == NULL) {
            perror("Failed to reallocate memory for file list");
            exit(EXIT_FAILURE);
        }
    }
    list->files[list->count++] = strdup(file_path);
}

// Free the memory used by the FileList
void free_file_list(FileList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->files[i]);
    }
    free(list->files);
}

// Callback function for nftw
int process_file(const char *file_path, const struct stat *sb, int type_flag, struct FTW *ftwbuf) {
    // Check if it's a regular file
    if (type_flag == FTW_F) {
        add_file(&file_list, file_path); // Add the file path to the list
    }
    return 0; // Return 0 to continue
}

// Function to recursively list files in the current directory and subdirectories
void list_files_recursively(const char *base_path) {
    init_file_list(&file_list); // Initialize the file list

    // Use nftw to traverse the directory tree
    if (nftw(base_path, process_file, 20, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
}
