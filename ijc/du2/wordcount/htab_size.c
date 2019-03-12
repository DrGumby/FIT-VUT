#include <stdio.h>
#include "libhtab.h"



size_t htab_size(htab_t * t)                        //Returns size of table
{
    if(t != NULL)
    {
        return t->size;
    }
    else
    {
        fprintf(stderr, "Table is NULL");
        return 0;
    }
}