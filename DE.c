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
	#include "DE.h"
#endif


/****************************** GLOBALS ******************************/

/* Description: These globals are used to keep a trace of all the operations
 * and their corresponding argument details which are later accessed to 
 * evaluate adjoints during the return sweep.  
 * */

elements trace[MAX_TRACE_SIZE];
elements* traceptr = trace;

/*********************************************************************/

/* Description: This function is used to create a new variable table 
 * basically a hashmap. It returns the pointer to the newly created 
 * table.
 * */
hshtbl* getNewTable(){
	return hshinit(varHash, varReHash,
					  varCmp,
					  varDup, varFree,
					  0);
}

/* Description: This function is used to read the input equation file,
 * parses the tokens and forms a stack which will be used during the
 * function evaluation phase.
 * Arguments: filename is the char pointer pointing to the array of 
 * characters representing the name of the input equation file. 			  
 * varTable is used for adding variables to the table that are read from
 * the input equation file.
 * Returns: It returns the postfix stack that is created using a linked
 * list.
 * */
Equation* readEquation(char* filename, hshtbl* varTable){
	FILE* fp;
	char* line;
	char* single_line;
	char* type;
	varMapP var;
	varMapP tempVar;
	Equation* head = NULL;
	Equation* temp = NULL;
	
	/*Open first file and read the contents*/		
	if ((fp = fopen(filename,"r")) == NULL){
		fprintf(stderr,"IO Error: File %s could not be opened",filename);
	}
	else{
		line = (char*) malloc(sizeof(char) * MAX_CHAR_LINE);
		single_line = fgets(line,MAX_CHAR_LINE,fp);
		if (single_line == NULL){
			fprintf(stderr,"EOF detected\n");	
			free(line);
			line=NULL;
			if (fp!=NULL){
				fclose(fp);
				fp=NULL;
			}
			return NULL;
		}
		while(single_line!=NULL){
			if (head == NULL){
				head = (Equation*) malloc(sizeof(Equation));	
				// Assuming there are no whitespaces at the end of a line and the string is trimmed. 
				if(strchr(single_line,32)!=NULL){
					type = strtok(single_line," ");
					head->type = getType(type);
					head->token = strtok(NULL,"\n");
				}
				else{
					type = strtok(single_line,"\n");
					head->token = type;
					head->type = getType(type);
				}
				if (head->type == indepv){
					tempVar = (varMapP) malloc(sizeof(varMap));
					tempVar->key=head->token;
					tempVar->value=0.0;
					var = hshinsert(varTable, tempVar);
					if (var == NULL){
						fprintf(stderr,"Memory Error");
						exit(1);
					}						
				}
				head->next=NULL;
			}
			else{
				temp = (Equation*) malloc(sizeof(Equation));
				// Assuming there are no whitespaces at the end of a line and the string is trimmed. 
				if(strchr(single_line,32)!=NULL){
					type = strtok(single_line," ");
					temp->type = getType(type);
					temp->token = strtok(NULL,"\n");
				}
				else{
					type = strtok(single_line,"\n");
					temp->token = type;
					temp->type = getType(type);
				}
				if (temp->type == indepv){
					tempVar = (varMapP) malloc(sizeof(varMap));
					tempVar->key=temp->token;
					tempVar->value=0.0;
					var = hshinsert(varTable, tempVar);
					if (var == NULL){
						fprintf(stderr,"Memory Error");
						exit(1);
					}
				}
				temp->next = (struct Equation*) head;
				head = temp;
			}
			line = (char*) malloc(sizeof(char) * MAX_CHAR_LINE);
			single_line = fgets(line,MAX_CHAR_LINE,fp);
			if (single_line == NULL){
				free(line);
				line=NULL;
				if (fp!=NULL){
					fclose(fp);
					fp=NULL;
				}
			}
			else;
		}
		return head;	
	}
	return NULL;
}

