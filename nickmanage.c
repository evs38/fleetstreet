/*---------------------------------------------------------------------------+
 | Titel: NICKMANAGE.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 09.04.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Verwaltung der Nicknames                                                |
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
#include "main.h"
#include "structs.h"
#include "nickmanage.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddNickname                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt einen Nickname zur Liste hinzu                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Listenkopf                                              */
/*            pNickname: neuer Nickname, wird kopiert                        */
/*            bMarkDirty: TRUE: Dirty-Flag wird beim neuen Eintrag gesetzt   */
/*                        FALSE: Flag wird nicht gesetzt                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf neuen Eintrag                                   */
/*                NULL  Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PNICKNAME AddNickname(PNICKNAMELIST pList, PNICKNAME pNickname, BOOL bMarkDirty)
{
   PNICKNAME pNewEntry;

   pNewEntry = calloc(1, sizeof(NICKNAME));

   if (pNewEntry)
   {
      /* vorne einhaengen */
      pNewEntry->next = pList->pFirstEntry;
      if (pNewEntry->next)
         pNewEntry->next->prev = pNewEntry;
      pList->pFirstEntry = pNewEntry;

      /* Daten uebernehmen */
      memcpy(pNewEntry->usertag, pNickname->usertag, LEN_USERNAME);
      strlwr(pNewEntry->usertag);
      memcpy(pNewEntry->username, pNickname->username, LEN_USERNAME);
      memcpy(pNewEntry->address, pNickname->address, LEN_5DADDRESS);
      memcpy(pNewEntry->subjectline, pNickname->subjectline, LEN_SUBJECT);
      memcpy(pNewEntry->firstline, pNickname->firstline, LEN_FIRSTLINE);
      if (pNickname->pchComment)
         pNewEntry->pchComment = strdup(pNickname->pchComment);
      pNewEntry->ulAttrib = pNickname->ulAttrib;
      pNewEntry->ulFlags = pNickname->ulFlags;

      /* Dirty-Flags */
      if (bMarkDirty)
      {
         pNewEntry->bDirty = TRUE;
         pList->bDirty = TRUE;
      }
      pList->ulNumEntries++;
   }

   return pNewEntry;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindNickname                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht einen Nickname in der Liste                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Listenkopf                                              */
/*            pchNickname: Kurzname, nach dem gesucht werden soll            */
/*                         NULL: naechster Eintrag nach pSearchAfter         */
/*            pSearchAfter: Zeiger auf den Eintrag, ab dem gesucht werden    */
/*                          soll (excl. diesem Eintrag). NULL: Vom Anfang    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf Nickname                                        */
/*                NULL: nicht gefunden oder Liste zu Ende                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Man kann leicht alle Eintraege aufzaehlen durch:               */
/*                                                                           */
/*            PNICKNAME pNick=NULL;                                          */
/*            while (pNick = FindNickname(&List, NULL, pNick))               */
/*            {                                                              */
/*                bearbeiten                                                 */
/*            }                                                              */
/*---------------------------------------------------------------------------*/

PNICKNAME FindNickname(PNICKNAMELIST pList, PCHAR pchNickname, PNICKNAME pSearchAfter)
{
   if (!pSearchAfter)
      pSearchAfter = pList->pFirstEntry;
   else
      pSearchAfter = pSearchAfter->next;

   while (pSearchAfter && pchNickname && stricmp(pSearchAfter->usertag, pchNickname))
      pSearchAfter = pSearchAfter->next;

   return pSearchAfter;
}

PNICKNAME FindNicknameSens(PNICKNAMELIST pList, PCHAR pchNickname)
{
   PNICKNAME pSearchAfter = pSearchAfter = pList->pFirstEntry;

   while (pSearchAfter && strcmp(pSearchAfter->usertag, pchNickname))
      pSearchAfter = pSearchAfter->next;

   return pSearchAfter;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteNickname                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht einen Nickname aus der Liste                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Listen-Kopf                                             */
/*            pchNickname: zu loeschender Nickname                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0   OK                                                     */
/*                sonst  nicht gefunden                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Dirty-Flag der Liste wird gesetzt                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int DeleteNickname(PNICKNAMELIST pList, PCHAR pchNickname)
{
   PNICKNAME pDel;

   pDel = FindNickname(pList, pchNickname, NULL);

   if (pDel)
   {
      if (pDel->pchComment)
         free(pDel->pchComment);

      if (pDel->next)
         pDel->next->prev = pDel->prev;
      if (pDel->prev)
         pDel->prev->next = pDel->next;

      if (pList->pFirstEntry == pDel)
         pList->pFirstEntry = pDel->next;

      free(pDel);

      pList->bDirty = TRUE;
      pList->ulNumEntries--;

      return 0;
   }
   else
      return -1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ChangeNickname                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: 始dert die Daten eines Nicknames                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pList: Listen-Kopf                                             */
/*            pNickToChange: Zeiger auf Eintrag, der ge�ndert werden soll.   */
/*            pNewNickname: neue Daten, werden kopiert.                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0   OK                                                     */
/*                1   Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Dirty-Flags werden gesetzt                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ChangeNickname(PNICKNAMELIST pList, PNICKNAME pNickToChange, PNICKNAME pNewNickname)
{
   /* Daten uebernehmen */
   memcpy(pNickToChange->usertag, pNewNickname->usertag, LEN_USERNAME);
   strlwr(pNickToChange->usertag);
   memcpy(pNickToChange->username, pNewNickname->username, LEN_USERNAME);
   memcpy(pNickToChange->address, pNewNickname->address, LEN_5DADDRESS);
   memcpy(pNickToChange->subjectline, pNewNickname->subjectline, LEN_SUBJECT);
   memcpy(pNickToChange->firstline, pNewNickname->firstline, LEN_FIRSTLINE);
   if (pNickToChange->pchComment)
   {
      free(pNickToChange->pchComment);
      pNickToChange->pchComment = NULL;
   }
   if (pNewNickname->pchComment)
      pNickToChange->pchComment = strdup(pNewNickname->pchComment);
   pNickToChange->ulAttrib = pNewNickname->ulAttrib;
   pNickToChange->ulFlags = pNewNickname->ulFlags;

   /* Dirty-Flags */
   pNickToChange->bDirty = TRUE;
   pList->bDirty = TRUE;

   return 0;
}


/*-------------------------------- Modulende --------------------------------*/

