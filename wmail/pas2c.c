/* PAS2C.C,
   Convert Pascal string to C string */

char *Pas2C(char *pchCString, const char *pchPascal, int sizeDest)
{
   int sizeSource = *pchPascal;
   int i=0;
   char *pchDest = pchCString;
   const char *pchSrc = pchPascal+1;

   while (i < (sizeDest-1) &&
          i < sizeSource)
   {
      *pchDest++ = *pchSrc++;
      i++;
   }
   if (sizeDest)
      *pchDest=0;

   return pchCString;
}
