#include "statdata.h"
#include <stdio.h>
#include <stdlib.h>

int StoreDump(const char* filename, const StatData* data, size_t n) {
    if (!filename || !data || n == 0) return -1; 

    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return -1;
    }

    size_t written = fwrite(data, sizeof(StatData), n, file);
    int success = (written == n) ? 0 : -1;
    fclose(file);
    return success;
}

StatData* LoadDump(const char* filename, size_t* n) {
    if (!filename || !n) {
        fprintf(stderr, "Invalid arguments to LoadDump\n");
        return NULL;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for reading");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek to end of file");
        fclose(file);
        return NULL;
    }

    long size = ftell(file);
    if (size == -1L) {
        perror("Failed to get file size");
        fclose(file);
        return NULL;
    }

    rewind(file);

    if (size % sizeof(StatData) != 0) {
        fprintf(stderr, "File size is not a multiple of StatData size\n");
        fclose(file);
        return NULL;
    }

    *n = size / sizeof(StatData);
    StatData* data = malloc(*n * sizeof(StatData));
    if (!data) {
        fprintf(stderr, "Memory allocation failed for LoadDump\n");
        fclose(file);
        return NULL;
    }

    size_t read = fread(data, sizeof(StatData), *n, file);
    
    if (fclose(file) != 0) {
        perror("Failed to close file");
        free(data);
        return NULL;
    }

    if (read != *n) {
        fprintf(stderr, "Read %zu elements, expected %zu\n", read, *n);
        free(data);
        return NULL;
    }

    return data;
}