#include <stdio.h>
#include <string.h>
#include "libhtab.h"




htab_t * htab_init(size_t size)                         //Initializes empty table
{
    htab_t * htab1;
    htab1 = (htab_t *)malloc(sizeof(htab_t) + size* sizeof(void *));
    if(htab1 == NULL)
    {
        return NULL;
    }
    memset(htab1, 0, sizeof(htab_t) + size* sizeof(void *));        //Sets all bytes to 0
    htab1->size = 0;
    htab1->arr_size = size;
    return htab1;
}
