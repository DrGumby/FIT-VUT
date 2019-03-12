#include <stdio.h>
#include "libhtab.h"



void htab_free(htab_t *t)               //Deallocates table
{
    if(t != NULL)       //Check if a table exists
    {
        htab_clear(t);
        free(t);
    }
    else
    {
        fprintf(stderr, "Table is NULL");
        return;
    }
}