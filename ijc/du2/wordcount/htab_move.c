#include <stdio.h>
#include <string.h>
#include "libhtab.h"






htab_t * htab_move(size_t newsize, htab_t * t2)             //Initializes table with values from different table
{
    htab_t * htab1;
    htab1 = (htab_t *)malloc(sizeof(htab_t) + newsize* sizeof(void*));
    if(htab1 == NULL)
        return  NULL;
    memset(htab1, 0, sizeof(htab_t) + newsize* sizeof(void *));     //Set all allocated bytes to 0
    htab1->size = t2->size;
    htab1->arr_size = newsize;
    struct htab_listitem * item_newtab = NULL;      //Create 3 auxiliary variables
    struct htab_listitem * item_t2 = NULL;
    struct htab_listitem * tmp_item = NULL;
    unsigned index = 0;
    for (unsigned i = 0; i < t2->arr_size; ++i)         //Iterate through flexible array member
    {
        item_t2 = t2->heads[i];
        while(item_t2 != NULL)                          //Iterate through linked list
        {
            tmp_item = item_t2;

            t2->heads[i] = t2->heads[i]->next;          //Move first element into variable, remove from original position
            item_t2 = item_t2->next;
            tmp_item->next = NULL;

            index = htab_hash_function(tmp_item->key) % htab1->arr_size;       //FInd new index
            if(htab1->heads[index] == NULL)
            {
                htab1->heads[index] = tmp_item;                     //Move new element into array
            }
            else
            {
                item_newtab = htab1->heads[index];
                while(item_newtab->next != NULL)
                {
                    item_newtab = item_newtab->next;
                }
                item_newtab->next = tmp_item;
            }
        }
    }

    return htab1;
}
