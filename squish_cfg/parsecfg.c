/*---------------------------------------------------------------------------+
 | Titel: ParseCfg.c                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 14.06.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Parst die Squish.Cfg, um die Echo-Informationen zu ermitteln            |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define OS_2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_BASE
#define INCL_WIN
#include <os2.h>
#include "../main.h"
#include "../msgheader.h"
#include "../structs.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "../util/fltutil.h"
#include "../util/addrcnv.h"
#include "../handlemsg/handlemsg.h"
#include "parsecfg.h"

static char *Keywords[]=
{
   "EchoArea",
   "LocalArea",
   "NetArea",
   "DupeArea",
   "BadArea",
   "Address",
   "Origin",
   "Outbound",
   "AreasBBS",
   "Include",
   NULL
};

#define KEYWORD_ECHO      0
#define KEYWORD_LOCAL     1
#define KEYWORD_NET       2
#define KEYWORD_DUPE      3
#define KEYWORD_BAD       4
#define KEYWORD_ADDRESS   5
#define KEYWORD_ORIGIN    6
#define KEYWORD_OUTBOUND  7
#define KEYWORD_AREASBBS  8
#define KEYWORD_INCLUDE   9

static int QueryKeyword(char *pchTest);
static int ParseAreasBBS(PAREALIST pListe, char *pchAreasBBS, USERDATAOPT *userdaten);


/*---------------------------------------------------------------------------*/
/* Funktionsname: ParseAreasBBS                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liest die Definitionen in AREAS.BBS ein                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pListe: Erzeugte Arealiste                                     */
/*            pchAreasBBS: Filename der AREAS.BBS                            */
/*            userdaten: Default-User-Einstellungen                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0 OK, Liste erzeugt                                        */
/*                1 keine Area-Definition                                    */
/*                2 Fehlerhafte Area-Definition                              */
/*                3 Lesefehler                                               */
/*                4 Kann Datei nicht lesen                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int ParseAreasBBS(PAREALIST pListe, char *pchAreasBBS, USERDATAOPT *userdaten)
{
   FILE *fileAreasBBS=NULL;
   char zeile[500]="";
   BOOL bFirstline=TRUE;
   char *pchPath=NULL;
   char *pchTag=NULL;
   BOOL bIsSquish=FALSE;
   BOOL bErrors=FALSE;
   AREADEFOPT AreaDef;

   fileAreasBBS=fopen(pchAreasBBS, "r");

   if (!fileAreasBBS)
      return 4;
   else
   {
      while (!feof(fileAreasBBS))
      {
         if (fgets(zeile, sizeof(zeile), fileAreasBBS))
         {
            bIsSquish=FALSE;

            StripWhitespace(zeile);
            pchPath=strtok(zeile, " \t\n");
            if (!pchPath)
               continue;    /* leere Zeile */
            if (bFirstline)
            {
               /* Erste Zeile ueberspringen */
               bFirstline=FALSE;
               continue;
            }
            if (*pchPath == '#')
               continue;    /* passthru-Area */
            if (*pchPath == '$')
            {
               bIsSquish=TRUE;   /* Squish-Area */
               pchPath++;
            }
            if (!(pchTag=strtok(NULL, " \t\n")))
            {
               bErrors=TRUE;
               continue;
            }

            /* Jetzt haben wir eine Area */
            memset(&AreaDef, 0, sizeof(AreaDef));

            /* Daten eintragen */
            strncpy(AreaDef.areatag, pchTag, LEN_AREATAG);
            strncpy(AreaDef.pathfile, pchPath, LEN_PATHNAME);
            strncpy(AreaDef.areadesc, pchTag, LEN_AREATAG);
            strcpy(AreaDef.areadesc, AreaDef.areatag);
            strcpy(AreaDef.username, userdaten->username[0]);
            strcpy(AreaDef.address, userdaten->address[0]);
            if (bIsSquish)
               AreaDef.areaformat=AREAFORMAT_SQUISH;
            else
               AreaDef.areaformat=AREAFORMAT_FTS;
            AreaDef.areatype = AREATYPE_ECHO;
            AreaDef.ulAreaOpt = AREAOPT_FROMCFG;

            /* Area anhaengen */
            AM_AddArea(pListe, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
         }
      }
      fclose(fileAreasBBS);
      if (bErrors)
         return 2;

      if (!pListe->ulNumAreas)
         return 1;
      else
         return 0;
   }
}

