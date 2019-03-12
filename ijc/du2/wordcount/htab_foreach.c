#include <stdio.h>
#include "libhtab.h"



void htab_foreach(htab_t * t, int (*func)(const char * constkey, int * valueptr) )      //Performs func for each element of array
{
    struct htab_listitem *cursor;
    if(t != NULL)
    {
        for(unsigned i = 0; i < htab_bucket_count(t); i++)
        {
            cursor = t->heads[i];
            while(cursor != NULL)
            {
                func(cursor->key, &cursor->data);
                cursor = cursor->next;
            }
        }
    }
    else
    {
        fprintf(stderr, "Table is NULL");
        return;
    }
}