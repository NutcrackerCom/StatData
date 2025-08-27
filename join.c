#include "statdata.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN_HASH_SIZE 16
#define MAX_HASH_SIZE 1000000
#define LOAD_FACTOR 0.6

typedef struct HashNode {
    long key;
    StatData value;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode** buckets;
    size_t size;
    size_t count;
} HashTable;

static unsigned long hash(long key, size_t table_size) {
    unsigned long h = 2166136261UL;
    unsigned char *p = (unsigned char *)&key;
    
    for (size_t i = 0; i < sizeof(key); i++) {
        h = (h ^ p[i]) * 16777619UL;
    }
    
    return h % table_size;
}

static void combine(StatData* a, const StatData* b) {
    a->count += b->count;
    a->cost += b->cost;
    if (b->primary == 0) a->primary = 0;
    if (b->mode > a->mode) a->mode = b->mode;
}

static int hash_table_init(HashTable* ht, size_t initial_size) {
    if (!ht) return -1;
    
    // Ограничиваем размер таблицы разумными пределами
    if (initial_size < MIN_HASH_SIZE) initial_size = MIN_HASH_SIZE;
    if (initial_size > MAX_HASH_SIZE) initial_size = MAX_HASH_SIZE;
    
    ht->buckets = calloc(initial_size, sizeof(HashNode*));
    if (!ht->buckets) {
        fprintf(stderr, "Memory allocation failed in hash_table_init\n");
        return -1;
    }
    
    ht->size = initial_size;
    ht->count = 0;
    return 0;
}

static int hash_table_resize(HashTable* ht) {
    size_t new_size = ht->size * 2;
    if (new_size > MAX_HASH_SIZE) new_size = MAX_HASH_SIZE;
    if (new_size <= ht->size) return 0; // Не можем увеличить дальше
    
    HashNode** new_buckets = calloc(new_size, sizeof(HashNode*));
    if (!new_buckets) {
        fprintf(stderr, "Memory allocation failed in hash_table_resize\n");
        return -1;
    }

    // Перехешируем все элементы
    for (size_t i = 0; i < ht->size; i++) {
        HashNode* node = ht->buckets[i];
        while (node) {
            HashNode* next = node->next;
            unsigned long new_index = hash(node->key, new_size);
            
            // Вставляем в новую таблицу
            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            
            node = next;
        }
    }

    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->size = new_size;
    return 0;
}

static int hash_table_insert(HashTable* ht, long key, StatData value) {
    if (!ht) return -1;
    
    // Проверяем необходимость ресайза
    if ((float)ht->count / ht->size > LOAD_FACTOR) {
        if (hash_table_resize(ht) != 0) {
            return -1;
        }
    }
    
    unsigned long index = hash(key, ht->size);
    HashNode* node = ht->buckets[index];

    // Проверяем существующий ключ
    while (node) {
        if (node->key == key) {
            combine(&node->value, &value);
            return 0;
        }
        node = node->next;
    }

    // Создаем новый узел
    HashNode* new_node = malloc(sizeof(HashNode));
    if (!new_node) {
        fprintf(stderr, "Memory allocation failed in hash_table_insert\n");
        return -1;
    }
    
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    ht->count++;
    
    return 0;
}

static int hash_table_collect(HashTable* ht, StatData** result, size_t* size) {
    if (!ht || !result || !size) return -1;

    *size = ht->count;
    if (*size == 0) {
        *result = NULL;
        return 0;
    }

    *result = malloc(*size * sizeof(StatData));
    if (!*result) {
        fprintf(stderr, "Memory allocation failed in hash_table_collect\n");
        return -1;
    }

    size_t j = 0;
    for (size_t i = 0; i < ht->size; i++) {
        HashNode* node = ht->buckets[i];
        while (node) {
            (*result)[j++] = node->value;
            node = node->next;
        }
    }
    
    return 0;
}

static void hash_table_free(HashTable* ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->size; i++) {
        HashNode* node = ht->buckets[i];
        while (node) {
            HashNode* next = node->next;
            free(node);
            node = next;
        }
    }
    
    free(ht->buckets);
    ht->buckets = NULL;
    ht->size = 0;
    ht->count = 0;
}

StatData* JoinDump(const StatData* a, size_t na, const StatData* b, size_t nb, size_t* nresult) {
    if (!nresult) {
        fprintf(stderr, "JoinDump: nresult pointer is NULL\n");
        return NULL;
    }
    
    if ((na > 0 && !a) || (nb > 0 && !b)) {
        fprintf(stderr, "JoinDump: invalid input arrays\n");
        return NULL;
    }

    if (na == 0 && nb == 0) {
        *nresult = 0;
        StatData* result = malloc(1); 
        if (!result) {
            fprintf(stderr, "Memory allocation failed in JoinDump for empty arrays\n");
            return NULL;
        }
        return result;
    }

    size_t total_elements = na + nb;
    size_t initial_size = MIN_HASH_SIZE;
    
    while (initial_size < total_elements / LOAD_FACTOR && initial_size < MAX_HASH_SIZE / 2) {
        initial_size *= 2;
    }
    
    HashTable ht;
    if (hash_table_init(&ht, initial_size) != 0) {
        return NULL;
    }

    for (size_t i = 0; i < na; i++) {
        if (hash_table_insert(&ht, a[i].id, a[i]) != 0) {
            hash_table_free(&ht);
            return NULL;
        }
    }
    
    for (size_t i = 0; i < nb; i++) {
        if (hash_table_insert(&ht, b[i].id, b[i]) != 0) {
            hash_table_free(&ht);
            return NULL;
        }
    }

    StatData* result = NULL;
    if (hash_table_collect(&ht, &result, nresult) != 0) {
        hash_table_free(&ht);
        return NULL;
    }
    
    hash_table_free(&ht);
    return result;
}