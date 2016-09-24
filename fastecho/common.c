/*---------------------------------------------------------------------------+
 | Titel: COMMON.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 01.07.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    FASTECHO.CFG: Gemeinsame Funktionen aller Revisions                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#pragma strings(readonly)

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "../main.h"
#include "../structs.h"
#include "..\msgheader.h"
#include "..\areaman\areaman.h"
#include "..\util\addrcnv.h"

#include "common.h"

/*--------------------------------- Defines ---------------------------------*/

#define NM_TAG    "NETMAIL"
#define NM_DESC   "Netmail"

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

/*-------------------------------- Modulende --------------------------------*/

void AddNMArea(char *pchPath, PAREALIST pRetList, char *pchUser, PFTNADDRESS pAddress)
{
   AREADEFOPT AreaDef;

   if (pchPath[0])
   {
      memset(&AreaDef, 0, sizeof(AreaDef));

      strcpy(AreaDef.areatag, NM_TAG);
      strcpy(AreaDef.areadesc, NM_DESC);

      CopyAreaPath(AreaDef.pathfile, pchPath);
      strcpy(AreaDef.username, pchUser);
      NetAddrToString(AreaDef.address, pAddress);
      AreaDef.areaformat = AREAFORMAT_FTS;
      AreaDef.areatype = AREATYPE_NET;
      AreaDef.ulAreaOpt = AREAOPT_FROMCFG;
      AreaDef.ulDefAttrib= ATTRIB_PRIVATE;

      AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
   }
   return;
}

/* Kopiert Area-Pfad, ersetzt Environment-Variablen */

void CopyAreaPath(char *pDest, char *pSrc)
{
   char Env[256];
   char *pEnv;
   int i;

   while (*pSrc)
   {
      if (*pSrc == '%') /* Anfang Environment */
      {
         /* Ende suchen (weiteres '%' oder Ende) */
         ++pSrc;
         i=0;
         while (*pSrc && *pSrc != '%')
            Env[i++] = *pSrc++;
         Env[i]='\0';

         if (*pSrc) /* '%' */
            pSrc++;

         pEnv = getenv(Env);
         if (pEnv)
            /* kopieren */
            while (*pEnv)
               *pDest++ = *pEnv++;
      }
      else
         *pDest ++ = *pSrc++; /* einfaches Zeichen kopieren */
   }
   *pDest = '\0';

   return;
}
