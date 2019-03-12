#include <stdio.h>
#include <string.h>
#include "libhtab.h"


struct htab_listitem * htab_lookup_add(htab_t * t, const char * key)        //Finds element in table, if it does not exist, creates it
{
    unsigned index = htab_hash_function(key) % (int)htab_bucket_count(t);
    struct htab_listitem * item = t->heads[index];
    while(item != NULL) {
        if (strcmp(item->key, key) == 0)
        {
            item->data++;                               
            return item;
        }
        if(item->next == NULL)
            break;
        else
           item = item->next;
    }
    struct htab_listitem * newItem = (struct htab_listitem *)malloc(sizeof(struct htab_listitem));
    if(newItem == NULL)
        return NULL;
    newItem->data = 1;
    strcpy(newItem->key, key);
    newItem->next = NULL;
    if(item == NULL)
        t->heads[index] = newItem;
    else
        item->next = newItem;

    t->size++;
    return newItem;
}