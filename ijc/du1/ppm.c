/*
    ppm.c
    Řešení IJC-DU1, příklad b)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje implementaci funkcí definovaných v ppm.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "ppm.h"


struct ppm * ppm_read(const char * filename)
{

    FILE * ppm_picture;
    if ((ppm_picture = fopen(filename, "rb")) == NULL)
    {
        warning_msg("Soubor neexistuje nebo nemuze byt otevren\n");
        return NULL;
    }

    unsigned xsize = 0;
    unsigned ysize = 0;
    int format_count = 0;
    format_count = fscanf(ppm_picture, "P6 %u %u 255 ", &xsize, &ysize);
    if (format_count != 2)
    {
        fclose(ppm_picture);
        warning_msg("Chyba formatu souboru: %s.\n", filename);
	return NULL;
    }
    if (3*xsize*ysize >= MAX_IMAGE_SIZE)
    {
    	fclose(ppm_picture);
        warning_msg("Soubor je prilis velky: %s.\n", filename);
	return NULL;
    }

    struct ppm * ppm_data = NULL;
    ppm_data = malloc(3*xsize*ysize + sizeof(struct ppm));
    if(ppm_data == NULL)
    {
        fclose(ppm_picture);
        warning_msg("Alokace pameti se nezdarila\n");
        return NULL;
    }
    format_count = fread(ppm_data->data, sizeof(char), 3*xsize*ysize, ppm_picture);
    if((unsigned)format_count != 3*xsize*ysize)
    {
        free(ppm_data);
        ppm_data = NULL;
        fclose(ppm_picture);
        warning_msg("Soubor se nepodarilo cist\n");
        return NULL;
    }
    ppm_data->xsize = xsize;
    ppm_data->ysize = ysize;


    fclose(ppm_picture);
    return ppm_data;
}

int ppm_write(struct ppm *p, const char * filename)
{
    FILE * ppm_picture;
    if((ppm_picture = fopen(filename, "wb")) == NULL)
    {
        warning_msg("Soubor: %s neexistuje nebo nemuze byt otevren \n", filename);
        return -1;
    }
    int byte_count = 0;
    byte_count = fprintf(ppm_picture, "P6\n%u %u\n255\n", p->xsize, p->ysize);
    if(byte_count == 0)
    {
        warning_msg("Do souboru nelze zapsat");
        fclose(ppm_picture);
        return -1;
    }
    byte_count = fwrite(p->data, 1, 3*p->xsize*p->ysize, ppm_picture);
    if ((unsigned)byte_count != 3*p->xsize*p->ysize)
    {
        warning_msg("Zapis se nezdaril");
        fclose(ppm_picture);
        return -1;
    }
    fclose(ppm_picture);
    return 0;



}