/* Description: This function determines the type of token which is passed 
 * to it and obviates the necessity for a further string comparison during
 * function evaluation phase and 1st differential evaluation phase.
 * Arguments: It takes a pointer to the stream of characters representing the
 * operation code.
 * Returns: one of constants defined in the opcode enumeration.
 * 
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
int getType(char* token){
		if (strcmp(token,"+")==0)
			return bplusv;
		else if (strcmp(token,"-")==0)
			return bminusv;
		else if (strcmp(token,"*")==0)
			return bmultv;
		else if (strcmp(token,"/")==0)
			return divv;
		else if (strcmp(token,"=")==0)
			return funcv;
		else if (strcmp(token,"variable")==0)
			return indepv;
		else if (strcmp(token,"constant")==0)
			return constv;
		else if (strcmp(token,"integer_constant")==0)
			return constv;
		else if (strcmp(token,"sin")==0)
			return sinv;
		else if (strcmp(token,"pow")==0)
			return powv;	
		return -1;
}

/* Description: This function is used to check if the input equation file
 * has been parsed properly. It generates a file called eqnchk.txt which
 * contains the type of token followed by the token itself
 * Arguments: It takes the head of the linked list stack formed during the
 * readEquation phase.
 * */
void printEquation(Equation* eqn){
	FILE* fp;
	Equation* head=eqn;
	fp=fopen("eqnchk.txt","w");
	if(fp!=NULL){
		while(head!=NULL){
			fprintf(fp,"Type\t%d\n",head->type);
			fprintf(fp,"Token\t%s\n",head->token);
			head= (Equation*)head->next;
		}
		fclose(fp);
	}
	else
		fprintf(stderr,"IO Error: Error opening file");
}

/* Description: This function is used by the Hashlib library. It is used 
 * to compare objects of varMap type. Comparison is done by comparing the
 * keys of the 2 items.
 * Arguments: The two items that are to be compared are passed as lvar and
 * rvar. These are pointers to the object of type varMap.
 * Returns: -1,0,1 is returned depending on
 * item1 < item 2
 * item1 == item2
 * item1 > item2
 * For more information look at hshusage.txt in hashlib folder under root.
 * */
int varCmp(void* lvar,void* rvar){
	varMapP left = (varMapP) lvar;
	varMapP right = (varMapP) rvar;
	return strcmp(left->key,right->key);
}

/* Description: This function is used to create duplicates of object type
 * varMap. It is used by the Hashlib Library.
 * For more information look at hshusage.txt in hashlib folder under root.
 * */
void* varDup(void* var){
	varMapP myVar = var;
	varMapP newVar;
	
	if((newVar = malloc(sizeof(newVar)))){
		if ((newVar->key = strdup(myVar->key))){
				newVar->value=myVar->value;	
		}
		else{
			free(newVar);
			newVar=NULL;
		}
	}	
	return newVar;
}

/* Description; This function is used by Hashlib Library. Call to hshkill()
 * calls this function, which contains details as to how objects of type 
 * varMap is to be deallocated.
 * Arguments: Accepts the object that is to be deallocated and performs
 * the necessary steps required to deallocate it.
 * hshkill() calls varFree() on each object in the map
 * */
void varFree(void* var){
	varMapP varToFree;
	varToFree=var;
	free(varToFree->key);
	free(varToFree);
}

/* Description: This function returns the hash value of the corresponding key
 * This is used by the Hashlib library. For more details refer the hshusage.txt
 * in hashlib folder under root.
 * You can implement your own hashing function which the hashlib library 
 * will call. Currently I am using the builtin hashing function.
 * Argument: The pointer to object of type varMap whose key is to be hashed.
 * */
unsigned long varHash(void* var){
	varMapP varToHash;
	varToHash=var;
	return hshstrhash(varToHash->key);
}

/* Description: This function returns the rehash value of the corresponding key
 * This is used by the Hashlib library. For more details refer the hshusage.txt
 * in hashlib folder under root.
 * You can implement your own rehashing function which the hashlib library 
 * will call. Currently I am using the builtin hashing function.
 * Argument: The pointer to object of type varMap whose key is to be 
 * rehashed.
 * */
unsigned long varReHash(void* var){
	varMapP varToReHash;
	varToReHash=var;
	return hshstrehash(varToReHash->key);
}

