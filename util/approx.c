/***************************************************************
 *
 * Fuzzy string searching subroutines
 *
 * Author:    John Rex
 * Date:      August, 1988
 * References: (1) Computer Algorithms, by Sara Baase
 *                 Addison-Wesley, 1988, pp 242-4.
 *             (2) Hall PAV, Dowling GR: "Approximate string matching",
 *                 ACM Computing Surveys, 12:381-402, 1980.
 *
 * Verified on:
 *    Datalite, DeSmet, Ecosoft, Lattice, MetaWare, MSC, Turbo, Watcom
 *
 * Compile time preprocessor switches:
 *    DEBUG - if defined, include test driver
 *
 * Usage:
 *
 *    char *pattern, *text;  - search for pattern in text
 *    int degree;      - degree of allowed mismatch
 *    char *start, *end;
 *    int howclose;
 *
 *    void App_init(pattern, text, degree);   - setup routine
 *    void App_next(&start, &end, &howclose); - find next match
 *
 *    - searching is done when App_next() returns start==NULL
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* local, static data */

static char *Text, *Pattern; /* pointers to search strings       */
static int Textloc;          /* current search position in Text  */
static int Plen;             /* length of Pattern                */
static int Degree;           /* max degree of allowed mismatch   */
static int *Ldiff, *Rdiff;   /* dynamic difference arrays        */
static int *pDyn;
static int *Loff,  *Roff;    /* used to calculate start of match */

void Fuzz_init(char *pattern, char *text, int degree)
{
      int i;

      /* save parameters */

      Text = text;
      Pattern = pattern;
      Degree = degree;

      /* initialize */

      Plen = strlen(pattern);
      Ldiff  = pDyn = (int *) malloc(sizeof(int) * (Plen + 1) * 4);
      Rdiff  = Ldiff + Plen + 1;
      Loff   = Rdiff + Plen + 1;
      Roff   = Loff +  Plen + 1;
      for (i = 0; i <= Plen; i++)
      {
            Rdiff[i] = i;   /* initial values for right-hand column */
            Roff[i]  = 1;
      }

      Textloc = -1; /* current offset into Text */
}

void Fuzz_next(char **start, char **end, int *howclose)
{
      int *temp, a, b, c, i;

      *start = NULL;
      while (*start == NULL)  /* start computing columns */
      {
            if (Text[++Textloc] == '\0') /* out of text to search! */
                  break;

            temp = Rdiff;   /* move right-hand column to left ... */
            Rdiff = Ldiff;  /* ... so that we can compute new ... */
            Ldiff = temp;   /* ... right-hand column */
            Rdiff[0] = 0;   /* top (boundary) row */

            temp = Roff;    /* and swap offset arrays, too */
            Roff = Loff;
            Loff = temp;
            Roff[1] = 0;

            for (i = 0; i < Plen; i++)    /* run through pattern */
            {
                  /* compute a, b, & c as the three adjacent cells ... */

                  if (Pattern[i] == Text[Textloc])
                        a = Ldiff[i];
                  else  a = Ldiff[i] + 1;
                  b = Ldiff[i+1] + 1;
                  c = Rdiff[i] + 1;

                  /* ... now pick minimum ... */

                  if (b < a)
                        a = b;
                  if (c < a)
                        a = c;

                  /* ... and store */

                  Rdiff[i+1] = a;
            }

            /* now update offset array */
            /* the values in the offset arrays are added to the
               current location to determine the beginning of the
               mismatched substring. (see text for details) */

            if (Plen > 1) for (i=2; i<=Plen; i++)
            {
                  if (Ldiff[i-1] < Rdiff[i])
                        Roff[i] = Loff[i-1] - 1;
                  else if (Rdiff[i-1] < Rdiff[i])
                        Roff[i] = Roff[i-1];
                  else if (Ldiff[i] < Rdiff[i])
                        Roff[i] = Loff[i] - 1;
                  else /* Ldiff[i-1] == Rdiff[i] */
                        Roff[i] = Loff[i-1] - 1;
            }

            /* now, do we have an approximate match? */

            if (Rdiff[Plen] <= Degree)    /* indeed so! */
            {
                  *end = Text + Textloc;
                  *start = *end + Roff[Plen];
                  *howclose = Rdiff[Plen];
            }
      }

      if (start == NULL) /* all done - free dynamic arrays */
      {
            free(pDyn);
            pDyn=NULL;
      }
}

void Fuzz_term(void)
{
   if (pDyn)
   {
         free(pDyn);
         pDyn=NULL;
   }
   return;
}
