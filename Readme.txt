Please pardon any inaccuracies, it was written a long long time ago (about ~5 years)


SCALABILTY TO NEW FUNCTIONS
----------------------------

The code is wholly scalable to adding new functions. There are 
only 4 places where we have to add new code to support more functions. 
The old code need not be disturbed.
Those places are 
	1) Add an opcode for the function in the enum
	2) Add code to return the appropriate opcode under getType()
	3) Add code under the function evaluate to pop appropriate number of
	arguments from the stack and perform the operation and push it back
	to the stack.
	4) Add code under reverseSweep function to calculate adjoints 
	appropriately.


SCALABILITY TO CALCULATING HIGHER DERIVATIVES
----------------------------------------------

I have not got time to think on those lines as the end of my semester is 
approaching and I am quite loaded with assignments and submissions.
There may be a few changes to the underlying structures. 


WHERE THE CODE MIGHT BE SLOW
----------------------------

The code might be slow near string comparisons and to obviate this, I have
used a hashmap for storing and retrieving variables and their values.
I dint store the values of variables in the equation itself because it
will result in unnecessary search scans. Besides the Hash library that has
been used promises O(1) storage and retrieval and can easily scale to 
100,000 entries.

Also I have used enumerated constants to identify tokens for faster 
comparisons during evaluation phase and obviate further string comparisons
to identify tokens.


EXPERIMENTS RUN TO TEST MY THEORY
---------------------------------

I ran profiling tests on the sample input files given on the website. 
But the results show no significant details which I could use to optimize
the code. I would have to run the code with bigger functions and with 
more number of independent variables so that significant data on where 
CPU cycles are spent would be available based on which code optimizations
can be done.

The function evaluation and the 1st derivatives and the gprof output are
shown below for the input function given on the wiki website with the 
1st set of variable values as operating point.

mahesh@mahesh-desktop:~/Desktop/GSOC$ ./a.out eqn.txt vars.txt 

Function Value: -10.044704
1st Derivative of F wrt f = 4.169356
1st Derivative of F wrt a = 1.000000
1st Derivative of F wrt b = 1.000000
1st Derivative of F wrt c = -9.300000
1st Derivative of F wrt d = -1.100000
1st Derivative of F wrt e = -0.924207

mahesh@mahesh-desktop:~/Desktop/GSOC$ gprof
Flat profile:

Each sample counts as 0.01 seconds.
 no time accumulated

  %   cumulative   self              self     total           
 time   seconds   seconds    calls  Ts/call  Ts/call  name    
  0.00      0.00     0.00       18     0.00     0.00  varHash
  0.00      0.00     0.00       16     0.00     0.00  getType
  0.00      0.00     0.00       16     0.00     0.00  push
  0.00      0.00     0.00       15     0.00     0.00  pop
  0.00      0.00     0.00       12     0.00     0.00  varCmp
  0.00      0.00     0.00        6     0.00     0.00  makeIndepv
  0.00      0.00     0.00        6     0.00     0.00  sumEachAdjoint
  0.00      0.00     0.00        6     0.00     0.00  varDup
  0.00      0.00     0.00        6     0.00     0.00  varFree
  0.00      0.00     0.00        2     0.00     0.00  makeConstv
  0.00      0.00     0.00        1     0.00     0.00  createStack
  0.00      0.00     0.00        1     0.00     0.00  evaluate
  0.00      0.00     0.00        1     0.00     0.00  evaluateFirstPartials
  0.00      0.00     0.00        1     0.00     0.00  getNewTable
  0.00      0.00     0.00        1     0.00     0.00  getNumberVariables
  0.00      0.00     0.00        1     0.00     0.00  readEquation
  0.00      0.00     0.00        1     0.00     0.00  readVariables
  0.00      0.00     0.00        1     0.00     0.00  reverseSweep

 %         the percentage of the total running time of the
time       program used by this function.

cumulative a running sum of the number of seconds accounted
 seconds   for by this function and those listed above it.

 self      the number of seconds accounted for by this
