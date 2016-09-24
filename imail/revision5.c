/*---------------------------------------------------------------------------+
 | Titel: REVISION5.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.08.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Revision 5 der IMail-CFGs                                               |
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
#include "../main.h"
#include "../structs.h"
#include "../msgheader.h"
#include <stdio.h>
#ifdef DEBUG
  #include <errno.h>
#endif
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "../util/fltutil.h"
#include "../util/addrcnv.h"
#include "imail170.h"
#include "revision5.h"
#include "common.h"


/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static struct im_config_type im_cf;
static struct sysop_names add_sysops;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int ReadAdditionalNames(char *pchFileName, struct sysop_names *padd_sysops);
static int ReadAreas(FILE *pfAR, PAREALIST pRetList, struct im_config_type *im_cf);
static int CopyUserData(USERDATAOPT *pUserdata, struct im_config_type *im_cf);
static int CopyOutbounds(OUTBOUND *pOutbounds, struct im_config_type *im_cf);

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

int ReadRev5(char *pchFileName, USERDATAOPT *pUserdata,
                 OUTBOUND *pOutbounds, PAREALIST pRetList,
                 ULONG ulOptions)
{
   FILE *pfCF=NULL;
   FILE *pfAR=NULL;
   int rc;

   memset(&im_cf, 0, sizeof(im_cf));
   memset(&add_sysops, 0, sizeof(add_sysops));

   if (pfCF = fopen(pchFileName, "rb"))
   {
      if (fread(&im_cf, sizeof(im_cf), 1, pfCF) >= 1)
      {
         fclose(pfCF);

         if (ulOptions & READCFG_USERDATA)
         {
            ReadAdditionalNames(pchFileName, &add_sysops);
            CopyUserData(pUserdata, &im_cf);
         }
         if (ulOptions & READCFG_OUTBOUNDS)
            CopyOutbounds(pOutbounds, &im_cf);

         if (ulOptions & READCFG_AREAS)
         {
            char *pchAreas=strdup(pchFileName);
            ULONG ulLen = strlen(pchAreas);
            if (ulLen >= 2)
            {
               pchAreas[ulLen-2]='A';
               pchAreas[ulLen-1]='R';

               pfAR = fopen(pchAreas, "rb");
               free(pchAreas);
               if (pfAR)
               {
                  rc = ReadAreas(pfAR, pRetList, &im_cf);

                  fclose(pfAR);
                  return rc;
               }
               else
                  return CFGFILE_OPEN; /* IMAIL.AR nicht lesbar */
            }
            else
            {
               free(pchAreas);
               return CFGFILE_OPEN; /* fehlerhafter Filename */
            }
         }
         return CFGFILE_OK;
      }
      else
      {
         fclose(pfCF);
         return CFGFILE_READ;  /* zu kurz */
      }
   }
   else
      return CFGFILE_OPEN; /* Kann CF-File nicht oeffnen */
}

static int ReadAdditionalNames(char *pchFileName, struct sysop_names *padd_sysops)
{
   FILE *pfSN=NULL;
   char *pchSysNames=strdup(pchFileName);
   ULONG ulLen = strlen(pchSysNames);

   if (ulLen >= 2)
   {
      pchSysNames[ulLen-2]='S';
      pchSysNames[ulLen-1]='N';

      pfSN = fopen(pchSysNames, "rb");

      if (pfSN)
      {
         if (fread(padd_sysops, sizeof(struct sysop_names), 1, pfSN) != 1)
            memset(padd_sysops, 0, sizeof(struct sysop_names));
         fclose(pfSN);
      }
   }
   free(pchSysNames);

   return 0;
}

