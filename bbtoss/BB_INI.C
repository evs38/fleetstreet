/*---------------------------------------------------------------------------+
 | Titel: BB_INI.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von:                           | Am: 27.11.1999                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   BBToss Configuration .INI File                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:  ab BBToss 2.30 bbtaka.cfg obsolete (Aka's in bbtoss.ini)    |
 |               ab BBToss 2.10 main config bbtoss.ini (bbtoss.cfg obsolete) |
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

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadBBIni(char *pchFileName, OUTBOUND *pOutbound, USERDATAOPT *pUserData)
/* bb: ReadBBIni ersetzt:
		static int ReadBBCfg(char *pchFileName, OUTBOUND *pOutbound);
		static int ReadBBAka(char *pchFileName, USERDATAOPT *pUserData);

	Alle anderen Routinen aus original bb_cfg.c unveraendert
*/
/* static int ReadBBCfg(char *pchFileName, OUTBOUND *pOutbound); */
/* static int ReadBBAka(char *pchFileName, USERDATAOPT *pUserData); */

static int ReadBBAlias(char *pchFileName, USERDATAOPT *pUserData);
static int ReadBBTossAreas(char *pchFileName, USERDATAOPT *pUserData, PAREALIST pRetList);
static void AddNetmailArea(PCHAR pchMailDir, USERDATAOPT *pUserData, PAREADEFOPT pAreaDef, PAREALIST pRetList);
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
        /* to call with pchFileName = path\BBTOSS.INI  not path\BBTOSS.CFG*/
int ReadBBTossCfg(char *pchFileName, USERDATAOPT *pUserdata, OUTBOUND *pOutbound,
                    PAREALIST pRetList, PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;

   driveremap = driveremap;
   memset(pchMailDir, 0, sizeof(pchMailDir));

   /* BB_INI lesen */
   if (rc = ReadBBIni(pchFileName, pOutbound, pUserData))
      return rc;
   else /* bb: readBBAka entfernt, da Aka in bbtoss.ini enthalten */
      if (rc = ReadBBAlias(pchFileName, pUserdata))
         return rc;

   if (ulOptions & READCFG_AREAS)
      if (rc = ReadBBTossAreas(pchFileName, pUserdata, pRetList))
         return rc;

   return CFGFILE_OK;
}

/* bb: original ReadBBCfg - obsolete, ersetzt durch ReadBBIni
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
|             Zeile zerlegen
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

bb: original ReadBBAka - obsolete - in ReadBBIni enthalten
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
|             Zeile zerlegen
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
*/

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
   int  isection; /* Merker welche Ini-file section aktuell ist */
   char *ptag;
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
         	if (!stricmp (zeile, "[BBTAKA]")) isection= BBTAKA_SECTION;
         	if (!stricmp (zeile, "[BBTPATH]")) isection= BBTPATH_SECTION;
         	if (!stricmp (zeile, "[NETMAILFOLDER]")) isection=BBTNETMAILFOLDER_SECTION;
      	}
      	else
         /* Zeile zerlegen, wenn kein Section Header, dann "Tag=Value" Zeile (oder Leerzeile) */
      	{
            ptag = strtok(zeile, " =\t\n");
         	if (ptag)
         	{
               pchValue = strtok(NULL, " \t\n");
         	}
      	}

      	switch (isection) /* Keywords aus aktueller Section setzen */
      	{
         	case BBTAKA_SECTION : /* [BBTAKA] */
            {
            	FTNADDRESS Address;

            	if (!stricmp (ptag, "MAINAKA") && (akanum < MAX_ADDRESSES))
            	{
               	/* set Main-Aka/Aka (pchValue)*/
               	StringToNetAddr(pchValue, &Address, NULL);
               	NetAddrToString(pUserData->address[akanum++], &Address);
            	}

            	if (!stricmp (ptag, "AKA"))
            	{
               	/* set Aka (pchValue)*/
               	StringToNetAddr(pchValue, &Address, NULL);
               	NetAddrToString(pUserData->address[akanum++], &Address);
            	}
            	break;
            }
         	case BBTPATH_SECTION : /* [BBTPATH] */
            {
            	if (!stricmp(ptag, "OUTBOUND"))
            	{
               	/* set outbound (pchValue)*/
                  strcpy(pOutbound->outbound, pchValue);
                  RemoveBackslash(pOutbound->outbound);
                  pOutbound->zonenum = 0;
            	}
            	break;
            }
         	case BBTNETMAILFOLDER_SECTION : /*[NETMAILFOLDER]*/
            {
            	if (!stricmp(ptag, "BASETYP")) /* not neccessarly N = *.MSG */
            	{
            	}

            	if (!stricmp(ptag, "PATH"))
            	{
               	/* set netmailfolder path */
               	/* make sure BASETYP is supported  'N' = *.MSG */
                  strcpy(pchMailDir, pchValue);
                  RemoveBackslash(pchMailDir);
            	}
            	break;
            }
         	default: /* andere section */
            {
            	break;
            }
         } /*switch*/

      } /*if fgets */
      else
      	if (ferror(pfCfg))
            rc = CFGFILE_READ;
   } /* while */

   fclose(pfIni);
   return rc;

} /* if pfIni */
else
	return CFGFILE_OPEN;

}

/* bb: ab hier original unveraendert */


/*-------------------------------- Modulende --------------------------------*/

