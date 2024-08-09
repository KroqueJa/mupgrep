#include "backend.h"
#include "frontend.h"

int main(int argc, char** argv) {
    Input in = parse_opts(argc, argv);

    // Check options using bitmask
    if (in.options == -1) {
        print_help();
        return 0;
    } else if (in.options == -2) {
        printf("ERROR - could not parse command, exiting\n");
        print_help();
        return 1;
    }

    // Initialize a file list
    FileList file_list;

    init_file_list(&file_list, 10);

    char cwd[PATH_MAX]; // Buffer to hold the current working directory

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    // Add the files to the file list
    list_files_recursively(&file_list, cwd);

    // Initialize the backend
    init(&in, &file_list);
    start_threads();
    print_output();

    free_file_list(&file_list);
    cleanup();

    return 0;
}
