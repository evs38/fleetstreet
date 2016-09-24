/*---------------------------------------------------------------------------+
 | Titel: WM_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 11.10.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   WMail   Configuration File                                              |
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
#include <stdio.h>
#include "../main.h"
#include "../structs.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "revision12.h"
#include "wm_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "WMail"
#define FORMAT_ID      7UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int GetRevision(char *pchFileName);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadWMailCfg                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ReadWMailCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbounds,
                 PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   driveremap = driveremap;

   switch(GetRevision(pchFileName))
   {
      case -1:
         return CFGFILE_OPEN; /* CFGFILE_OPEN */

      case -2:
         return CFGFILE_READ; /* CFGFILE_READ */

      case 12: /* Revision 12 */
         return ReadRev12(pchFileName, pUserdata, pOutbounds, pRetList,
                          ulOptions);

      default:
         return CFGFILE_VERSION; /* CFGFILE_VERSION */
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetRevision                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt die Revisionsnummer des Config-Files              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchFileName: Dateiname                                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: >=0:  Revisionsnummer                                      */
/*                -1:   Datei nicht gefunden                                 */
/*                -2:   Lesefehler                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int GetRevision(char *pchFileName)
{
   FILE *pfConfig;
   unsigned short usRev=0;

   if (pfConfig = fopen(pchFileName, "r"))
   {
      if (fread(&usRev, sizeof(usRev), 1, pfConfig) <1)
      {
         fclose(pfConfig);
         return -2;
      }
      else
      {
         fclose(pfConfig);
         return usRev;
      }
   }
   else
      return -1;
}

PCHAR QueryFormatName(void)
{
   return FORMAT_NAME;
}

ULONG QueryFormatID(void)
{
   return FORMAT_ID;
}

ULONG QueryVer(void)
{
   return CURRENT_CFGVER;
}
/*-------------------------------- Modulende --------------------------------*/