/* Description: This function is used to read variables from the 2nd input file
 * and to update the variables in the hashmap. Variables not in the hashmap 
 * are ignored. And those variables that are already in the map 
 * for which the value is not specified in the input file, are defaulted
 * to 0.0;
 * Arguments: filename points to the stream of characters that represent the
 * 2nd input file and full path. 
 * varTable points to the table in which the variables are to be updated.
 * Return: 0 on success
 * 		   1 on failure 
 * */
int readVariables(char* filename,hshtbl* varTable){
	
	FILE* fp;
	char* line;
	char* single_line;
	varMapP var;
	varMapP varToFind;
	
	/*Open first file and read the contents*/		
	if ((fp = fopen(filename,"r")) == NULL){
		fprintf(stderr,"IO Error: File %s could not be opened",filename);
	}
	else{
		line = (char*) malloc(sizeof(char) * MAX_CHAR_LINE);
		varToFind = (varMapP) malloc(sizeof(varMap));
		single_line = fgets(line,MAX_CHAR_LINE,fp);
		if (single_line == NULL){
			fprintf(stderr,"EOF detected\n");	
			free(line);
			line=NULL;
			if (fp!=NULL){
				fclose(fp);
				fp=NULL;
			}
			return 1;
		}
		while(single_line!=NULL){
			// Assuming there are no whitespaces at the end of a line and the string is trimmed. 
			if(strchr(single_line,32)!=NULL){
				varToFind->key = strtok(single_line," ");
				varToFind->value = strtod(strtok(NULL,"\n"),NULL);	
				var = hshfind(varTable, varToFind);
				if (var == NULL){
					fprintf(stderr,"Memory Error");
				}								
				else
					var->value=varToFind->value;
			}
			
			line = (char*) malloc(sizeof(char) * MAX_CHAR_LINE);
			single_line = fgets(line,MAX_CHAR_LINE,fp);
			if (single_line == NULL){
				free(line);
				line=NULL;
				if (fp!=NULL){
					fclose(fp);
					fp=NULL;
				}
			}
			else;
		}
		return 0;
	}
	return 1;
}

/* Description: This function returns the number of variables that are currently
 * in the hashmap.
 * Arguments: Takes the pointer to the hash table that contains the variables.
 * Return: Returns the number of entries in the table.
 * */
int getNumberVariables(hshtbl* varTable){
	hshstats varStats;
	varStats = hshstatus(varTable);	
	return varStats.hentries;
}

/* Description: This function is used to check whether each variable has
 * been read and parsed properly. Its used for debugging purposes.
 * Arguments: varElement is the pointer to the object of type
 * varMap recorded in the hashmap.
 * This function is called by the hshwalk() function from Hashlib library
 * which in turn is called by a call to printVariables.
 * For hshwalk to walk through the next element, function has to return 0 
 * For more details refer the hshusage.txt in the hashlib folder under root.
 * */
int printEachVariable(void *varElement, void *data, void *extra){
	varMapP var = varElement;
	fprintf(stderr,"%s\t%f\n",var->key,var->value);
	return 0;
}


/* Description: This function is used to check whether all variables have
 * been read and parsed properly. Its used for debugging purposes.
 * Arguments: varTable is the pointer to the hashmap.
 * This function calls hshwalk() function from Hashlib library
 * which in turn calls printEachVariable with a pointer to each object
 * of type varMap in the hashmap.
 * For more details refer the hshusage.txt in the hashlib folder under root.
 * */
void printVariables(hshtbl* varTable){	
	hshtbl* tblToPrint = varTable;
	hshwalk(tblToPrint, printEachVariable, NULL);
}

/* Description: This function is used to push values of type redouble onto
 * the stack during function evaluation phase.
 * Arguments: eqnStack - The pointer to the stack into which the object 
 * of type redouble is to be pushed 
 * operand - The redouble object that is to be pushed onto the stack
 * Returns: Exits with 1 on failure and returns 0 on success
 * */
