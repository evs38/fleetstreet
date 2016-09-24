/*---------------------------------------------------------------------------+
 | Titel: TM_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 06.09.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   TerMail Configuration File                                              |
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
#include "../msgheader.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "../util/fltutil.h"
#include "../util/addrcnv.h"
#include "../handlemsg/handlemsg.h"
#include "tm_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define KEY_NETMAIL      0
#define KEY_OUTBOUND     1
#define KEY_AREAFILE     2
#define KEY_ORIGIN       3
#define KEY_NAME         4
#define KEY_ADDRESS      5
#define KEY_ADESC        6

#define FORMAT_NAME      "TerMail"
#define FORMAT_ID        6UL

/*---------------------------------- Typen ----------------------------------*/

typedef struct desclist
{
   struct desclist *next;
   char pchAreaTag[LEN_AREATAG+1];
   char pchDesc[LEN_AREADESC+1];
} DESCLIST, *PDESCLIST;

/*---------------------------- Globale Variablen ----------------------------*/

static const char * const KeywordTable[]=
{
   "Netmail",
   "Outbound",
   "AreaFile",
   "Origin",
   "Name",
   "Address",
   "Desc",
   NULL
};

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int QueryKeyword(char *pchWord);
static int AddAddress(USERDATAOPT *pUserData, char *pchAddress);
static int AddName(USERDATAOPT *pUserData, char *pchName);
static int AddOrigin(USERDATAOPT *pUserData, char *pchLine);
static int ReadTmCfg(char *pchFileName,
                     char *pchNameBuffer, char *pchOutbound,
                     char *pchNetmail, USERDATAOPT *pUserData, PDESCLIST *ppDescList);
static int ReadTmBbs(char *pchFileName, char *TmBbsName, char *NetmailPath,
                     USERDATAOPT *pUserData, PDRIVEREMAP pDriveRemap, PAREALIST pRetList, PDESCLIST pDescList);
static char *CopyPath(char *pchDest, char *pchSrc);
static void CopyOutbound(OUTBOUND *pOutbounds, USERDATAOPT *pUserdata, char *pchFileName, char *OutboundPath);
static void AddNetmailArea(AREADEFOPT *pAreaDef, char *pchFileName, char *NetmailPath,
                           USERDATAOPT *pUserData, PAREALIST pRetList);
static void FreeDesc(PDESCLIST pDescList);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadTerMailCfg                                             */
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

int ReadTerMailCfg(char *pchFileName, USERDATAOPT *pUserdata,
                   OUTBOUND *pOutbounds, PAREALIST pRetList, PDRIVEREMAP pDriveRemap, ULONG ulOptions)
{
   int rc;
   char TmBbsName[LEN_PATHNAME+1]="";
   char OutboundPath[LEN_PATHNAME+1]="";
   char NetmailPath[LEN_PATHNAME+1]="";
   USERDATAOPT UserData;
   PDESCLIST pDescList=NULL;

   memset(&UserData, 0, sizeof(UserData));

   /* TM_CFG lesen */
   if (rc = ReadTmCfg(pchFileName, TmBbsName,
                      OutboundPath, NetmailPath, &UserData,
                      (ulOptions & READCFG_AREAS)?(&pDescList):NULL))
   {
      if (pDescList)
         FreeDesc(pDescList);
      return rc;
   }

   if (ulOptions & READCFG_USERDATA)
      *pUserdata = UserData;

   if (ulOptions & READCFG_OUTBOUNDS)
      if (OutboundPath[0])
         CopyOutbound(pOutbounds, &UserData, pchFileName, OutboundPath);

   /* TM.BBS lesen */
   if (ulOptions & READCFG_AREAS)
      if (rc = ReadTmBbs(pchFileName, TmBbsName, NetmailPath, &UserData, pDriveRemap, pRetList, pDescList))
      {
         if (pDescList)
            FreeDesc(pDescList);
         return rc;
      }

   if (pDescList)
      FreeDesc(pDescList);

   return CFGFILE_OK;
}

