#include "backend.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static Input* current_input;
static FileList* current_files;
static Output* output;

static void print_to_output(char* input_buffer, Output* out) {
    pthread_mutex_lock(&out->mtx);

    size_t input_len = strlen(input_buffer);

    // Check if resizing is necessary
    if (out->size + input_len >= out->capacity) {
        size_t new_capacity = 2 * out->capacity;
        char* new_buffer = realloc(out->buffer, new_capacity * sizeof(char));
        if (new_buffer == NULL) {
            fprintf(stderr, "Failed to allocate memory for output buffer.\n");
            pthread_mutex_unlock(&out->mtx);
            return;
        }
        out->buffer = new_buffer;
        out->capacity = new_capacity;
    }

    char* dest = out->buffer + out->size;
    strcpy(dest, input_buffer);
    out->size += input_len;

    pthread_mutex_unlock(&out->mtx);
}

static void compute_prefix_function(const char* pattern, int* pi, size_t m) {
    size_t k = 0;
    pi[0] = 0;
    for (size_t q = 1; q < m; q++) {
        while (k > 0 && pattern[k] != pattern[q]) {
            k = pi[k - 1];
        }
        if (pattern[k] == pattern[q]) {
            k++;
        }
        pi[q] = k;
    }
}

static const char* kmp_search(const char* text, size_t n, const char* pattern, size_t m) {
    int* pi = (int*)malloc(m * sizeof(int));
    if (!pi) {
        fprintf(stderr, "Failed to allocate memory for KMP prefix function.\n");
        return NULL;
    }

    compute_prefix_function(pattern, pi, m);

    size_t q = 0; // number of characters matched
    for (size_t i = 0; i < n; i++) {
        while (q > 0 && pattern[q] != text[i]) {
            q = pi[q - 1];
        }
        if (pattern[q] == text[i]) {
            q++;
        }
        if (q == m) {
            free(pi);
            return text + i - m + 1;
        }
    }

    free(pi);
    return NULL; // no match
}
// Process function using KMP for pattern matching
static void process(const char* file_path, Input* in, Output* out) {
    // Open the file
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Failed to get file size");
        close(fd);
        return;
    }
    size_t file_size = st.st_size;

    // Map the file into memory
    char* data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to map file");
        close(fd);
        return;
    }

    size_t line_nbr = 0;
    int printed_fname = 0;
    char* line_start = data;
    char* line_end = NULL;
    char output_buffer[4096]; // Larger buffer to batch outputs
    size_t buffer_pos = 0;

    // Process the file line by line
    for (size_t i = 0; i < file_size; ++i) {
        if (data[i] == '\n' || i == file_size - 1) {
            // Determine the end of the line
            line_end = (data[i] == '\n') ? &data[i] : &data[i + 1];
            size_t line_len = line_end - line_start;

            // Use KMP to search for the pattern in the current line
            const char* match_pos = kmp_search(line_start, line_len, in->pattern, strlen(in->pattern));
            if (match_pos != NULL) {
                // Pre-format the line number
                char line_info[32];
                int line_info_len = snprintf(line_info, sizeof(line_info), "%lu: ", line_nbr);

                // Check if the buffer can hold the line info and the line
                if (buffer_pos + line_info_len + line_len + 2 >= sizeof(output_buffer)) {
                    // Flush the buffer
                    output_buffer[buffer_pos] = '\0';  // Null-terminate before printing
                    print_to_output(output_buffer, out);
                    buffer_pos = 0;
                }

                // Append the line info and line to the buffer
                if (!printed_fname) {
                    buffer_pos += snprintf(output_buffer + buffer_pos, sizeof(output_buffer) - buffer_pos, "%s\n", file_path);
                    printed_fname = 1;
                }

                memcpy(output_buffer + buffer_pos, line_info, line_info_len);
                buffer_pos += line_info_len;
                memcpy(output_buffer + buffer_pos, line_start, line_len);
                buffer_pos += line_len;
                output_buffer[buffer_pos++] = '\n';
            }

            // Move to the next line
            line_start = line_end + 1;
            line_nbr++;
        }
    }

    // Flush any remaining output in the buffer
    if (buffer_pos > 0) {
        output_buffer[buffer_pos] = '\0';  // Null-terminate the final part of the buffer
        print_to_output(output_buffer, out);
    }

    // Unmap the file and close the file descriptor
    munmap(data, file_size);
    close(fd);
}

// Initialization function
void init(Input* input, FileList* files) {
    current_input = input;
    current_files = files;

    output = (Output*)malloc(sizeof(Output));
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory for Output structure.\n");
        exit(EXIT_FAILURE);
    }

    size_t buffer_size = 2048;
    output->buffer = (char*)malloc(buffer_size * sizeof(char));
    if (output->buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for Output buffer.\n");
        free(output); // Clean up already allocated memory
        exit(EXIT_FAILURE);
    }
    output->capacity = buffer_size;
    output->size = 0;

    // Initialize the mutex
    if (pthread_mutex_init(&output->mtx, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex.\n");
        free(output->buffer);
        free(output);
        exit(EXIT_FAILURE);
    }
}

// TODO: un-globalize the file list and input
static void* thread_func(void* arg) {
    (void)arg; // If arg is not used, suppress unused parameter warning
    while (1) {

        // Get the next file (locks the file list)
        char* next_file = get_next_file(current_files);

        if (!next_file)
            break;

        process(next_file, current_input, output);
    }
    return NULL;
}

void start_threads() {
    const int num_threads = 12;
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
}

void print_output() {
    // TODO: definitely un-globalize this
    pthread_mutex_lock(&output->mtx);

    printf("%s\n", output->buffer);

    pthread_mutex_unlock(&output->mtx);
}

void cleanup() {
    free(output->buffer);
    free(output);
}