int push(stackP eqnStack,redouble operand){
	if (eqnStack->topOfStack<MAX_STACK_SIZE){
		eqnStack->element[eqnStack->topOfStack++]=operand;
		return 0;
	}
	else{
		fprintf(stderr,"Stack Overflow Error\n");
		exit(1);
	}
	return 1;
}

/* Description: This function is used to pop values of type redouble* off
 * the stack during function evaluation phase.
 * Argument: Accepts the pointer to the stack from which pointer to 
 * object of type redouble which is at the top of stack is returned
 * Returns: Returns the pointer to the object of type redouble at the top 
 * 			of the stack.
 * 			Exits with 1 on underflow.
 * */
redouble* pop(stackP eqnStack){
	if (eqnStack->topOfStack>0)
		return &(eqnStack->element[(--eqnStack->topOfStack)]);
	else{
		fprintf(stderr,"Stack Underflow Error\n");	
		exit(1);
	}
	return NULL;
}

/* Description: This function is used to create a stack. It returns a stack
 * pointer to the new stack. The dimension of the stack are specified by
 * MAX_STACK_SIZE value in DE.h
 * */
stackP createStack(){
	stackP eqnStack = (stackP) malloc(sizeof(stack));
	eqnStack->topOfStack=0;
	if(eqnStack!=NULL);
		return eqnStack;
}

/* Description: This function is used to kill an existing stack.
 * Arguments: The pointer to the stack that is to be killed.
 * */
int killStack(stackP eqnStack){
		free(eqnStack);	
		return 0;	
}

/* Description: This function takes the pointer to the hashmap and that to
 * the Equation linked list formed during the readEquation function and
 * evaluates it using a double stack approach with 1 stack being the 
 * linked list of Equation and the other eqnStack into which operands
 * and intermediate results are pushed and manipulated.
 * Also in this function, the trace list of the operations performed and
 * the corresponding arguments is maintained which is later used during the
 * return sweep or reverseSweep to calculate the partial adjoints.
 * Arguments: The pointer to the linked list of parsed equation and
 * the table of key, value pairs for variables - hashmap of vairables.
 * Returns: The function value evaluated as LHS-RHS in an equation LHS=RHS
 * (the residual error if the variable values given do not satify 
 * the equation).
 * */
double evaluate(Equation* eqn,hshtbl* vars){
	Equation* head = eqn;
	varMap var;
	varMapP locVar;
	double result;
	hshtbl* varTable=vars;
	redouble temp;
	redouble* operand1;
	redouble* operand2;
	stackP operandStack = createStack();
	while (head!=NULL){
		switch(head->type){
			case constv:
				push(operandStack,makeConstv((strtod(head->token,NULL)),NULL));
				break;
			case indepv:
				var.key=head->token;
				locVar=hshfind(varTable,&var);
				push(operandStack,makeIndepv((locVar->value),head->token));
				break;
			case bplusv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = (operand1->ref->val)+(operand2->ref->val);
				traceptr->bar = 0.0;
				traceptr->operation = bplusv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case bminusv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = (operand1->ref->val)-(operand2->ref->val);
				traceptr->bar = 0.0;
				traceptr->operation = bminusv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case bmultv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = (operand1->ref->val)*(operand2->ref->val);
				traceptr->bar = 0.0;
				traceptr->operation = bmultv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case divv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				if (operand2->ref->val!=0)
					traceptr->val = (operand1->ref->val)/(operand2->ref->val);
				else{
					printf("Divide by Zero Error\n");
					exit(1);
				}
				traceptr->bar = 0.0;
				traceptr->operation = divv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case sinv:
				operand1 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = sin((operand1->ref->val));
				traceptr->bar = 0.0;
				traceptr->operation = sinv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case powv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = pow((operand1->ref->val),(operand2->ref->val));
				traceptr->bar = 0.0;
				traceptr->operation = powv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;
			case funcv:
				operand1 = pop(operandStack);
				operand2 = pop(operandStack);
				temp.ref = traceptr;
				traceptr->val = (operand1->ref->val)-(operand2->ref->val);
				traceptr->bar = 1.0;
				traceptr->operation = bminusv;
				traceptr->arg1 = (struct elements*) operand1->ref;
				traceptr->arg2 = (struct elements*) operand2->ref;
				traceptr++;
				push(operandStack,temp);
				break;			
			default:
				break;
		}
		head=(Equation*)head->next;
	}
	result = ((operandStack->element[0]).ref)->val;
	killStack(operandStack);
	return result;		
}

