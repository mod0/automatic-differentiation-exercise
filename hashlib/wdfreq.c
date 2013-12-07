/* Demonstration filter.
   This scans an input file for words, which consist of alpha-
   characters only, delimited by anything else.  Each word is
   converted into lower case, and entered into a database with
   an occurance count.  When all entry is complete the word
   list is formed and sorted, and the results dumped in order
   of number of occurances.

   The database is implemented through use of the hashlib
   package.  When loaded, a singly linked list is formed from
   the content by the hashwalk feature, and sorted with a
   mergesort.

   by C.B. Falconer, 2002-03-12
   Put in public domain.  Attribution appreciated.
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "hashlib.h"

#define MAXWD 72

/* ================================== */
/* Routines for text input and output */
/* ================================== */

/* 1------------------1 */

/* This is very likely to be non-portable */
/* DOES NOT check fp open for reading     */
/* NULL fp is considered a keyboard here! */
static int akeyboard(FILE *fp)
{
#ifndef __TURBOC__  /* Turbo C is peculiar */
# ifdef __STDC__
  /* This dirty operation allows gcc -ansi -pedantic */
  extern int fileno(FILE *fp);
  extern int isatty(int fn);
# endif
#endif
   return ((fp != NULL) && isatty(fileno(fp)));
} /* akeyboard */

/* 1------------------1 */

/* Get the next word consisting of alpha chars */
/* Return the terminating character (or EOF)   */
/* Downshift the acquired word                 */
static int nextword(FILE *f, char *buffer, int max)
{
   int i, ch;

   while ((!isalpha(ch = getc(f))) && (ch != EOF))
      continue;
   if (EOF != ch) {
      i = 0; max--;
      do {
         if (i < max) buffer[i++] = tolower(ch);
      } while (isalpha(ch = getc(f)));
      buffer[i] = '\0';    /* terminate string */
   }
   return ch;
} /* nextword */

/* ==================================== */
/* Routines to interface the hash table */
/* ==================================== */

/* This type stores individual words and their counts */
typedef struct wordlink {
   char            *word;   /* Points to the word itself */
   unsigned long    count;  /* of occurances */
   struct wordlink *next;   /* Used to create a list */
} wordlink, *wordlinkp;

/* ==================================================== */
/* The word storage facility.                           */
/* The following are the interface functions to hashlib */
/* NOTE: functions, not macros, as they are passed out  */
/* NOTE: We can pass out static functions in hshinit.   */
/* NOTE how void* pointers are converted to known types */
/* ==================================================== */

/* Notice this does a deep copy, with all fresh storage */
/* and that it zeroes the count field.                  */
static void * hwdupe(void * hwdin)
{
   wordlinkp item, hwd = hwdin;
   int       lgh;

   lgh = strlen(hwd->word) + 1;
   if ((item = malloc(sizeof *item))) {
      if ((item->word = malloc(lgh))) {
         strcpy(item->word, hwd->word);
         item->count = 0;
         item->next = NULL; /* on principle */
      }
      else {
         free(item);
         item = NULL;
      }
   }
   return item;
} /* hwdupe */

/* 1------------------1 */

/* Reverses the action of hwdupe above */
static void hwdundupe(void * hwdin)
{
   wordlinkp  hwd = hwdin;

   free(hwd->word);
   free(hwd);
} /* hwdundupe */

/* 1------------------1 */

/* The hash table only cares about equality      */
/* Reused in sorting, where comparison is needed */
static int hwdcmp(void *litem, void *ritem)
{
   wordlinkp li = litem,
             ri = ritem;

   return strcmp(li->word, ri->word);
} /* hwdcmp */

/* 1------------------1 */

static unsigned long hwdhash(void * hwd)
{
   wordlinkp hi = hwd;

   return hshstrhash(hi->word);
} /* hwdhash */

/* 1------------------1 */

static unsigned long hwdrehash(void * hwd)
{
   wordlinkp hi = hwd;

   return hshstrehash(hi->word);
} /* hwdrehash */

/* =============================================== */
/* End of auxiliaries for the word table           */
/* =============================================== */

/* 1------------------1 */

/* Stuff a copy into the database (hash table) */
/* returns a pointer to the stored item        */
/* NULL on failure.                            */
static void * store(char *wd, hshtbl *h)
{
   wordlink  wditem;
   wordlinkp wdstore;

   wditem.word = wd;
   if ((wdstore = hshinsert(h, &wditem))) {
      /* success, either old item or new item */
      wdstore->count++;   /* count occurances */
   }
   return wdstore;
} /* store */

/* 1------------------1 */

/* define a data type for the datum item in tablewalkfn */
typedef struct walkglobals {
   void *previtem;
} walkglobals;

/* This function is called for each item in the database
   during a hashwalk.  It can perform some operation on a
   data item.  It is passed the equivalent of global storage
   (for it) in datum.  It is only used in walking the
   complete stored database. It returns 0 for success.
   xtra will normally be NULL, but may be used for debug
   purposes.  During a database walk, the item parameter
   will never be NULL.  See hshexecfn in hashlib.h
*/
static int tablewalkfn(void *item, void *datum, void *xtra)
{
   walkglobals *global = datum;
   wordlinkp    wdlink = item;

   wdlink->next     = global->previtem;
   global->previtem = wdlink;
   return 0;  /* i.e. no error occured */
} /* tablewalkfn */

