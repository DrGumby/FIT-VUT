#include <stdio.h>
#include "libhtab.h"


void htab_clear(htab_t * t)                 //Clears table to empty array
{
    struct htab_listitem *cursor = NULL;
    struct htab_listitem *temp_item = NULL;
    if(t != NULL)
    {
        for(unsigned i = 0; i < htab_bucket_count(t); i++)
        {
            cursor = t->heads[i];
            while(cursor != NULL)
            {
                temp_item = cursor;
                t->heads[i] = cursor->next;
                free(temp_item);
                cursor = t->heads[i];
            }
        }
    }
    else
    {
        fprintf(stderr, "Table is NULL");
        return;
    }
}