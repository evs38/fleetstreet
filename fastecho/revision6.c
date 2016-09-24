/*---------------------------------------------------------------------------+
 | Titel: REVISION6.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 16.02.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Einlesen der Fastecho.Cfg Revision 6                                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

/*#define DEBUG*/

#pragma strings(readonly)

#include <os2.h>
#include "../main.h"
#include "../structs.h"
#include <stdio.h>
#include <string.h>
#ifdef DEBUG
   #include <errno.h>
#endif
#include "fecfg142.h"
#include "..\areaman\areaman.h"
#include "..\cfgfile_interface.h"
#include "..\msgheader.h"
#include "..\util\addrcnv.h"
#include "common.h"
#include "revision6.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static CONFIG Config;
static SysAddress Akas[MAX_AKAS];
static OriginLines Origins[MAX_ORIGINS];

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadExtensions(FILE *pfConfig);
static int ReadAreas(FILE *pfConfig, PAREALIST pRetList);
static int CopyUserData(USERDATAOPT *pUserData, SysAddress *Akas, OriginLines *pOrigins);
static int CopyOutbounds(OUTBOUND *pOutbounds, SysAddress *Akas);

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
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

int ReadRevision6(FILE *pfConfig, USERDATAOPT *pUserdata,
                  OUTBOUND *pOutbounds, PAREALIST pRetList,
                  ULONG ulOptions)
{
   int rc;

   memset(&Config, 0, sizeof(Config));
   memset(Akas, 0, sizeof(Akas));

#ifdef DEBUG
   printf("Revision 6\n");
#endif

   /* zurÅcksetzen */
   fseek(pfConfig, 0, SEEK_SET);

   /* CONFIG einlesen */
   if (1==fread(&Config, sizeof(Config), 1, pfConfig))
   {
#ifdef DEBUG
      printf("INF: Areas: %d\n", Config.AreaCnt);
      printf("INF: Akas: %d\n",  Config.AkaCnt);
      printf("INF: Outb: %s\n",  Config.OutBound);
#endif
      if (rc = ReadExtensions(pfConfig))
         return rc;
      else
      {
         if (ulOptions & READCFG_USERDATA)
            CopyUserData(pUserdata, Akas, Origins);

         if (ulOptions & READCFG_OUTBOUNDS)
            CopyOutbounds(pOutbounds, Akas);

         if ((ulOptions & READCFG_AREAS) && (rc = ReadAreas(pfConfig, pRetList)))
            return rc;
         else
            return CFGFILE_OK;
      }
   }
   else
   {
#ifdef DEBUG
      printf("ERROR: Config unreadable (%d)\n", errno);
#endif
      return CFGFILE_READ;
   }

}

static int ReadExtensions(FILE *pfConfig)
{
   long offset=0;
   ExtensionHeader Header;
#if 0
   SysopNames SysName;
#endif
   SysAddress SysAd;
   OriginLines OneOrigin;
   int i;

   while (offset < Config.offset-1)
   {
      if (1==fread(&Header, sizeof(Header), 1, pfConfig))
      {
         switch(Header.type)
         {
#if 0
            case EH_SYSOPNAMES:
               for (i=0; i < Config.NodeCnt; i++)
               {
                  if (1 == fread(&SysName, sizeof(SysName), 1, pfConfig))
                  {
#ifdef DEBUG
                     printf("INF: Name '%s'\n", SysName.name);
#endif
                  }
                  else
                  {
#ifdef DEBUG
                     printf("ERROR: Read Name (%d)\n", errno);
#endif
                     return CFGFILE_READ;
                  }
               }
               break;
#endif

            case EH_AKAS:
               for (i=0; i < Config.AkaCnt; i++)
               {
                  if (1 == fread(&SysAd, sizeof(SysAd), 1, pfConfig))
                  {
                     memcpy(&Akas[i], &SysAd, sizeof(SysAd));
#ifdef DEBUG
                        printf("INF: Aka #%d: %d:%d/%d.%d@%s\n", i, SysAd.main.zone,
                                                            SysAd.main.net,
                                                            SysAd.main.node,
                                                            SysAd.main.point,
                                                            SysAd.domain);
#endif
                  }
                  else
                  {
#ifdef DEBUG
                     printf("ERROR: Read Aka (%d)\n", errno);
#endif
                     return CFGFILE_READ;
                  }
               }
               break;

            case EH_ORIGINS:
               for (i=0; i < Config.OriginCnt; i++)
               {
                  if (1 == fread(&OneOrigin, sizeof(OriginLines), 1, pfConfig))
                  {
                     memcpy(&Origins[i], &OneOrigin, sizeof(OneOrigin));
#ifdef DEBUG
                        printf("INF: Origin #%d: %s\n", i, OneOrigin.line);
#endif
                  }
                  else
                  {
#ifdef DEBUG
                     printf("ERROR: Read Origin (%d)\n", errno);
#endif
                     return CFGFILE_READ;
                  }
               }
               break;

            default:
#ifdef DEBUG
               printf("INF: Unsupported extension %d\n", Header.type);
#endif
               fseek(pfConfig, Header.offset, SEEK_CUR);
               break;
         }
      }
      else
      {
#ifdef DEBUG
         printf("ERROR: Read error ExtHeader (%d)\n", errno);
#endif
         return CFGFILE_READ;
      }
      offset += Header.offset+sizeof(Header);
   }
   return 0;
}

