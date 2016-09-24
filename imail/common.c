/*---------------------------------------------------------------------------+
 | Titel: COMMON.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 08.08.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Gemeinsame Funktionen v. IM_CFG.DLL                                     |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "../main.h"
#include "../structs.h"
#include "../msgheader.h"
#include "../areaman/areaman.h"
#include "../util/fltutil.h"
#include "../util/addrcnv.h"

#include "common.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*-----------------------------------------------------------------------------
 | Funktionsname: AddNetmailArea
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fuegt eine Netmail-Area der Arealiste hinzu
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:  pRetList: Erzeugte Liste
 |             pchAreaPath: Pfad der Area
 |             pAddress: Adresse fuer die Area
 |             pchUsername: Username fuer die Area
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Area ist FTS-Format
 |
 +---------------------------------------------------------------------------*/

void AddNetmailArea(PAREALIST pRetList, char *pchAreaPath, PFTNADDRESS pAddress, char *pchUsername)
{
   AREADEFOPT AreaDef;

   memset(&AreaDef, 0, sizeof(AreaDef));

   strcpy(AreaDef.areatag, "NETMAIL");
   strcpy(AreaDef.areadesc, "Netmail");
   strncpy(AreaDef.pathfile, pchAreaPath, LEN_PATHNAME);
   RemoveBackslash(AreaDef.pathfile);
   NetAddrToString(AreaDef.address, pAddress);
   strncpy(AreaDef.username, pchUsername, LEN_USERNAME);
   AreaDef.areaformat = AREAFORMAT_FTS;
   AreaDef.areatype = AREATYPE_NET;
   AreaDef.ulDefAttrib = ATTRIB_PRIVATE;
   AreaDef.ulAreaOpt = AREAOPT_FROMCFG;

   AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);

   return;
}

/*-------------------------------- Modulende --------------------------------*/

