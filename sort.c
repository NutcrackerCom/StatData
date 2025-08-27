#include "statdata.h"
#include <stdlib.h>

static int compare(const void* a, const void* b) {
    const StatData* da = (const StatData*)a;
    const StatData* db = (const StatData*)b;
    if (da->cost < db->cost) return -1;
    if (da->cost > db->cost) return 1;
    return 0;
}

void SortDump(StatData* data, size_t n) {
    qsort(data, n, sizeof(StatData), compare);
}