 /*
            Zaklady programovani­ - IZP
            Projekt cislo 1

            Autor: Kamil Vojanec
            Login: xvojan00

            Verze: 0.9
*/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>
#define LINE_LENGTH 100 		//Defines maximum line length as specified in the assingment.

int enabled_letters(char * enLetters,char * usrInp, char * cityName);
/*
@PARAMS
char * enLetters: Requests an array into which the return value is stored by reference.
char * usrInp: String of searched letters specified by user input.
char * cityName: String, into which the restulting cityName is input.

Function iterates line by line in STDIN input and calls functions: int is_in_array, char next_letter, int compare.
Function references char * enLetters.
Returns 1, if a clear result is found. Returns 0, if enabling characters.
*/

int is_in_array(char letter, char array[], int arrLen);
/*
Function checks if given character (char letter) is a part of given array (char array[]).
Returns 0 if letter is NOT in array and 1 if letter is in array.

@PARAMS
char letter: Character to be checked.
char array[]: Array in which the function searcehs for letter.
int arrLen: Specifies the length of array.
*/

int compare(char * string, char * usrInp);
/*
Function returns character following the one specified by usrInp.

@PARAMS
char * string: One line from STDIN.
char * usrInp: String of searched letters specified bz user input.

*/

char * bubble_sort(char * string);
/*
Function sorts string specified by the parameter.

@PARAMS
char * string: String to be sorted passed by reference.
*/

char * string_to_uppercase(char * string);
/*
Function returns sring changed to all characters uppercase.

@PARAMS
char * string: String to be cast to uppercase pased by reference.
*/

int main(int argc, char * argv[])
{
	char * usrInp;
	if(argc < 2)					//Checks if enough arguments have been entered.
	{
		usrInp = 0;
	}
	else
	{
		usrInp = argv[1];
	}
	
	char enLetters[128] = {[0 ... 127] = '\0'};		//Declares array of enabled letters inititalized to empty.
	char cityName[LINE_LENGTH] = {[0 ... LINE_LENGTH - 1] = '\0'}; //Declares array of maximum length of city name that is empty.
	int result = enabled_letters(enLetters, usrInp, cityName);		//Assigns return value of enabled_letters to variable result.
	if(result == 1)			//Checks the value of result. If it is 1, a city has been clearly found.
					//If it is 2, a city name can also be a prefix to other names, therefore a name and enabled letters are printed. If it is 0, either letters or nothing has been found.
	{
		printf("Found: %s", cityName);
	}
	else if(result == 2)
	{
		if(strlen(enLetters) == 0)
			printf("Found: %s", cityName);
		else
			printf("Found: %sEnable: %s\n", cityName, enLetters);
	}
	else
	{
		if(strlen(enLetters) != 0)
			printf("Enable: %s\n", enLetters);
		else
			printf("Not found\n");
	}


	return 0;
}


