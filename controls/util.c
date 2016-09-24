/*---------------------------------------------------------------------------+
 | Titel: UTIL.C                                                             |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 30.07.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Utility-Funktionen f. Message-Viewer                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include "viewer_int.h"
#include "util.h"

/*--------------------------------- Defines ---------------------------------*/

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
 | Rckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void FreeLines(PVIEWERPARAMS pViewerParams)
{
   if (pViewerParams->pLines)
   {
      PVIEWERLINE pTemp = pViewerParams->pLines[0];
      PVIEWERLINE pTemp2;

      while (pTemp)
      {
         pTemp2 = pTemp->nextseg;
         free (pTemp);
         pTemp = pTemp2;
      }
      free (pViewerParams->pLines);
      pViewerParams->pLines = NULL;
      pViewerParams->ulCountLines=0;
   }

   if (pViewerParams->pIncrements)
   {
      free(pViewerParams->pIncrements);
      pViewerParams->pIncrements=NULL;
   }

   return;
}

/*-------------------------------- Modulende --------------------------------*/
