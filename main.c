/******************************************************************
* Author: Mahesh Narayanamurthi
* e- Mail : mahesh.mach@gmail.com
* Description: Differentiation Exercise -  GSoC
* Created with: Geany 
* Libraries: Hashlib by Charles B. Falconer 
* Adapted Ideas: Simple Reverse Mode Automatic Diff.
* from "Evaluating Derivatives - Principles and Techniques of Algorithmic 
* Differentiation" by Andreas Griewank
******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>  
#include "hashlib/hashlib.h"
#include "DE.h"
#define MAX_CHAR_LINE 20
#define MAX_STACK_SIZE 100
#define MAX_TRACE_SIZE 1000


int main(int argc, char** argv){
	/* Declarations */
	
	Equation* eqn;
	hshtbl* varTable;
	firstPartials frstpartials;
	int i;
		
	/*Test for number of command line arguments*/
	if (argc < 3){
		fprintf(stderr,"Usage: DE <TYPE-I filename> <TYPE-II filename>\n");
		return 0;		
	}	
	else if (argc > 2) {
		
		/* Creates a new Hashmap and returns a pointer to it
		 **/
		varTable = getNewTable();
		
		/* Reads the equation given in the equation file and parses it.
		 **/
		eqn=readEquation(*++argv,varTable);
		
		/* Reads the variables given in the equation file and records it
		 * in the hashmap. 
		 **/
		readVariables(*++argv,varTable);
		
		/* evaluates the expression based on the Equation which has been
		 * read and using values from the variable map and prints the result.
		 **/
		fprintf(stderr,"Function Value:\t%f\n",evaluate(eqn,varTable));
			
		/* Uses the trace information to perform return sweep and 
		 * accumulated the partial adjoints. 
		 * */	
		frstpartials =  evaluateFirstPartials(varTable);
		
		for(i=0; i<frstpartials.count;i++){
			printf("1st Derivative of F wrt %s = %f\n",(frstpartials.varName[i]),*((frstpartials.partials)+i));
		}
				
		/* Deallocated the memory allocated for the hashmap */		
		hshkill(varTable);
		
		/* Deallocated the memory allocated during equation formation */
		killEquation(eqn);
	}
	
			
	return 0;
}


		


