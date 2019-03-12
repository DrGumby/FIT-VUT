/*
    error.c
    Řešení IJC-DU1, příklad b)
    6. březen 2018
    Autor: Kamil Vojanec (xvojan00), FIT
    Přeloženo GCC 6.4

    Obsahuje implementaci funkcí definovaných v error.h
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define ERR_MSG "CHYBA: "

void warning_msg(const char * fmt, ...)
{

    va_list args;
    char c[strlen(fmt) * sizeof(char) + strlen(ERR_MSG) * sizeof(char)];
    strcpy(c, ERR_MSG);
    strcat(c, fmt);
    va_start(args, fmt);
    vfprintf(stderr, c, args);
    va_end(args);
}

void error_exit(const char *fmt, ...)
{
    va_list args;
    char c[strlen(fmt) * sizeof(char) + strlen(ERR_MSG) * sizeof(char)];
    strcpy(c, ERR_MSG);
    strcat(c, fmt);
    va_start(args, fmt);
    vfprintf(stderr, c, args);
    va_end(args);
    exit(1);
}
