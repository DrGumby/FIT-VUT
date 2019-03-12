#include <stdio.h>
#include <string.h>
#include "libhtab.h"




struct htab_listitem * htab_find(htab_t *t, const char *key)            //Finds element in table
{
    unsigned index = htab_hash_function(key) % (int)htab_bucket_count(t);
    struct htab_listitem *cursor = t->heads[index];
    while(cursor!=NULL)
    {
        if(strcmp(cursor->key,key) == 0)
        {
            cursor->data++;                             //TODO Check if data should be incremented
            return cursor;
        }
        cursor = cursor->next;
    }
    return NULL;
}
