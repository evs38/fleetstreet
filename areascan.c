/*---------------------------------------------------------------------------+
 | Titel: AREASCAN.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 21.06.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Area-Scan von FleetStreet                                             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <share.h>

#include "main.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "handlemsg\handlemsg.h"
#include "dump\expt.h"

#include "areascan.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*-----------------------------------------------------------------------------
 | Funktionsname: ScanAreas
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Thread-Funktion, scannt die Areas, die mit dem SCAN-Flag
 |               markiert sind.
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: scanlist: Anfang der Area-Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void _Optlink ScanAreas(void *scanlist)
{
   AREADEFLIST *zeiger;
   extern BOOL DoingAreaScan;
   extern BOOL StopAreaScan;
   extern HWND client;
   extern MISCOPTIONS miscoptions;
   extern char CurrentArea[LEN_AREATAG+1];

   INSTALLEXPT("Areascan");

   /* Flag setzen */
   DoingAreaScan=TRUE;
   StopAreaScan=FALSE;

   zeiger=((PAREALIST)scanlist)->pFirstArea;
   while (zeiger)
   {
      if (zeiger->flWork & WORK_SCAN)
      {
         if (stricmp(zeiger->areadata.areatag, CurrentArea) &&
             !zeiger->bLocked)
         {
            extern DRIVEREMAP driveremap;

            /* Meldung in Statuszeile */
            WinPostMsg(client, SM_AREASCANNED, MPFROMP(zeiger->areadata.areatag), NULL);

            if (!MSG_OpenArea(scanlist, zeiger->areadata.areatag, miscoptions.lastreadoffset, &driveremap))
               MSG_CloseArea(scanlist, zeiger->areadata.areatag, FALSE, miscoptions.lastreadoffset, &driveremap);
         }
         zeiger->flWork &= ~WORK_SCAN;
      }
      zeiger=zeiger->next;
      if (StopAreaScan)
         break;
   }
   /* Ende-Meldung */
   while (!WinPostMsg(client, SM_SCANENDED, NULL, NULL))
      DosSleep(500);

   /* Flag wegnehmen */
   DoingAreaScan=FALSE;

   DEINSTALLEXPT;

   return;
}

/*-------------------------------- Modulende --------------------------------*/