seconds    function alone.  This is the major sort for this
           listing.

calls      the number of times this function was invoked, if
           this function is profiled, else blank.
 
 self      the average number of milliseconds spent in this
ms/call    function per call, if this function is profiled,
           else blank.

 total     the average number of milliseconds spent in this
ms/call    function and its descendents per call, if this 
           function is profiled, else blank.

name       the name of the function.  This is the minor sort
           for this listing. The index shows the location of
           the function in the gprof listing. If the index is
           in parenthesis it shows where it would appear in
           the gprof listing if it were to be printed.
           
 If we look at the detailed call stack of the program, one can observe
 that maximum time is spen in the hashing algorithms and that spent in
 function evaluation, reverse sweep and accumulation of partial adjoints
 is substantially less.


 DIVIDE BY ZERO AND OTHER EXCEPTION HANDLING
 -------------------------------------------
 
 A very elementary error handling has been done. 
 
 On DIVIDE BY ZERO - action taken
 					 print Error message and exit(1)
 
 Vaiables that are in the postfix notation but not given a value in the
 2nd file are automatically initialized to 0.0
 
 Variables that are not in the postfix notation but present in the 2nd
 file are silently dropped.
 
 Elementary File Exceptions have been handled.
 
 
 FIXED STACK SIZE
 -----------------
 
 A fixed stack size has been assumed to speed up pushing and poping
 operations but an arbitrarily long one can be implemented with linked 
 lists (still limited by memory).
 
 The same applies to FIXED TRACE SIZE. 
 
 This solution is sufficient from low to medium sized problems.
 
 
 MEMORY ALLOCATION AND DEALLOCATION
 ----------------------------------
 
 The valgrind memcheck tool can be used to check for memory leaks. And
 using the leak information generated by valgrind tool, calls to free 
 can be issued suitably.
 
 Apart from this, places where memory deallocation is obvious has to be 
 done.
 
 In my code, I have run valgrind and fixed 2 leaks. I am pasting a short
 summary of each before fixing the leaks and after.
 
 Before fixing leak 1 and leak 2.
 --------------------------------
 
==11338== LEAK SUMMARY:
==11338==    definitely lost: 654 bytes in 16 blocks.
==11338==    indirectly lost: 380 bytes in 25 blocks.
==11338==      possibly lost: 140 bytes in 7 blocks.
==11338==    still reachable: 10,946 bytes in 1 blocks.
==11338==         suppressed: 0 bytes in 0 blocks.

 
 After fixing leak 1, before fixing leak 2
 -----------------------------------------
 
==11482== LEAK SUMMARY:
==11482==    definitely lost: 250 bytes in 15 blocks.
==11482==    indirectly lost: 380 bytes in 25 blocks.
==11482==      possibly lost: 140 bytes in 7 blocks.
==11482==    still reachable: 10,974 bytes in 1 blocks.
==11482==         suppressed: 0 bytes in 0 blocks.

 
 After fixing both the leaks
 ---------------------------
 
==11914== LEAK SUMMARY:
==11914==    definitely lost: 438 bytes in 24 blocks.
==11914==      possibly lost: 140 bytes in 7 blocks.
==11914==    still reachable: 10,974 bytes in 1 blocks.
==11914==         suppressed: 0 bytes in 0 blocks.

There are few other places where leaks are to be fixed. But surprisingly
after fixing the 2nd leak, the definitely lost count has increased.
 



 CODE IS COMPLETE IN A WAY 
 -------------------------

 * TASK 1 ^/
 * TASK 2 ^/
 * TASK 3 ^/

 Its tested and working for the subset of operations specified by the
 Differentiation Exercise in the wiki pages. 
 
 
 There is one possible condition where segmentation fault can occur
 -  unnecessary characters  in the input files and an extra new line at
 the end of file (1+1=2). I have not bothered about these constraints as
 the input files are computer generated  and will have a specifc format
 which can be implemented during parsing.
 
 
 PS: I have not implemented functions to trim strings before parsing. It 
 can be easily implemented.
