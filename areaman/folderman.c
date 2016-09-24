/*---------------------------------------------------------------------------+
 | Titel: FOLDERMAN.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 14.04.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Folder-Manager fuer FleetStreet                                       |
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
#include "folderman.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PAREAFOLDER FM_AddFolder(PFOLDERANCHOR pAnchor, PAREAFOLDER pNewFolder, ULONG ulOptions)
{
   PAREAFOLDER pAddFolder;

   /* Speicher holen */
   pAddFolder = calloc(1, sizeof(AREAFOLDER));

   /* Daten kopieren */
   pAddFolder->pchName = strdup(pNewFolder->pchName);
   pAddFolder->ParentFolder = pNewFolder->ParentFolder;
   pAddFolder->ulFlags = pNewFolder->ulFlags;

   /* Neue ID vergeben */
   if (ulOptions & ADDFOLDER_NEWID)
   {
      pAddFolder->FolderID = ++pAnchor->HighID;
      pAnchor->bDirty = TRUE;
   }
   else
      pAddFolder->FolderID = pNewFolder->FolderID;

   /* einhaengen */
   if (ulOptions & ADDFOLDER_TAIL)
   {
      if (pAnchor->pListLast)
      {
         pAnchor->pListLast->next = pAddFolder;
         pAddFolder->prev = pAnchor->pListLast;
      }
      else
         pAnchor->pList = pAddFolder;
      pAnchor->pListLast = pAddFolder;
   }
   else
   {
      if (pAnchor->pList)
      {
         pAnchor->pList->prev = pAddFolder;
         pAddFolder->next = pAnchor->pList;
      }
      else
         pAnchor->pListLast = pAddFolder;
      pAnchor->pList = pAddFolder;
   }

   if (ulOptions & ADDFOLDER_MARKDIRTY)
   {
      pAddFolder->bDirty = TRUE;
      pAnchor->bDirty = TRUE;
   }

   pAnchor->ulNumFolders++;

   return pAddFolder;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PAREAFOLDER FM_FindFolder(PFOLDERANCHOR pAnchor, LONG FolderID)
{
   PAREAFOLDER pFolder= pAnchor->pList;

   while (pFolder && pFolder->FolderID != FolderID)
      pFolder = pFolder->next;

   return pFolder;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PAREAFOLDER FM_FindFolderWithParent(PFOLDERANCHOR pAnchor, PAREAFOLDER pLast, LONG ParentID)
{
   if (!pLast)
      pLast = pAnchor->pList;  /* Anfang der Liste */
   else
      pLast = pLast->next;     /* weiter beim n�chsten */

   while (pLast && pLast->ParentFolder != ParentID)
      pLast = pLast->next;

   return pLast;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL FM_DeleteFolder(PFOLDERANCHOR pAnchor, LONG FolderID)
{
   if (FolderID == FOLDERID_ALL)
   {
      while (pAnchor->pList)
         if (!FM_DeleteFolderDirect(pAnchor, pAnchor->pList))
            return FALSE;

      return TRUE;
   }
   else
   {
      PAREAFOLDER pDelFolder = FM_FindFolder(pAnchor, FolderID);

      if (pDelFolder)
         return FM_DeleteFolderDirect(pAnchor, pDelFolder);
      else
         return FALSE;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL FM_DeleteFolderDirect(PFOLDERANCHOR pAnchor, PAREAFOLDER pFolder)
{
   if (pFolder)
   {
      /* Aushaengen */
      if (pFolder->prev)
         pFolder->prev->next = pFolder->next;
      if (pFolder->next)
         pFolder->next->prev = pFolder->prev;
      if (pAnchor->pList == pFolder)
         pAnchor->pList = pFolder->next;
      if (pAnchor->pListLast == pFolder)
         pAnchor->pListLast = pFolder->prev;

      /* Loeschen */
      free(pFolder->pchName);
      free(pFolder);

      pAnchor->bDirty = TRUE;
      pAnchor->ulNumFolders--;

      return TRUE;
   }
   else
      return FALSE;
}

/*-------------------------------- Modulende --------------------------------*/