static int ReadTmCfg(char *pchFileName,
                     char *pchNameBuffer, char *pchOutbound,
                     char *pchNetmail, USERDATAOPT *pUserData, PDESCLIST *ppDescList)
{
   FILE *pfTmCfg;
   char zeile[200];

   pfTmCfg=fopen(pchFileName, "r");

   if (pfTmCfg)
   {
      while (!feof(pfTmCfg))
      {
         if (fgets(zeile, sizeof(zeile), pfTmCfg))
         {
            /* Whitespace u. Kommentare entfernen */
            StripWhitespace(zeile);

            if (zeile[0]) /* Zeile m. Inhalt */
            {
               char *pchKeyword;
               char *pchParam, *pchParam2;
               char *pchDup = strdup(zeile);

               pchKeyword = strtok(zeile, " \t");

               if (pchKeyword)
               {
                  pchParam = strtok(NULL, " \t");

                  switch(QueryKeyword(pchKeyword))
                  {
                     case KEY_NETMAIL:
                        if (!pchNetmail[0] && pchParam)
                        {
                           strcpy(pchNetmail, pchParam);
                           RemoveBackslash(pchOutbound);
                        }
                        break;

                     case KEY_OUTBOUND:
                        if (!pchOutbound[0] && pchParam)
                        {
                           strcpy(pchOutbound, pchParam);
                           RemoveBackslash(pchOutbound);
                        }
                        break;

                     case KEY_AREAFILE:
                        if (!pchNameBuffer[0] && pchParam)
                           strcpy(pchNameBuffer, pchParam);
                        break;

                     case KEY_ORIGIN:
                        if (pchParam)
                           AddOrigin(pUserData, pchDup);
                        break;

                     case KEY_NAME:
                        if (pchParam)
                           AddName(pUserData, pchDup);
                        break;

                     case KEY_ADDRESS:
                        if (pchParam)
                           AddAddress(pUserData, pchParam);
                        break;

                     case KEY_ADESC:
                        if (ppDescList)
                        {
                           pchParam2 = strtok(NULL, " \t");
                           if (pchParam2)
                           {
                              PDESCLIST pNewDesc;
                              pNewDesc = calloc(1, sizeof(DESCLIST));
                              if (pNewDesc)
                              {
                                 char *pchTemp;

                                 /* vorne einhaengen */
                                 pNewDesc->next = *ppDescList;
                                 *ppDescList = pNewDesc;

                                 /* Daten */
                                 strncpy(pNewDesc->pchAreaTag, pchParam, LEN_AREATAG);
                                 strncpy(pNewDesc->pchDesc, pchParam2, LEN_AREADESC);

                                 /* _ umwandeln */
                                 pchTemp = pNewDesc->pchDesc;
                                 while (*pchTemp)
                                 {
                                    if (*pchTemp == '_')
                                       *pchTemp = ' ';
                                    pchTemp++;
                                 }
                              }
                           }
                        }
                        break;

                     default:
                        /* unbekannt/unbehandelt */
                        break;
                  }
               }
               free(pchDup);
            }
         }
         else
            if (ferror(pfTmCfg))
            {
               /* Fehler beim Lesen der Datei */
               fclose(pfTmCfg);

               return CFGFILE_READ;
            }
      }
      fclose(pfTmCfg);

      return CFGFILE_OK;
   }
   else
      return CFGFILE_OPEN;
}


