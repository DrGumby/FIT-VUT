/*
		   Zaklady programovani­ - IZP
		   Projekt ci­slo 2

		   Autor: Kamil Vojanec
		   Login: xvojan00

		   Verze: 0.5
*/


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>


#define EPS 0.000000001			//EPS specifies the accuracy of the result to ten decimal places
#define HALFPI 1.4			//Defines the value of pi/2 as 1.4

double taylor_tan(double x, unsigned int n);
/*
@PARAMS
double x: Specifies the angle in radians.
unsigned int n: Specifies the number of polynoms to be added in the sequence.

Function approximates tan based on Taylor's polynom.
*/

double cfrac_tan(double x, unsigned int n);
/*
@PARAMS
doble x: Specifies the angle in radians.
unsigned int n: Specifies the number of iterations.

Function approximates tan based on continued fraction.
*/

void help();
/*
Prints help text.
*/

void print_tan(double angle, int firstPrintedIteration, int lastPrintedIteration);
/*
@PARAMS
double angle: Specifies the angle to be calculated.
int firstPrintedIteration: Specifies, which iteration should be printed first.
int lastPrintedIteration: Specifies, which iteration should be printed last.

Function prints formatted result of both cfrac_tan and taylor_tan. It compares the results to function tan from math.h.
*/

double get_distance(double angle, double height);
/*
@PARAMS
double angle: Specifies the angle to be calculated.
double height: Specifies observation height.

Function calculates distance from object based on cfrac_tan. It approximates to 10 decimal numbers.
*/

double get_height(double angle, double observationHeight, double distance);
/*
@PARAMS
double angle: Specifies the angle from observer to top of observed object.
double observationHeight: Specifies the height from which object is observed.
double distance: Specifies the distance to objcet.

Function calculates the height of an object based on cfrac_tan.
*/

int is_double_correct(char * stringToCheck, double * doubleToReturn);
/*
@PARAMS
char * stringToCheck: Passes a string to be checked if angle is correct
double *doubleToReturn: Passes a pointer into which a double is returned

Function converts a string into double. Returns 0 if the string is not valid. Converted string is passed into doubleToReturn.
*/

int is_iteration_correct(char * stringToCheck, int * intToReturn);
/*
@PARAMS
char * stringToCheck: Passes a string to be checked if angle is correct
double *intToReturn: Passes a pointer into which an integer is returned

Function converts a string into integer. Returns 0 if the string is not valid. Converted string is passed into doubleToReturn.
*/

int main(int argc, char * argv[])
{

		if(argc > 1)		//Checks if arguments have been passed
		{
				int intToReturn;
				double doubleToReturn;

				if(strcmp(argv[1],"--help") == 0)
				{
						help();
				}
				else if(strcmp(argv[1], "--tan") == 0)
				{

						double angle;

						int firstPrintedIteration;
						int lastPrintedIteration;

						if (argc == 5)
						{
								if(is_double_correct(argv[2], &doubleToReturn))
								{
										angle = doubleToReturn;
								}
								else
								{
										return 1;
								}

								if(is_iteration_correct(argv[3], &intToReturn))
								{
										firstPrintedIteration = intToReturn;
								}
								else
								{
										return 1;
								}

								if(is_iteration_correct(argv[4], &intToReturn))
								{
										lastPrintedIteration = intToReturn;
								}
								else
								{
										return 1;
								}


								if (firstPrintedIteration > lastPrintedIteration)	//Checks values of variables
								{
										fprintf(stderr, "Iterations are incorrect. See --help for instructions.\n");
										return 1;
								}
								print_tan(angle, firstPrintedIteration, lastPrintedIteration);			//Prints formatted tan
						}
						else
						{
								fprintf(stderr, "Incorrect program arguments. Enter --help for help.\n");
								return 1;
						}
				}
				else if(strcmp(argv[1], "-m") == 0 || (strcmp(argv[1], "-c") == 0 && (argc > 4)))		//Checks that another option has been selected
				//argv[1] is either -m or -c. If ir is -c, number of arguments must be grater than 4.
				{

						int counter = 0;		//Specifies counter to be used in case that argv[1] is -c
						double height = 1.5;
						if (strcmp(argv[1], "-c") == 0)
						{
								counter = 2;				//Counter is set to 2 to adjust for the argument
								if (is_double_correct(argv[2], &doubleToReturn))
								{
									height = doubleToReturn;
								}
								else
								{
									return 1;
								}

								if ( height < 0 || height > 100)
								{
										fprintf(stderr, "Invalid value entered. See --help for instructions.\n");
										return 1;
								}
						}
						double angleAlpha;
						double distance;
						double objectHeight;
						double angleBeta;
						if (argc == (counter + 3) || argc == (counter + 4))				//Checks that argc is 3 or 4
						{
								if (is_double_correct(argv[counter + 2], &doubleToReturn))
								{
									angleAlpha = doubleToReturn;
								}
								else
								{
									return 1;
								}

								if (angleAlpha < 0 || angleAlpha > HALFPI)				//Checks if angleAlpha is valid
								{
										fprintf(stderr, "Angle is incorrect. See --help for instructions\n");
										return 1;
								}
								distance = get_distance(angleAlpha, height);				//calculates distance.
								printf("%.10e\n", distance);



								if (argc == (counter + 4))				//If angle beta is entered, height has to be calculated.
								{
										if(is_double_correct(argv[counter+ 3], &doubleToReturn))
										{
											angleBeta = doubleToReturn;
										}
										else
										{
											return 1;
										}

										if (angleBeta < 0 || angleBeta > HALFPI)			//Checks if angle beta has valid value.
										{
												fprintf(stderr, "Angle is incorrect. See --help for instructions\n");
												return 1;
										}
										objectHeight = get_height(angleBeta, height, distance);		//Calculates the object height.
										printf("%.10e\n", objectHeight);

								}
						}


				}
				else
				{
						fprintf(stderr, "Incorrect program arguments. Use --help for explanation\n");		//Show this if incorrect arguments have been entered
				}
		}
		else
		{
			fprintf(stderr, "No program arguments. Use --help for explanation\n");				//Show this if no arguments have been entered
		}


}



