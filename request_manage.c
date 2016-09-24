/*---------------------------------------------------------------------------+
 | Titel: REQUEST_MANAGE.C                                                   |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 18.10.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Low-Level-Funktionen f. File-Requests von FleetStreet                  |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "structs.h"
#include "dump\expt.h"

#include "request_manage.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/
static PRAMLIST AddSingleFiles(PRAMLIST pLine, char *pchLineText);

/*-----------------------------------------------------------------------------
 | Funktionsname: AddNewFileList
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fuegt eine neue Liste in die Liste ein.
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: ppList: Zeiger auf Listenanfang
 |            pNewList: Zeiger auf neue Knotendaten
 |            pbDirty: Zeiger auf Dirty-Marke oder NULL
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: Zeiger auf neuen Knoten
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Wenn pbDirty == NULL ist, wird auch das Dirty-Flag im neuen
 |            Knoten nicht gesetzt.
 +---------------------------------------------------------------------------*/

PFILELIST AddNewFileList(PFILELIST *ppList, PFILELIST pNewList, PBOOL pbDirty)
{
   PFILELIST pAddList=NULL;

   pAddList = calloc(1, sizeof(FILELIST));

   /* vorne einhaengen */
   pAddList->next = *ppList;
   if (*ppList)
      (*ppList)->prev = pAddList;
   *ppList = pAddList;

   /* Dirty setzen */
   if (pbDirty)
      *pbDirty = pAddList->bDirty = TRUE;

   /* Daten kopieren */
   memcpy(pAddList->pchFileName, pNewList->pchFileName, LEN_PATHNAME);
   memcpy(pAddList->pchAddress, pNewList->pchAddress, LEN_5DADDRESS);
   memcpy(pAddList->pchDesc, pNewList->pchDesc, LEN_LISTDESC);

   return pAddList;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: DeleteFileList
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: L”scht einen Knoten aus der Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: ppList: Zeiger auf Listenanfang
 |            pDelList: Zeiger auf zu l”schenden Knoten
 |            pbDirty: Zeiger auf Dirty-Flag
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte:  TRUE  Erfolg
 |                 FALSE Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

BOOL DeleteFileList(PFILELIST *ppList, PFILELIST pDelList, PBOOL pbDirty)
{
   if (!pDelList || !ppList)
      return FALSE;

   /* aushaengen */
   if (pDelList->next)
      pDelList->next->prev = pDelList->prev;
   if (pDelList->prev)
      pDelList->prev->next = pDelList->next;
   if (*ppList == pDelList)
      *ppList = pDelList->next;

   /* Speicher freigeben */
   free(pDelList);

   /* Dirty setzen */
   *pbDirty = TRUE;

   return TRUE;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: ListReadThread
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Liesst eine Fileliste ein und erzeugt eine String-Liste
 |               mit dem Inhalt
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: Thread-Parameter-Block (FILELISTREAD)
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Erfolgsmeldung des Threads wird in ulRetCode des Parameter-
 |            Blocks abgelegt
 +---------------------------------------------------------------------------*/

void _Optlink ListReadThread(PVOID pData)
{
   PFILELISTREAD pReadParam = pData;
   FILE *pfList;
   PRAMLIST pLast=NULL;
   char pchLineBuff[300];
   char *pchTemp;

   INSTALLEXPT("ReadFList");

   if (pfList = fopen(pReadParam->pList->pchFileName, "r"))
   {
      while (!feof(pfList) && !pReadParam->bStop)
      {
         if (fgets(pchLineBuff, sizeof(pchLineBuff), pfList))
         {
            /* \n am Ende entfernen */
            if (pchTemp = strrchr(pchLineBuff, '\n'))
               *pchTemp = 0;

            /* neue Zeile anhaengen */
            if (pLast)
            {
               pLast->next = calloc(1, sizeof(RAMLIST));
               pLast = pLast->next;
            }
            else
               pLast = pReadParam->pReadList = calloc(1, sizeof(RAMLIST));

            pLast->pchLine = strdup(pchLineBuff);
         }
      }
      if (ferror(pfList))
         pReadParam->ulRetCode = FILELIST_READERR;

      fclose(pfList);
   }
   else
      pReadParam->ulRetCode = FILELIST_NOTF;

   WinPostMsg(pReadParam->hwndNotify, REQM_LISTREAD, NULL, NULL);

   DEINSTALLEXPT;

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: MessageToFileList
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Wandelt einen Message-Text in eine String-Liste um (wie
 |               eine File-Liste)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchMessageText: Zeiger auf den Text. Die Abs„tze sind mit \n
 |                            getrennt
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: Zeiger auf Anfang der String-Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

PRAMLIST MessageToFileList(char *pchMessageText)
{
   char *pchTemp = pchMessageText, *pchTemp2;
   PRAMLIST pFirst=NULL, pLast=NULL;

   while (pchTemp)
   {
      /* Speicher holen */
      if (pLast)
      {
         pLast->next = calloc(1, sizeof(RAMLIST));
         pLast = pLast->next;
      }
      else
         pLast = pFirst = calloc(1, sizeof(RAMLIST));

      pchTemp2 = strchr(pchTemp, '\n');

      if (pchTemp2)
      {
         /* Zeile in der Mitte kopieren */
         int len = pchTemp2 - pchTemp;

         pLast->pchLine = malloc(len+1);
         if (len)
            memcpy(pLast->pchLine, pchTemp, len);
         pLast->pchLine[len]=0;
         pLast = AddSingleFiles(pLast, pLast->pchLine);
      }
      else
      {
         /* Zeile am Ende kopieren */
         pLast->pchLine = strdup(pchTemp);
         pLast = AddSingleFiles(pLast, pchTemp);
      }

      if (pchTemp2)
         pchTemp = pchTemp2+1;
      else
         pchTemp = NULL;
   }

   return pFirst;
}

#define SEPA_CHARS " \t,:;"

static PRAMLIST AddSingleFiles(PRAMLIST pLine, char *pchLineText)
{
   char *pchDup = strdup(pchLineText);

   if (pchDup)
   {
      char *pchWord;

      pchWord = strtok(pchDup, SEPA_CHARS);

      while (pchWord)
      {
         if (IsFileName(pchWord, TRUE))
         {
            pLine->next = calloc(1, sizeof(RAMLIST));
            pLine = pLine->next;

            pLine->pchLine = strdup(pchWord);
         }
         pchWord = strtok(NULL, SEPA_CHARS);
      }

      free(pchDup);
   }


   return pLine;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: FreeFileList
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Gibt den Speicher einer Stringliste frei
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pList: Zeiger auf Anfang der Liste
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void FreeFileList(PRAMLIST pList)
{
   PRAMLIST pNext;

   while (pList)
   {
      pNext = pList->next;
      free(pList);
      pList = pNext;
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: IsFileName
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Prueft, ob ein Wort einen Filenamen darstellt
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchName: Wort
 |            bDotRequired: Punkt muá in Name vorkommen
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: TRUE   ist Filename
 |                FALSE  ist kein Filename
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

#define VALID_CHARS  ".$%'-_@{}~`!#()*?"

BOOL IsFileName(char *pchName, BOOL bDotRequired)
{
   char *pchTemp=pchName;
   BOOL bHaveDot =FALSE;
   int count_pre=0;
   int count_post=0;

   while (*pchTemp == ' ')
      pchTemp++;

   if ((pchTemp - pchName) > 5)  /* muá in den ersten 5 Zeichen beginnen */
      return FALSE;

   while (*pchTemp)
      if (isalnum(*pchTemp) ||
          strchr(VALID_CHARS, *pchTemp))
      {
         if (*pchTemp == '.')
         {
            if (bHaveDot)
               return FALSE; /* mehr als 1 Punkt */
            else
               bHaveDot = TRUE;
         }
         else
            if (bHaveDot)
               count_post++;
            else
               count_pre++;

         pchTemp++;
      }
      else
         if (*pchTemp == ' ')
         {
            if ((count_post+count_pre) > 23)
               return FALSE;
            break;
         }
         else
            return FALSE;

   if (count_pre && count_pre <=20 &&
       count_post <=3)
   {
      if (bDotRequired && !count_post)
         return FALSE;
      else
         return TRUE;
   }
   else
      return FALSE;
}

/*-------------------------------- Modulende --------------------------------*/

