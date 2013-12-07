# hashlib makefile, works with runtest.bat for regression tests
# This makefile is tested under W98 and DJGPP 2.03/gcc 2.953

objects = hashlib.o cokusmt.o
sources = hashtest.c hashlib.c hashlib.h \
          cokusmt.c cokusmt.h markov.c \
          wdfreq.c hashlib.lst makefile
utils = xref.exe runtests.bat gpl.txt readme.txt
runtests = test1.txt test2.txt test3a.txt test3.txt \
           test4.txt test4a.txt test4b.txt test5.txt \
           test6.txt test7.txt markov.txt

# Set DEBUG=-DNDEBUG to inhibit pointer printouts
DEBUG = 
CFLAGS = -W -Wall -ansi -pedantic -O2 -gstabs+ $(DEBUG)

hashtest.exe : $(objects) hashtest.o
	gcc -o hashtest.exe hashtest.o $(objects)
	
markov.exe : $(objects) markov.o 
	gcc -o markov.exe $(objects) markov.o
	
wdfreq.exe : hashlib.o wdfreq.o
	gcc -o wdfreq.exe wdfreq.o hashlib.o
	
hshtstm.exe : hshtstm.o cokusmt.o hashlib.o malloc.o malldbg.o
	gcc -o hshtstm.exe hshtstm.o $(objects) malloc.o malldbg.o
	
# this shows up all use of magic numbers.  Not for cygwin
hashlib.xrf : hashlib.lst hashlib.c hashlib.h hashtest.c
	xref -cfn9 hashlib.lst hashlib.xrf

hashtest.o : hashtest.c cokusmt.h hashlib.h
hashlib.o  : hashlib.c hashlib.h
cokusmt.o  : cokusmt.c cokusmt.h
markov.o   : markov.c cokusmt.h hashlib.h
wdfreq.o   : wdfreq.c hashlib.h
hshtstm.o  : hashtest.c cokusmt.h hashlib.h malldbg.h sysquery.h
	gcc $(CFLAGS) -o hshtstm.o -DMALLDBG -c hashtest.c

# ######################################################
# Stuff for tracking free problems in DJGPP library ONLY
malloc.o   : malloc.c
	*gcc -W -Wall -O2 -pg -gstabs+ -Dinline= -c malloc.c

hshtestp.exe : malloc.o $(objects) hashtest.o
	gcc -pg -o hshtestp.exe hashtest.o cokusMT.o hashlib.o malloc.o

hshtestp : hshtestp.exe
# END of tracking stuff
# #####################

# pack it all up for shipping
hashlib.zip   : $(sources) $(utils) $(runtests)
	zip -o -u hashlib.zip $(sources) $(utils) $(runtests)

.PHONY : zip all xrf hashtest hshtestp markov wdfreq \
         clean build hshtstm

zip   : hashlib.zip

all   : hashtest markov wdfreq

xrf   : hashlib.xrf

hashtest : hashtest.exe

hshtestp : hshtestp.exe

markov : markov.exe

wdfreq : wdfreq.exe

hshtstm : hshtstm.exe

clean :
	rm -f hashtest.exe hshtestp.exe markov.exe wdfreq.exe \
	$(objects) hashtest.o markov.o wdfreq.o \
	hashlib.xrf hshtstm.o hshtstm.exe

# Used to build with NDEBUG set.
# This eliminates system specific output, such as pointer values
build :
	rm -f $(objects) markov.o hashtest.o
	make hashtest.exe DEBUG=-DNDEBUG
	make markov.exe DEBUG=-DNDEBUG
	echo Test it all by executing "runtests"
