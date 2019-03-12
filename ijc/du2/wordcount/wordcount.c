//
// Created by kamil on 30.3.18.
//

#include <stdio.h>
#include "libhtab.h"
#include "io.h"

#define hash_size 65599     //Podle hashovací funkce odvozen počet ukazatelů

int print_table(const char * data, int * value)
{
    printf("%s\t%d\n", data, *value);
    return 0;
}

int main()
{
    int max = 127;
    FILE *fp = stdin;
    if(fp == NULL)
        return 1;



    htab_t * table = htab_init(hash_size);

    if(table == NULL)
        return 1;

    int c;
    char str[127];
    while((c = get_word(str,max,fp)) != EOF)
    {


        if(c) {
            struct htab_listitem *item = htab_lookup_add(table, str);
            if (item == NULL)
                return 1;
        }
    }



    htab_foreach(table, &print_table);
    htab_free(table);

}
