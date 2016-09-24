/*---------------------------------------------------------------------------+
 | Titel: MARKMANAGE.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 27.08.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Verwaltung der Markierungen in Ketten und Buckets                      |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_PM
#define INCL_DOSSEMAPHORES
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "structs.h"
#include "msgheader.h"
#include "markmanage.h"
#include "areaman\areaman.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static PMARKERAREA FindMarkerArea(PMARKERLIST pList, PCHAR pchAreaTag);
static PMARKERBUCKET FindMarkerBucket(PMARKERAREA pArea, ULONG ulMsgID, PLONG plFoundIndex);
static int SetAreaToFront(PMARKERLIST pList, PMARKERAREA pArea);
static ULONG CalcAreaSize(PMARKERAREA pArea);
static int CopyMarkerItems(PMARKERAREA pArea, PMARKERITEM pDest);
static int LoadMarkerItems(PMARKERAREA pArea, PMARKERITEM pSrc, ULONG ulSizeBuffer);

/*---------------------------------------------------------------------------*/
/* Funktionsname: MarkMessage                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Nimmt eine Message in die Markierungs-Liste auf             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pchAreaTag: Area-Tag                                           */
/*            ulMsgID:    Message-ID                                         */
/*            ulMsgNr:    Message-Nummer                                     */
/*            pHeader:    Message-Header                                     */
/*            pchFindText: Suchtext (im Body)                                */
/*            ulFlags:    Markierungs-Flags (MARKFLAG_*)                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  war schon markiert                                      */
/*                2  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int MarkMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulMsgNr,
                MSGHEADER *pHeader, PCHAR pchFindText, ULONG ulFlags, ULONG ulHow, ULONG ulWhere)
{
   PMARKERAREA pArea=NULL;
   PMARKERBUCKET pBucket=NULL;
   LONG lFoundIndex = -1;
   int i=0;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   /* Area suchen */
   pArea = FindMarkerArea(pList, pchAreaTag);

   if (pArea)
   {
      /* gefunden, an den Anfang setzen */
      SetAreaToFront(pList, pArea);
   }
   else
   {
      /* nicht gefunden, neue Area anlegen */
      pArea = calloc(1, sizeof(MARKERAREA));

      /* vorne einhaengen */
      if (pList->pAreas)
      {
         pArea->next = pList->pAreas;
         pList->pAreas->prev = pArea;
         pList->pAreas = pArea;
      }
      else
         pList->pAreas = pArea;

      /* Daten setzen */
      memcpy(pArea->pchAreaTag, pchAreaTag, LEN_AREATAG);
   }

   /* pArea zeigt nun auf eine existierende oder neue Area */

   /* Message suchen */
   pBucket = FindMarkerBucket(pArea, ulMsgID, &lFoundIndex);

   if (pBucket)
   {
      /* Bucket gefunden, Index pruefen */
      if (lFoundIndex >= 0)
      {
         int ret;

         /* Message war schon markiert, nur Flags updaten */
         if (pBucket->aItems[lFoundIndex].ulFlags & ulFlags)
            ret = 1;
         else
            ret = 0;
         pBucket->aItems[lFoundIndex].ulFlags |= ulFlags;
         pArea->bDirty = TRUE;
         pList->bDirty = TRUE;

         DosReleaseMutexSem(pList->hmtxAccess);

         return ret;
      }
   }
   else
   {
      /* nicht gefunden, kein Bucket, neuen Bucket */
      pBucket = calloc(1, sizeof(MARKERBUCKET));
      pArea->pBuckets = pBucket;
   }

   /* pBucket zeigt auf den Ziel-Bucket, evtl. ein neuer */
   if (pBucket->ulCountItems == SIZE_BUCKET)
   {
      /* Bucket ist voll, aufteilen */
      PMARKERBUCKET pBucket2;

      pBucket2 = calloc(1, sizeof(MARKERBUCKET));
      pBucket2->next = pBucket->next;
      pBucket2->prev = pBucket;
      if (pBucket->next)
         pBucket->next->prev = pBucket2;
      pBucket->next = pBucket2;

      pBucket2->ulCountItems= SIZE_BUCKET/2;
      pBucket->ulCountItems= SIZE_BUCKET/2;
      memcpy(pBucket2->aItems, &(pBucket->aItems[SIZE_BUCKET/2]), (SIZE_BUCKET/2)*sizeof(MARKERITEM));

      /* pruefen, in welchen der zwei Buckets die Message soll */
      if (pBucket2->aItems[0].ulMsgID <= ulMsgID)
         pBucket = pBucket2;
   }

   /* pBucket zeigt auf ein Bucket mit einem freien Platz */

   /* Einfuegestelle suchen */
   i=0;
   while(i < pBucket->ulCountItems && pBucket->aItems[i].ulMsgID <= ulMsgID)
      i++;

   if (i < pBucket->ulCountItems)
   {
      /* mittendrin, nach hinten rutschen */
      memmove(&pBucket->aItems[i+1], &pBucket->aItems[i], (pBucket->ulCountItems-i)*sizeof(MARKERITEM));
   }

   pBucket->ulCountItems++;

   /* Daten kopieren */
   pBucket->aItems[i].ulMsgID = ulMsgID;
   pBucket->aItems[i].ulMsgNr = ulMsgNr;
   pBucket->aItems[i].ulFlags = ulFlags;
   pBucket->aItems[i].ulHow   = ulHow;
   pBucket->aItems[i].ulWhere = ulWhere;
   memcpy(pBucket->aItems[i].pchFrom, pHeader->pchFromName, LEN_USERNAME);
   memcpy(pBucket->aItems[i].pchSubj, pHeader->pchSubject, LEN_SUBJECT);
   if (pchFindText)
      strncpy(pBucket->aItems[i].pchFindText, pchFindText, LEN_FINDTEXT);
   else
      pBucket->aItems[i].pchFindText[0]=0;

   /* Dirty-Flags setzen */
   pArea->bDirty = TRUE;
   pList->bDirty = TRUE;

   DosReleaseMutexSem(pList->hmtxAccess);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UnmarkMessage                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt eine Message aus der Markierungsliste              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pchAreaTag: Area-Tag                                           */
