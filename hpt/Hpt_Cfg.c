/*---------------------------------------------------------------------------+
 | Titel: HPT_CFG.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Alexander A. Batalov      | Am: 12.09.1999                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   hpt Configuration File                                                  |
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
#include "main.h"
#include "structs.h"
#include "areaman/areaman.h"
#include "cfgfile_interface.h"
#include "hpt/hpt_cfg.h"
#include "hpt/hptparse.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME      "HPT"
#define FORMAT_ID        15UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadHptCfg                                                 */
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

int ReadHptCfg(char *pchFileName, USERDATAOPT *pUserdata,
               OUTBOUND *pOutbounds, PAREALIST pRetList, PDRIVEREMAP pDriveRemap,
               ULONG ulOptions)
{
  return ParseHptCfg(pchFileName, pUserdata, pOutbounds, pRetList, pDriveRemap,
                     ulOptions);
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


