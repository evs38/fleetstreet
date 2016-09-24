/*---------------------------------------------------------------------------+
 | Titel: SO_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 26.02.1997                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   SOS Configuration File                                                  |
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
#include "../util/addrcnv.h"
#include "so_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "SOS"
#define FORMAT_ID      13UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadSOCfg(char *pchFileName, USERDATAOPT *pUserData);
static int ReadSOSAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadSOSCfg                                                 */
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

int ReadSOSCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
                    PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;

   pOutbound = pOutbound;
   driveremap = driveremap;

   /* SO_CFG lesen */
   if (rc = ReadSOCfg(pchFileName, pUserdata))
      return rc;

   if (ulOptions & READCFG_AREAS)
      if (rc = ReadSOSAreas(pchFileName, pUserdata, pRetList))
         return rc;

   return CFGFILE_OK;
}

static int ReadSOCfg(char *pchFileName, USERDATAOPT *pUserData)
{
  return CFGFILE_OPEN;
}


static int ReadSOSAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList)
{
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