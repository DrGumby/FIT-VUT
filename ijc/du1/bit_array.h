/*
    bit_array.h
    Řešení IJC-DU1, příklad a)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Soubor obsahuje deklaraci a implementaci maker pro práci s bitovými poli. V případě, že se překládá s definovaným USE_INLINE se makra nahradí zde implementovanými inline funkcemi.
*/

//Definuje velikost typu char v bitech

#include "error.h"
#include <limits.h>


typedef unsigned long bit_array_t;


//Vytváří bitové pole za použití typu bit_array_t. Na nultém indexu je umístěna velikost bitového pole.
#define bit_array_create(jmeno_pole, velikost) bit_array_t jmeno_pole[(velikost)/(sizeof(unsigned long)*CHAR_BIT) + ((velikost % sizeof (unsigned long) == 0) ? 0 : 1) +1] = {velikost, 0,}

#ifdef USE_INLINE

//Vrací velikost bitového pole uloženou na nultém indexu bitového pole.
static inline unsigned long bit_array_size(bit_array_t jmeno_pole[])
{
    return jmeno_pole[0];
}

#else
#define bit_array_size(jmeno_pole) jmeno_pole[0]
#endif

#ifdef USE_INLINE

//Nastavuje bit na indexu index na hodnotu danou výrazem
static inline void bit_array_setbit(bit_array_t jmeno_pole[], unsigned long index, int vyraz)
{
    if(index > bit_array_size(jmeno_pole)-1)
    {
        error_exit("Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bit_array_size(jmeno_pole));
    }
    if(vyraz)
    {
        jmeno_pole[index/(sizeof(unsigned long)*CHAR_BIT) + 1] |= (1ul << (index%(sizeof(unsigned long)*CHAR_BIT)));
    }
    else
    {
        (jmeno_pole[index/(sizeof(unsigned long)*CHAR_BIT) + 1] &= ~(1ul << (index%(sizeof(unsigned long)*CHAR_BIT))));
    }

}

#else
#define bit_array_setbit(jmeno_pole, index, vyraz) do {if(index > jmeno_pole[0]-1) error_exit("Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bit_array_size(jmeno_pole)); \
                                                        if(vyraz) (jmeno_pole[index/(sizeof(unsigned long)*CHAR_BIT) + 1] |= (1ul << (index%(sizeof(unsigned long)*CHAR_BIT)))); \
                                                        else (jmeno_pole[index/(sizeof(unsigned long)*CHAR_BIT) + 1] &= ~(1ul << (index%(sizeof(unsigned long)*CHAR_BIT)))); \
                                                    } while(0);
#endif


#ifdef USE_INLINE

//Vrací 0 nebo 1 v závislosti na stavu bitu na indexu
static inline int bit_array_getbit(bit_array_t jmeno_pole[], unsigned long index)
{
    if(index > bit_array_size(jmeno_pole)-1)
    {
        error_exit("Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bit_array_size(jmeno_pole));
        return -1;
    }
    else
    {
        return ((unsigned long)(jmeno_pole[(index/(sizeof(unsigned long)*CHAR_BIT))+1] & (1ul << (index%(sizeof(unsigned long)*CHAR_BIT)))) > 0);
    }
}

#else

#define bit_array_getbit(jmeno_pole, index) (index > jmeno_pole[0]-1)?  \
                                            error_exit("Index %lu mimo rozsah 0..%lu", (unsigned long)index, (unsigned long)bit_array_size(jmeno_pole)),0\
                                            :((unsigned long)(jmeno_pole[(index/(sizeof(unsigned long)*CHAR_BIT))+1] & (1ul << (index%(sizeof(unsigned long)*CHAR_BIT)))) > 0)
#endif

#define bit_array_getbit_no_array_check_char_only(jmeno_pole, index) ((jmeno_pole & (1 << (index%CHAR_BIT))) > 0)   //Tato makra jsou definována z důvodu použití jednoho bajtu pro vyhledávání skryté informace. Tomuto použití neodpovídají výše implementovaná makra
#define bit_array_setbit_no_array_check_char_only(jmeno_pole, index, vyraz) do{if(vyraz) (jmeno_pole |= (1 << (index%CHAR_BIT))); \
                                                                                else (jmeno_pole &= ~(1 << (index%CHAR_BIT))); \
                                                                            }while(0);
