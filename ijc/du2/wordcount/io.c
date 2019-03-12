//
// Created by kamil on 30.3.18.
//

#include <ctype.h>
#include <stdbool.h>
#include "io.h"
int get_word(char *s, int max, FILE *f)
{
    if(f == NULL)
        return -2;
    if(s == NULL)
        return -2;
    int i = 0;
    int c = 0;
    bool err_flag = false;
    while((c = fgetc(f)) != EOF && !isspace(c) && i < max)
    {
        
        if(i == max - 1)
        {
        	if(err_flag == false)
        	{
        		fprintf(stderr, "Warning: Buffer overflow, shortening line");
        		err_flag = true;
        		
        	}
        }
        else
        {
        	s[i] = (char)c;
        	i++;
        }
    }
    s[i] = '\0';
    if(c == EOF)
        return EOF;
    return i;
}
