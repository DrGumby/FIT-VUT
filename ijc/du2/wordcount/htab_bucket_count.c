#include <stdio.h>
#include "libhtab.h"





size_t htab_bucket_count(htab_t * t)            //Returns count of all pointers in table (rows of table)
{
    if(t != NULL)
    {
        return t->arr_size;
    }
    else 
    {
        fprintf(stderr, "Table does not exist");
        return 0;
    }
}