/*            ulMsgID:    Message-ID                                         */
/*            ulFlags:    Markierungs-Flags (MARKFLAG_*)                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  nicht gefunden                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int UnmarkMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulFlags)
{
   PMARKERAREA pArea=NULL;
   PMARKERBUCKET pBucket=NULL;
   LONG lFoundIndex=-1;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   /* Area suchen */
   pArea = FindMarkerArea(pList, pchAreaTag);
   if (pArea)
   {
      SetAreaToFront(pList, pArea);

      /* Bucket suchen */
      pBucket = FindMarkerBucket(pArea, ulMsgID, &lFoundIndex);

      if (pBucket && lFoundIndex >=0)
      {
         /* Flags zuruecksetzen */
         pBucket->aItems[lFoundIndex].ulFlags &= ~ulFlags;

         /* Dirty-Flags */
         pArea->bDirty = TRUE;
         pList->bDirty = TRUE;

         if (pBucket->aItems[lFoundIndex].ulFlags == 0)
         {
            /* keine Flags mehr gesetzt, Item loeschen */
            pBucket->ulCountItems--;

            if (pBucket->ulCountItems)
            {
               /* Items uebrig, zusammenrutschen */
               if (pBucket->ulCountItems != lFoundIndex)
                  memmove(&(pBucket->aItems[lFoundIndex]), &(pBucket->aItems[lFoundIndex+1]), (pBucket->ulCountItems-lFoundIndex)*sizeof(MARKERITEM));
            }
            else
            {
               /* Bucket ist leer geworden, entfernen */
               if (pBucket->prev)
                  pBucket->prev->next = pBucket->next;
               if (pBucket->next)
                  pBucket->next->prev = pBucket->prev;
               if (pArea->pBuckets == pBucket)
                  pArea->pBuckets = pBucket->next;
               free(pBucket);

               if (pArea->pBuckets == NULL)
               {
                  /* letztes Bucket entfernt, Area entfernen */

                  if (pArea->next)
                     pArea->next->prev = pArea->prev;
                  if (pArea->prev)
                     pArea->prev->next = pArea->next;
                  if (pList->pAreas == pArea)
                     pList->pAreas = pArea->next;
                  free(pArea);
               }
            }
            DosReleaseMutexSem(pList->hmtxAccess);
            return 0;
         }
         else
         {
            DosReleaseMutexSem(pList->hmtxAccess);
            return 0;
         }
      }
      else
      {
         DosReleaseMutexSem(pList->hmtxAccess);
         return 1;
      }
   }
   else
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: IsMessageMarked                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht eine Message in der Markierungsliste                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pchAreaTag: Area-Tag                                           */
/*            ulMsgID:    Message-ID                                         */
/*            ulFlags:    Markierungs-Flags (MARKFLAG_*)                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE   Message ist mit mindestens einem Flag markiert      */
/*                FALSE  Message ist nicht markiert                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL IsMessageMarked(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulFlags)
{
   PMARKERAREA pArea;
   PMARKERBUCKET pBucket;
   LONG lFoundIndex;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   /* Area suchen */
   pArea = FindMarkerArea(pList, pchAreaTag);
   if (pArea)
   {
      /* Area nach vorne setzen */
      SetAreaToFront(pList, pArea);

      /* Message in den Buckets suchen */
      pBucket = FindMarkerBucket(pArea, ulMsgID, &lFoundIndex);

      if (pBucket && lFoundIndex >=0)
      {
         /* Flags pruefen */
         if (pBucket->aItems[lFoundIndex].ulFlags & ulFlags)
         {
            DosReleaseMutexSem(pList->hmtxAccess);
            return TRUE;
         }
         else
         {
            DosReleaseMutexSem(pList->hmtxAccess);
            return FALSE;
         }
      }
      else
      {
         DosReleaseMutexSem(pList->hmtxAccess);
         return FALSE;
      }
   }
   else
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return FALSE;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ChangeMarkedMessage                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Aendert den Text einer markierten Message                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pchAreaTag: Area-Tag                                           */
/*            pHeader:    Neuer Header                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0      OK, Message geaendert                               */
/*                1      Message nicht gefunden                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ChangeMarkedMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, MSGHEADER *pHeader)
{
   PMARKERAREA pArea;
   PMARKERBUCKET pBucket;
   LONG lFoundIndex;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   /* Area suchen */
   pArea = FindMarkerArea(pList, pchAreaTag);
   if (pArea)
   {
      /* Area nach vorne setzen */
      SetAreaToFront(pList, pArea);

      /* Message in den Buckets suchen */
      pBucket = FindMarkerBucket(pArea, ulMsgID, &lFoundIndex);

      if (pBucket && lFoundIndex >=0)
      {
         memcpy(pBucket->aItems[lFoundIndex].pchFrom, pHeader->pchFromName, LEN_USERNAME);
         memcpy(pBucket->aItems[lFoundIndex].pchSubj, pHeader->pchSubject, LEN_SUBJECT);
         pArea->bDirty=TRUE;
         pList->bDirty=TRUE;

         DosReleaseMutexSem(pList->hmtxAccess);
         return 0;
      }
      else
      {
         DosReleaseMutexSem(pList->hmtxAccess);
         return 1;
      }
   }
   else
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindMarkerArea                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht eine Area in der Liste                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pchAreaTag: Area-Tag                                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: NULL   Area nicht gefunden                                 */
/*                sonst  Zeiger auf die Area-Struktur                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PMARKERAREA FindMarkerArea(PMARKERLIST pList, PCHAR pchAreaTag)
{
   PMARKERAREA pArea=pList->pAreas;

   while (pArea && stricmp(pArea->pchAreaTag, pchAreaTag))
      pArea = pArea->next;

   return pArea;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindMarkerBucket                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht einen Bucket in der Area                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pArea: Area                                                    */
