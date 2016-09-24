/*---------------------------------------------------------------------------+
 | Titel: CNRUTIL.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 22.02.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Utility-Funktionen fuer FleetStreet, speziell f. Container            |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif

#define ALLOC_BLOCK_SIZE  100

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: CollectRecordPointers                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert ein Array von PRECORDCORE, gemaess einem PRECORDCORE*/
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pppDest: Zeiger auf den Array-Pointer                          */
/*            pPopupRecord: Zeiger auf den Record, fr den ein Popup         */
/*                          geoeffnet werden soll.                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Anzahl der Records im Array                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Wenn pPopupRecord selektiert ist, werden alle selektierten     */
/*            Records zurueckgeliefert, sonst nur der pPopupRecord           */
/*---------------------------------------------------------------------------*/

ULONG CollectRecordPointers(HWND hwndCnr, PRECORDCORE **pppDest, PRECORDCORE pPopupRecord)
{
   ULONG ulCount=0;
   ULONG ulAlloc=0;
   PRECORDCORE pTemp=NULL;
   PRECORDCORE *pReturn=NULL;
   PRECORDCORE *pSave=NULL;

   if (pPopupRecord->flRecordAttr & CRA_SELECTED)
   {
      /* alle selektierten */
      while (pTemp = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pTemp?pTemp:MPFROMLONG(CMA_FIRST),
                                MPFROMLONG(CRA_SELECTED)))
      {
         if (ulCount+1 > ulAlloc)
         {
            /* neu allokieren */
            pSave = pReturn;
            pReturn = malloc((ulAlloc+ALLOC_BLOCK_SIZE)*sizeof(PRECORDCORE));
            if (ulAlloc > 0)
            {
               /* alte kopieren */
               memcpy(pReturn, pSave, ulAlloc * sizeof(PRECORDCORE));
               free(pSave);
            }
            ulAlloc+=ALLOC_BLOCK_SIZE;
         }
         pReturn[ulCount] = pTemp;
         ulCount++;
      }
      *pppDest = pReturn;
   }
   else
   {
      /* nur ein selektierter */
      ulCount=1;
      *pppDest = malloc(sizeof(RECORDCORE));
      **pppDest = pPopupRecord;
   }

   return ulCount;
}

void ApplySourceEmphasis(HWND hwndCnr, PRECORDCORE pPopupRecord)
{
   PRECORDCORE pTemp=NULL;

   if (pPopupRecord->flRecordAttr & CRA_SELECTED)
   {
      while (pTemp = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                pTemp?pTemp:MPFROMLONG(CMA_FIRST),
                                MPFROMLONG(CRA_SELECTED)))
      {
         WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pTemp, MPFROM2SHORT(TRUE, CRA_SOURCE));
      }
   }
   else
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pPopupRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

   return;
}

void RemoveSourceEmphasis(HWND hwndCnr)
{
   PRECORDCORE pRecord=NULL;

   while (pRecord = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                               pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                               MPFROMLONG(CRA_SOURCE)))
   {
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
   }

   return;
}

void SelectAllRecords(HWND hwndCnr)
{
   PRECORDCORE pRecord=NULL;

   WinEnableWindowUpdate(hwndCnr, FALSE);

   while (pRecord = WinSendMsg(hwndCnr, CM_QUERYRECORD,
                               pRecord,
                               MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SELECTED));
   }

   WinEnableWindowUpdate(hwndCnr, TRUE);

   return;
}

void DeselectAllRecords(HWND hwndCnr)
{
   PRECORDCORE pRecord=NULL;

   WinEnableWindowUpdate(hwndCnr, FALSE);

   while (pRecord = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                               pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                               MPFROMLONG(CRA_SELECTED)))
   {
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SELECTED));
   }


   WinEnableWindowUpdate(hwndCnr, TRUE);

   return;
}
/*-------------------------------- Modulende --------------------------------*/
