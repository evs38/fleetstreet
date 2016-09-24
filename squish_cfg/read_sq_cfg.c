/*---------------------------------------------------------------------------+
 | Titel: READ_SQ_CFG.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 05.01.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Einlesen der Squish.Cfg.                                                |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define OS_2
#include <os2.h>
#include <string.h>
#include "../main.h"
#include "../structs.h"
#include "../msgheader.h"
#include "../areaman/areaman.h"
#include "../handlemsg/handlemsg.h"
#include <stdio.h>
#ifdef DEBUG
  #include <errno.h>
#endif
#include "../cfgfile_interface.h"
#include "read_sq_cfg.h"
#include "parsecfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME   "Squish, ParToss"
#define FORMAT_ID     0UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
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

int ReadSquishCfg(char *pchFileName, USERDATAOPT *pUserdata,
                  OUTBOUND *pOutbounds, PAREALIST pRetList,
                  PDRIVEREMAP driveremap, ULONG ulOptions)
{
   return ParseSquishCfg(pchFileName, pUserdata, pOutbounds, pRetList, driveremap, ulOptions);
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

