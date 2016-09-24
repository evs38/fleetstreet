/*---------------------------------------------------------------------------+
 | Titel: REVISION1.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 01.07.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Config von FMail   (Revision 1)                                        |
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
#include <stdio.h>
#include "../main.h"
#include "../structs.h"
#include "../msgheader.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "../util/fltutil.h"
#include "../util/addrcnv.h"
#pragma pack(1)
#include "fmstruct.h"
#pragma pack()

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static configType config;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int CopyUserData(USERDATAOPT *pUserdata, configType *pconfig);
static int CopyOutbounds(OUTBOUND *pOutbounds, configType *pconfig);
static int ReadAreas(char *pchFileName, PAREALIST pRetList, configType *pconfig);
static void AddFTSArea(PAREALIST pRetList, AREADEFOPT *pAreaDef,
                       char *pchPath, char *pchTag, configType *pconfig);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadRev1                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst FMail-Cfg der Revision 1                             */
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

int ReadRev1(char *pchFileName, USERDATAOPT *pUserdata,
             OUTBOUND *pOutbounds, PAREALIST pRetList,
             ULONG ulOptions)
{
   int rc;
   FILE *pfConfig;

   memset(&config, 0, sizeof(config));

   if (pfConfig = fopen(pchFileName, "rb"))
   {
      if (fread(&config, sizeof(config), 1, pfConfig)==1)
      {
         fclose(pfConfig);

         if (ulOptions & READCFG_USERDATA)
            CopyUserData(pUserdata, &config);

         if (ulOptions & READCFG_OUTBOUNDS)
            CopyOutbounds(pOutbounds, &config);

         if ((ulOptions & READCFG_AREAS) && (rc = ReadAreas(pchFileName, pRetList, &config)))
            return rc;
         else
            return CFGFILE_OK;
      }
      else
      {
         fclose(pfConfig);
         return CFGFILE_READ;  /* CFGFILE_READ */
      }
   }
   else
      return CFGFILE_OPEN; /* CFGFILE_OPEN */

}

static int CopyUserData(USERDATAOPT *pUserdata, configType *pconfig)
{
   int i=0, j=0;

   while (i < MAX_AKAS && j < MAX_ADDRESSES)
   {
      if (pconfig->akaList[i].nodeNum.zone  ||
          pconfig->akaList[i].nodeNum.net   ||
          pconfig->akaList[i].nodeNum.node  ||
          pconfig->akaList[i].nodeNum.point)
      {
         NetAddrToString(pUserdata->address[j], (PFTNADDRESS) &pconfig->akaList[i]);
         j++;
      }
      i++;
   }

   strncpy(pUserdata->username[0], pconfig->sysopName, LEN_USERNAME);

   return 0;
}

static int CopyOutbounds(OUTBOUND *pOutbounds, configType *pconfig)
{
   memset(pOutbounds, 0, MAX_ADDRESSES * sizeof(OUTBOUND));

   if (pconfig->outPath[0])
   {
      strncpy(pOutbounds[0].outbound, pconfig->outPath, LEN_PATHNAME);
      RemoveBackslash(pOutbounds[0].outbound);
      pOutbounds[0].zonenum = pconfig->akaList[0].nodeNum.zone;
   }

   return 0;
}