static int ReadAreas(FILE *pfConfig, PAREALIST pRetList)
{
   int i;
   Area FEArea;
   AREADEFOPT AreaDef;
   SysAddress *pAddr;

   /* Netmail-Area gesondert behandeln */
   AddNMArea(Config.NetMPath, pRetList, Config.sysops[0].name, (PFTNADDRESS) &Akas[0].main);

   /* Extensions ueberspringen */
   fseek(pfConfig, sizeof(Config)+Config.offset, SEEK_SET);

   /* Nodes ueberspringen */
   if (Config.NodeCnt)
      fseek(pfConfig, Config.NodeCnt*Config.NodeRecSize, SEEK_CUR);

   /* Areas einlesen */
   for (i=0; i<Config.AreaCnt; i++)
   {
      if (1==fread(&FEArea, sizeof(FEArea), 1, pfConfig))
      {
#ifdef DEBUG
         printf("INF: Area %s\n", FEArea.name);
         printf("     Type ");
#endif
         switch(FEArea.flags.atype)
         {
            case AREA_ECHOMAIL:
#ifdef DEBUG
               printf("Echo\n");
#endif
               break;

            case AREA_NETMAIL:
#ifdef DEBUG
               printf("Net\n");
#endif
               break;

            case AREA_LOCAL:
#ifdef DEBUG
               printf("Local\n");
#endif
               break;

            case AREA_BADMAILBOARD:
#ifdef DEBUG
               printf("Bad\n");
#endif
               break;

            case AREA_DUPEBOARD:
#ifdef DEBUG
               printf("Dupe\n");
#endif
               break;

            default:
#ifdef DEBUG
               printf("Unknown\n");
#endif
               continue;
         }
#ifdef DEBUG
         printf("     Format ");
#endif
         switch(FEArea.flags.storage)
         {
            case QBBS:
#ifdef DEBUG
               printf("Hudson\n");
#endif
               continue;

            case FIDO:
#ifdef DEBUG
               printf("*.MSG\n");
#endif
               break;

            case SQUISH:
#ifdef DEBUG
               printf("Squish\n");
#endif
               break;

            case JAM:
#ifdef DEBUG
               printf("JAM\n");
#endif
               break;

            case PASSTHRU:
#ifdef DEBUG
               printf("PT\n");
#endif
               continue;

            default:
#ifdef DEBUG
               printf("Unknown\n");
#endif
               continue;
         }
#ifdef DEBUG
         printf("     Aka #%d\n", FEArea.info.aka);
         printf("     Path %s\n", FEArea.path);
         printf("     Desc %s\n", FEArea.desc);
#endif
      }
      else
      {
#ifdef DEBUG
         printf("ERROR: Read error Area (%d)\n", errno);
#endif
         return CFGFILE_READ;
      }
      /* Hier kommen wir nur hin, wenn wir eine gueltige Area gefunden haben */
      memset(&AreaDef, 0, sizeof(AreaDef));
      strncpy(AreaDef.areatag, FEArea.name, LEN_AREATAG);
      if (FEArea.desc[0])
         strncpy(AreaDef.areadesc, FEArea.desc, LEN_AREADESC);
      else
         strncpy(AreaDef.areadesc, FEArea.name, LEN_AREADESC);
      CopyAreaPath(AreaDef.pathfile, FEArea.path);
      strcpy(AreaDef.username, Config.sysops[0].name);
      pAddr = &Akas[FEArea.info.aka];
      NetAddrToString(AreaDef.address, (PFTNADDRESS) &pAddr->main);

      switch(FEArea.flags.storage)
      {
         case FIDO:
            AreaDef.areaformat = AREAFORMAT_FTS;
            break;

         case SQUISH:
            AreaDef.areaformat = AREAFORMAT_SQUISH;
            break;

         case JAM:
            AreaDef.areaformat = AREAFORMAT_JAM;
            break;

         default:
            /* oben schon abgefangen */
            break;
      }

      switch(FEArea.flags.atype)
      {
         case AREA_ECHOMAIL:
            AreaDef.areatype = AREATYPE_ECHO;
            break;

         case AREA_NETMAIL:
         case AREA_DUPEBOARD:
         case AREA_BADMAILBOARD:
            AreaDef.areatype = AREATYPE_NET;
            AreaDef.ulDefAttrib = ATTRIB_PRIVATE;
            break;

         case AREA_LOCAL:
            AreaDef.areatype = AREATYPE_LOCAL;
            break;

         default:
            /* oben schon abgefangen */
            break;
      }
      AreaDef.ulAreaOpt = AREAOPT_FROMCFG;

      /* Area eintragen */
      AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
   }
   return CFGFILE_OK;
}

