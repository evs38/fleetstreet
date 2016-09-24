/*---------------------------------------------------------------------------+
 | Titel: BB_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 10.07.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   BBToss Configuration File                                               |
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
#include "bb_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME    "BBToss"
#define FORMAT_ID      12UL

#define MAXLINELEN   300

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static char pchMailDir[LEN_PATHNAME+1];
static char MailDirBaseType; /* Merker, da BBToss auch andere Formate als    */
                             /* Fido *.MSG als Netmail-Folder unterstuetzt.  */
                             /* 'N' = *.MSG                                  */

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadBBCfg(char *pchFileName, OUTBOUND *pOutbound);
static int ReadBBIni(char *pchFileName, OUTBOUND *pOutbound, USERDATAOPT *pUserData);
static int ReadBBAka(char *pchFileName, USERDATAOPT *pUserData);
static int ReadBBAlias(char *pchFileName, USERDATAOPT *pUserData);
static int ReadBBTossAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList);
static void AddNetmailArea(PCHAR pchMailDir, USERDATAOPT *pUserData, PAREADEFOPT pAreaDef,
                           PAREALIST pRetList);
static void AddArea(PAREADEFOPT pAreaDef, USERDATAOPT *pUserData, PAREALIST pRetList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadBBTossCfg                                              */
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

int ReadBBTossCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
                    PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc = CFGFILE_OK;
   char *pchExt;

   driveremap = driveremap;
   memset(pchMailDir, 0, sizeof(pchMailDir));

   pchExt = strrchr(pchFileName, '.');
   if (pchExt &&  !stricmp(pchExt, ".ini"))
   {
      /* BBToss 2.30 */
      rc = ReadBBIni(pchFileName, pOutbound, pUserdata);
   }
   else
   {
      /* BB_CFG lesen */
      if (!(rc = ReadBBCfg(pchFileName, pOutbound)))
         rc = ReadBBAka(pchFileName, pUserdata);
   }

   if (!rc)
      rc = ReadBBAlias(pchFileName, pUserdata);

   if (!rc && (ulOptions & READCFG_AREAS))
      rc = ReadBBTossAreas(pchFileName, pUserdata, pRetList);

   return rc;
}

/* BBTOSS.CFG einlesen */

