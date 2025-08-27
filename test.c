#include "statdata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const StatData case_1_in_a[2] = {
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode=3 },
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode=0 }
};
const StatData case_1_in_b[2] = {
    {.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode=2 },
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode=2}
};
const StatData case_1_out[3] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2 },
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3 },
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2 }
};

const StatData empty_data[0] = {};
const StatData single_data[1] = {{.id = 1, .count = 1, .cost = 1.0, .primary = 1, .mode = 1}};
const StatData duplicate_data[3] = {
    {.id = 1, .count = 1, .cost = 1.0, .primary = 1, .mode = 1},
    {.id = 1, .count = 2, .cost = 2.0, .primary = 0, .mode = 2},
    {.id = 1, .count = 3, .cost = 3.0, .primary = 1, .mode = 3}
};
const StatData duplicate_expected[1] = {
    {.id = 1, .count = 6, .cost = 6.0, .primary = 0, .mode = 3}
};

int compare_statdata(const StatData* a, const StatData* b) {
    return a->id == b->id && 
           a->count == b->count &&
           fabs(a->cost - b->cost) < 0.01f && 
           a->primary == b->primary &&
           a->mode == b->mode;
}

void print_statdata_debug(const StatData* d) {
    printf("id=%ld, count=%d, cost=%.5f, primary=%d, mode=%d\n",
           d->id, d->count, d->cost, d->primary, d->mode);
}

int test_case(const char* name, const StatData* a, size_t na, const StatData* b, size_t nb, 
              const StatData* expected, size_t ne, int need_sort) {
    if (!name) return 0;

    printf("Running test: %s\n", name);

    size_t nresult = 0;
    StatData* result = JoinDump(a, na, b, nb, &nresult);

    if (!result) {
        printf("Test %s failed: JoinDump returned NULL\n", name);
        return 0;
    }

    if (nresult != ne) {
        printf("Test %s failed: expected %zu records, got %zu\n", name, ne, nresult);
        free(result);
        return 0;
    }

    if (need_sort) {
        SortDump(result, nresult);
    }

    int passed = 1;
    for (size_t i = 0; i < ne; i++) {
        if (!compare_statdata(&result[i], &expected[i])) {
            printf("Test %s failed at record %zu:\n", name, i);
            printf("Expected: ");
            print_statdata_debug(&expected[i]);
            printf("Got:      ");
            print_statdata_debug(&result[i]);
            passed = 0;
        }
    }

    free(result);
    if (passed) {
        printf("Test %s passed\n", name);
    }

    return passed;
}

int test_store_load() {
    printf("Testing StoreDump/LoadDump...\n");

    const StatData test_data[2] = {
        {.id = 123, .count = 456, .cost = 7.89, .primary = 1, .mode = 5},
        {.id = 987, .count = 654, .cost = 3.21, .primary = 0, .mode = 2}
    };

    if (StoreDump("test_file.bin", test_data, 2) != 0) {
        printf("StoreDump failed\n");
        return 0;
    }

    size_t n = 0;
    StatData* loaded = LoadDump("test_file.bin", &n);
    if (!loaded) {
        printf("LoadDump failed\n");
        remove("test_file.bin");
        return 0;
    }

    if (n != 2) {
        printf("LoadDump returned wrong size: %zu\n", n);
        free(loaded);
        remove("test_file.bin");
        return 0;
    }

    int passed = 1;
    for (int i = 0; i < 2; i++) {
        if (!compare_statdata(&loaded[i], &test_data[i])) {
            printf("Data mismatch at index %d\n", i);
            passed = 0;
        }
    }

    free(loaded);
    remove("test_file.bin");

    if (passed) {
        printf("StoreDump/LoadDump test passed\n");
    }

    return passed;
}

int run_tests() {
    printf("Running all tests...\n");

    int passed = 0;
    int total = 0;

    total++;
    passed += test_case("Basic", case_1_in_a, 2, case_1_in_b, 2, case_1_out, 3, 1);

    total++;
    passed += test_case("Empty arrays", empty_data, 0, empty_data, 0, empty_data, 0, 0);

    total++;
    passed += test_case("Empty first", empty_data, 0, single_data, 1, single_data, 1, 0);

    total++;
    passed += test_case("Empty second", single_data, 1, empty_data, 0, single_data, 1, 0);

    total++;
    passed += test_case("Multiple duplicates", duplicate_data, 3, empty_data, 0, 
                       duplicate_expected, 1, 0);

    total++;
    passed += test_store_load();

    printf("\nTest results: %d/%d passed\n", passed, total);

    return passed == total ? 0 : 1;
}
