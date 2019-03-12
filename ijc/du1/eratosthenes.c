/*
    eratosthenes.c
    Řešení IJC-DU1, příklad a)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje implementaci funkcí definovaných v eratosthenes.h
*/

#include <stdio.h>
#include "bit_array.h"
#include "eratosthenes.h"
#include "error.h"
#include <stdlib.h>
#include <math.h>

void Eratosthenes(bit_array_t pole[])
{
    for(unsigned i = 2; i < round(sqrt(bit_array_size(pole)));i++)
    {
        if(bit_array_getbit(pole, i) == 0)
        {
            for(unsigned j = i*i; j < bit_array_size(pole); j+=i)
            {
                bit_array_setbit(pole, j, 1);
            }
        }
    }

}