/* 1------------------1 */

/* together with tablewalkfn, this uses the 'next' fields
   of the stored items to form a singly linked list */
static wordlinkp formlist(hshtbl *h)
{
   walkglobals globs;
   int         err;

   globs.previtem = NULL;  /* start with end of list */

   /* Now we can scan all active items in the table */
   /* We are saving the err return for completeness */
   /* only.  tablewalkfn() never returns an error   */
   err = hshwalk(h, tablewalkfn, &globs);

   return globs.previtem;  /* now this is head of list */
} /* formlist */

/* 1------------------1 */

/* split list p into 2 nearly equal lists, return 2nd part */
static wordlinkp splitlist(wordlinkp p)
{
   wordlinkp p1, p2, p3;

   p1 = p2 = p3 = p;
   if (!p) return NULL;
   do {
      p3 = p2;
      p2 = p2->next;           /* advance 1 */
      p1 = p1->next;
      if (p1) p1 = p1->next;   /* advance 2 */
   } while (p1);

   /* now form new list after p2 */
   p3->next = NULL; /* terminate 1st half */
   return p2;
} /* splitlist */

/* 1------------------1 */

/* Merge two ordered lists into one */
static wordlinkp mergelists(wordlinkp p1, wordlinkp p2,
                            hshcmpfn cmp) /* compare */
{
   wordlink  n;
   wordlinkp p;

   p = &n;
   n.next = p;

   while (p1 && p2) {
      if (cmp(p1, p2) <= 0) {
         p->next = p1; p = p1; p1 = p1->next;
      }
      else {
         p->next = p2; p = p2; p2 = p2->next;
      }
   }
   /* at least one list now empty, copy other */
   /* one or both of these operations is null */
   if (p1) p->next = p1;
   if (p2) p->next = p2;

   /* check for empty lists */
   if (n.next == &n) return NULL;
   return n.next;
} /* mergelists */

/* 1------------------1 */

/* Recursively sort a linked list. The sort is stable */
/* This is an O(NlogN) process for all lists.         */
static wordlinkp mergesort(wordlinkp root, hshcmpfn cmp)
{
   wordlinkp p;

   if (root && root->next) {  /* 2 up items in list */
      p = splitlist(root);    /* alters list root */
      root = mergelists(mergesort(root, cmp),
                        mergesort(   p, cmp),
                        cmp);
   }
/* else the unit or empty list is already sorted */

   return root;
} /* mergesort */

/* 1------------------1 */

/* Compare the count fields, return +1, 0, -1 */
static int countcmp(void *litem, void *ritem)
{
   wordlinkp li = litem,
             ri = ritem;

   return (li->count < ri->count) - (li->count > ri->count);
} /* hwdcmp */

/* 1------------------1 */

int main(void)
{
   char     wdbuffer[MAXWD];
   hshtbl  *h = NULL; /* Stores only one copy of each word */
   hshstats hs;
   unsigned long wdcount, sigma;
   wordlinkp wds;     /* head of singly linked list */

   if (akeyboard(stdin)) {
      puts("Usage: wdfreq < inputfile > outputfile");
      puts(" collects all words in inputfile and outputs a");
      puts(" sorted (by frequency) list of words and the");
      puts(" frequency of their occurences, ignores case.\n");
      puts("Signal EOF to terminate (^D or ^Z usually)");
   }

   /* Create a table for storing words */
   if ((h = hshinit(hwdhash, hwdrehash,
                    hwdcmp,
                    hwdupe,  hwdundupe,
                    0))) {
      wdcount = 0;
      while (EOF != nextword(stdin, wdbuffer, MAXWD)) {
         if (!store(wdbuffer, h)) break;
         wdcount++;
      }
      /* Either EOF or we ran out of storage space */
      /* In either case we can collect no more data */

      /* collect some statistics, for curiosity */
      hs = hshstatus(h);
      if (hs.herror & (hshTBLFULL | hshNOMEM)) {
         puts("No more room in table or memory exhausted");
      }
      if (hs.herror & hshINTERR) {
         puts("Should not have happened, report a bug");
      }
      printf("%lu words, %lu entries, %lu probes, %lu misses\n",
              wdcount, hs.hentries, hs.probes, hs.misses);

      /* Make a singly linked list of the words */
      wds = formlist(h);

      /* For stability demo, this first sort uses
         the string comparison function for the hashtable */
      wds = mergesort(wds, hwdcmp);

      /* because the sort is stable, all items with
         the same count value will be in alpha order
         Note how the sort is controlled by cmp function */
      wds = mergesort(wds, countcmp);

      /* dump everything */
      sigma = 0;
      while (wds) {
         printf("%6lu %s\n", wds->count, wds->word);
         sigma += wds->count;
         wds = wds->next;
      }

      /* Sanity check */
      if (sigma != wdcount)
         printf("Fishy, wdcount = %lu, sigma = %lu\n",
                 wdcount, sigma);
   }
   /* For large tables and poor system free() functions
      the following statement can take a long time.  No
      problems should be encountered below 10000 or so
      items in the table.  Omitting it does no harm here
      because program exit will normally release it all */
   hshkill(h);   /* free all the storage */

   return 0;
} /* main */ /* wdfreq.c */