/* Description: This function is used to print the contents of an existing
 * stack. Its used for debugging purpose.
 * Arguments: The pointer to the stack that is to be printed.
 * */
void printStack(stackP operandStack){
	int i=0;
	for(;i<operandStack->topOfStack;i++)
		printf("%f  ",((operandStack->element[i]).ref)->val);
	printf("\n*****************************************\n");
}

/* Description: This function is used to calculate the partial adjoints of 
 * the independent variables. This is the return sweep phase where the trace
 * information that has been built up during the function evaluation phase
 * is used to calculate the partial adjoints which would later be accumulated.
 * Return: Currently return value is not used. But can later be used to flag
 * errors in situations like forward evaluation phase has not been performed.
 * */
int reverseSweep(){
  double deriv1;
  double deriv2;
  
  double arg1val;
  double arg2val;
  double combibar;
  
  elements* tracer = traceptr;
  
  while (--tracer > trace){
  
  
  	switch (tracer->operation){
         case bplusv:
    		combibar = (((elements*)(tracer))->bar);     	
         	(((elements*)(tracer->arg1))->bar) += combibar;
         	(((elements*)(tracer->arg2))->bar) += combibar;
         	break;
         case bminusv:
         	combibar = (((elements*)(tracer))->bar);
            (((elements*)(tracer->arg1))->bar) += combibar;
         	(((elements*)(tracer->arg2))->bar) -= combibar;
         	break;
         case bmultv: 
         	combibar = (((elements*)(tracer))->bar);
         	arg1val=(((elements*)(tracer->arg1))->val);
  			arg2val=(((elements*)(tracer->arg2))->val);
         	(((elements*)(tracer->arg1))->bar) += combibar * arg2val;
         	(((elements*)(tracer->arg2))->bar) += combibar * arg1val;
        	break;
         case divv:
    		combibar = (((elements*)(tracer))->bar);
         	arg1val=(((elements*)(tracer->arg1))->val);
  			arg2val=(((elements*)(tracer->arg2))->val);
  	       	deriv1 = (1/arg2val);
         	deriv2 = (-arg1val)/(pow(arg2val,2));
         	(((elements*)(tracer->arg1))->bar) += combibar * deriv1;
         	(((elements*)(tracer->arg2))->bar) += combibar * deriv2;
         	break;
         case sinv:
         	combibar = (((elements*)(tracer))->bar);
         	arg1val=(((elements*)(tracer->arg1))->val);
            deriv1 = cos(arg1val) ;
            (((elements*)(tracer->arg1))->bar) += combibar * deriv1;
         	break;         	
         case powv:
         	combibar = (((elements*)(tracer))->bar);
         	arg1val=(((elements*)(tracer->arg1))->val);
  			arg2val=(((elements*)(tracer->arg2))->val);
         	deriv1 = arg2val*pow(arg1val,arg2val-1);
         	deriv2 = pow(arg1val,arg2val) * log(arg1val);
         	(((elements*)(tracer->arg1))->bar) += combibar * deriv1;
         	(((elements*)(tracer->arg2))->bar) += combibar * deriv2;
         	break;
         case funcv:
         	(((elements*)(tracer->arg1))->bar) += combibar;
         	(((elements*)(tracer->arg2))->bar) -= combibar;
         	break;
         default:
         	break;
  	} 
  }
  
  return 0;
}

/* Description: This function is used to make independent variables and 
 * assign values to them from the hashmap. A trace of it is created as well.
 * The return value is pushed onto the stack and used for further function
 * evaluation.
 * Arguments: value is the value of the variable which is got from the hashmap
 * Note: If the variable is not found in the hashmap (which is formed during
 * equation parse phase) this function is not executed and 
 * such a variable is silently discarded. It could as well be made to flag
 * an error condition.
 * varName field is included to distinguish one variable from another
 * and is later used during accumulation of partial adjoints. It is the
 * character pointer to the stream of characters representing the name of the 
 * variable.
 * Returns: It returns an object of type redouble which is pushed onto the 
 * stack and used during the evaluation phase.
 * */
