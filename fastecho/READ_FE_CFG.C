/*---------------------------------------------------------------------------+
 | Titel: READ_FE_CFG.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.01.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Gemeinsames Interface, Einlesen der Fastecho.Cfg                       |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)
/*#define DEBUG*/

#define OS_2
#include <os2.h>
#include <string.h>
#include "../main.h"
#include "../structs.h"
#include <stdio.h>
#ifdef DEBUG
  #include <errno.h>
#endif
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"
#include "read_fe_cfg.h"
#include "revision4.h"
#include "revision5.h"
#include "revision6.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_NAME   "Fastecho"
#define FORMAT_ID     1UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadFastechoCfg                                            */
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

int ReadFastechoCfg(char *pchFileName, USERDATAOPT *pUserdata,
                    OUTBOUND *pOutbounds, PAREALIST pRetList,
                    PDRIVEREMAP driveremap, ULONG ulOptions)
{
   FILE *pfConfig=NULL;
   unsigned short usTemp=0;

   memset(pRetList, 0, sizeof(*pRetList));

   driveremap = driveremap;

   pfConfig=fopen(pchFileName, "rb");
   if (pfConfig)
   {
      /* Revision lesen */
      if (1==fread(&usTemp, sizeof(usTemp), 1, pfConfig))
      {
         int ret;

         switch(usTemp)
         {
            case 4:
               ret=ReadRevision4(pfConfig, pUserdata, pOutbounds, pRetList,
                                 ulOptions );
               break;

            case 5:
               ret=ReadRevision5(pfConfig, pUserdata, pOutbounds, pRetList,
                                 ulOptions );
               break;

            case 6:
               ret=ReadRevision6(pfConfig, pUserdata, pOutbounds, pRetList,
                                 ulOptions );
               break;

            default:
               ret=CFGFILE_VERSION;
#ifdef DEBUG
               printf("ERROR: unknown revision %d\n", usTemp);
#endif
               break;
         }
         fclose(pfConfig);
         return ret;
      }
      else
      {
#ifdef DEBUG
         printf("ERROR: %s unreadable\n", pchFileName);
#endif
         fclose(pfConfig);
         return CFGFILE_READ;
      }
   }
   else
   {
#ifdef DEBUG
      printf("ERROR: can't open %s (%d)\n", pchFileName, errno);
#endif
      return CFGFILE_OPEN;
   }
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