static int ReadBBCfg(char *pchFileName, OUTBOUND *pOutbound)
{
   FILE *pfCfg=NULL;
   int rc=CFGFILE_OK;
   char zeile[MAXLINELEN];
   char *pchKeyword;
   char *pchValue;

   if (pfCfg=fopen(pchFileName, "r"))
   {
      while (!rc && !feof(pfCfg))
      {
         if (fgets(zeile, sizeof(zeile), pfCfg))
         {
            /* Zeile zerlegen */
            StripWhitespace(zeile);
            pchKeyword = strtok(zeile, " =:\t\n");
            if (pchKeyword)
            {
               pchValue = strtok(NULL, " \t\n");

               if (pchValue)
                  if (!stricmp(pchKeyword, "OUTBOUNDDIR"))
                  {
                     strcpy(pOutbound->outbound, pchValue);
                     RemoveBackslash(pOutbound->outbound);
                     pOutbound->zonenum = 0;
                  }
                  else
                     if (!stricmp(pchKeyword, "MSGDIR"))
                     {
                        strcpy(pchMailDir, pchValue);
                        RemoveBackslash(pchMailDir);
                     }
            }
         }
         else
            if (ferror(pfCfg))
               rc = CFGFILE_READ;
      }

      fclose(pfCfg);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}

/* bb: BBTOSS.INI einlesen */
/* aktuelle INI file sections */
#define BBTAKA_SECTION           1
#define BBTPATH_SECTION          2
#define BBTNETMAILFOLDER_SECTION 3

static int ReadBBIni(char *pchFileName, OUTBOUND *pOutbound, USERDATAOPT *pUserData)
{
   FILE *pfIni=NULL;
   int rc=CFGFILE_OK;
   char zeile[MAXLINELEN];
   char *pchValue;
   int  isection=0;      /* Merker welche Ini-file section aktuell ist */
   char *pchTag;
   int akanum=0;

   if (pfIni=fopen(pchFileName, "r"))
   {
      while (!rc && !feof (pfIni))
      {
         if (fgets(zeile, sizeof(zeile), pfIni))
         {
            StripWhitespace(zeile);

            if (zeile[0] == '[' ) /* jede Section beginnt mit "[section_name]" */
            {
               /* Merker setzen welche INI File Section aktuell ist */
               isection=0;

               if (!stricmp (zeile, "[BBTAKA]"))
                  isection= BBTAKA_SECTION;
               if (!stricmp (zeile, "[BBTPATH]"))
                  isection= BBTPATH_SECTION;
               if (!stricmp (zeile, "[NETMAILFOLDER]"))
                  isection=BBTNETMAILFOLDER_SECTION;
            }
            else /* Zeile zerlegen, wenn kein Section Header, dann "Tag=Value" Zeile (oder Leerzeile) */
            {
               pchTag = strtok(zeile, " =\t\n");
               if (pchTag)
                  pchValue = strtok(NULL, " \t\n");

               if (pchTag && pchValue)
                  switch (isection) /* Keywords aus aktueller Section setzen */
                  {
                     case BBTAKA_SECTION : /* [BBTAKA] */
                        {
                           FTNADDRESS Address;

                           if ((!stricmp (pchTag, "MAINAKA") || !stricmp (pchTag, "AKA")) &&
                               (akanum < MAX_ADDRESSES))
                           {
                              /* set Main-Aka/Aka (pchValue)*/
                              StringToNetAddr(pchValue, &Address, NULL);
                              NetAddrToString(pUserData->address[akanum++], &Address);
                           }
                        }
                        break;

                     case BBTPATH_SECTION : /* [BBTPATH] */
                        if (!stricmp(pchTag, "OUTBOUND"))
                        {
                           /* set outbound (pchValue)*/
                           strcpy(pOutbound->outbound, pchValue);
                           RemoveBackslash(pOutbound->outbound);
                           pOutbound->zonenum = 0;
                        }
                        break;

                     case BBTNETMAILFOLDER_SECTION : /*[NETMAILFOLDER]*/
                        if (!stricmp(pchTag, "BASETYP")) /* not neccessarly N = *.MSG */
                        {
                           MailDirBaseType = *pchValue; /* Merker BaseType */
                        }

                        if (!stricmp(pchTag, "PATH"))
                        {
                           /* set netmailfolder path */
                           /* make sure BASETYP is supported  'N' = *.MSG */
                           strcpy(pchMailDir, pchValue);
                           RemoveBackslash(pchMailDir);
                        }
                        break;

                     default: /* andere section */
                        break;
                  } /*switch*/
            }
         } /*if fgets */
         else
            if (ferror(pfIni))
               rc = CFGFILE_READ;
      } /* while */

      fclose(pfIni);
      return rc;

   } /* if pfIni */
   else
      return CFGFILE_OPEN;

}

static int ReadBBAka(char *pchFileName, USERDATAOPT *pUserData)
{
   char pchAkaName[LEN_PATHNAME+1];
   char *pchTemp;
   FILE *pfCfg;
   int rc=CFGFILE_OK;
   char zeile[MAXLINELEN];
   int akanum=0;

   strcpy(pchAkaName, pchFileName);
   pchTemp = strrchr(pchAkaName, '\\');
   if (pchTemp)
   {
      pchTemp++;
      strcpy(pchTemp, "BBTAKA.CFG");
   }

   if (pfCfg=fopen(pchAkaName, "r"))
   {
      while (!rc && !feof(pfCfg) && akanum < MAX_ADDRESSES)
      {
         if (fgets(zeile, sizeof(zeile), pfCfg))
         {
            /* Zeile zerlegen */
            StripWhitespace(zeile);
            if (zeile[0])
            {
               FTNADDRESS Address;

               StringToNetAddr(zeile, &Address, NULL);
               NetAddrToString(pUserData->address[akanum++], &Address);
            }
         }
         else
            if (ferror(pfCfg))
               rc = CFGFILE_READ;
      }

      fclose(pfCfg);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}

static int ReadBBAlias(char *pchFileName, USERDATAOPT *pUserData)
{
   char pchAliasName[LEN_PATHNAME+1];
   char *pchTemp;
   FILE *pfCfg;
   int rc=CFGFILE_OK;
   char zeile[MAXLINELEN];
   int aliasnum=0;

   strcpy(pchAliasName, pchFileName);
   pchTemp = strrchr(pchAliasName, '\\');
   if (pchTemp)
   {
      pchTemp++;
      strcpy(pchTemp, "BBTALIAS.CFG");
   }

   if (pfCfg=fopen(pchAliasName, "r"))
   {
      while (!rc && !feof(pfCfg) && aliasnum < MAX_USERNAMES)
      {
         if (fgets(zeile, sizeof(zeile), pfCfg))
         {
            /* Zeile zerlegen */
            StripWhitespace(zeile);
            if (zeile[0])
               strcpy(pUserData->username[aliasnum++], zeile);
         }
         else
            if (ferror(pfCfg))
               rc = CFGFILE_READ;
      }

      fclose(pfCfg);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}


static int ReadBBTossAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList)
{
   char pchAreasName[LEN_PATHNAME+1];
   char *pchTemp;
   FILE *pfCfg;
   int rc=CFGFILE_OK;
   char zeile[MAXLINELEN];
   AREADEFOPT AreaDef;
   char *pchKeyword;
   char *pchValue;

   strcpy(pchAreasName, pchFileName);
   pchTemp = strrchr(pchAreasName, '\\');
   if (pchTemp)
   {
      pchTemp++;
      strcpy(pchTemp, "BBTAREA.INI");
   }

   if (MailDirBaseType == 'N' && pchMailDir[0])
      AddNetmailArea(pchMailDir, pUserData, &AreaDef, pRetList);

   memset(&AreaDef, 0, sizeof(AreaDef));

   if (pfCfg=fopen(pchAreasName, "r"))
   {
      while (!rc && !feof(pfCfg))
      {
         if (fgets(zeile, sizeof(zeile), pfCfg))
         {
            /* Zeile zerlegen */
            StripWhitespace(zeile);
            switch(zeile[0])
            {
               case '\0':
                  /* Leerzeile */
                  break;

               case '[':
                  /* Neue Area */
                  if (AreaDef.areatag[0]) /* vorherige Area noch eintragen */
                     AddArea(&AreaDef, pUserData, pRetList);
                  memset(&AreaDef, 0, sizeof(AreaDef));
                  strncpy(AreaDef.areatag, zeile+1, LEN_AREATAG);
                  pchTemp = strchr(AreaDef.areatag, ']');
                  if (pchTemp)
                     *pchTemp = 0;
                  break;

               default:
                  pchKeyword = strtok(zeile, " =\t\n");
                  if (pchKeyword)
                  {
                     pchValue = strtok(NULL, "=\t\n");
                     if (pchValue)
                        if (!stricmp(pchKeyword, "PATH"))
                        {
                           strncpy(AreaDef.pathfile, pchValue, LEN_PATHNAME);
                           RemoveBackslash(AreaDef.pathfile);
                        }
                        else
                           if (!stricmp(pchKeyword, "BASETYP"))
                           {
                              switch(*pchValue)
                              {
                                 case 'S':
                                    AreaDef.areaformat = AREAFORMAT_SQUISH;
                                    break;

                                 case 'J':
                                    AreaDef.areaformat = AREAFORMAT_JAM;
                                    break;

                                 case 'N':
                                    AreaDef.areaformat = AREAFORMAT_FTS;
                                    break;

                                 default:
                                    /* invalidieren */
                                    AreaDef.areatag[0] = 0;
                                    break;
                              }
                           }
                           else
                              if (!stricmp(pchKeyword, "TYP"))
                              {
                                 switch(*pchValue)
                                 {
                                    case 'B':
                                    case 'D':
                                    case 'N':
                                       AreaDef.areatype = AREATYPE_NET;
                                       break;

                                    case 'E':
                                       AreaDef.areatype = AREATYPE_ECHO;
                                       break;

                                    case 'L':
                                       AreaDef.areatype = AREATYPE_LOCAL;
                                       break;

                                    default:
                                       /* invalidieren */
                                       AreaDef.areatag[0] = 0;
                                       break;
                                 }
                              }
                              else
                                 if (!stricmp(pchKeyword, "MAINAKA"))
                                    strncpy(AreaDef.address, pchValue, LEN_5DADDRESS);
                                 else
                                    if (!stricmp(pchKeyword, "DESC"))
                                       strncpy(AreaDef.areadesc, pchValue, LEN_AREADESC);
                  }
                  break;
            }
         }
         else
            if (ferror(pfCfg))
               rc = CFGFILE_READ;
      }

      /* letzte Area */
      if (AreaDef.areatag[0]) /* vorherige Area noch eintragen */
         AddArea(&AreaDef, pUserData, pRetList);

      fclose(pfCfg);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}

static void AddNetmailArea(PCHAR pchMailDir, USERDATAOPT *pUserData, PAREADEFOPT pAreaDef,
                           PAREALIST pRetList)
{
   memset(pAreaDef, 0, sizeof(AREADEFOPT));

   strcpy(pAreaDef->areatag, "NETMAIL");
   strcpy(pAreaDef->areadesc, "Netmail");
   strcpy(pAreaDef->address, pUserData->address[0]);
   strcpy(pAreaDef->username, pUserData->username[0]);
   strcpy(pAreaDef->pathfile, pchMailDir);

   pAreaDef->areaformat = AREAFORMAT_FTS;
   pAreaDef->areatype = AREATYPE_NET;
   pAreaDef->ulDefAttrib = ATTRIB_PRIVATE;
   pAreaDef->ulAreaOpt = AREAOPT_FROMCFG;

   AM_AddArea(pRetList, pAreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
}

static void AddArea(PAREADEFOPT pAreaDef, USERDATAOPT *pUserData, PAREALIST pRetList)
{
   /* Weitere Daten */
   strcpy(pAreaDef->username, pUserData->username[0]);
   pAreaDef->ulAreaOpt = AREAOPT_FROMCFG;
   if (!pAreaDef->areadesc[0])
      strcpy(pAreaDef->areadesc, pAreaDef->areatag);

   /* pruefen und anhaengen */
   if (pAreaDef->areatag[0] &&
       pAreaDef->pathfile[0] &&
       pAreaDef->username[0] &&
       pAreaDef->address[0] &&
       pAreaDef->areaformat)
      AM_AddArea(pRetList, pAreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
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

