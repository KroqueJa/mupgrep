#ifndef BACKEND_H
#define BACKEND_H

#include "frontend.h"

typedef struct Output {
    char* buffer;
    size_t capacity;
    size_t size;
    pthread_mutex_t mtx;
} Output;

void init(Input* input, FileList* files);
void cleanup();

void start_threads();
void join_threads();

void print_output();

#endif
