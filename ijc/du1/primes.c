/*
    primes.c
    Řešení IJC-DU1, příklad a)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje funkci main, která vypíše posledních 10 prvočísel menších než 222 000 000, a to ve vzestupném pořadí
*/

#include <stdio.h>
#include <stdlib.h>
#include "bit_array.h"
#include "eratosthenes.h"
#include "error.h"

#define TEST_SIZE 222000000



int main(void)
{
    bit_array_create(test_array, TEST_SIZE);
    Eratosthenes(test_array);
    int no_of_last_primes = 0;
    unsigned long i = bit_array_size(test_array)-1;
    while(no_of_last_primes < 10)
    {
        if(bit_array_getbit(test_array, i) == 0)
        {
            no_of_last_primes++;
        }
        i--;
    }
    no_of_last_primes = 0;
    while(no_of_last_primes < 10)
    {
        if(bit_array_getbit(test_array, i) == 0)
        {
            printf("%lu\n", i);
            no_of_last_primes++;
        }
        i++;
    }
}
