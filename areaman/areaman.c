/*---------------------------------------------------------------------------+
 | Titel: AREAMAN.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 07.01.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Managen der Area-Liste                                                  |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "..\main.h"
#include "..\structs.h"
#include "areaman.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_FindArea                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Suchen einer Area in der Liste                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Kopf der Liste                                          */
/*            pchAreaTag: Area-Tag der gesuchten Area                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: NULL   Area nicht gefunden                                 */
/*                sonst  Zeiger auf Knoten                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PAREADEFLIST AM_FindArea(PAREALIST pList, const char *pchAreaTag)
{
   PAREADEFLIST pTemp = pList->pFirstArea;

   while (pTemp && stricmp(pTemp->areadata.areatag, pchAreaTag))
      pTemp = pTemp->next;

   return pTemp;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_AddArea                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Einfuegen einer Area in die Liste                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ppList: Zeiger auf den Listenkopf                              */
/*            pAreaDef: Area-Definitions-Struktur                            */
/*            ulOptions: Optionen, s. Header                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: NULL  Kein Speicher mehr                                   */
/*                sonst Zeiger auf neuen Knoten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Die Definitionsstruktur wird kopiert                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PAREADEFLIST AM_AddArea(PAREALIST pList, AREADEFOPT *pAreaDef, ULONG ulOptions)
{
   PAREADEFLIST pNewArea=NULL;

   if (ulOptions & ADDAREA_UNIQUE)  /* pruefen, ob schon da */
      if (AM_FindArea(pList, pAreaDef->areatag))
         return NULL;

   pNewArea = calloc(1, sizeof(AREADEFLIST));

   if (pNewArea)
   {
      if (ulOptions & ADDAREA_TAIL) /* Am Ende einhaengen */
      {
         pNewArea->prev = pList->pLastArea;
         if (pList->pLastArea)
            pList->pLastArea->next = pNewArea;
         pList->pLastArea = pNewArea;
         if (!pList->pFirstArea)
            pList->pFirstArea = pNewArea;
      }
      else
      {
         pNewArea->next = pList->pFirstArea;
         if (pList->pFirstArea)
            pList->pFirstArea->prev = pNewArea;
         pList->pFirstArea = pNewArea;
         if (!pList->pLastArea)
            pList->pLastArea = pNewArea;
      }
      pList->ulNumAreas++;

      /* pNewArea zeigt auf neuen Knoten */
      memcpy(&pNewArea->areadata, pAreaDef, sizeof(AREADEFOPT));

      if (ulOptions & ADDAREA_MARKDIRTY)
         pNewArea->dirty=TRUE;
   }

   return pNewArea;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_DeleteArea                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loeschen einer Area aus der Liste                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ppListe: Zeiger auf den Listenkopf                             */
/*            pchAreaTag: Area-Tag der zu loeschenden Area                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE    Erfolg                                             */
/*                FALSE   Fehler                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL AM_DeleteArea(PAREALIST pList, char *pchAreaTag)
{
   return AM_DeleteAreaDirect(pList, AM_FindArea(pList, pchAreaTag));
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_DeleteAreaDirect                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loeschen einer Area aus der Liste (direkt ueber den Zeiger  */
/*               auf den Knoten)                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ppList: Zeiger auf den Listenkopf                              */
/*            pDel:   Zeiger auf zu loeschenden Knoten                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL AM_DeleteAreaDirect(PAREALIST pList, PAREADEFLIST pDel)
{
   if (pDel)
   {
      if (pDel->next)
         pDel->next->prev = pDel->prev;
      if (pDel->prev)
         pDel->prev->next = pDel->next;

      if (pList->pFirstArea == pDel)
         pList->pFirstArea = pDel->next;

      if (pList->pLastArea == pDel)
         pList->pLastArea = pDel->prev;
      free(pDel);

      pList->ulNumAreas--;

      return TRUE;
   }
   else
      return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_DeleteAllAreas                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loeschen aller Areas in der Liste                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ppList: Zeiger auf den Listenkopf                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE   Erfolg                                              */
/*                FALSE  Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL AM_DeleteAllAreas(PAREALIST pList)
{
   while (pList->pFirstArea)
      if (AM_DeleteAreaDirect(pList, pList->pFirstArea))
         return FALSE;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AM_MergeAreas                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Haengt zwei Listen aneinander                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ppList1: Zeiger auf den Kopf der ersten Liste (= Zielliste)    */
/*            ppList2: Zeiger auf den Kopf der zweiten Liste                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Zeiger auf den ersten Knoten der Ergebnisliste             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: ppList2 wird auf NULL gesetzt, so daá auf die Knoten nur       */
/*            noch ueber ppList1 zugegriffen werden kann.                    */
/*---------------------------------------------------------------------------*/

PAREADEFLIST AM_MergeAreas(PAREALIST pList1, PAREALIST pList2)
{
   if (pList1->ulNumAreas && pList2->ulNumAreas) /* beide Listen mit Areas */
   {
      if (pList1->pLastArea)
         pList1->pLastArea->next = pList2->pFirstArea;
      if (pList2->pFirstArea)
         pList2->pFirstArea->prev = pList1->pLastArea;

      pList1->pLastArea = pList2->pLastArea;
      pList1->ulNumAreas += pList2->ulNumAreas;
   }
   else
      if (!pList1->ulNumAreas && pList2->ulNumAreas) /* nur Nr. 2 hat Areas */
         *pList1 = *pList2;

   memset(pList2, 0, sizeof(*pList2));

   return pList1->pFirstArea;
}

#if 0
BOOL AM_TraverseAreas(PAREALIST pAreaList, ULONG ulOptions, TRAVERSEFUNC CallBack, PVOID pParam)
{
   PAREADEFLIST pTemp = pAreaList->pFirstArea;

   while (pTemp)
   {
      if ((ulOptions & TRAVERSE_DIRTYONLY) &&
          !pTemp->dirty)
      {
         pTemp = pTemp->next;
         continue;
      }

      if (!CallBack(pTemp, pParam))
         return FALSE;
      else
         pTemp = pTemp->next;
   }

   return TRUE;
}
#endif
/*-------------------------------- Modulende --------------------------------*/
