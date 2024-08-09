#include "backend.h"

static void process(const char* file_path) {
    printf("%s\n", file_path);
}

void process_files(FileList* list)
{
    for (int i = 0; i < list->count; ++i) {
        process(list->files[i]);
    }
}
