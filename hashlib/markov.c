/* ---------------- file markov.c ---------------- */
/* Use hashlib to collect the words in a text file */

/* This demonstration program illustrates the use of hashlib for   */
/* data collection, storage, and manipulation.  Note that multiple */
/* hash tables are instantiated and controlled with the same code. */

/* Copyright (c) 2002 by C.B. Falconer.  All rights reserved.
   This program may be used, modified, sold etc. without
   restrictions provided only that the copyright is preserved

   Some ideas have been stolen from Kernighan & Pike's book
   "The Practice of Programming" on Markov chains.

   This program has been deliberately made to consider paragraph
   markers, and break its output into such.

   Although the hash functions here are using the string hashing
   functions provided in hashlib, this is not necessary.

   Note the very different use of the two hash tables.  One is
   used purely to handle the storage of individual words without
   any repetition.  Another is used to store the word pairs and
   a list of their suffixes, together with the length of that
   suffix list.  This makes for an easy randomization of the
   suffix output after any given prefix.

   The randomization is deliberately deterministic, since the
   purpose of this is to test the hashlib system.  To alter this
   call seedMT with some number (possibly derived from the time
   of day) at initialization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "hashlib.h"
#include "cokusmt.h"

enum {LINEMAX   = 72,      /* on output */
      MAXWD     = 72,      /* must be <= LINEMAX */
      PREFSZ    = 2,       /* words in a prefix */
      MAXOUTwds = 1000     /* default output words */
     };

/* this stores all unique words in the input as strings
   A word is anything delimited by white space
   The typedef is unused, since this table is simple */
typedef char wditem, *wditemp;

/* this stores the word following some word pair */
typedef struct listlink {
   char            *word;
   struct listlink *next;
} listlink, *listlinkp;

/* This stores the word pairs seen in the input */
typedef struct markov {
   char    * prefix[PREFSZ];
   int       count;   /* length of the suffix chain */
   listlinkp suffix;
} markov, *markovp;

/* all the above items are handled by hashlib operations */

/* 1------------------1 */

static void usage(char *prgname)
{
   fprintf(stderr, "Usage: %s filename [maxout [maxin [variants]]]\n"
           "  where maxout and maxin default to 0, i.e. unlimited\n"
           "  odd values of maxin cause the input to echo in words\n"
           "  odd values of maxout suppress the actual output\n"
           "  However the default output limit is %d words\n"
           "ANY entry for variants causes the output to be randomized\n"
           "  an odd value for variants suppresses performance data\n"
           "  a numerical value generates that many variations\n"

           "\nAll output is to stdout\n"

           "\nCopyright (c) 2002 by C.B. Falconer\n"
           "  released under GPL license\n",
           prgname, MAXOUTwds);
   exit(EXIT_FAILURE);
} /* usage */

/* 1------------------1 */

static void error(char *why)
{
   fprintf(stderr, "%s\n", why);
   exit(EXIT_FAILURE);
} /* error */

/* ================================== */
/* Routines for text input and output */
/* ================================== */

static void skipblanks(FILE *f)
{
   int ch;

   while ( (' ' == (ch = getc(f))) || ('\t' == ch) ||
           ('\v' == ch) || ('\f' == ch) || ('\a' == ch) )
      continue;
   ungetc(ch, f);
} /* skipblanks */

/* 1------------------1 */

/* The file is assumed to hold no control chars */
/* other than \n \t \v \a and \f.  A blank line */
/* marks a paragraph ending word                */
static int nextword(FILE *f, char *buffer, int max)
{
   int i, ch;

   skipblanks(f);
   if (EOF == (ch = getc(f))) return 0;

   /* Detect paragraph endings as \n\n */
   if ('\n' == ch) {
      skipblanks(f); ch = getc(f);
      if ('\n' == ch) {            /* paragraph ending */
         buffer[0] = buffer[1] = ch;    /* wd = "\n\n" */
         buffer[2] = '\0';
         /* now we have to absorb any more blank lines */
         do {
            skipblanks(f); ch = getc(f);
         } while ('\n' == ch);
         ungetc(ch, f);
         return 1;
      }
   }
   /* now ch holds the first non-blank.  Use all printable */
   if (EOF == ch) return 0;
   if (!isgraph(ch)) {
      fprintf(stderr, "'%c', 0x%x WARN: Invalid character\n", ch, ch);
   }

   i = 0;
   do {
      buffer[i++] = ch;
      ch = getc(f);
      if (i >= max) {   /* truncate over long words */
         i--;
         break;         /* leaving ch for next word */
      }
   } while (isgraph(ch));

   ungetc(ch, f);       /* save for next word, may be \n */
   buffer[i] = '\0';    /* terminate string */
   return 1;
} /* nextword */

