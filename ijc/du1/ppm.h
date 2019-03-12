/*
    ppm.h
    Řešení IJC-DU1, příklad b)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje deklarace struktury a funkcí pro práci s obrázky tzpu ppm
*/


struct ppm
{
    unsigned xsize;
    unsigned ysize;
    char data[]; // RGB bajty, celkem 3*xsize*ysize
};

//Funkce čte data z obrázku typu ppm a vrací ukazatel na dynamicky alokovanou strukturu s daty.
struct ppm * ppm_read(const char * filename);


//Funkce zapisuje data ze struktury do obrázku
int ppm_write(struct ppm *p, const char * filename);

#define MAX_IMAGE_SIZE 3000000
