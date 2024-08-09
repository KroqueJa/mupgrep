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
    char cwd[PATH_MAX]; // Buffer to hold the current working directory

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    // List files recursively starting from the current working directory
    list_files_recursively(cwd);

    // Output the list of files
    for (int i = 0; i < file_list.count; i++) {
        printf("%s\n", file_list.files[i]);
    }

    return 0;
}