/* 1------------------1 */

/* dump words wrapping lines as needed to stdout */
/* \n ONLY occurs in words that mark paragraphs  */
static void write(char * buf)
{
   static int linelgh;
   size_t     wdlgh;

   if ('\n' == buf[0]) {  /* Only in \n\n para. marks */
      linelgh = 0;
      printf("%s", buf);
   }
   else {
      wdlgh = strlen(buf) + 1;  /* allow for blank */
      if ((linelgh + wdlgh) > LINEMAX) {
         printf("\n"); linelgh = 0;
      }
      printf("%s ", buf);
      linelgh += wdlgh;
   }
} /* write */

/* ==================================================== */
/* The word storage facility.                           */
/* The following are the interface functions to hashlib */
/* Without added fields char* pointers can be used here.*/
/* NOTE: functions, not macros, as they are passed out  */
/* NOTE: We can pass out static functions in hshinit.   */
/* ==================================================== */

static void * hwdupe(void * hwdin)
{
   char   *item, *hwd = hwdin;
   int     lgh;

   lgh = strlen(hwd) + 1;
   if ((item = malloc(lgh))) {
      strcpy(item, hwd);
      return item;
   }
   return NULL;
} /* hwdupe */

/* 1------------------1 */

static void hwdundupe(void * hwdin)
{
   char  *hwd = hwdin;

   free(hwd);
} /* hwdundupe */

/* 1------------------1 */

static int hwdcmp(void *litem, void *ritem)
{
   char * li = litem,
        * ri = ritem;

   return strcmp(li, ri);
} /* hwdcmp */

/* 1------------------1 */

static unsigned long hwdhash(void * hwd)
{
   char * hi = hwd;

   return hshstrhash(hi);
} /* hwdhash */

/* 1------------------1 */

static unsigned long hwdrehash(void * hwd)
{
   char * hi = hwd;

   return hshstrehash(hi);
} /* hwdrehash */

/* =============================================== */
/* End of auxiliaries for the word table           */
/* Now define the auxiliaries for the Markov table */
/* They illustrate adding fields to the basic item.*/
/* =============================================== */

/* Note that the prefix pointer storage is */
/* under the control of a different table. */
static void * hmkdupe(void * hmkin)
{
   markov  *item, *hmk = hmkin;

   if ((item = malloc(sizeof *item)))
      *item = *hmk;   /* this is a struct, so we can do this */
   return item;
} /* hmkdupe */

/* 1------------------1 */

/* here we have to see if any links have been added */
/* i.e. reverse the accumulated effect of store()   */
static void hmkundupe(void * hmkin)
{
   markov   *hmk = hmkin;
   listlink *next, *prev;

   next = hmk->suffix;
   while (next) {
      prev = next->next;
      free(next);
      next = prev;
   }
   free(hmk);
} /* hmkundupe */

/* 1------------------1 */

/* Here we treat the lower order prefixes as more significant */
static int hmkcmp(void *litem, void *ritem)
{
   markov * li = litem,
          * ri = ritem;
   int      i, cmp = 0;

   for (i = 0; i < PREFSZ; i++)
      if ((cmp = strcmp(li->prefix[i], ri->prefix[i])))
         break;
   return cmp;
} /* hmkcmp */

/* 1------------------1 */

/* Resist any temptation to hash on the pointer value proper */
/* Any conversion to an integral object is non-portable      */
/* We will be very simple and just sum the precooked strhash */
static unsigned long hmkhash(void * hmk)
{
   markov      * mi = hmk;
   unsigned long hsv;
   int           i;

   hsv = 0;
   for (i = 0; i < PREFSZ; i++)
      hsv += hshstrhash(mi->prefix[i]);

   return hsv;
} /* hmkhash */

/* 1------------------1 */

