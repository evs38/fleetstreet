/*---------------------------------------------------------------------------+
 | Titel: SG_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 27.04.1997                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Shotgun Configuration File                                              |
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
#include "shotgun.h"
#include "pas2c.h"
#include "sg_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "Shotgun BBS"
#define FORMAT_ID      14UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

SystemDat_Record SystemDat;
Network_Record Networks[MAX_ADDRESSES];
int NumAkas;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadSysCfg(char *pchFileName);
static int ReadSGAkas(char *pchFileName);
static int CopyInfo(USERDATAOPT *pUserData);
static int ReadSGAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList);
static int ReadOneArea(char *pchName, USERDATAOPT *pUserData, PAREALIST pRetList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadSGCfg                                                  */
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

int ReadSGCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
              PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;

   pOutbound = pOutbound;
   driveremap = driveremap;

   memset(&SystemDat, 0, sizeof(SystemDat));
   memset(Networks, 0, sizeof(Networks));
   NumAkas=0;

   /* SG_CFG lesen */
   if (rc = ReadSysCfg(pchFileName))
      return rc;

   /* AKAs lesen */
   if (rc = ReadSGAkas(pchFileName))
      return rc;

   CopyInfo(pUserdata);

   if (ulOptions & READCFG_AREAS)
      if (rc = ReadSGAreas(pchFileName, pUserdata, pRetList))
         return rc;

   return CFGFILE_OK;
}

static int ReadSysCfg(char *pchFileName)
{
   FILE *pfSys;
   int rc=CFGFILE_OK;

   if (pfSys = fopen(pchFileName, "rb"))
   {
      if (fread(&SystemDat, sizeof(SystemDat), 1, pfSys) != 1)
         rc = CFGFILE_READ;

      fclose(pfSys);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}

static int ReadSGAkas(char *pchFileName)
{
   FILE *pfNet;
   int rc=CFGFILE_OK;
   char pchName[LEN_PATHNAME+1];
   char *pchTemp;

   strcpy(pchName, pchFileName);
   pchTemp=strrchr(pchName, '\\');
   if (pchTemp)
      strcpy(pchTemp, "\\NETWORK.DAT");
   else
      return CFGFILE_OPEN;

   if (pfNet = fopen(pchName, "rb"))
   {
      if ((NumAkas=fread(Networks, sizeof(Networks[0]), MAX_ADDRESSES, pfNet)) < 1)
         rc = CFGFILE_READ;

      fclose(pfNet);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}

static int CopyInfo(USERDATAOPT *pUserData)
{
   int i=0, j=0;

   while (i < NumAkas && j < MAX_ADDRESSES)
   {
      NetAddrToString(pUserData->address[j], (PFTNADDRESS) &(Networks[i].Net_Address));
      j++;
      i++;
   }

   Pas2C(pUserData->username[0], SystemDat.Sysop, LEN_USERNAME+1);
   Pas2C(pUserData->username[1], SystemDat.Alias, LEN_USERNAME+1);

   return 0;
}

static int ReadSGAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList)
{
   char pchName[LEN_PATHNAME+1];
   char *pchTemp;
   FILEFINDBUF3 FindBuf;
   HDIR hDir=HDIR_CREATE;
   ULONG ulFound=1;
   APIRET rc;

   strcpy(pchName, pchFileName);
   pchTemp=strrchr(pchName, '\\');
   if (pchTemp)
      strcpy(pchTemp, "\\MA??????.DAT");
   else
      return CFGFILE_OPEN;

   if (!(rc = DosFindFirst(pchName, &hDir, FILE_ARCHIVED | FILE_READONLY,
                           &FindBuf, sizeof(FindBuf), &ulFound, FIL_STANDARD)))
   {
      while (!rc)
      {
         strcpy(pchName, pchFileName);
         pchTemp=strrchr(pchName, '\\');
         pchTemp++;
         strcpy(pchTemp, FindBuf.achName);

         ReadOneArea(pchName, pUserData, pRetList);

         ulFound = 1;
         rc = DosFindNext(hDir, &FindBuf, sizeof(FindBuf), &ulFound);
      }
   }

   if (hDir != HDIR_CREATE)
      DosFindClose(hDir);

   return CFGFILE_OK;
}

static int ReadOneArea(char *pchName, USERDATAOPT *pUserData, PAREALIST pRetList)
{
   FILE *pfAreas;
   int rc=CFGFILE_OK;
   MessageArea_Record MessageArea;
   AREADEFOPT AreaDef;
   int bSkip;

   if (pfAreas = fopen(pchName, "rb"))
   {
      while (!feof(pfAreas))
         if (fread(&MessageArea, sizeof(MessageArea), 1, pfAreas)==1)
         {
            if (MessageArea.Area_Type != SGAREATYPE_UUCP &&
                MessageArea.Area_Type != SGAREATYPE_UUCP+1 &&
                MessageArea.Area_Type != SGAREATYPE_UUCP+2)
            {
               memset(&AreaDef, 0, sizeof(AreaDef));
               bSkip=FALSE;

               Pas2C(AreaDef.areadesc, MessageArea.Area_Name, LEN_AREADESC+1);
               Pas2C(AreaDef.pathfile, MessageArea.Dos_Name, LEN_PATHNAME+1);
               RemoveBackslash(AreaDef.pathfile);
               switch(MessageArea.Area_Type)
               {
                  case SGAREATYPE_LOCAL:
                  case SGAREATYPE_LOCAL+1:
                  case SGAREATYPE_LOCAL+2:
                     AreaDef.areatype = AREATYPE_LOCAL;
                     break;

                  case SGAREATYPE_NET:
                  case SGAREATYPE_NET+1:
                  case SGAREATYPE_NET+2:
                     AreaDef.areatype = AREATYPE_NET;
                     break;

                  case SGAREATYPE_ECHO:
                  case SGAREATYPE_ECHO+1:
                  case SGAREATYPE_ECHO+2:
                     AreaDef.areatype = AREATYPE_ECHO;
                     break;

                  default:
                     bSkip=TRUE;
                     break;
               }
               switch(MessageArea.Base_Type)
               {
                  case BASETYPE_JAM:
                     AreaDef.areaformat = AREAFORMAT_JAM;
                     break;

                  case BASETYPE_SQUISH:
                     AreaDef.areaformat = AREAFORMAT_SQUISH;
                     break;

                  case BASETYPE_FIDO:
                     AreaDef.areaformat = AREAFORMAT_FTS;
                     break;

                  default:
                     bSkip=TRUE;
                     break;
               }
               Pas2C(AreaDef.areatag, MessageArea.Area_Tag, LEN_AREATAG+1);
               if (!AreaDef.areatag[0])
                  bSkip = TRUE;

               if (!bSkip)
               {
                  strcpy(AreaDef.address, pUserData->address[MessageArea.Address-1]);
                  if (MessageArea.Msg_Type == USENAME_ALIAS)
                     strcpy(AreaDef.username, pUserData->username[1]);
                  else
                     strcpy(AreaDef.username, pUserData->username[0]);
                  AreaDef.ulAreaOpt = AREAOPT_FROMCFG;
                  AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
               }
            }
         }

      if (ferror(pfAreas))
         rc = CFGFILE_READ;

      fclose(pfAreas);
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

