/*---------------------------------------------------------------------------+
 | Titel: REV099.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 07.11.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Revision 0.99 von NewsWave                                              |
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
#include "rev099.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadNwCfg(HINI hini, USERDATAOPT *pUserData);
static int ReadNwAreas(HINI hini, USERDATAOPT *pUserData, PAREALIST pRetList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadNewsWaveCfg                                            */
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

int ReadRev099(HINI hini, USERDATAOPT *pUserdata, PAREALIST pRetList, ULONG ulOptions)
{
   int rc;

   /* NP_CFG lesen */
   if (rc = ReadNwCfg(hini, pUserdata))
      return rc;

   if (ulOptions & READCFG_AREAS)
      if (rc = ReadNwAreas(hini, pUserdata, pRetList))
         return rc;

   return CFGFILE_OK;
}

static int ReadNwCfg(HINI hini, USERDATAOPT *pUserData)
{
   PrfQueryProfileString(hini, "common", "fAddress", NULL, pUserData->address[0], sizeof(pUserData->address[0]));
   PrfQueryProfileString(hini, "common", "name", NULL, pUserData->username[0], sizeof(pUserData->username[0]));
   PrfQueryProfileString(hini, "common", "organisation", NULL, pUserData->defaultorigin, sizeof(pUserData->defaultorigin));

   if (!pUserData->address[0][0] ||
       !pUserData->username[0][0])
      return CFGFILE_GENDATA;
   else
      return CFGFILE_OK;
}

static int ReadNwAreas(HINI hini, USERDATAOPT *pUserData, PAREALIST pRetList)
{
   AREADEFOPT Area;
   ULONG ulNumAreas=0, ulArea=0, ulRead;
   char pchKey[50];

   memset(&Area, 0, sizeof(Area));
   Area.areaformat = AREAFORMAT_SQUISH; /* Default */
   Area.areatype = AREATYPE_LOCAL;      /* Default */
   strcpy(Area.username, pUserData->username[0]);
   strcpy(Area.address, pUserData->address[0]);
   Area.ulAreaOpt = AREAOPT_FROMCFG;

   PrfQueryProfileString(hini, "areas", "emailName", "", Area.areatag, sizeof(Area.areatag));
   strcpy(Area.areadesc, Area.areatag);
   PrfQueryProfileString(hini, "areas", "emailPath", "", Area.pathfile, sizeof(Area.pathfile));

   if (Area.pathfile[0] && Area.areatag[0])
      AM_AddArea(pRetList, &Area, ADDAREA_TAIL | ADDAREA_UNIQUE);

   ulRead = sizeof(ulNumAreas);
   PrfQueryProfileData(hini, "areas", "areas", &ulNumAreas, &ulRead);

   for (ulArea=1; ulArea <= ulNumAreas; ulArea++)
   {
      memset(Area.areatag, 0, sizeof(Area.areatag));
      memset(Area.pathfile, 0, sizeof(Area.pathfile));

      sprintf(pchKey, "area%dname", ulArea);
      PrfQueryProfileString(hini, "areas", pchKey, "", Area.areatag, sizeof(Area.areatag));
      strcpy(Area.areadesc, Area.areatag);
      sprintf(pchKey, "area%dpath", ulArea);
      PrfQueryProfileString(hini, "areas", pchKey, "", Area.pathfile, sizeof(Area.pathfile));

      if (Area.pathfile[0] && Area.areatag[0])
         AM_AddArea(pRetList, &Area, ADDAREA_TAIL | ADDAREA_UNIQUE);
   }

   if (pRetList->ulNumAreas)
      return CFGFILE_OK;
   else
      return CFGFILE_NOAREA;
}

/*-------------------------------- Modulende --------------------------------*/