static unsigned long hmkrehash(void * hmk)
{
   markov      * mi = hmk;
   unsigned long hsv;
   int           i;

   hsv = 0;
   for (i = 0; i < PREFSZ; i++)
      hsv += hshstrehash(mi->prefix[i]);

   return hsv;
} /* hmkrehash */

/* ======================================== */
/* End of auxiliaries for the Markov table  */
/* End of all hashlib table auxiliaries     */
/* ======================================== */

/* This forms the i/o buffer for creating the Markov Chain    */
/* markov is initialized so that the chain options count is 0 */
/* and the list of possible suffixes is empty.  It is only    */
/* copied into markovtbl storage if it is a new prefix entry  */
/* Note that prefix values must be those stored in wdtable or */
/* the special "can't happen" NONWORD value below.            */
static char* NONWORD = "\n";
markov       prefixes;

/* 1------------------1 */

/* Revise the prefix for next entry */
static void reviseprefixes(char *item)
{
   int i;

   for (i = 1; i < PREFSZ; i++)
      prefixes.prefix[i-1] = prefixes.prefix[i];
   prefixes.prefix[PREFSZ-1] = item;
} /* reviseprefixes */

/* 1------------------1 */

/* Store buf content in wdtbl, and as a suffix in markovtbl */
/* Revise prefixes to discard the eldest and add buf.       */
static void store(char *buf, int echo,
                  hshtbl *wdtbl, hshtbl *markovtbl)
{
   char     *hitem;
   markov   *mitem;
   listlink *litem;

   if (!(hitem = hshinsert(wdtbl, buf)))
      error("Memory full, aborting");
   if (echo) write(hitem);      /* echo the input, wrapped */

   /* Look up or create the prefix entry, append hitem as suffix */
   if (!(mitem = hshinsert(markovtbl, &prefixes)))
      error("Memory full, aborting");
   else {  /* append hitem as suffix */
      if ((litem = malloc(sizeof *litem))) {
         litem->word = hitem;
         litem->next = mitem->suffix;
         mitem->count++;
         mitem->suffix = litem;
      }
      else error("Memory full, aborting");
   }
   reviseprefixes(hitem);
} /* store */

/* 1------------------1 */

/* initialize prefixes */
static void initprefix(void)
{
   int i;

   for (i = 0; i < PREFSZ; i++) prefixes.prefix[i] = NONWORD;
   prefixes.count = 0;
   prefixes.suffix = NULL;
} /* initprefix */

/* 1------------------1 */

/* show prefixes (not used any more) */
static void showprefix(void)
{
   int i;

   for (i = 0; i < PREFSZ; i++) write(prefixes.prefix[i]);
   putc('\n', stdout);
} /* showprefix */

/* 1------------------1 */

/* After all that, we can make the scrambled output file */
/* This returns the count of words actually output.      */
static unsigned long dump(hshtbl *markovtbl, unsigned long maxout)
{
   unsigned long  wdsout;
   markov        *mitem;
   unsigned long  random;
   listlink      *litem;

   if (0 == maxout) maxout = MAXOUTwds;
   initprefix();
   wdsout = 0;
   if (!(maxout & 1)) write(NONWORD);  /* to clear line size */

   do {
      if (!(mitem = hshfind(markovtbl, &prefixes))) break;
      if (0 == mitem->count) break;          /* Can't happen */
      random = randomMT() % mitem->count;           /* crude */
      litem = mitem->suffix;
      while (random) {
         random--;
         litem = litem->next;
      }
      if (!(maxout & 1)) write(litem->word);
      wdsout++;
      reviseprefixes(litem->word);
   } while (wdsout < maxout);
   if (!(maxout & 1) && (*litem->word != '\n'))
      write(NONWORD);               /* append final \n */
   return wdsout;
} /* dump */

/* 1-------- Table walking coding ----------1 */

#define HISTMAX 9

/* This type accumulates data over the whole table */
typedef struct histogram {
   int     ixsz;
   int     sizes[HISTMAX+1];
   int     maxsize;
   markovp maxitem;
} histogram, *histogramptr;

/* 1------------------1 */

/* Scan the distribution of count fields   */
/* This gets executed once per table entry */
static int getcounts(void *stored, void *datum, void *xtra)
{

   markovp      mkp   = stored;
   histogramptr hgptr = datum;

   if (mkp->count > HISTMAX) ++(hgptr->sizes[HISTMAX]);
   else                      ++(hgptr->sizes[mkp->count]);

   if (mkp->count >= hgptr->maxsize) {
      hgptr->maxsize = mkp->count;
      hgptr->maxitem = mkp;
   }
   return 0;
} /* getcounts */