static int CopyUserData(USERDATAOPT *pUserData, SysAddress *Akas, OriginLines *pOrigins)
{
   int i=0, j=0;

   while (i < MAX_AKAS && j < MAX_ADDRESSES)
   {
      if (Akas[i].main.zone  ||
          Akas[i].main.net   ||
          Akas[i].main.node  ||
          Akas[i].main.point)
      {
         NetAddrToString(pUserData->address[j], (PFTNADDRESS) &Akas[i].main);
         j++;
      }
      i++;
   }
   strncpy(pUserData->defaultorigin, pOrigins[0].line, LEN_ORIGIN);

   return 0;
}

static int CopyOutbounds(OUTBOUND *pOutbounds, SysAddress *Akas)
{
   int i=0, j=0;

   memset(pOutbounds, 0, MAX_ADDRESSES * sizeof(OUTBOUND));

   while (i < MAX_AKAS)
   {
      if (Akas[i].main.zone ||
          Akas[i].main.net  ||
          Akas[i].main.node ||
          Akas[i].main.point)
      {
         if ( j == 0) /* Erste AKA */
         {
            pOutbounds[j].zonenum=Akas[i].main.zone;
            CopyAreaPath(pOutbounds[j].outbound, Config.OutBound);
            j++;
         }
         else
            if (Akas[i].domain[0])
            {
               int x=0;

               /* evtl. vorhandenen Outbound suchen */
               while (x < MAX_ADDRESSES && pOutbounds[x].zonenum != Akas[i].main.zone)
                  x++;

               if (x >= MAX_ADDRESSES) /* nicht gef. -> neu */
               {
                  char *pchLast;

                  pOutbounds[j].zonenum=Akas[i].main.zone;

                  CopyAreaPath(pOutbounds[j].outbound, Config.OutBound);

                  pchLast = strrchr(pOutbounds[j].outbound, '\\');
                  if (pchLast)
                  {
                     /* altes Verz. abschneiden, Domain anhaengen */
                     pchLast++;
                     *pchLast=0;
                     strcat(pOutbounds[j].outbound, Akas[i].domain);
                  }
                  else
                     strcpy(pOutbounds[j].outbound, Akas[i].domain);

                  j++;
               }
            }
      }
      i++;
   }
   return 0;
}
/*-------------------------------- Modulende --------------------------------*/

