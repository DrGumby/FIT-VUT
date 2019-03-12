#include <stdio.h>
#include <string.h>
#include "libhtab.h"





bool htab_remove(htab_t *t, const char * key)                   //Removes element form array
{
    unsigned index = htab_hash_function(key) % (int)htab_bucket_count(t);
    struct htab_listitem *cursor = t->heads[index];
    struct htab_listitem *temp_item;

    if(cursor != NULL && strcmp(cursor->key, key) == 0)
    {
        temp_item = cursor->next;
        free(cursor);
        t->heads[index] = temp_item;
        return true;
    }

    while (cursor != NULL)
    {
        if (cursor->next != NULL && strcmp(cursor->next->key, key) == 0)
            break;
        cursor = cursor->next;
    }
    if (cursor != NULL)
    {
        temp_item = cursor->next;
        cursor->next = temp_item->next;
        temp_item->next = NULL;
        free(temp_item);
        temp_item = NULL;
        return true;
    }

    return false;
}