double taylor_tan(double x, unsigned int n)					//Calculates tan based on taylor sequence
{
		double taylorResult = 0.;
		double power = x;


		unsigned long int taylorNumerators[] = {1, 1, 2, 17, 62, 1382, 21844, 929569, 6404582, 443861162, 18888466084, 113927491862, 58870668456604}; //Defines taylor sequence numerators
		unsigned long int taylorDenominators[] = {1, 3, 15, 315, 2835, 155925, 6081075, 638512875, 10854718875, 1856156927625, 194896477400625, 49308808782358125, 3698160658676859375}; //Defines taylor sequence denominators
		for(unsigned int i = 0; i < n; i++)		//Cycles through individual fractions in taylor sequence
		{

					taylorResult += (taylorNumerators[i]*power)/taylorDenominators[i];		//Adds another fraction into the taylor sequence
					power *= (x*x);				//Increments power by two powers of x.

		}
		return taylorResult;
}


double cfrac_tan(double x, unsigned int n)				//Calculates tan with continued fraction
{

		double result = x;

		for(unsigned int i = n ; i > 0; i--)			//Cycles through Iterations in reverse order
		{
				int final = 2*i - 1;					//Sets the current final integer in the fraction
				result = final - (x*x) / result;			//Calculates the continued fraction


		}
		return x/result;			//In the original fraction, x divides every fraction below itself. This ensures the same.
}

void help()
{
		printf("Possible input options are: \n --tan A N M \nThis calculates tan using both Taylor's sequence and continued fraction. It also compares the two results with function tan from math.h. The first collumn in the result shows which iteration the program stopped. The second collumn shows the result calculated by tan from math.h. The third collumn shows the tan as calculated by Taylor's sequence. The fourth collumn shows the absolute error between Taylor's sequence and math.h. The fifth collumn shows tan as calculated by continued fraction. The sixth collumn shows the absolute error between continued fraction and math.h. Parameters are \n \tA is the angle in radians \n \tN is the first iteration to be printed.\n \tM is the last iteration to be printed\n [-c X] -m A [B]\nThis calculates the distance between the observer and an object If B is entered, it also calculates the height of the object. The parameters are:\n \t[-c X]: If entered, this changes the height of the observer. The default height is 1.5 metres. \n \t-m A: This specifies the angle between observer and the foot of an object. \n \t[B]: If entered, this specifies the angle between the observer and the top of observed object. This causes program to calculate the height of given object.\n");
}

void print_tan(double angle, int firstPrintedIteration, int lastPrintedIteration)		//Prints formatted result of taylor_tan and cfrac_tan and compares to tan from math.h
{

		double mathTan = tan(angle);
		double taylor;
		double cfrac;
		double taylorAbsErr;
		double cfracAbsErr;
		for (int i = firstPrintedIteration; i <= lastPrintedIteration ; i++)		//Iterates from first given iteration to last given iteration
		{
				taylor = taylor_tan(angle, i);
				taylorAbsErr = fabs(mathTan - taylor);
				cfrac  = cfrac_tan(angle, i);
				cfracAbsErr = fabs(cfrac - mathTan);
				printf("%d %e %e %e %e %e\n", i, mathTan, taylor, taylorAbsErr, cfrac, cfracAbsErr);
		}


}

double get_distance(double angle, double height)			//Calculates distance using cfrac_tan
{
		double distance;
		double currentIteration = 0;
		double previousIteration = currentIteration;
		int i = 0;
		do {
				previousIteration = currentIteration;
				currentIteration = cfrac_tan(angle, i);
				i++;

		} while(fabs(previousIteration - currentIteration) > EPS);
		distance = height / currentIteration;

		return distance;

}

double get_height(double angle, double observationHeight, double distance)		//Calculates height using cfrac_tan
{
		double currentIteration = 0;
		double previousIteration = currentIteration;
		double objectHeight;
		int i = 0;
		do {
				previousIteration = currentIteration;
				currentIteration = cfrac_tan(angle, i);
				i++;

		} while(fabs(previousIteration - currentIteration) > EPS);
		objectHeight = (currentIteration*distance)+observationHeight;
		return objectHeight;
}

int is_double_correct(char * stringToCheck, double * doubleToReturn)
{
		char * err;

		*doubleToReturn = strtod(stringToCheck, &err);
		if(*err != '\0')
		{
			fprintf(stderr, "Nondigit character entered. See --help for instructions.\n");
			return 0;
		}
		if (isnan(*doubleToReturn) || isinf(*doubleToReturn))
		{
				fprintf(stderr, "Invalid value entered. See --help for instructions.\n");
				return 0;
		}

		return 1;

}

int is_iteration_correct(char * stringToCheck, int * intToReturn)
{
		char * err;
		*intToReturn = strtol(stringToCheck, &err, 0);
		if(*err != '\0')
		{
			fprintf(stderr, "Nondigit character entered. See --help for instructions.\n");
			return 0;
		}

		if(*intToReturn < 1 || *intToReturn > 13)
		{
			fprintf(stderr, "Invalid iteration entered. See --help for instructions.\n");
			return 0;
		}
		return 1;


}
