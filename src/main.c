#include "frontend.h"

int main(int argc, char **argv) {
    int options = parse_opts(argc, argv);

    if (options < 0) {
        // Error handling
        return 1;
    }

    // Check options using bitmask
    if (options & IGNORE_CASE) {
        printf("Ignore case option is set.\n");
    }
    if (options & INVERT_MATCH) {
        printf("Invert match option is set.\n");
    }

    // Initialize a file list
    FileList file_list;
    file_list.capacity = 10;

    init_file_list(&file_list);

    char cwd[PATH_MAX]; // Buffer to hold the current working directory

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    // List files recursively starting from the current working directory
    list_files_recursively(&file_list, cwd);

    free_file_list(&file_list);

    return 0;
}



