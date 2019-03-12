/*
    error.h
    Řešení IJC-DU1, příklad b)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Deklaruje funkce s volitelným počtem argumentů pro výpis chybových hlášení
*/

//Vypíše chybový text 'CHYBA: ' a uživatelem zadaný text chyby
void warning_msg(const char * fmt, ...);

//Vypíše chybový text 'CHYBA: ' a uživatelem zadaný text chyby. Na konci ukončí program funkcí exit(1)
void error_exit(const char *fmt, ...);
