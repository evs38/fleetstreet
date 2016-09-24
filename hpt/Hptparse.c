/*---------------------------------------------------------------------------+
 | Titel: ParseCfg.c                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Alexander A. Batalov      | Am: 2.08.1999                   |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Parst die Hpt config, um die Echo-Informationen zu ermitteln            |
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
#include "main.h"
#include "msgheader.h"
#include "structs.h"
#include "areaman/areaman.h"
#include "cfgfile_interface.h"
#include "util/fltutil.h"
#include "util/addrcnv.h"
#include "handlemsg/handlemsg.h"
#include "hpt/hptparse.h"

static char *Keywords[]=
{
   "EchoArea",
   "NetMailArea",
   "BadArea",
   "DupeArea",
   "LocalArea",
   "Sysop",
   "Address",
   "Outbound",
   "AreaDescription",
   "Include",
   NULL
};

#define KEYWORD_ECHO      0
#define KEYWORD_NET       1
#define KEYWORD_BAD       2
#define KEYWORD_DUPE      3
#define KEYWORD_LOCAL     4
#define KEYWORD_SYSOP     5
#define KEYWORD_ADDRESS   6
#define KEYWORD_OUTBOUND  7
#define KEYWORD_AREASDESC 8
#define KEYWORD_INCLUDE   9

#if 0
#define MAXSYSNAME           20       /* max # names for PERSMAIL */
#endif

static int QueryKeyword(char *pchTest);
static int AddName(USERDATAOPT *pUserData, char *pchName);

static int QueryKeyword(char *pchTest)
{
   char *pchKey = Keywords[0];
   int i=0;

   while (pchKey && stricmp(pchKey, strupr(pchTest)))
      pchKey = Keywords[++i];

   if (pchKey)
      return i;
   else
      return -1;
}


int ParseHptCfg(char *pchFileName, USERDATAOPT *pUserdata,
                  OUTBOUND *pOutbounds, PAREALIST pRetList,
                  PDRIVEREMAP driveremap, ULONG ulOptions)
{
   FILE *infile=NULL;
   char zeile[500];
   char *keyword;
   char *token;
   char *pchDup;
   char *upZeile;
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
            pchDup  = strdup(zeile);
            upZeile = strdup(zeile);

            /* leere Zeile und Passthru uebergehen (!zeile[0]=='#' && ) */
            if (zeile[0] && !strstr(strupr(upZeile),"PASSTHROUGH"))
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
                  case KEYWORD_NET:
                  case KEYWORD_DUPE:
                  case KEYWORD_BAD:
                  case KEYWORD_LOCAL:
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
                        if (token[0]== '-')
                        {
                           switch(token[1])
                           {
                              case 'a':
                              case 'A':
                                token=strtok(NULL," \t");
                                /* andere Primary-Address */
                                StringToNetAddr(token, &NetAddr, NULL);
                                NetAddrToString(Area.address, &NetAddr);
                                break;
                              /*area descriptions*/
                              case 'b':
                              case 'B':
                                token=strtok(NULL," \t");
                                if (stricmp(strupr(token),"SQUISH") == 0)
                                   Area.areaformat = AREAFORMAT_SQUISH;
                                else
                                   if (stricmp(strupr(token),"JAM") == 0)
                                      Area.areaformat = AREAFORMAT_JAM;
                                   else
                                      if (stricmp(strupr(token),"MSG") == 0)
                                         Area.areaformat = AREAFORMAT_FTS;
                                break;

                              case 'd':
                              case 'D':
                                if (token[2]=='\0')
                                {
                                   token=strtok(NULL,"\x22\t");
#if 0
                                   *token++;
                                   token_len = strlen(token);
                                   while(token[token_len - 2]!='"')
                                   {
                                      token=strtok(zeile," \t");
                                      token_len = strlen(token);
                                   }
#endif
                                   strncpy(Area.areadesc, token, LEN_AREADESC);
                                }
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
                     if (!Area.areadesc[0])
                        strcpy(Area.areadesc, Area.areatag);
                     strcpy(Area.username, pUserdata->username[0]);

                     /* Ueberpruefung */
                     strupr(Area.areatag);
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

                  case KEYWORD_SYSOP:
                     if(!(ulOptions & READCFG_USERDATA))
                       break;

                     AddName(pUserdata, pchDup);

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

                  /*case KEYWORD_AREASDESC:*/
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
                        /* Include */
                        rc = ParseHptCfg(pchIncFile, pUserdata, pOutbounds,
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

/*add sysop name*/
static int AddName(USERDATAOPT *pUserData, char *pchName)
{
   int i=0;

   while (*pchName && *pchName != ' ' && *pchName != '\t')
      pchName++;
   while (*pchName == ' ' || *pchName == '\t')
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

/*------------------------------- Modulende ---------------------------------*/