static int ReadAreas(char *pchFileName, PAREALIST pRetList, configType *pconfig)
{
   FILE *pfAreas;
   char *pchAreaFileName;
   char *pchTemp;
   int rc=CFGFILE_OK;
   rawEchoType FArea;
   headerType AreaHeader;
   AREADEFOPT AreaDef;
   int iNum=0;
   unsigned short usRecordSize; /* gelesene groesse */
   unsigned short usGap;        /* Rest v. Area-Record */

   /* Netmail besonders behandeln */
   AddFTSArea(pRetList, &AreaDef, pconfig->netPath, "NETMAIL", pconfig);
   if (pconfig->pmailPath[0])
      AddFTSArea(pRetList, &AreaDef, pconfig->pmailPath, "PERS_MAIL", pconfig);

   /* FMAIL.AR anhaengen */
   pchAreaFileName = malloc(strlen(pchFileName)+20);
   strcpy(pchAreaFileName, pchFileName);
   pchTemp = strrchr(pchAreaFileName, '\\');
   if (!pchTemp)
      pchTemp = pchAreaFileName;
   if (*pchTemp == '\\')
      pchTemp++;
   strcpy(pchTemp, "FMAIL.AR");

   /* File oeffnen */
   if (pfAreas = fopen(pchAreaFileName, "rb"))
   {
      /* File-Header lesen */
      if (fread(&AreaHeader, sizeof(AreaHeader), 1, pfAreas))
      {
         if (AreaHeader.revNumber == 0x0100 &&
             AreaHeader.dataType == DATATYPE_AE)
         {
            if (AreaHeader.recordSize < sizeof(FArea))
               usRecordSize = AreaHeader.recordSize;
            else
               usRecordSize = sizeof(FArea);
            usGap = AreaHeader.recordSize - usRecordSize;

            while (!feof(pfAreas) && iNum < AreaHeader.totalRecords)
            {
               if (fread(&FArea, usRecordSize, 1, pfAreas))
               {
                  iNum++;

                  /* Area-Definition auslesen */
                  memset(&AreaDef, 0, sizeof(AreaDef));

                  if (FArea.options.active &&
                      FArea.msgBasePath[0])
                  {
                     strncpy(AreaDef.areatag, FArea.areaName, LEN_AREATAG);
                     strncpy(AreaDef.areadesc, FArea.comment, LEN_AREADESC);
                     strncpy(AreaDef.pathfile, FArea.msgBasePath, LEN_PATHNAME);
                     RemoveBackslash(AreaDef.pathfile);

                     AreaDef.areaformat = AREAFORMAT_JAM;

                     NetAddrToString(AreaDef.address, (FTNADDRESS*) &pconfig->akaList[FArea.address]);

                     if (FArea.options.local)
                        AreaDef.areatype = AREATYPE_LOCAL;
                     else
                        AreaDef.areatype = AREATYPE_ECHO;

                     AreaDef.ulAreaOpt = AREAOPT_FROMCFG;
                     strncpy(AreaDef.username, pconfig->sysopName, LEN_USERNAME);

                     /* Area zur Liste hinzufuegen */
                     AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
                  }
               }
               else
                  if (ferror(pfAreas))
                  {
                     rc = CFGFILE_READ;
                     break;
                  }

               if (usGap) /* muss Daten ueberspringen */
                  fseek(pfAreas, usGap, SEEK_CUR);
            }
         }
         else
            rc= CFGFILE_VERSION;
      }


      fclose(pfAreas);
   }
   else
      rc=CFGFILE_OPEN;

   free(pchAreaFileName);

   return rc;
}

static void AddFTSArea(PAREALIST pRetList, AREADEFOPT *pAreaDef,
                       char *pchPath, char *pchTag, configType *pconfig)
{
   if (pchPath[0])
   {
      memset(pAreaDef, 0, sizeof(AREADEFOPT));

      strcpy(pAreaDef->areatag, pchTag);
      strcpy(pAreaDef->areadesc, pchTag);
      strncpy(pAreaDef->pathfile, RemoveBackslash(pchPath), LEN_PATHNAME);

      pAreaDef->areaformat = AREAFORMAT_FTS;
      pAreaDef->areatype = AREATYPE_NET;
      pAreaDef->ulAreaOpt = AREAOPT_FROMCFG;
      pAreaDef->ulDefAttrib = ATTRIB_PRIVATE;
      NetAddrToString(pAreaDef->address, (PFTNADDRESS) &pconfig->akaList);
      strncpy(pAreaDef->username, pconfig->sysopName, LEN_USERNAME);

      AM_AddArea(pRetList, pAreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
   }
   return;
}

/*-------------------------------- Modulende --------------------------------*/

