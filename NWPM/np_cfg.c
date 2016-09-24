/*---------------------------------------------------------------------------+
 | Titel: NP_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 06.06.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   NewsWave PM Configuration File                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../main.h"
#include "../structs.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "../util/fltutil.h"
#include "../msgheader.h"
#include "np_cfg.h"
#include "rev099.h"
#include "rev100.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "NewsWave PM"
#define FORMAT_ID      11UL

#define REV_099 "0.99"
#define REV_100 "1.00"

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadNewsWaveCfg                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ReadNewsWaveCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
                    PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;
   HINI hini;
   char VersString[10]="";

   pOutbound = pOutbound;
   driveremap = driveremap;

   hini = PrfOpenProfile(NULLHANDLE, pchFileName);

   if (hini)
   {
      PrfQueryProfileString(hini, "common", "version", "", VersString, sizeof(VersString));
      if (!strcmp(VersString, REV_099))
         rc = ReadRev099(hini, pUserdata, pRetList, ulOptions);
      else
         if (!strcmp(VersString, REV_100))
            rc = ReadRev100(hini, pUserdata, pRetList, ulOptions);
      PrfCloseProfile(hini);
      return rc;
   }
   else
      return CFGFILE_OPEN;
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