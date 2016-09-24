/*---------------------------------------------------------------------------+
 | Titel: IM_CFG.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 25.01.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Einlesen der IMAIL-Cfg-Files                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

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
#include "common.h"
#include "revision4.h"
#include "revision5.h"
#if 0
#include "revision6.h"
#endif
#include "im_cfg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME   "IMail"
#define FORMAT_ID     2UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int QueryRevision(char *pchFileName);

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
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

int ReadIMailCfg(char *pchFileName, USERDATAOPT *pUserdata,
                 OUTBOUND *pOutbounds, PAREALIST pRetList,
                 PDRIVEREMAP driveremap, ULONG ulOptions)
{
   int rc;

   driveremap = driveremap;

   switch(rc = QueryRevision(pchFileName))
   {
      case -4: /* Rev 4*/
         return ReadRev4(pchFileName, pUserdata, pOutbounds, pRetList, ulOptions);

      case -5: /* Rev 5*/
         return ReadRev5(pchFileName, pUserdata, pOutbounds, pRetList, ulOptions);

#if 0  /* IMail 1.85 structs are not public */
      case -6: /* Rev 6*/
         return ReadRev6(pchFileName, pUserdata, pOutbounds, pRetList, ulOptions);
#endif

      default:
         return rc;
   }
}


static int QueryRevision(char *pchFileName)
{
   FILE *pfCF=NULL;
   IMAIL_VER im_ver;

   memset(&im_ver, 0, sizeof(im_ver));

   if (pfCF = fopen(pchFileName, "rb"))
   {
      if (fread(&im_ver, sizeof(im_ver), 1, pfCF) >= 1)
      {
         fclose(pfCF);

         if (im_ver.struct_maj == STRUCT_MAJ_REV4 &&
             im_ver.struct_min == STRUCT_MIN_REV4)
            return -4;

         if (im_ver.struct_maj == STRUCT_MAJ_REV5)
            return -5;

         if (im_ver.struct_maj == STRUCT_MAJ_REV6)
            return -6;

         return CFGFILE_VERSION; /* Versionsfehler */
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

