/*---------------------------------------------------------------------------+
 | Titel: CFGFILE_INTERFACE.C                                                |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 12.01.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Dynamisches CFG-Interface                                             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "structs.h"

#include "areaman\areaman.h"
#include "cfgfile_interface.h"

/*--------------------------------- Defines ---------------------------------*/

#define CFGDLL_MASK    "FLTCF_??.DLL"

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*-----------------------------------------------------------------------------
 | Funktionsname: CFG_ReadFormatList
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Erstellt eine Liste der CFG-DLLs mit den unterst》zten
 |               Formaten
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: Zeiger auf Anfang der Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

PCFGDLL CFG_ReadFormatList(void)
{
   PCFGDLL pRetList=NULL, pRetListLast=NULL;
   LOADEDCFGDLL LoadedDLL;
   FILEFINDBUF3 FindBuf;
   HDIR FindHandle = HDIR_CREATE;
   ULONG ulNumFiles=1;
   APIRET rc;
   char pchCurrentDir[LEN_PATHNAME+1];
   char pchDLLName[LEN_PATHNAME+1];
   ULONG ulLen = sizeof(pchCurrentDir);
   ULONG ulDisk, ulMapping;

   DosQueryCurrentDisk(&ulDisk, &ulMapping);
   DosQueryCurrentDir(0, pchCurrentDir, &ulLen);

   rc = DosFindFirst(CFGDLL_MASK, &FindHandle, FILE_ARCHIVED, &FindBuf, sizeof(FindBuf),
                     &ulNumFiles, FIL_STANDARD);

   while (!rc)
   {
      char *pchTemp;

      /* gefundene DLL pruefen */
      sprintf(pchDLLName, "%c:\\%s\\%s", '@'+ulDisk, pchCurrentDir, FindBuf.achName);

      /* DLL laden */
      if (!CFG_LoadDLL(pchDLLName, &LoadedDLL))
      {
         /* korrekte DLL, Daten holen */
         if (pRetList)
         {
            pRetListLast->next = calloc(1, sizeof(CFGDLL));
            pRetListLast = pRetListLast->next;
         }
         else
            pRetList = pRetListLast = calloc(1, sizeof(CFGDLL));

         strcpy(pRetListLast->pchDLLName, pchDLLName);
         pRetListLast->ulFormatID = LoadedDLL.QueryFormatID();
         pchTemp = LoadedDLL.QueryFormatName();
         if (pchTemp)
            strcpy(pRetListLast->pchFormatName, pchTemp);
         else
            _itoa(pRetListLast->ulFormatID, pRetListLast->pchFormatName, 10);

         /* DLL wieder freigeben */
         DosFreeModule(LoadedDLL.hmodCfgDLL);
      }

      /* naechste DLL suchen */
      ulNumFiles = 1;
      rc = DosFindNext(FindHandle, &FindBuf, sizeof(FindBuf), &ulNumFiles);
   }

   if (FindHandle != HDIR_CREATE)
      DosFindClose(FindHandle);

   return pRetList;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CFG_FindFormat
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Sucht ein Format in der Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pList: Anfang der Liste
 |            ulFormatID: Gesuchtes Format (CFGTYPE_ANY = beliebig)
 |            pPrev: vorheriges gefundenes (NULL = vom Anfang)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: NULL   nicht gefunden/Ende der Liste
 |                sonst  Pointer auf Element
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

PCFGDLL CFG_FindFormat(PCFGDLL pList, ULONG ulFormatID, PCFGDLL pPrev)
{
   if (!pPrev)
      pPrev = pList;       /* vom Anfang */
   else
      pPrev = pPrev->next; /* vom naechsten Element */

   while (pPrev &&
          (ulFormatID != CFGTYPE_ANY) &&
          (pPrev->ulFormatID !=ulFormatID))
      pPrev = pPrev->next;

   return pPrev;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CFG_LoadDLL
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Laedt eine CFG-DLL
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchDLLName: Name der DLL
 |            pLoadedCfgDLL: Lade-Block
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: LOADCFGDLL_OK          kein Fehler
 |                LOADCFGDLL_CANTLOAD    DLL nicht ladbar
 |                LOADCFGDLL_FUNCMISSING Funktion fehlt
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

int CFG_LoadDLL(PCHAR pchDLLName, PLOADEDCFGDLL pLoadedCfgDLL)
{
   char fail[50]="";
   APIRET rc=0;

   if (!(rc = DosLoadModule(fail, sizeof(fail), pchDLLName, &pLoadedCfgDLL->hmodCfgDLL)))
   {
      if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYVER, NULL,
                                (PFN*)&pLoadedCfgDLL->QueryVer)))
      {
         if ((rc=pLoadedCfgDLL->QueryVer()) == CURRENT_CFGVER)
         {
            if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYID, NULL,
                                  (PFN*)&pLoadedCfgDLL->QueryFormatID)))
               if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYNAME, NULL,
                                     (PFN*)&pLoadedCfgDLL->QueryFormatName)))
                  if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_READCFG, NULL,
                                        (PFN*)&pLoadedCfgDLL->ReadCfgFile)))
                     return LOADCFGDLL_OK;
         }
         else
         {
            DosFreeModule(pLoadedCfgDLL->hmodCfgDLL);
            return LOADCFGDLL_VERSION;
         }
      }

      /* Fehler */
      DosFreeModule(pLoadedCfgDLL->hmodCfgDLL);
      return LOADCFGDLL_FUNCMISSING;
   }
   else
   {
      return LOADCFGDLL_CANTLOAD;
   }
}

/*-------------------------------- Modulende --------------------------------*/

