#include "statdata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_statdata(const StatData* d) {
    printf("%lx\t%d\t%.3e\t%s\t", 
           d->id, d->count, d->cost, 
           d->primary ? "y" : "n");
    for (int i = 2; i >= 0; i--) {
        printf("%d", (d->mode >> i) & 1);
    }
    printf("\n");
}

void print_usage() {
    printf("Usage:\n");
    printf("  main <file1> <file2> <output>  - Process files\n");
    printf("  main -test                     - Run tests\n");
}

int main(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "-test") == 0) {
        extern int run_tests();
        return run_tests();
    }

    if (argc != 4) {
        print_usage();
        return 1;
    }

    size_t n1, n2;
    StatData* data1 = LoadDump(argv[1], &n1);
    StatData* data2 = LoadDump(argv[2], &n2);

    if (!data1 || !data2) {
        fprintf(stderr, "Failed to load input files\n");
        if (data1) free(data1);
        if (data2) free(data2);
        return 1;
    }

    size_t nresult;
    StatData* result = JoinDump(data1, n1, data2, n2, &nresult);
    free(data1); free(data2);

    if (!result) {
        fprintf(stderr, "Join failed\n");
        return 1;
    }

    SortDump(result, nresult);

    printf("First 10 records:\n");
    printf("ID\tCount\tCost\tPrimary\tMode\n");
    for (size_t i = 0; i < 10 && i < nresult; i++) {
        print_statdata(&result[i]);
    }

    if (StoreDump(argv[3], result, nresult) != 0) {
        fprintf(stderr, "Failed to save result\n");
        free(result);
        return 1;
    }

    free(result);
    return 0;
}
