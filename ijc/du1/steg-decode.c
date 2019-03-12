/*
    steg-decode.c
    Řešení IJC-DU1, příklad b)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje funkci main, která načte obrázek za pomocí funkcí z ppm.h a rozkóduje tajnou zprávu uloženou v obrázku
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "bit_array.h"
#include "eratosthenes.h"
#include "error.h"
#include "ppm.h"

#define MAX_IMG_SIZE 3000000

int main(int argc, char * argv[])
{
	if(argc == 1)
	{
		error_exit("Nezadan soubor pro otevreni\n");

	}


	struct ppm * ppm_data = ppm_read(argv[1]);
	if(ppm_data == NULL)
	{
		return 1;
	}
	bit_array_create(test, (MAX_IMG_SIZE));
	Eratosthenes(test);
	unsigned char msg = 0;
	int cycles = 0;
	unsigned int bit = 0;
	for(unsigned i = 11; i < 3*ppm_data->xsize*ppm_data->ysize; i++)
	{
		if(bit_array_getbit(test,i) == 0)
		{
			bit = bit_array_getbit_no_array_check_char_only(ppm_data->data[i], 0);
			bit_array_setbit_no_array_check_char_only((msg), (cycles), (bit));
			cycles++;



			if(cycles == 8 )
			{
				if(msg == 0)
				{
					printf("\n");
					break;
				}
				else if(!isprint(msg))
				{
					free(ppm_data);
					error_exit("Nalezen netisknutelny znak");

				}
				cycles = 0;
				printf("%c", msg);
				
			}
		}
	}
	free(ppm_data);


}
