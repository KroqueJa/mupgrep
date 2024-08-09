#include "frontend.h"

/* ======== OPTIONS ======== */
// Function to print help message
void print_help() {
    printf("Usage: mupgrep [options] pattern [file...]\n");
    printf("Options:\n");
    printf("  -i, --ignore-case   Ignore case distinctions\n");
    printf("  -v, --invert-match  Invert match (select non-matching lines)\n");
    printf("  -h, --help          Display this help and exit\n");
}

// Function to parse command line options and return a bitmask of options
Input parse_opts(int argc, char** argv) {
    Input input;
    input.options = 0;
    input.pattern = NULL;

    int option;
    int option_flags = 0; // Variable to hold the bitmask of options

    struct option long_options[] = {{"ignore-case", no_argument, 0, 'i'},
                                    {"invert-match", no_argument, 0, 'v'},
                                    {"help", no_argument, 0, 'h'},
                                    {0, 0, 0, 0}};

    // Parse command line options
    while ((option = getopt_long(argc, argv, "ivh", long_options, NULL)) !=
           -1) {
        switch (option) {
        case 'i':
            option_flags |= IGNORE_CASE; // Set IGNORE_CASE flag
            break;
        case 'v':
            option_flags |= INVERT_MATCH; // Set INVERT_MATCH flag
            break;
        case 'h':
            input.options = -1;
            return input; // Return -1 if help is requested
        default:
            input.options = -2;
            return input; // Return -2 on invalid option
        }
    }

    // Check for required pattern argument
    if (optind < argc) {
        input.pattern = argv[optind++];
        input.options = option_flags;

        return input;
    } else {
        input.options = -1;
        return input;
    }

}

/* ======== LIST FILES ======== */
// Global variable to hold the list of file paths
static FileList* current_file_list = NULL;

// Function to determine if a file is likely binary
#define CHUNK_SIZE 512 // Number of bytes to read for binary check
static int is_bin_file(const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    unsigned char buffer[CHUNK_SIZE];
    size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
    fclose(file);

    for (size_t i = 0; i < bytes_read; ++i) {
        if (!buffer[i]) {
            return 1; // Found a null byte, treat as binary
        }
    }

    return 0; // No null bytes found, treat as text
}

// Initialize a FileList
void init_file_list(FileList* list, int capacity) {
    list->count = 0;
    list->capacity = capacity;
    list->files = malloc(list->capacity * sizeof(char*));
    if (list->files == NULL) {
        perror("Failed to allocate memory for file list");
        exit(EXIT_FAILURE);
    }
}

// Add a file path to the FileList
static void add_file(FileList* list, const char* file_path) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->files = realloc(list->files, list->capacity * sizeof(char*));
        if (list->files == NULL) {
            perror("Failed to reallocate memory for file list");
            exit(EXIT_FAILURE);
        }
    }
    list->files[list->count++] = strdup(file_path);
}

// Free the memory used by the FileList
void free_file_list(FileList* list) {
    for (int i = 0; i < list->count; i++) {
        free(list->files[i]);
    }
    free(list->files);
}

// Callback function for nftw
static int callback(const char* file_path, const struct stat* sb,
                        int type_flag, struct FTW* ftwbuf) {
    (void)sb;     // Suppress unused parameter warning
    (void)ftwbuf; // Suppress unused parameter warning

    // TODO: add a flag to not ignore the .git dir
    if (type_flag == FTW_F && !is_bin_file(file_path) &&
        !(strstr(file_path, "/.git/") || strstr(file_path, "/.git"))) {
        add_file(current_file_list, file_path); // Add the file path to the list
    }
    return 0;
}

// Function to recursively list files in the current directory and
// subdirectories
void list_files_recursively(FileList* file_list, const char* base_path) {

    // Copy the provided pointer into the local file list
    current_file_list = file_list;

    // Use nftw to traverse the directory tree
    if (nftw(base_path, callback, 20, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
}
