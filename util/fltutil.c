/*---------------------------------------------------------------------------+
 | Titel: FLTUTIL.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 21.02.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Utility-Funktionen fuer FleetStreet                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#include <os2.h>
#include <string.h>
#include <ctype.h>

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaInAreaSet                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Prueft, ob ein Area-Tag in einer Aufzaehlung von Areas      */
/*               vorhanden ist. Die Area-Tags sind durch Leerzeichen         */
/*               getrennt.                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchAreaSet: Aufzaehlung von Areas                              */
/*            pchAreaTag: gesuchte Area                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE    ist vorhanden                                      */
/*                FALSE   ist nicht vorhanden                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL AreaInAreaSet(const char *pchAreaSet, const char *pchAreaTag)
{
   char *pchTemp;

   if (!pchAreaSet)
      return FALSE;

   pchTemp = strstr(pchAreaSet, pchAreaTag); /* Suchen */

   if (pchTemp)
   {
      /* nach gef. Tag muá Leerzeichen oder \0 stehen, sonst wurde nur ein
         Teilstring des Area-Tag gefunden */
      ULONG ulLen = strlen(pchAreaTag);
      if (pchTemp[ulLen] != ' ' &&
          pchTemp[ulLen] != '\0')
         return FALSE;
      else
      {
         if (pchTemp != pchAreaSet) /* nicht am Anfang */
             if (*(pchTemp-1) != ' ')
                return FALSE;

         return TRUE;
      }
   }
   else
      return FALSE;
}

/* stristr, sucht pNeedle in pHaystack, wie strstr, nur case-insensitive */

char *stristr(const char *pHaystack, const char *pNeedle)
{
   while (*pHaystack)
   {
      const char *pComp1 = pHaystack;
      const char *pComp2 = pNeedle;

      while (*pComp1 && *pComp2) /* Teilstrings vergleichen */
      {
         if (toupper(*pComp1) != toupper(*pComp2))
            break;
         pComp1++;
         pComp2++;
      }

      if (*pComp2 == '\0') /* Needle am Ende */
         return (char*) pHaystack;

      pHaystack++;
   }
   return NULL;
}

/*------------------------------ StripWhitespace ----------------------------*/
/* Entfernt Leerzeichen, Tabs und Newlines am Anfang und Ende eines Strings  */
/*---------------------------------------------------------------------------*/

char *StripWhitespace(char *string)
{
   char *pchAnf=string;
   char *pchStart=string;

   /* Whitespace am Anfang uebergehen */
   while (*pchAnf == ' ' ||
          *pchAnf == '\t')
     pchAnf++;

   /* Weiter bis zum Ende oder Kommentar */
   while (*pchAnf &&
          *pchAnf != '\n' &&
          *pchAnf != ';')
   {
      /* nach vorne kopieren */
      *pchStart++ = *pchAnf;

      /* Wenn Whitespace kopiert, folgende Whitespaces ueberlesen */
      if (*pchAnf == ' ' ||
          *pchAnf == '\t')
      {
         pchAnf++;
         while (*pchAnf == ' ' ||
                *pchAnf == '\t')
            pchAnf++;
      }
      else
         pchAnf++;
   }
   *pchStart = 0;
   if (pchStart > string && *(pchStart-1) == ' ') /* letztes Leerzeichen l”schen */
      *(pchStart-1) = 0;

   return string;
}

/*------------------------------ RemoveBackslash ----------------------------*/
/* Entfernt einen Backslash am Ende einer Pfadangabe                         */
/*---------------------------------------------------------------------------*/

char *RemoveBackslash(char *pchPath)
{
   int i;

   i=strlen(pchPath)-1;

   if (pchPath[i] == '\\')
      pchPath[i] = '\0';

   return pchPath;
}

/*------------------------------ StripRe       ------------------------------*/
/* Loescht "Re:" u. dgl. am Anfang des Strings                               */
/*---------------------------------------------------------------------------*/

char *StripRe(char *pchString)
{
   char *src=NULL;
   char *dest=NULL;

   while (pchString && *pchString) /* Alle Re's abraeumen */
   {
      src=NULL;

      if (!strnicmp(pchString, "Re:", 3))
      {
         src=pchString+2;

         /* Anfang des Topics suchen */
         while (*src && (*src==' ' || *src==':'))
            src++;
      }
      else
         if (!strnicmp(pchString, "Re^",3) && (src=strchr(pchString, ':')))
         {
            /* Es liegt ein "Re^xyz:" vor. Jetzt den ersten Buchstaben
               des eigentlichen Subjects suchen */
            while (*src && (*src==' ' || *src==':'))
               src++;
         }

      if (src)
      {
         /* String nach vorne rutschen; strcpy nicht verwenden, da evtl.
            Source <-> Dest-Konflikt */
         dest=pchString;
         while (*src)
            *dest++=*src++;
         *dest=*src;
      }
      else
         break;
   }

   /* Fuehrende Leerzeichen entfernen */
   src=pchString;

   while (*src == ' ')
      src++;

   if (src != pchString)
   {
      dest=pchString;
      while (*src)
         *dest++=*src++;
      *dest=*src;
   }

   return pchString;
}

/*-------------------------------- Modulende --------------------------------*/