redouble makeIndepv(double value,char* varName){
	redouble temp;
	temp.ref = traceptr;
	traceptr->val = value;
	traceptr->bar = 0.0;
	traceptr->operation = indepv;
	traceptr->varName = varName;
	traceptr++;
	return temp;
}

/* Description: This function is used to make constant variables. A trace of it
 * is created as well. The return value is pushed onto the stack and used
 * for further evaluation.
 * Arguments: value is the value of the constant.
 * Note: I am parsing all constants Integer and Real as double for simplicity
 * though I am aware of the memory constraints. I could parse Integer constants
 * as integer and I could do a type cast when using them as well.
 * varName is NULL always and could be used for future expansion and to keep the
 * interface similar.
 * Returns: It returns an object of type redouble which is pushed onto the 
 * stack and used during the evaluation phase.
 * */
redouble makeConstv(double value,char* varName){
	redouble temp;
	temp.ref = traceptr;
	traceptr->val = value;
	traceptr->bar = 0.0;
	traceptr->operation = constv;
	traceptr->varName = varName;
	traceptr++;
	return temp;
}

/* Description: This function is used to accumulate the partial adjoints
 * that are calculated during the return trace phase. This function is called
 * by hshwalk() from Hashlib which in turn is called by evaluateFirstPartials.
 * For more information on Hashwalk, please refer to the hshusage.txt in
 * hashlib folder under root
 * Arguments: varItem - a pointer to object of type varMap 
 * 			  data - pointer to object of type firstPartials
 * Return: This function returns 0 for hashwalk to walk throught the next
 * item in varMap.
 * */
int sumEachAdjoint(void* varItem, void* data, void* extra){
	int i=0;
	elements* tracer = traceptr;
	*((((firstPartials*) data)->partials) + (((firstPartials*)data)->index)) = 0.0;
	*((((firstPartials*) data)->varName) + (((firstPartials*)data)->index)) = ((varMap*)(varItem))->key;
	while(--tracer > trace){
			if (tracer->operation == indepv){
				if((strcmp(((varMap*)(varItem))->key, tracer->varName))==0){
					*((((firstPartials*)data)->partials)+(((firstPartials*)data)->index)) += tracer->bar;
				}
			}
	}
	((firstPartials*)data)->index++;
	return 0;
}

/* Description: This function calls the hshwalk() which in turn calls the
 * sumEachAdjoint for each entry in the hashmap to generate accumulated 
 * value for adjoint of an independent variable.
 * Arguments: varTable - pointer to the table which contains the key,value
 * pair associated with each independent variable
 * Returns: the first partials of each independent variable in an object
 * of type firstPartials. 
 * */
firstPartials evaluateFirstPartials(hshtbl* varTable){
	hshtbl* tblToEvaluate = varTable;
	firstPartials firstPartialAdjoints;
	reverseSweep();
	firstPartialAdjoints.count = getNumberVariables(varTable);
	firstPartialAdjoints.partials = (double*) malloc(sizeof(double) * firstPartialAdjoints.count);
	firstPartialAdjoints.varName = (char**) malloc(sizeof(char)*firstPartialAdjoints.count);
	firstPartialAdjoints.index = 0;
	hshwalk(tblToEvaluate, sumEachAdjoint, &firstPartialAdjoints);
	return firstPartialAdjoints;
}


/* Description: This function deallocates the resources that have been 
 * allocated during the formation of the Equation Linked List.
 * This function has to be called after a call to hshkill()
 * Arguments: Accepts the head pointer pointing to the equation linked
 * list which is to be deallocated.
 * */

void killEquation(Equation* eqn){
	Equation* temp;
	temp = eqn;
	while(temp!=NULL){
		free(temp);
		eqn=(Equation*)eqn->next;
		temp=eqn;
	}
}



