//
// Created by kamil on 26.3.18.
//

#include <stdio.h>
#include <memory.h>
#include "libhtab.h"



void htab_free(htab_t *t)               //Deallocates table
{
    htab_clear(t);
    free(t);
}