/*---------------------------------------------------------------------------+
 | Titel: REVISION3.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 30.06.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Config von LoraBBS (Revision 3)                                        |
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
#include "lora.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

struct _configuration config;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int CopyUserData(USERDATAOPT *pUserdata, struct _configuration *pconfig);
static int CopyOutbounds(OUTBOUND *pOutbounds, struct _configuration *pconfig);
static int ReadAreas(char *pchFileName, PAREALIST pRetList, struct _configuration *pconfig);
static void AddFTSArea(PAREALIST pRetList, AREADEFOPT *pAreaDef,
                       char *pchPath, char *pchTag, struct _configuration *pconfig);
static void BuildPath(char *pchSysPath, char *pchAreaPath, char *pchResult);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadRev3                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst LoraBBS-Cfg der Revision 3                           */
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

int ReadRev3(char *pchFileName, USERDATAOPT *pUserdata,
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

static int CopyUserData(USERDATAOPT *pUserdata, struct _configuration *pconfig)
{
   int i=0, j=0;

   while (i < MAX_ALIAS && j < MAX_ADDRESSES)
   {
      if (pconfig->alias[i].zone  ||
          pconfig->alias[i].net   ||
          pconfig->alias[i].node  ||
          pconfig->alias[i].point)
      {
         NetAddrToString(pUserdata->address[j], (PFTNADDRESS) &pconfig->alias[i]);
         j++;
      }
      i++;
   }

   strncpy(pUserdata->username[0], pconfig->sysop, LEN_USERNAME);
   strncpy(pUserdata->defaultorigin, pconfig->system_name, LEN_ORIGIN);

   return 0;
}

static int CopyOutbounds(OUTBOUND *pOutbounds, struct _configuration *pconfig)
{
   memset(pOutbounds, 0, MAX_ADDRESSES * sizeof(OUTBOUND));

   if (pconfig->outbound[0] &&
       pconfig->outbound[1] &&
       pconfig->outbound[1] != ':')
   {
      /* relativer Pfad */
      size_t len;

      strncpy(pOutbounds[0].outbound, pconfig->sys_path, LEN_PATHNAME);
      len = strlen(pOutbounds[0].outbound);
      if (len && pOutbounds[0].outbound[len-1] != '\\')
         strcat(pOutbounds[0].outbound, "\\");
      strcat(pOutbounds[0].outbound, pconfig->outbound);
   }
   else
      strncpy(pOutbounds[0].outbound, pconfig->outbound, LEN_PATHNAME);
   RemoveBackslash(pOutbounds[0].outbound);
   pOutbounds[0].zonenum = pconfig->alias[0].zone;

   return 0;
}

static int ReadAreas(char *pchFileName, PAREALIST pRetList, struct _configuration *pconfig)
{
   FILE *pfAreas;
   char *pchAreaFileName;
   char *pchTemp;
   int rc=CFGFILE_OK;
   struct _sys LArea;
   AREADEFOPT AreaDef;

   /* Netmail, BAD und DUPES besonders behandeln */
   AddFTSArea(pRetList, &AreaDef, pconfig->netmail_dir, "NETMAIL", pconfig);
   AddFTSArea(pRetList, &AreaDef, pconfig->bad_msgs,    "BAD",     pconfig);
   AddFTSArea(pRetList, &AreaDef, pconfig->dupes,       "DUPES",   pconfig);
   AddFTSArea(pRetList, &AreaDef, pconfig->my_mail,     "MY_MAIL", pconfig);

   /* SYSMSG.DAT anhaengen */
   pchAreaFileName = malloc(strlen(pchFileName)+20);
   strcpy(pchAreaFileName, pchFileName);
   pchTemp = strrchr(pchAreaFileName, '\\');
   if (!pchTemp)
      pchTemp = pchAreaFileName;
   if (*pchTemp == '\\')
      pchTemp++;
   strcpy(pchTemp, "SYSMSG.DAT");

   /* File oeffnen */
   if (pfAreas = fopen(pchAreaFileName, "rb"))
   {
      while (!feof(pfAreas))
      {
         if (fread(&LArea, SIZEOF_MSGAREA, 1, pfAreas))
         {
            /* Area-Definition auslesen */
            memset(&AreaDef, 0, sizeof(AreaDef));

            if (!LArea.passthrough &&
                LArea.msg_path[0] &&
                LArea.echotag[0] )
            {
               strncpy(AreaDef.areatag, LArea.echotag, LEN_AREATAG);
               strncpy(AreaDef.areadesc, LArea.msg_name, LEN_AREADESC);

               BuildPath(pconfig->sys_path, LArea.msg_path, AreaDef.pathfile);

               if (LArea.squish)
                  AreaDef.areaformat = AREAFORMAT_SQUISH;
               else
                  AreaDef.areaformat = AREAFORMAT_FTS;

               NetAddrToString(AreaDef.address, (FTNADDRESS*) &pconfig->alias[LArea.use_alias]);

               if (LArea.echomail)
                  AreaDef.areatype = AREATYPE_ECHO;
               else
                  if (!LArea.netmail)
                     AreaDef.areatype = AREATYPE_LOCAL;
                  else
                  {
                     AreaDef.areatype = AREATYPE_NET;
                     AreaDef.ulDefAttrib = ATTRIB_PRIVATE;
                  }

               AreaDef.ulAreaOpt = AREAOPT_FROMCFG;
               strncpy(AreaDef.username, pconfig->sysop, LEN_USERNAME);

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
      }

      fclose(pfAreas);
   }
   else
      rc=CFGFILE_OPEN;

   free(pchAreaFileName);
   return rc;
}

static void AddFTSArea(PAREALIST pRetList, AREADEFOPT *pAreaDef,
                       char *pchPath, char *pchTag, struct _configuration *pconfig)
{
   if (pchPath[0])
   {
      memset(pAreaDef, 0, sizeof(AREADEFOPT));

      strcpy(pAreaDef->areatag, pchTag);
      strcpy(pAreaDef->areadesc, pchTag);

      BuildPath(pconfig->sys_path, pchPath, pAreaDef->pathfile);

      pAreaDef->areatype = AREATYPE_NET;
      pAreaDef->areaformat = AREAFORMAT_FTS;
      pAreaDef->ulDefAttrib = ATTRIB_PRIVATE;
      pAreaDef->ulAreaOpt = AREAOPT_FROMCFG;
      NetAddrToString(pAreaDef->address, (PFTNADDRESS) &pconfig->alias[0]);
      strncpy(pAreaDef->username, pconfig->sysop, LEN_USERNAME);

      AM_AddArea(pRetList, pAreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
   }
   return;
}

static void BuildPath(char *pchSysPath, char *pchAreaPath, char *pchResult)
{
   if (pchAreaPath[0])
   {
      if (pchAreaPath[1] &&
          pchAreaPath[1] != ':')
      {
         /* Area ohne Laufwerk */

         strncpy(pchResult, pchSysPath, LEN_PATHNAME);

         if (pchAreaPath[0] == '\\')
         {
            /* absolut ohne LW */
            char *pchDrive;

            pchDrive = strchr(pchResult, ':');
            if (pchDrive)
               pchDrive++;
            else
               pchDrive = pchResult;

            strncpy(pchDrive, pchAreaPath, LEN_PATHNAME);
         }
         else
         {
            /* relativ */
            size_t len;

            len=strlen(pchResult);
            if (len && pchResult[len-1] != '\\')
               strcat(pchResult, "\\");

            strcat(pchResult, pchAreaPath);
         }
      }
      else
         /* Area mit Laufwerk */
         strncpy(pchResult, pchAreaPath, LEN_PATHNAME);

      RemoveBackslash(pchResult);
   }
}

/*-------------------------------- Modulende --------------------------------*/