/*            ulMsgID: gesuchte Message-ID                                   */
/*            plFoundIndex: Index im Array                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: NULL   Message nicht gefunden                              */
/*                sonst  Zeiger auf den Bucket                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PMARKERBUCKET FindMarkerBucket(PMARKERAREA pArea, ULONG ulMsgID, PLONG plFoundIndex)
{
   PMARKERBUCKET pBucket=pArea->pBuckets;
   int i=0;

   *plFoundIndex = -1;

   if (pBucket == NULL || pBucket->ulCountItems == 0)
      return NULL;

   do
   {
      if (pBucket->aItems[0].ulMsgID <= ulMsgID &&
          pBucket->aItems[pBucket->ulCountItems-1].ulMsgID >= ulMsgID)
      {
         /* binaere Suche */
         int low=0;
         int high = pBucket->ulCountItems-1;

         while(high >= low)
         {
            i = (low+high)/2;

            if (ulMsgID == pBucket->aItems[i].ulMsgID)
            {
               *plFoundIndex = i;
               return pBucket;
            }

            if (ulMsgID < pBucket->aItems[i].ulMsgID)
               high = i-1;
            else
               low = i+1;
         }
      }
      if (pBucket->next)
         pBucket = pBucket->next;
      else
         break;
   }
   while(pBucket);

   return pBucket;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetAreaToFront                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt eine Area an den Anfang der Liste                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungs-Liste                                       */
