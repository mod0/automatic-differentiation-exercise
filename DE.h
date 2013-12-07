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

#ifndef _DE_H_
#define _DE_H_

#ifndef _MATH_H
	#include <math.h>
#endif

#ifndef _STDLIB_H
	#include <stdlib.h>
#endif

#ifndef _STRING_H
	#include <string.h>
#endif

#ifndef _STDIO_H
	#include <stdio.h>
#endif

#ifndef hashlib_h
	#include "hashlib/hashlib.h"
#endif


/* The maximum number of characters in one line of an input file */
#ifndef MAX_CHAR_LINE
	#define MAX_CHAR_LINE 20
#endif

/* The maximum depth of stack allowed for function evaluation */
#ifndef MAX_STACK_SIZE
	#define MAX_STACK_SIZE 100
#endif

/* The maximum length of Trace allowed for adjoint evaluation */
#ifndef MAX_TRACE_SIZE
	#define MAX_TRACE_SIZE 1000
#endif


/************************ Structures & Enums *************************/

/* Description: This enum contains the various opcodes.To extend this enum,
 * simply add new opcodes to the list. This has been maintained this way to 
 * support scalability.
 * */
typedef enum opcode{
	emptyv,
	constv,
	indepv,
	bplusv,
	bminusv,
	bmultv,
	divv,
	recipv,
	sinv,
	cosv,
	powv,	
	funcv
}opcode;


/* Description: This structure holds the various tokens that form the 
 * equation. It is later used by function evaluate to evaluate the value 
 * of the function.
 * */
typedef struct {
	opcode type;			
	char* token;
	struct Equation* next;
}Equation,*EquationP;


/* Description: This structure is used by the Hashmap to keep record of a 
 * variable as a key,value pair. The Hashlib used by this code has been 
 * written by Charles B. Falconer and is licensed under GPL. It has O(1) 
 * storage and retrieval performance.
 * */
typedef struct{
	char* key;
	double value;
}varMap,*varMapP;


/* Description: This structure is used for maintaining Trace information 
 * of all the operations performed during function evaluation. It is 
 * later used for adjoint calculation during the return sweep.
 * This structure idea has been adapted from the book 
 * "Evaluating Derivatives - Principles and Techniques of Algorithmic 
 * Differentiation by Andreas Griewank"
 * and modified suitably.
 * */
typedef struct{
   double val;
   double bar;
   char* varName;
   opcode operation;
   struct elements *arg1;
   struct elements *arg2;
}elements;

/* Description: This structure is used for maintaining pointer to trace 
 * information of all the operations performed during function evaluation.
 * This structure idea has been adapted from the book 
 * "Evaluating Derivatives - Principles and Techniques of Algorithmic 
 * Differentiation by Andreas Griewank" 
 * */
typedef struct{
	elements* ref;
}redouble;


/*Description: This structure is used for maintaining the stack during 
 *function evaluation.
 * */
typedef struct{
	redouble element[MAX_STACK_SIZE];
	int topOfStack;
}stack, *stackP;


/* Description: This structure is used to store the values of the first 
 * partials of the function which can later be used for further calculations.
 * */
typedef struct{
	double* partials;
	int index;
	int count;
	char** varName;
}firstPartials;

/********* These functions help access and modify the stack **********/
/* Description: This function is used to push values of type redouble onto
 * the stack during function evaluation phase.
 * Returns: Exits with 1 on failure and returns 0 on success
 * */
int push(stackP,redouble);

/* Description: This function is used to pop values of type redouble* off
 * the stack during function evaluation phase.
 * Returns: Returns the pointer to the object of type redouble at the top 
 * 			of the stack.
 * 			Exits with 1 on underflow.
 * */
redouble* pop(stackP);

/* Description: This function is used to create a stack. It returns a stack
 * pointer to the new stack.
 * */
stackP createStack();

/* Description: This function is used to kill an existing stack.
 * Arguments: The pointer to the stack that is to be killed.
 * */
int killStack(stackP);

/* Description: This function is used to print the contents of an existing
 * stack. Its used for debugging purpose.
 * Arguments: The pointer to the stack that is to be printed.
 * */
void printStack(stackP);
/*********************************************************************/


/****** These Functions help acess and modify the variable map *******/

/* Description: varCmp is used to compare two items belonging to the same
 * variable map and returns 1, 0, -1 according to 1>2,1==2,1<2
 * */
int varCmp(void*,void*);

/* Description: varDup is used to create a copy of the element that is passed 
 * to it. It is used by Hashlib library. For more details look at hashusage.txt
 * in hashlib folder at the root directory. 
 * */
void* varDup(void*);

/* Description: varFree is used to free the allocated memory on the hashmap
 * It is used by Hashlib library. For more details look at hashusage.txt in
 * hashlib folder at the root directory.
 * */
void varFree(void*);

/* Description: varHash and varReHash are used by Hashlib library.
 * For more details look at hashusage.txt in hashlib folder at the 
 * root directory.
 * */
unsigned long varHash(void*);
unsigned long varReHash(void*);


/*********************************************************************/

/* Description: During the return sweep, the partial adjoints of each 
 * input is calulated and this has to be summed up before I can get the
 * adjoint of that particular input.
 * */
int sumEachAdjoint(void*,void*,void*);

/* Description: Calculates all the partial adjoints during return sweep 
 * */
int reverseSweep();

/* Description: Reads the equation file and sotres it in a Equation Linked List 
 * */
Equation* readEquation(char* , hshtbl* );

/* Description: Reads the variables from the variable file and updates the hashmap 
 * */
int readVariables(char* , hshtbl*);

/* Description: Prints the read Variables to screen for Debugging 
 * */
void printVariables(hshtbl*);
int printEachVariable(void*, void* , void* );

/* Description: Prints the alread read Equation to a file named eqnchk.txt 
 * for debugging 
 * */
void printEquation(Equation* );

/* Description: This function is used to get the type of token that is 
 * passed to it.
 * Arguments: char* to the token.
 * Returns: The type of the token 
 * empty variable 0
 * constant variable 1
 * independent variable 2
 * bplusv 3
 * bminusv 4
 * bmultv 5
 * divv 6
 * recipv 7
 * sinv 8
 * powv 9
 * */
int getType(char* );

/* Description: Returns the number of distinct variables does the eqn have 
 * */
int getNumberVariables(hshtbl*);

/* Description: Evaluation is computing F = left-hand-side - right-hand-side
 * (the residual error if the variable values given do not satify 
 * the equation). 
 * */
double evaluate(Equation*, hshtbl*);

/* Description: Evaluate dF/d(all-variables) partial derivatives).
 * and return the results as a firstPartials object
 * */
firstPartials evaluateFirstPartials(hshtbl*);

/* Description: Makes independent variables to be pushed onto the stack 
 * which are later used during the evaluation phase.
 * */
redouble makeIndepv(double,char*);

/* Description: Makes constant entries to be pushed onto the stack which
 * are later used during the evaluation phase
 * */
redouble makeConstv(double,char*);

/* Description: Returns a new table to hold key,value pairs for variables.
 * */
hshtbl* getNewTable();

/*Description: This function deallocates the resources that have been 
 * allocated during the formation of the Equation Linked List.
 * This function has to be called after a call to hshkill()
 * */
void killEquation(Equation*);

/**********************************************************************************************/
#endif
