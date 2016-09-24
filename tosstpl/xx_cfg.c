/*---------------------------------------------------------------------------+
 | Titel: XX_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: xx.xx.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   xxConfiguration File                                                    |
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
#include "xx_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "FORMAT"
#define FORMAT_ID      0UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadxxCfg(char *pchFileName, USERDATAOPT *pUserData);
static int ReadxxAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadxxCfg                                            */
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

int ReadxxCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
                    PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;

   pOutbound = pOutbound;
   driveremap = driveremap;

   /* XX_CFG lesen */
   if (rc = ReadxxCfg(pchFileName, pUserdata))
      return rc;

   if (ulOptions & READCFG_AREAS)
      if (rc = ReadxxAreas(pchFileName, pUserdata, pRetList))
         return rc;

   return CFGFILE_OK;
}

static int ReadxxCfg(char *pchFileName, USERDATAOPT *pUserData)
{
  return CFGFILE_OPEN;
}


static int ReadxxAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList)
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