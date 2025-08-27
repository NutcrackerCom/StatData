#ifndef STATDATA_H
#define STATDATA_H

#include <stddef.h>

typedef struct StatData {
    long id;
    int count;
    float cost;
    unsigned int primary:1;
    unsigned int mode:3;
} StatData;

int StoreDump(const char* filename, const StatData* data, size_t n);
StatData* LoadDump(const char* filename, size_t* n);
StatData* JoinDump(const StatData* a, size_t na, const StatData* b, size_t nb, size_t* nresult);
void SortDump(StatData* data, size_t n);

#endif