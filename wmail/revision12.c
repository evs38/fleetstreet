/*---------------------------------------------------------------------------+
 | Titel: REVISION12.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 11.10.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Config von WMail   (Revision 12)                                       |
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
#include "pas2c.h"
#include "wmstruct.h"

/*--------------------------------- Defines ---------------------------------*/

/*#define DEBUG*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static struct SetRec Setup;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int CopyUserData(USERDATAOPT *pUserdata, struct SetRec *pconfig);
static int CopyOutbounds(OUTBOUND *pOutbounds, struct SetRec *pconfig);
static int ReadAreas(char *pchFileName, PAREALIST pRetList, struct SetRec *pconfig);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadRev12                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst WMail-Cfg der Revision 12                            */
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

int ReadRev12(char *pchFileName, USERDATAOPT *pUserdata,
              OUTBOUND *pOutbounds, PAREALIST pRetList,
              ULONG ulOptions)
{
   int rc;
   FILE *pfConfig;

   memset(&Setup, 0, sizeof(Setup));

   if (pfConfig = fopen(pchFileName, "rb"))
   {
      if (fread(&Setup, sizeof(Setup), 1, pfConfig)==1)
      {
         fclose(pfConfig);

         if (ulOptions & READCFG_USERDATA)
            CopyUserData(pUserdata, &Setup);

         if (ulOptions & READCFG_OUTBOUNDS)
            CopyOutbounds(pOutbounds, &Setup);

         if ((ulOptions & READCFG_AREAS) && (rc = ReadAreas(pchFileName, pRetList, &Setup)))
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

static int CopyUserData(USERDATAOPT *pUserdata, struct SetRec *pconfig)
{
   int i=0, j=0;

   while (i < 11 && j < MAX_ADDRESSES)
   {
      if (pconfig->Aka[i].Zone  ||
          pconfig->Aka[i].Net   ||
          pconfig->Aka[i].Node  ||
          pconfig->Aka[i].Point)
      {
         NetAddrToString(pUserdata->address[j], (PFTNADDRESS) &pconfig->Aka[i]);
         j++;
      }
      i++;
   }

   Pas2C(pUserdata->username[0], pconfig->Sysop, LEN_USERNAME+1);
   Pas2C(pUserdata->defaultorigin, pconfig->Origin[0], LEN_ORIGIN+1);

   return 0;
}

static int CopyOutbounds(OUTBOUND *pOutbounds, struct SetRec *pconfig)
{
   int i=0, j=0;

   memset(pOutbounds, 0, MAX_ADDRESSES * sizeof(OUTBOUND));

   while (i < 11)
   {
      if (pconfig->Aka[i].Zone ||
          pconfig->Aka[i].Net  ||
          pconfig->Aka[i].Node ||
          pconfig->Aka[i].Point)
      {
         if ( j == 0) /* Erste AKA */
         {
            pOutbounds[j].zonenum=pconfig->Aka[i].Zone;
            Pas2C(pOutbounds[0].outbound, pconfig->OutDir, LEN_PATHNAME+1);
            RemoveBackslash(pOutbounds[0].outbound);
            j++;
         }
         else
            if (pconfig->Aka[i].Domain[0])
            {
               int x=0;

               /* evtl. vorhandenen Outbound suchen */
               while (x < MAX_ADDRESSES && pOutbounds[x].zonenum != pconfig->Aka[i].Zone)
                  x++;

               if (x >= MAX_ADDRESSES) /* nicht gef. -> neu */
               {
                  char *pchLast;

                  pOutbounds[j].zonenum=pconfig->Aka[i].Zone;

                  Pas2C(pOutbounds[j].outbound, pconfig->OutDir, LEN_PATHNAME+1);

                  pchLast = strrchr(pOutbounds[j].outbound, '\\');
                  if (pchLast)
                  {
                     /* altes Verz. abschneiden, Domain anhaengen */
                     pchLast++;
                     *pchLast=0;
                     Pas2C(pchLast, pconfig->Aka[i].Domain, 30);
                  }
                  else
                     Pas2C(pOutbounds[j].outbound, pconfig->Aka[i].Domain, LEN_PATHNAME+1);

                  j++;
               }
            }
      }
      i++;
   }

   return 0;
}

static int ReadAreas(char *pchFileName, PAREALIST pRetList, struct SetRec *pconfig)
{
   FILE *pfAreas;
   char *pchAreaFileName;
   char *pchTemp;
   int rc=CFGFILE_OK;
   AREADEFOPT AreaDef;
   struct AreasRecord Area;
   unsigned short usRecordSize=0; /* gelesene groesse */
   int iLocalCount=0, iNetCount=0, iPersCount=0;

   /* AREAS.PRM anhaengen */
   pchAreaFileName = malloc(strlen(pchFileName)+20);
   strcpy(pchAreaFileName, pchFileName);
   pchTemp = strrchr(pchAreaFileName, '\\');
   if (!pchTemp)
      pchTemp = pchAreaFileName;
   if (*pchTemp == '\\')
      pchTemp++;
   strcpy(pchTemp, "AREAS.PRM");

   /* File oeffnen */
   if (pfAreas = fopen(pchAreaFileName, "rb"))
   {
      while (fread(&Area, sizeof(Area), 1, pfAreas) == 1)
      {
         if (!usRecordSize)
            if (Area.IsExtRec == 0xf00f)
               usRecordSize = Area.RecSize;
            else
               usRecordSize = sizeof(Area);

#ifdef DEBUG
         printf("Format: %d\n", Area.Format);
#endif

         /* Area konvertieren */
         if (!Area.Inactive)
            if (Area.Format == FORMAT_FIDO ||
                Area.Format == FORMAT_SQUISH ||
                Area.Format == FORMAT_JAM)
            {
               memset(&AreaDef, 0, sizeof(AreaDef));

               Pas2C(AreaDef.areadesc, Area.Title, LEN_AREADESC+1);
               Pas2C(AreaDef.pathfile, Area.Path, LEN_PATHNAME+1);
               RemoveBackslash(AreaDef.pathfile);
               if (Area.AttribPvt)
                  AreaDef.ulDefAttrib |= ATTRIB_PRIVATE;
               if (Area.AttribCrash)
                  AreaDef.ulDefAttrib |= ATTRIB_CRASH;
               if (Area.AttribFA)
                  AreaDef.ulDefAttrib |= ATTRIB_FILEATTACHED;
               if (Area.AttribKS)
                  AreaDef.ulDefAttrib |= ATTRIB_KILLSENT;

               switch(Area.Kind)
               {
                  case AREAKIND_LOCAL:
                     AreaDef.areatype = AREATYPE_LOCAL;
                     sprintf(AreaDef.areatag, "LOCAL.%03d", iLocalCount);
                     iLocalCount++;
                     break;

                  case AREAKIND_ECHO:
                     AreaDef.areatype = AREATYPE_ECHO;
                     break;

                  case AREAKIND_NET:
                  case AREAKIND_BAD:
                  case AREAKIND_DUPES:
                  case AREAKIND_VIRTUAL_NET:
                     AreaDef.areatype = AREATYPE_NET;
                     if (iNetCount)
                        sprintf(AreaDef.areatag, "NETMAIL.%03d", iNetCount);
                     else
                        strcpy(AreaDef.areatag, "NETMAIL");
                     iNetCount++;
                     break;

                  case AREAKIND_PERSONAL:
                     AreaDef.areatype = AREATYPE_ECHO;
                     sprintf(AreaDef.areatag, "PERSMAIL.%03d", iPersCount);
                     iPersCount++;
                     break;

                  default:
                     AreaDef.areatype = AREATYPE_ECHO;
                     break;
               }
               switch(Area.Format)
               {
                  case FORMAT_FIDO:
                     AreaDef.areaformat = AREAFORMAT_FTS;
                     break;

                  case FORMAT_SQUISH:
                     AreaDef.areaformat = AREAFORMAT_SQUISH;
                     break;

                  case FORMAT_JAM:
                     AreaDef.areaformat = AREAFORMAT_JAM;
                     break;
               }
               NetAddrToString(AreaDef.address, (PFTNADDRESS) &pconfig->Aka[Area.Aka]);
               Pas2C(AreaDef.username, pconfig->Sysop, LEN_USERNAME+1);

               if (!AreaDef.areatag[0])
                  Pas2C(AreaDef.areatag, Area.Tag, LEN_AREATAG+1);

               if (!AreaDef.areadesc[0])
                  strcpy(AreaDef.areadesc, AreaDef.areatag);

               AreaDef.ulAreaOpt = AREAOPT_FROMCFG;

               AM_AddArea(pRetList, &AreaDef, ADDAREA_TAIL | ADDAREA_UNIQUE);
            }


         /* Rest des Records ueberlesen */
         fseek(pfAreas, usRecordSize - sizeof(Area), SEEK_CUR);
      }

      if (ferror(pfAreas))
         rc = CFGFILE_READ;

      fclose(pfAreas);
   }
   else
      rc=CFGFILE_OPEN;

   free(pchAreaFileName);

   return rc;
}

/*-------------------------------- Modulende --------------------------------*/