static int ReadAreas(FILE *pfAR, PAREALIST pRetList, struct im_config_type *im_cf)
{
   struct areas_record_type IArea;
   AREADEFOPT NewArea;
   int i;

   memset(pRetList, 0, sizeof(*pRetList));

   if (im_cf->netmail[0])
      AddNetmailArea(pRetList, im_cf->netmail, (PFTNADDRESS) &im_cf->aka[0], im_cf->sysop);

   while(fread(&IArea, sizeof(IArea), 1, pfAR) == 1)
   {
      /* erfolgreich gelesen */
      if (!IArea.active || IArea.deleted)
         continue; /* nicht aktive Area */

      if (IsSdm(IArea.msg_base_type) ||
          IsJam(IArea.msg_base_type) ||
          IsSquish(IArea.msg_base_type))
      {
         /* Gltiger Typ */
         memset(&NewArea, 0, sizeof(NewArea));

         if (IsSdm(IArea.msg_base_type))
            NewArea.areaformat = AREAFORMAT_FTS;
         else
            if (IsJam(IArea.msg_base_type))
               NewArea.areaformat = AREAFORMAT_JAM;
            else
               NewArea.areaformat = AREAFORMAT_SQUISH;

         if (IsEcho(IArea.msg_base_type))
            NewArea.areatype = AREATYPE_ECHO;
         else
            if (IsLocal(IArea.msg_base_type))
               NewArea.areatype = AREATYPE_LOCAL;
            else
            {
               NewArea.areatype = AREATYPE_NET;
               NewArea.ulDefAttrib = ATTRIB_PRIVATE;
            }

         NewArea.ulAreaOpt = AREAOPT_FROMCFG;

         strncpy(NewArea.areatag, IArea.aname, LEN_AREATAG);
         if (IArea.comment[0])
            strncpy(NewArea.areadesc, IArea.comment, LEN_AREADESC);
         else
            strncpy(NewArea.areadesc, IArea.aname, LEN_AREADESC);
         strncpy(NewArea.pathfile, IArea.msg_path, LEN_PATHNAME);
         strncpy(NewArea.username, im_cf->sysop, LEN_USERNAME);
         if (IArea.o_addr == 0)
            i=0;
         else
            i = IArea.o_addr-1;
         if (im_cf->aka[i].point)
            sprintf(NewArea.address, "%d:%d/%d.%d", im_cf->aka[i].zone,
                                                    im_cf->aka[i].net,
                                                    im_cf->aka[i].node,
                                                    im_cf->aka[i].point);
         else
            sprintf(NewArea.address, "%d:%d/%d", im_cf->aka[i].zone,
                                                 im_cf->aka[i].net,
                                                 im_cf->aka[i].node);
         AM_AddArea(pRetList, &NewArea, ADDAREA_TAIL | ADDAREA_UNIQUE);
      }
   }
   if (ferror(pfAR))
      return CFGFILE_READ;

   return CFGFILE_OK;
}

static int CopyUserData(USERDATAOPT *pUserdata, struct im_config_type *im_cf)
{
   int i, j=0;

   for (i=0; i<MAXAKAS && im_cf->aka[i].zone; i++) /* alle eingetragenen AKAs */
   {
      if (j < MAX_ADDRESSES)
      {
         /* AKA erzeugen */
         NetAddrToString(pUserdata->address[j], (FTNADDRESS *) &im_cf->aka[i]);
         j++;
      }
   }

   strncpy(pUserdata->username[0], im_cf->sysop, LEN_USERNAME);

   for (i=0, j=1; i<MAXSYSNAME && j< MAX_USERNAMES; i++)
      if (add_sysops.names[i][0])
         strncpy(pUserdata->username[j++], add_sysops.names[i], LEN_USERNAME);

   strncpy(pUserdata->defaultorigin, im_cf->dflt_origin, LEN_ORIGIN);

   return 0;
}

static int CopyOutbounds(OUTBOUND *pOutbounds, struct im_config_type *im_cf)
{
   int i=0, j, k;

   while (i < MAXAKAS && im_cf->domains[i].akas[0])
   {
      j=0;
      while (j < MAXAKAS && im_cf->domains[i].akas[j])
      {
         /* pruefen, ob Outbound nicht schon da */
         k=0;
         while (k < MAX_ADDRESSES && pOutbounds[k].zonenum)
         {
            if (pOutbounds[k].zonenum == im_cf->aka[im_cf->domains[i].akas[j]-1].zone)
            {
               /* haben wir schon, Ende */
               break;
            }
            k++;
         }
         if (k < MAX_ADDRESSES && !pOutbounds[k].zonenum)
         {
            /* Outbound bernehmen */
            pOutbounds[k].zonenum = im_cf->aka[im_cf->domains[i].akas[j]-1].zone;
            strncpy(pOutbounds[k].outbound, im_cf->domains[i].outbound, LEN_PATHNAME);
            pOutbounds[k].outbound[LEN_PATHNAME]=0;
            RemoveBackslash(pOutbounds[k].outbound);
         }
         j++;
      }
      i++;
   }
   return 0;
}

/*-------------------------------- Modulende --------------------------------*/

