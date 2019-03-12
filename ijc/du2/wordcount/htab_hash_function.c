#include <stdio.h>
#include "libhtab.h"


unsigned int htab_hash_function(const char *str)        //Function generates hash. Use with modulo to get index
{
    unsigned int h=0; // 32bit
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
        h = 65599*h + *p;
    return h;
}