int enabled_letters(char * enLetters,char * usrInp, char * cityName)
{
	char lineOfInput[LINE_LENGTH];		//Declares array to be tested for given letters.
	char tempCityName[LINE_LENGTH];
	int cityCounter = 0;
  	int enLettersLength = sizeof(enLetters)/sizeof(char);		//Gets length of array enLetters.
	int enLetterCount = 0;
	char nextLetter;
	int nextIndex;
  	while (fgets(lineOfInput, LINE_LENGTH, stdin) != NULL)				//Reads form STDIN and passes each line into variable lineOfInput.
 	{
		nextLetter = 0;
  		string_to_uppercase(lineOfInput);				//Calls function to cast lineOfInput to uppercase.
    		if(usrInp != '\0')		//Checks if variable usrInp is not empty. If it is, then it puts the first character ofeach line into variable nextletter.
		{
			nextIndex = compare(lineOfInput, usrInp);					//Assigns return value of function compare to variable nextIndex.
			if (nextIndex > 0)								//Checks the value of nextIndex.
			{
				nextLetter = lineOfInput[nextIndex];		//If nextIndex is greater than 0, nextIndex is used as index for lineOfInput.
			}
			else if (nextIndex == 0) 					//If compare returns 0, usrInp and lineOfInput are completely equal.
			{
				strcpy(cityName, lineOfInput);		//Copies the value of lineOfInput into variable cityName.
			}
 		}
		else
		{
  			nextLetter = lineOfInput[0];
		}
		
		
		if (nextLetter != 0) 			//Checks whether nextletter has been found.
		{
			strcpy(tempCityName, lineOfInput);			//Copies the value of lineOfInput into variable tempCityName.
			cityCounter++;
			if (is_in_array(nextLetter, enLetters, enLettersLength) == 0)	//Checks if nextLetter is in array enLetters. If not, adds nextLetter to enLetters.
	   		{
	      			enLetters[enLetterCount] = nextLetter;
	      			enLetterCount++;
	    		}
		}
  	}
  	bubble_sort(enLetters);			//Sorts enLetters in alphabetical order.
  	enLetters[enLetterCount] = '\0';		//Adds 0 character at the end of enLetters.

	if(cityCounter == 1 && strlen(cityName) == 0)				//Checks if only one city has been passed to cityCounter (only one city has been found).
	{
		strcpy(cityName, tempCityName);		//Copies value of cityName into enLetters.
		return 1;
	}
	else if(strlen(cityName) > 0)			//If cityName has a value assigned, return 2.
	{
		return 2;
	}
	else
	{
		return 0;
	}

}

int is_in_array(char letter, char array[], int arrLen){			//Checks if character is in array.
    for (int i = 0; i < arrLen; ++i) 				//Iterates through each character in array.
		{
        if(array[i] == letter)
            return 1;

    }
    return 0;
}

int compare(char * string, char * usrInp)			//Finds next character in array.
{
	int lenToCmp;
	lenToCmp = strlen(usrInp);			//Assigns length of usrInp into variable lenToCmp.
	int lengthString = strlen(string);
	if(lengthString == LINE_LENGTH -1)
		fprintf(stderr, "Address is longer than 100 characters. Errors may occur.\n");
  int counter = 0;
	for (int i = 0; i < lenToCmp; i++)		//Iterates through characters until lenToCmp is reached (end of usrInp).
	{

		if(toupper(usrInp[i]) == toupper(string[i]))	//Checks if character in usrInp equals to chracter in string. Function toupper is used to ensure case insensitivity.
		{
        		counter++;				//Increments counter of how many characters have been equal.
		}else
		{
			break;				//If any character is not equal, break the cycle.
		}

	}	
	
	if (counter == lengthString - 1 && counter == lenToCmp) 
	{				//Checks if checked array is the same as usrInp and if the length is equal to checked length.
		return 0;
	}
	else if(counter == lenToCmp)				// Chcecks if all characters have been equal.
	{
		return counter;
	}
	else
	{
		return (-1);
	}
}



char * bubble_sort(char * string)			//Sorts characters in string.
{
	int length = strlen(string);
	//printf("%d\n", length);
	char temp;
	for(int i = 0; i < length; i++)
	{					//Iterates through every character. Two cycles are needed to assure that all characters have been checked and moved.
        	for(int j = 0; j < length; j++)
        	{
			if(string[j-1] > string[j] && isalpha(string[j]))
			{			//Checks whether the ASCII value of character is greater than the ASCII value of the following character.
                		temp = string[j];
                		string[j] = string[j-1];
                		string[j-1] = temp;
                
            		}	
  		}
	}


	return string;
}

char * string_to_uppercase(char * string)				//Casts string to uppercase.
{
	for(unsigned int i = 0; i < strlen(string); i++)			//Iterates through every character.
	{
		string[i] = toupper(string[i]);				//Casts character into uppercase.
	}
	return string;
}
