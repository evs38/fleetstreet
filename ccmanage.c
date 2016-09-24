/*---------------------------------------------------------------------------+
 | Titel: CCMANAGE.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 28.07.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Funktionen zur Behandlung von CC-Listen                                 |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "structs.h"
#include "ccmanage.h"

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryCCList                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht die CC-Liste nach ID und liefert Zeiger auf Liste     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker der Listen                                      */
/*            ulListID: gesuchte ID                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: NULL  nicht gefunden                                       */
/*                sonst Zeiger auf Liste                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PCCLIST QueryCCList(PCCANCHOR pAnchor, ULONG ulListID)
{
   PCCLIST pList = pAnchor->pLists;

   while(pList && pList->ulListID != ulListID)
      pList = pList->next;

   return pList;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: HaveCCListName                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Stellt fest, ob eine Liste mit dem gegebenen Namen existiert*/
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker der Listen                                      */
/*            pchName: gesuchter Name                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Liste mit Name existiert                             */
/*                FALSE Liste existiert nicht                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Wird fuer Erzeugen eines Names gebraucht                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int HaveCCListName(void *pAnchor, char *pchName)
{
   PCCLIST pList = ((PCCANCHOR)pAnchor)->pLists;

   while(pList)
      if (!strcmp(pList->pchListName, pchName))
         return TRUE;
      else
         pList = pList->next;

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddCCList                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine neue, leere CC-Liste                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker aller Listen                                    */
/*            pchListName: Name fuer die neue Liste                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf die neue Liste                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PCCLIST AddCCList(PCCANCHOR pAnchor, PCHAR pchListName)
{
   PCCLIST pNewList;
   PCCLIST pTemp;
   ULONG   ulNewID = 0;

   pNewList = calloc(1, sizeof(CCLIST));
   if (!pNewList)
      return NULL;

   /* Daten vorbelegen */
   pNewList->bDirty = TRUE;
   if (pchListName)
      pNewList->pchListName = strdup(pchListName);

   /* neue ID suchen */
   pTemp = pAnchor->pLists;
   while (pTemp)
   {
      if (pTemp->ulListID > ulNewID)
         ulNewID = pTemp->ulListID;
      pTemp = pTemp->next;
   }
   ulNewID++;   /* ist beim ersten Mal 1, danach groesser */

   pNewList->ulListID = ulNewID;

   if (pAnchor->pLists)
   {
      /* Liste vorne einhaengen */
      pNewList->next = pAnchor->pLists;
      pAnchor->pLists->prev = pNewList;
      pAnchor->pLists = pNewList;
   }
   else
   {
      /* erste Liste */
      pAnchor->pLists = pNewList;
   }

   /* Eine mehr */
   pAnchor->ulNumLists++;
   pAnchor->bDirty = TRUE;

   return pNewList;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteCCList                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht eine Liste, ggf. mit allen anh�ngenden              */
/*               Eintraegen                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker aller Listen                                    */
/*            pList: Zeiger der zu loeschenden Liste                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL DeleteCCList(PCCANCHOR pAnchor, PCCLIST pList)
{
   PCCENTRY pEntry, pEntry2;

   /* Erst auf Eintraege pruefen */
   pEntry = pList->pEntries;

   while (pEntry)
   {
      pEntry2 = pEntry;
      pEntry = pEntry->next;
      free(pEntry2);
   }

   /* erste Liste? */
   if (pList == pAnchor->pLists)
      pAnchor->pLists = pList->next;

   /* Liste loeschen */
   if (pList->prev)
      pList->prev->next = pList->next;
   if (pList->next)
      pList->next->prev = pList->prev;
   if (pList->pchListName)
      free(pList->pchListName);
   free(pList);

   /* eine weniger */
   pAnchor->ulNumLists--;
   pAnchor->bDirty = TRUE;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddCCEntry                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt neuen Eintrag in der Liste, kopiert vorgegebene     */
/*               Daten                                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker aller Listen                                    */
/*            pList: Zeiger auf Liste                                        */
/*            pEntry: Default-Daten                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf neuen Eintrag                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PCCENTRY AddCCEntry(PCCANCHOR pAnchor, PCCLIST pList, PCCENTRY pEntry)
{
   PCCENTRY pNewEntry;

   pNewEntry = calloc(1, sizeof(CCENTRY));

   if (pList->pEntries)
   {
      /* vorne Anhaengen */
      pNewEntry->next = pList->pEntries;
      pList->pEntries->prev = pNewEntry;
      pList->pEntries = pNewEntry;
   }
   else
   {
      /* erster Eintrag */
      pList->pEntries = pNewEntry;
   }

   /* Default setzen */
   pNewEntry->ulFlags = CCENTRY_MENTION;

   /* evtl. Daten uebernehmen */
   if (pEntry)
   {
      strncpy(pNewEntry->pchName, pEntry->pchName, LEN_USERNAME);
      strncpy(pNewEntry->pchAddress, pEntry->pchAddress, LEN_5DADDRESS);
      strncpy(pNewEntry->pchFirstLine, pEntry->pchFirstLine, LEN_FIRSTLINE);
      pNewEntry->ulFlags = pEntry->ulFlags;
   }

   pList->bDirty = TRUE;
   if (pAnchor)
      pAnchor->bDirty = TRUE;

   return pNewEntry;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteCCEntry                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht einen Eintrag                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAnchor: Anker aller Listen                                    */
/*            pList: Zeiger auf Listen-Anfang                                */
/*            pEntry: zu loeschender Eintrag                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE   Erfolg                                              */
/*                FALSE  Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL DeleteCCEntry(PCCANCHOR pAnchor, PCCLIST pList, PCCENTRY pEntry)
{
   /* erster Eintrag ? */
   if (pList->pEntries == pEntry)
      pList->pEntries = pEntry->next;

   /* freigeben */
   if (pEntry->prev)
      pEntry->prev->next = pEntry->next;
   if (pEntry->next)
      pEntry->next->prev = pEntry->prev;
   free(pEntry);

   pList->bDirty = TRUE;
   if (pAnchor)
      pAnchor->bDirty = TRUE;

   return TRUE;
}

/*-------------------------------- Modulende --------------------------------*/