static int QueryKeyword(char *pchTest)
{
   char *pchKey = Keywords[0];
   int i=0;

   while (pchKey && stricmp(pchKey, pchTest))
      pchKey = Keywords[++i];

   if (pchKey)
      return i;
   else
      return -1;
}

int ParseSquishCfg(char *pchFileName, USERDATAOPT *pUserdata,
                  OUTBOUND *pOutbounds, PAREALIST pRetList,
                  PDRIVEREMAP driveremap, ULONG ulOptions)
{
   FILE *infile=NULL;
   char zeile[500];
   char *keyword;
   char *token;
   int rc=CFGFILE_OK;
   AREADEFOPT Area;
   int iAddress=0;

   if (infile = fopen(pchFileName, "r"))
   {
      while (!rc && !feof(infile))
      {
         if (fgets(zeile, sizeof(zeile),infile))
         {
            StripWhitespace(zeile);

            /* leere Zeile und Passthru uebergehen */
            if (zeile[0] && !strstr(zeile,"-0"))
            {
               int keyw;
               FTNADDRESS NetAddr;
               char *pchPath;
               char *pchZone;
               int i, iZone;

               keyword=strtok(zeile," \t");

               switch(keyw = QueryKeyword(keyword))
               {
                  case KEYWORD_ECHO:
                  case KEYWORD_LOCAL:
                  case KEYWORD_NET:
                  case KEYWORD_DUPE:
                  case KEYWORD_BAD:
                     if (!(ulOptions & READCFG_AREAS))
                        break;

                     memset(&Area, 0, sizeof(AREADEFOPT));
                     Area.areaformat = AREAFORMAT_FTS;
                     Area.ulAreaOpt = AREAOPT_FROMCFG;
                     switch(keyw)
                     {
                        case KEYWORD_ECHO:
                           Area.areatype = AREATYPE_ECHO;
                           break;

                        case KEYWORD_LOCAL:
                           Area.areatype = AREATYPE_LOCAL;
                           break;

                        case KEYWORD_NET:
                        case KEYWORD_DUPE:
                        case KEYWORD_BAD:
                           Area.areatype = AREATYPE_NET;
                           Area.ulDefAttrib = ATTRIB_PRIVATE;
                           break;
                     }
                     /* Rest der Zeile analysieren */
                     while (token=strtok(NULL," \t"))
                     {
                        /* -$, -$m, -$d oder -$s sind Squish-Kennzeichen */
                        if (token[0]== '-')
                        {
                           switch(token[1])
                           {
                              case '$':
                                 Area.areaformat = AREAFORMAT_SQUISH;
                                 break;

                              case 'p':
                                 /* andere Primary-Address */
                                 StringToNetAddr(token+2, &NetAddr, NULL);
                                 NetAddrToString(Area.address, &NetAddr);
                                 break;

                              /* Alle uebrigen Optionen ignorieren */
                              default:
                                 break;
                           }
                        }
                        else
                           /* Ansonsten kommt der Areaname */
                           if (!Area.areatag[0])
                              strncpy(Area.areatag, token, LEN_AREATAG);
                           else
                              /* danach kommt der Pfadname bzw. der Pfad */
                              if (!Area.pathfile[0])
                                 strncpy(Area.pathfile, token, LEN_PATHNAME);
                     }
                     /* Default-Adresse eintragen */
                     if (!Area.address[0])
                        strncpy(Area.address, pUserdata->address[0], LEN_5DADDRESS);
                     strcpy(Area.areadesc, Area.areatag);
                     strcpy(Area.username, pUserdata->username[0]);

                     /* Ueberpruefung */
                     if (Area.areatag[0] && Area.pathfile[0] && Area.address[0])
                     {
                        /* Area einfuegen */
                        PAREADEFLIST pOldArea = AM_FindArea(pRetList, Area.areatag);

                        if (pOldArea)
                        {
                           /* Daten updaten */
                           pOldArea->areadata = Area;
                        }
                        else
                           /* neue Area */
                           AM_AddArea(pRetList, &Area, ADDAREA_TAIL | ADDAREA_UNIQUE);
                     }
                     break;

                  case KEYWORD_ADDRESS:
                     if (!(ulOptions & READCFG_USERDATA))
                        break;

                     iAddress = 0;
                     while (iAddress < MAX_ADDRESSES && pUserdata->address[iAddress][0])
                        iAddress++;

                     token = strtok(NULL, " \t");
                     if (token && token[0] && iAddress < MAX_ADDRESSES)
                     {
                        StringToNetAddr(token, &NetAddr, NULL);
                        NetAddrToString(pUserdata->address[iAddress], &NetAddr);
                     }
                     break;

                  case KEYWORD_ORIGIN:
                     if (!(ulOptions & READCFG_USERDATA))
                        break;

                     if (!pUserdata->defaultorigin[0])
                     {
                        char *pchTemp;
                        int iTemp;

                        pchTemp=zeile+strlen(keyword)+1;
                        iTemp=strspn(pchTemp, " ");

                        if (iTemp<strlen(pchTemp))
                           strncpy(pUserdata->defaultorigin, pchTemp+iTemp, LEN_ORIGIN);
                     }
                     pUserdata->defaultorigin[LEN_ORIGIN]='\0';
                     break;

                  case KEYWORD_OUTBOUND:
                     if (!(ulOptions & READCFG_OUTBOUNDS))
                        break;

                     pchPath=strtok(NULL, " \t");
                     pchZone=strtok(NULL, " \t");

                     if (pchZone)
                        iZone=strtol(pchZone, NULL, 10);
                     else
                     {
                        StringToNetAddr(pUserdata->address[0], &NetAddr, NULL);
                        iZone=NetAddr.usZone;
                     }
                     i=0;
                     while (i<MAX_ADDRESSES && iZone!=pOutbounds[i].zonenum && pOutbounds[i].zonenum!=0)
                        i++;

                     if (i<MAX_ADDRESSES)
                     {
                        pOutbounds[i].zonenum=iZone;
                        strncpy(pOutbounds[i].outbound, pchPath, LEN_PATHNAME);
                        RemoveBackslash(pOutbounds[i].outbound);
                     }
                     break;

                  case KEYWORD_AREASBBS:
                  case KEYWORD_INCLUDE:
                     pchPath = strtok(NULL, " \t");
                     if (pchPath)
                     {
                        char pchIncFile[LEN_PATHNAME+1];

                        if (pchPath[1] != ':' && pchPath[0] != '\\')
                        {
                           /* Pfad ergaenzen */
                           strcpy(pchIncFile, pchFileName);
                           token = strrchr(pchIncFile, '\\');
                           if (token)
                           {
                              token++;
                              strcpy(token, pchPath);
                           }
                           else
                              strcpy(pchIncFile, pchPath);
                        }
                        else
                        {
                           /* Remap */
                           strcpy(pchIncFile, pchPath);
                           MSG_RemapDrive(pchIncFile, driveremap);
                        }
                        if (keyw == KEYWORD_AREASBBS)
                        {
                           if (ulOptions & READCFG_AREAS)
                              if (ParseAreasBBS(pRetList, pchIncFile, pUserdata) >= 3)
                                 rc = CFGFILE_READ;
                        }
                        else
                           /* Include */
                           rc = ParseSquishCfg(pchIncFile, pUserdata, pOutbounds,
                                               pRetList, driveremap, ulOptions);
                     }
                     break;

                  default:
                     break;
               }
            }
         }
         else
            if (ferror(infile))
               rc = CFGFILE_READ;
      }

      fclose(infile);
      return rc;
   }
   else
      return CFGFILE_OPEN;
}
/*------------------------------- Modulende ---------------------------------*/