static int ReadTmBbs(char *pchFileName, char *TmBbsName, char *NetmailPath,
                     USERDATAOPT *pUserData, PDRIVEREMAP pDriveRemap, PAREALIST pRetList, PDESCLIST pDescList)
{
   char pchBBSName[LEN_USERNAME+1];
   FILE *pfBBS;
   AREADEFOPT Area;

   if (NetmailPath[0])
      AddNetmailArea(&Area, pchFileName, NetmailPath, pUserData, pRetList);

   if (TmBbsName[0] && TmBbsName[1] == ':')
   {
      /* voller Name */
      strcpy(pchBBSName, TmBbsName);
      MSG_RemapDrive(pchBBSName, pDriveRemap);
   }
   else
   {
      /* Teil-Name */
      CopyPath(pchBBSName, pchFileName);
      strcat(pchBBSName, "\\");
      strcat(pchBBSName, TmBbsName);
   }

   pfBBS = fopen(pchBBSName, "r");

   if (pfBBS)
   {
      char zeile[200];
      char *pchTemp;

      while (!feof(pfBBS))
      {
         if (fgets(zeile, sizeof(zeile), pfBBS))
         {
            StripWhitespace(zeile);
            if (zeile[0] && zeile[0] != '@')
            {
               BOOL bNotValid = FALSE;

               memset(&Area, 0, sizeof(Area));
               Area.areaformat = AREAFORMAT_FTS; /* Default */
               Area.areatype = AREATYPE_ECHO;    /* Default */
               strcpy(Area.username, pUserData->username[0]);
               strcpy(Area.address, pUserData->address[0]);
               Area.ulAreaOpt = AREAOPT_FROMCFG;

               /* Pfad ermitteln */
               pchTemp = strtok(zeile, " \t");

               if (pchTemp)
               {
                  if (pchTemp[0] && pchTemp[1] == ':')
                     /* voller Pfad */
                     strncpy(Area.pathfile, pchTemp, LEN_PATHNAME);
                  else
                  {
                     /* relativer pfad */
                     CopyPath(Area.pathfile, pchFileName);
                     strcat(Area.pathfile, "\\");
                     strcat(Area.pathfile, pchTemp);
                     Area.ulTempFlags = AREAFLAG_NOREMAP;
                  }
                  RemoveBackslash(Area.pathfile);

                  if (pchTemp = strtok(NULL, " \t"))
                  {
                     PDESCLIST pDesc = pDescList;

                     strncpy(Area.areatag, pchTemp, LEN_AREATAG);

                     /* Description suchen */
                     while (pDesc && stricmp(pDesc->pchAreaTag, pchTemp))
                        pDesc = pDesc->next;

                     if (pDesc)
                        strcpy(Area.areadesc, pDesc->pchDesc);
                     else
                        strncpy(Area.areadesc, pchTemp, LEN_AREADESC);

                     while (pchTemp = strtok(NULL, " \t"))
                     {
                        if (!stricmp(pchTemp, "JAM"))
                           Area.areaformat = AREAFORMAT_JAM;
                        else
                           if (!stricmp(pchTemp, "HOLD"))
                              Area.areatype = AREATYPE_LOCAL;
                           else
                              if (!stricmp(pchTemp, "QWK"))
                              {
                                 bNotValid = TRUE;
                                 break;
                              }
                     }
                     if (!bNotValid)
                        AM_AddArea(pRetList, &Area, ADDAREA_TAIL | ADDAREA_UNIQUE);
                  }
               }
            }
         }
         else
            if (ferror(pfBBS))
            {
               fclose(pfBBS);
               return CFGFILE_READ;
            }
      }
      fclose(pfBBS);
      return CFGFILE_OK;
   }
   else
      return CFGFILE_OPEN;
}

static void AddNetmailArea(AREADEFOPT *pAreaDef, char *pchFileName, char *NetmailPath,
                           USERDATAOPT *pUserData, PAREALIST pRetList)
{
   memset(pAreaDef, 0, sizeof(AREADEFOPT));

   if (NetmailPath[0] && NetmailPath[1] == ':')
      strncpy(pAreaDef->pathfile, NetmailPath, LEN_PATHNAME);
   else
   {
      CopyPath(pAreaDef->pathfile, pchFileName);
      strcat(pAreaDef->pathfile, "\\");
      strcat(pAreaDef->pathfile, NetmailPath);
      pAreaDef->ulTempFlags = AREAFLAG_NOREMAP;
   }

   strcpy(pAreaDef->areatag, "NETMAIL");
   strcpy(pAreaDef->areadesc, "Netmail");
   strcpy(pAreaDef->username, pUserData->username[0]);
   strcpy(pAreaDef->address, pUserData->address[0]);
   pAreaDef->areatype = AREATYPE_NET;
   pAreaDef->areaformat = AREAFORMAT_FTS;
   pAreaDef->ulAreaOpt = AREAOPT_FROMCFG;
   pAreaDef->ulDefAttrib = ATTRIB_PRIVATE;

   AM_AddArea(pRetList, pAreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);

   return;
}

/*------------------------------ StripWhitespace ----------------------------*/
/* Entfernt Leerzeichen, Tabs und Newlines am Anfang und Ende eines Strings  */
/*---------------------------------------------------------------------------*/

