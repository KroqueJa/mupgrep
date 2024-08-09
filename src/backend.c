#include "backend.h"

static void process(const char* file_path, Input* in) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, in->pattern) != NULL) {
            printf("%s\n%s", file_path, line);
        }
    }

    fclose(file);
}

void process_files(FileList* list, Input* in)
{
    char* next_file = get_next_file(list);

    while (next_file) {
        process(next_file, in);
        next_file = get_next_file(list);
    }
}