/* 1------------------1 */

static void clearhistogram(histogramptr hgp)
{
   int i;

   for (i = 0; i <= HISTMAX; i++) {
      hgp->sizes[i] = 0;
   }
   hgp->ixsz = HISTMAX;
   hgp->maxsize = 0;
   hgp->maxitem = NULL;
} /* clearhistogram */

/* 1------------------1 */

static void showchains(histogramptr hgp)
{
   int i;

   for (i = 0; i < HISTMAX; i++) {
      printf("%8d suffix chains are %d long\n",
             hgp->sizes[i], i);
   }
   /* leaving i == HISTMAX */
   printf("%8d suffix chains are %d or more long\n",
           hgp->sizes[i], i);
   printf("Maximum chain length is %d", hgp->maxsize);
   if (hgp->maxitem)
      printf(" with prefix \"%s %s \"",
           hgp->maxitem->prefix[0], hgp->maxitem->prefix[1]);
   printf("\n");
} /* showchains */

/* 1-------- End Table walk coding ----------1 */

int main(int argc, char* *argv)
{
   FILE    *f;
   char     wdbuffer[MAXWD];
   hshtbl  *h = NULL; /* this table stores one copy of each word */
   hshtbl  *m = NULL; /* this table stores the Markov chains */
   hshstats hs;
   unsigned long param2, param3, param4, seed, wdcount, copies;
   histogram dtag;

   param2 = param3 = param4 = 0;
   if (argc > 2) param2 = strtoul(argv[2], NULL, 0);
   if (argc > 3) param3 = strtoul(argv[3], NULL, 0);
   if (argc > 4) {
      param4 = strtoul(argv[4], NULL, 0);
      seed = time(NULL);
      seedMT(seed);
   }

   if (argc < 2) usage(argv[0]);                  /* exits */
   else if (NULL == (f = fopen(argv[1], "r"))) {
      printf("Can't open %s\n", argv[1]);
      usage(argv[0]);                             /* exits */
   }
   else if (  /* table for storing words */
              (h = hshinit(hwdhash, hwdrehash,
                           hwdcmp,
                           hwdupe,  hwdundupe,
                           0))
           &&  /* table for storing prefixes and chains */
              (m = hshinit(hmkhash, hmkrehash,
                           hmkcmp,
                           hmkdupe, hmkundupe,
                           0)) ) {
      wdcount  = 0;
      initprefix();

      /* Read input and make the tables */
      while (nextword(f, wdbuffer, sizeof wdbuffer)) {
         wdcount++;
         store(wdbuffer, (param3 & 1), h, m);
         if (param3 && (wdcount >= param3)) break;
      }
      hs = hshstatus(h);
      if (!(param4 & 1))
         printf("\n%lu words captured, %lu unique, "
                "%lu probes, %lu misses\n",
                wdcount, hs.hentries, hs.probes, hs.misses);

      hs = hshstatus(m);
      if (!(param4 & 1))
         printf("%lu unique prefixes captured, "
                "%lu probes, %lu misses\n",
                hs.hentries, hs.probes, hs.misses);

      clearhistogram(&dtag);
      (void)hshwalk(m, getcounts, &dtag);
      if (!(param4 & 1))
         showchains(&dtag);

      if (param4) copies = param4;
      else copies = 1;

      /* Create the distorted output(s) */
      do {
         wdcount = dump(m, param2);

         hs = hshstatus(m);
         copies--;
         if (!(param4 & 1))
            printf("\n%lu words output, %lu unique prefixes, "
                   "%lu probes, %lu misses\n",
                   wdcount, hs.hentries, hs.probes, hs.misses);
         if (copies)
            printf("\n========= next version =========\n");
      } while (copies);

      /* Clean up */
if (0) {            /* let os do it, this can be slow!! */
      hshkill(m); hshkill(h);   /* Note: m depends on h */
}
      return 0;   /* all went well */
   }

if (0) {            /* let os do it, this can be slow!! */
   hshkill(m); hshkill(h);
}
   return EXIT_FAILURE;
} /* main */
/* -------------- end markov.c ---------------*/