/*            pArea: Area                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0      OK                                                  */
/*                sonst  Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int SetAreaToFront(PMARKERLIST pList, PMARKERAREA pArea)
{
   if (pArea == pList->pAreas) /* schon vorne */
      return 0;
   else
   {
      /* aushaengen */
      if (pArea->next)
         pArea->next->prev = pArea->prev;
      if (pArea->prev)
         pArea->prev->next = pArea->next;

      /* vorne einhaengen */
      pArea->next = pList->pAreas;
      pArea->prev = NULL;
      if (pList->pAreas)
         pList->pAreas->prev = pArea;
      pList->pAreas = pArea;

      return 0;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CalcAreaSize                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet die Groesse aller Items in einer Area             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pArea: Area                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Groesse der Items in Byte                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG CalcAreaSize(PMARKERAREA pArea)
{
   ULONG ulItems=0;
   PMARKERBUCKET pBucket= pArea->pBuckets;

   while (pBucket)
   {
      ulItems += pBucket->ulCountItems;

      pBucket = pBucket->next;
   }

   return ulItems * sizeof(MARKERITEM);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CopyMarkerItems                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Kopiert alle Items einer Area in einen Puffer               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pArea: Area                                                    */
/*            pDest: Zeiger auf Zielpuffer                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Puffergroesse vorher durch CalcAreaSize berechnen lassen       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CopyMarkerItems(PMARKERAREA pArea, PMARKERITEM pDest)
{
   ULONG ulItems=0;
   PMARKERBUCKET pBucket= pArea->pBuckets;

   while (pBucket)
   {
      if (pBucket->ulCountItems)
      {
         memcpy(&(pDest[ulItems]), pBucket->aItems, pBucket->ulCountItems * sizeof(MARKERITEM));
         ulItems += pBucket->ulCountItems;
      }

      pBucket = pBucket->next;
   }
   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: LoadMarkerItems                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Kopiert alle Items aus einem Puffer in eine Area            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pArea: Area                                                    */
/*            pSrc: Quellpuffer                                              */
/*            ulSizeBuffer: Groesse des Puffers in Byte                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Area wird als leer angenommen                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int LoadMarkerItems(PMARKERAREA pArea, PMARKERITEM pSrc, ULONG ulSizeBuffer)
{
   ULONG ulBufferItems = ulSizeBuffer / sizeof(MARKERITEM);
   ULONG ulItemsCopied=0;
   ULONG ulItemsToCopy=0;
   PMARKERBUCKET pBucket=NULL;
   PMARKERBUCKET pNewBucket=NULL;

   while(ulItemsCopied < ulBufferItems)
   {
      /* neuen Bucket anfordern, hinten anhaengen */
      pNewBucket = calloc(1, sizeof(MARKERBUCKET));

      /* einhaengen */
      if (pBucket)
      {
         /* hinten anhaengen */
         pBucket->next = pNewBucket;
         pNewBucket->prev = pBucket;
      }
      else
      {
         /* erster Bucket */
         pArea->pBuckets = pNewBucket;
      }
      pBucket = pNewBucket;

      ulItemsToCopy = (ulBufferItems - ulItemsCopied);
      if (ulItemsToCopy > SIZE_BUCKET)
         ulItemsToCopy = SIZE_BUCKET;

      memcpy(pBucket->aItems, &(pSrc[ulItemsCopied]), ulItemsToCopy * sizeof(MARKERITEM));
      pBucket->ulCountItems = ulItemsToCopy;

      ulItemsCopied += ulItemsToCopy;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadIniMarkers                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Laedt die Markierungsliste aus einem INI-File               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: inifile: Handle des INI-Files                                  */
/*            pList: Markierungsliste                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: App ist "Mark", Keys sind Area-Tags                            */
/*            Liste wird als leer angenommen                                 */
/*---------------------------------------------------------------------------*/

int ReadIniMarkers(HINI inifile, PMARKERLIST pList)
{
   PMARKERAREA pArea=NULL;
   PMARKERAREA pNewArea=NULL;
   PCHAR pchKeys=NULL;
   PCHAR pchCurrentKey=NULL;
   ULONG ulKeyLen=0;
   PMARKERITEM pItems=NULL;
   ULONG ulBufSize=0;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);
   pList->bDirty = FALSE;
   pList->pAreas = NULL;

   if (!inifile)
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   if (!PrfQueryProfileSize(inifile, "Mark", NULL, &ulKeyLen) || ulKeyLen == 0)
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   pchKeys=calloc(ulKeyLen+1, 1);

   if (!PrfQueryProfileData(inifile, "Mark", NULL, pchKeys, &ulKeyLen))
   {
      free(pchKeys);
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      /* neue Area anlegen */
      pNewArea = calloc(1, sizeof(MARKERAREA));

      if (pArea)
      {
         pArea->next = pNewArea;
         pNewArea->prev = pArea;
      }
      else
      {
         /* Erste Area */
         pList->pAreas = pNewArea;
      }
      pArea = pNewArea;

      strncpy(pArea->pchAreaTag, pchCurrentKey, LEN_AREATAG);

      /* Puffer fuer die Area ermitteln */
      if (PrfQueryProfileSize(inifile, "Mark", pchCurrentKey, &ulBufSize) && ulBufSize)
      {
         /* Puffer belegen */
         pItems = malloc(ulBufSize);
         if (PrfQueryProfileData(inifile, "Mark", pchCurrentKey, pItems, &ulBufSize))
         {
            LoadMarkerItems(pArea, pItems, ulBufSize);
         }

         /* Puffer wieder freigeben */
         free(pItems);
      }

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   DosReleaseMutexSem(pList->hmtxAccess);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SaveIniMarkers                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Speichert die Markierungsliste in einem INI-File            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: inifile: Handle des INI-Files                                  */
/*            pList: Markierungsliste                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: App ist "Mark", Keys sind Area-Tags                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int SaveIniMarkers(HINI inifile, PMARKERLIST pList)
{
   PMARKERAREA pArea=NULL;
   PMARKERITEM pItems=NULL;
   ULONG ulItemSize=0;
   PCHAR pchCurrentKey=NULL;
   PCHAR pchKeys=NULL;
   ULONG ulKeyLen=0;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   pArea=pList->pAreas;

   if (!inifile)
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   /* Areas speichern */
   while(pArea)
   {
      if (pArea->bDirty)
      {
         ulItemSize = CalcAreaSize(pArea);
         pItems = malloc(ulItemSize);
         CopyMarkerItems(pArea, pItems);
         PrfWriteProfileData(inifile, "Mark", pArea->pchAreaTag, pItems, ulItemSize);
         free(pItems);

         pArea->bDirty = FALSE;
      }
      pArea = pArea->next;
   }
   pList->bDirty = FALSE;

   /* geloeschte Areas pruefen */
   if (!PrfQueryProfileSize(inifile, "Mark", NULL, &ulKeyLen) || ulKeyLen == 0)
   {
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   pchKeys=calloc(ulKeyLen+1, 1);

   if (!PrfQueryProfileData(inifile, "Mark", NULL, pchKeys, &ulKeyLen))
   {
      free(pchKeys);
      DosReleaseMutexSem(pList->hmtxAccess);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      /* Case-sensitiv vergleichen */
      pArea = pList->pAreas;
      while (pArea && strcmp(pArea->pchAreaTag, pchCurrentKey))
         pArea = pArea->next;

      if (!pArea) /* alte Area loeschen */
         PrfWriteProfileData(inifile, "Mark", pchCurrentKey, NULL, 0);

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   DosReleaseMutexSem(pList->hmtxAccess);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenMarkerList                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Bereitet die Liste zur Verwendung vor                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungsliste                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int OpenMarkerList(PMARKERLIST pList)
{
   if (DosCreateMutexSem(NULL, &pList->hmtxAccess, 0, FALSE))
      return 1;
   else
      return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CloseMarkerList                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beendet den Zugriff auf die Markerliste                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungsliste                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int CloseMarkerList(PMARKERLIST pList)
{
   if (DosCloseMutexSem(pList->hmtxAccess))
      return 1;
   else
      return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CheckMarkerAreas                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Prueft, ob noch alle Areas in den Markierungen vorhanden    */
/*               sind, loescht ggf. die Area                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Markierungsliste                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int CheckMarkerAreas(PMARKERLIST pList)
{
   extern AREALIST arealiste;
   PMARKERAREA pArea=NULL;

   WinRequestMutexSem(pList->hmtxAccess, SEM_INDEFINITE_WAIT);

   pArea=pList->pAreas;
   while (pArea)
   {
      if (AM_FindArea(&arealiste, pArea->pchAreaTag))
         pArea=pArea->next; /* OK, weiter */
      else
      {
         /* Area existiert nicht mehr, loeschen */
         PMARKERBUCKET pBucket, pBucket2;
         PMARKERAREA pArea2;

         pBucket=pArea->pBuckets;
         while (pBucket)
         {
            pBucket2=pBucket;
            pBucket=pBucket->next;
            free(pBucket2);
         }
         pArea2=pArea;
         if (pArea->next)
            pArea->next->prev = pArea->prev;
         if (pArea->prev)
            pArea->prev->next = pArea->next;
         if (pList->pAreas == pArea)
            pList->pAreas = pArea->next;
         pArea=pArea->next;
         free(pArea2);
         pList->bDirty=TRUE;
      }
   }

   DosReleaseMutexSem(pList->hmtxAccess);

   return 0;
}
/*-------------------------------- Modulende --------------------------------*/

