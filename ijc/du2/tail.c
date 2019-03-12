#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024

struct line{
    char line[MAX_LINE_LENGTH];
};

void printArray (struct line * lnArray, int length)
{
    for(int i = 0; i <length; i++)
    {
        if(lnArray[i].line[strlen(lnArray[i].line)-1] != '\n')
            printf("%s\n", lnArray[i].line);
        else
            printf("%s", lnArray[i].line);
    }
}



void tail(int lineCount, FILE* source)
{
    struct line lnArray[lineCount];
    bool err = false;
    int errCnt = 0;
    int errCntNext = 0;
    int i = 0;
    int j = 0;
    char lineOfInput[MAX_LINE_LENGTH];
    while (fgets(lineOfInput, MAX_LINE_LENGTH, source) != NULL)
    {
        if (errCnt != errCntNext)
        {
            errCnt = errCntNext;
            continue;
        }
        if (lineOfInput[strlen(lineOfInput)-1] != '\n')
        {
            if (err == false)
            {
                fprintf(stderr, "Warning: buffer overflow.\n");
                err = true;
            }
            errCntNext++;
        }
        
        strcpy(lnArray[i % lineCount].line, lineOfInput);
        i++;
        if(i < lineCount)
        	j = i;
        else
        	j++;
    }
    printArray(lnArray, j);
}

long argCheck(char* arga, char* argb)
{
    if (strcmp(arga, "-n") == 0)
    {
        char * ptr;
        long lineCount = strtol(argb, &ptr, 0);
        if (*ptr == '\0')
        {
            return lineCount;
        }
        else
        {
        fprintf(stderr, "Error, parse number of lines. Exiting\n");
        return -1;
        }
    }
    else
    {
        fprintf(stderr, "Invalid parameters, exiting.\n");
        return -1;
    }
}

int main (int argc, char *argv[])
{
    switch (argc) {
        case 1:
            {
                int lineCount = 10;
                tail(lineCount, stdin);
                break;
            }
        case 2:
            {
                int lineCount = 10;
                FILE* fp = fopen(argv[1], "r");
                if (fp != NULL) {
                    tail(lineCount, fp);
                }
                else{
                    fprintf(stderr, "Error, file cannot be opened\n");
                    return 1;
                }
                fclose(fp);
                break;
            }
        case 3:
            {
                int lineCount = (int)argCheck(argv[1], argv[2]);
                if (lineCount > 0) {
                    tail(lineCount, stdin);
                }
                else{
                    fprintf(stderr, "Error, invalid argument\n");
                    return 1;
                }
                break;
            }
        case 4:
            {
                int lineCount = (int)argCheck(argv[1], argv[2]);
                if (lineCount > 0) {
                    FILE* fp = fopen(argv[3], "r");
                    if (fp != NULL) {
                        tail(lineCount, fp);
                    }
                    else{
                        fprintf(stderr, "Error opening file\n");
                        return 1;
                    }
                    fclose(fp);
                }
                else{
                    fprintf(stderr, "Error, invalid argument\n");
                    return 1;

                }
                break;
            }
    }
    return 0;
}
