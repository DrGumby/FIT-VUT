//
// Created by kamil on 26.3.18.
//


#include <stdlib.h>
#include <stdbool.h>

struct htab_listitem{
    char  key[127];
    int data;
    struct htab_listitem * next;
};

typedef struct htab{
    size_t size;
    size_t arr_size;
    struct htab_listitem * heads[];
}htab_t;

unsigned int htab_hash_function(const char *str);

htab_t * htab_init(size_t size);

htab_t * htab_move(size_t newsize, htab_t * t2);

size_t htab_size(htab_t * t);

size_t htab_bucket_count(htab_t * t);

struct htab_listitem * htab_lookup_add(htab_t * t, const char * key);

struct htab_listitem * htab_find(htab_t *t, const char *key);

void htab_foreach(htab_t * t, int (*func)(const char * constkey, int * valueptr) );

bool htab_remove(htab_t *t, const char * key);

void htab_clear(htab_t * t);

void htab_free(htab_t *t);