char *StripWhitespace(char *string)
{
   char *pchAnf=string;
   char *pchStart=string;

   /* Whitespace am Anfang uebergehen */
   while (*pchAnf == ' ' ||
          *pchAnf == '\t')
     pchAnf++;

   /* Weiter bis zum Ende oder Kommentar */
   while (*pchAnf &&
          *pchAnf != '\n' &&
          *pchAnf != '%')
   {
      /* nach vorne kopieren */
      *pchStart++ = *pchAnf;

      /* Wenn Whitespace kopiert, folgende Whitespaces ueberlesen */
      if (*pchAnf == ' ' ||
          *pchAnf == '\t')
      {
         pchAnf++;
         while (*pchAnf == ' ' ||
                *pchAnf == '\t')
            pchAnf++;
      }
      else
         pchAnf++;
   }
   *pchStart = 0;

   return string;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryKeyword                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Analysiert das Wort und gibt zurueck, um welches Keyword    */
/*               es sich handelt                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchWord: Wort                                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: KEY_*    Keyword gefunden                                  */
/*                -1       Keyword nicht gefunden                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int QueryKeyword(char *pchWord)
{
   int i=0;

   while (KeywordTable[i] && stricmp(pchWord, KeywordTable[i]))
      i++;

   if (KeywordTable[i])
      return i;
   else
      return -1;
}

static int AddAddress(USERDATAOPT *pUserData, char *pchAddress)
{
   int i=0;

   while (i < MAX_ADDRESSES && pUserData->address[i][0] && stricmp(pchAddress, pUserData->address[i]))
      i++;

   if (i < MAX_ADDRESSES)
   {
      strncpy(pUserData->address[i], pchAddress, LEN_5DADDRESS);
      return 0;
   }
   else
      return 1;
}

static int AddName(USERDATAOPT *pUserData, char *pchName)
{
   int i=0;

   while (*pchName && *pchName != ' ')
      pchName++;
   while (*pchName == ' ')
      pchName++;

   if (!pchName[0])
      return 1;

   while (i < MAX_USERNAMES && pUserData->username[i][0] && stricmp(pchName, pUserData->username[i]))
      i++;

   if (i < MAX_USERNAMES)
   {
      strncpy(pUserData->username[i], pchName, LEN_USERNAME);
      return 0;
   }
   else
      return 1;
}

static int AddOrigin(USERDATAOPT *pUserData, char *pchLine)
{
   while (*pchLine && *pchLine != ' ')
      pchLine++;
   while (*pchLine == ' ')
      pchLine++;

   if (!pchLine[0])
      return 1;

   if (pUserData->defaultorigin[0])
      return 1;
   else
   {
      strncpy(pUserData->defaultorigin, pchLine, LEN_ORIGIN);
      return 0;
   }
}

static void CopyOutbound(OUTBOUND *pOutbounds, USERDATAOPT *pUserdata, char *pchFileName, char *OutboundPath)
{
   FTNADDRESS Address;

   memset(pOutbounds, 0, sizeof(OUTBOUND)*MAX_ADDRESSES);

   /* Pfad konstruieren */
   if (OutboundPath[0] && OutboundPath[1] == ':')
   {
      /* kompletter Pfad */
      strcpy(pOutbounds[0].outbound, OutboundPath);
   }
   else
   {
      /* Teilpfad */
      CopyPath(pOutbounds[0].outbound, pchFileName);
      strcat(pOutbounds[0].outbound, "\\");
      strcat(pOutbounds[0].outbound, OutboundPath);
   }

   StringToNetAddr(pUserdata->address[0], &Address, NULL);
   pOutbounds[0].zonenum = Address.usZone;

   return;

}

static char *CopyPath(char *pchDest, char *pchSrc)
{
   char *pchTemp;

   strcpy(pchDest, pchSrc);

   pchTemp = strrchr(pchDest, '\\');
   if (pchTemp)
      *pchTemp = 0;

   return pchDest;
}

static void FreeDesc(PDESCLIST pDescList)
{
   PDESCLIST pNext;

   while (pDescList)
   {
      pNext = pDescList->next;
      free(pDescList);
      pDescList = pNext;
   }
   return;
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