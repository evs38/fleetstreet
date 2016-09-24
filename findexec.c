/*---------------------------------------------------------------------------+
 | Titel: FINDEXEC.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 13.09.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Ausfuehrung eines Suchjobs                                              |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "main.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "finddlg.h"
#include "util\approx.h"
#include "util\match.h"
#include "markmanage.h"
#include "util\fltutil.h"
#include "utility.h"
#include "dump\expt.h"
#include "findexec.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

typedef int (*CompareFunc)(char *, char *, PFINDJOB);

/*---------------------------- Globale Variablen ----------------------------*/

extern HWND hwndFindResults;
extern HWND hwndFindDlg;
extern HWND client;
extern volatile BOOL DoingFind;
extern volatile BOOL StopFind;
extern int tidFind;
extern MARKERLIST MarkerList;

/*----------------------- interne Funktionsprototypen -----------------------*/
static int FindText(PFINDJOB pFindJob, AREADEFLIST *pAreaDef, CompareFunc pCompareFunc);
static int FindPersmail(PFINDJOB pFindJob, AREADEFLIST *pAreaDef);
static int FindUnsent(PFINDJOB pFindJob, AREADEFLIST *pAreaDef);

static int MarkFindAreas(PAREALIST pAreaList, PFINDJOB pFindJob, ULONG ulMask);
static int UnmarkAllAreas(PAREALIST pAreaList, ULONG ulMask);

static int SensSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob);
static int CaseSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob);
static int FuzzySearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob);
static int RegexSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob);
static int PrepareRegexSearch(PFINDJOB pFindJob);
static int TerminateRegexSearch(PFINDJOB pFindJob);

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindThread2                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Thread-Funktion fuer's Suchen                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf FINDJOB                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void _Optlink FindThread2(PVOID pParam)
{
   PFINDJOB pFindJob=(PFINDJOB) pParam;
   AREADEFLIST *zeiger=NULL;
   extern AREALIST arealiste;
   extern HAB anchor;
   CompareFunc FindCompareFunc;
   HMQ hmq = NULLHANDLE;
   BOOL bRegOK=TRUE;

   INSTALLEXPT("Find");

   /* Flag setzen */
   DoingFind=TRUE;
   StopFind=FALSE;

   hmq=WinCreateMsgQueue(anchor, 0);
   WinCancelShutdown(hmq, TRUE);


   if ((pFindJob->ulHow & FINDHOW_METHOD_MASK) == FINDHOW_PERSMAIL)
   {
      SendMsg(client, WORKM_STARTFIND, MPFROMLONG(MARKFLAG_PERSMAIL), NULL);
      MarkFindAreas(&arealiste, pFindJob, WORK_PERSMAIL);

      zeiger= arealiste.pFirstArea;
      while(zeiger && !StopFind)
      {
         if (zeiger->flWork & WORK_PERSMAIL)
         {
            FindPersmail(pFindJob, zeiger);

            zeiger->flWork &= ~WORK_PERSMAIL;
         }
         zeiger = zeiger->next;
      }
   }
   else
      if ((pFindJob->ulHow & FINDHOW_METHOD_MASK) == FINDHOW_UNSENT)
      {
         SendMsg(client, WORKM_STARTFIND, MPFROMLONG(MARKFLAG_UNSENT), NULL);
         MarkFindAreas(&arealiste, pFindJob, WORK_FIND);

         zeiger= arealiste.pFirstArea;
         while(zeiger && !StopFind)
         {
            if (zeiger->flWork & WORK_FIND)
            {
               FindUnsent(pFindJob, zeiger);

               zeiger->flWork &= ~WORK_FIND;
            }
            zeiger = zeiger->next;
         }
      }
      else
      {
         SendMsg(client, WORKM_STARTFIND, MPFROMLONG(MARKFLAG_FIND), NULL);
         MarkFindAreas(&arealiste, pFindJob, WORK_FIND);

         /* Vergleichsfunktion zuordnen */
         switch(pFindJob->ulHow & FINDHOW_METHOD_MASK)
         {
            case FINDHOW_SENS:
               FindCompareFunc = SensSearch;
               break;

            case FINDHOW_CASE:
               FindCompareFunc = CaseSearch;
               break;

            case FINDHOW_FUZZY:
               FindCompareFunc = FuzzySearch;
               break;

            case FINDHOW_REGEX:
               FindCompareFunc = RegexSearch;
               bRegOK = PrepareRegexSearch(pFindJob);
               break;

            default:
               FindCompareFunc = CaseSearch;
               break;
         }

         zeiger= arealiste.pFirstArea;
         while(zeiger && !StopFind && bRegOK)
         {
            if (zeiger->flWork & WORK_FIND)
            {
               FindText(pFindJob, zeiger, FindCompareFunc);

               zeiger->flWork &= ~WORK_FIND;
            }
            zeiger = zeiger->next;
         }
         if (bRegOK)
            switch(pFindJob->ulHow & FINDHOW_METHOD_MASK)
            {
               case FINDHOW_REGEX:
                  TerminateRegexSearch(pFindJob);
                  break;
            }
      }

   UnmarkAllAreas(&arealiste, WORK_FIND | WORK_PERSMAIL);

   /* Flag loeschen und Ende */
   DoingFind=FALSE;
   SendMsg(client, WORKM_STOPFIND, NULL, NULL);

   if (pFindJob->pchAreas)
      free(pFindJob->pchAreas);
   free(pFindJob);
   WinDestroyMsgQueue(hmq);

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MarkFindAreas                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert die zu durchsuchenden Areas                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAreaList: Area-Liste                                          */
/*            pFindJob: Zeiger auf Suchjob                                   */
/*            ulMask:   Bitmaske f. Work-Feld                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int MarkFindAreas(PAREALIST pAreaList, PFINDJOB pFindJob, ULONG ulMask)
{
   extern char CurrentArea[LEN_AREATAG+1];

   AREADEFLIST *zeiger=pAreaList->pFirstArea;
   ULONG ulAreas = pFindJob->ulWAreas & 0x0f;
   ULONG ulThisType=0;

   while (zeiger) /* Areas abklappern */
   {
      switch(ulAreas)
      {
         case FINDAREAS_CURRENT:
            if (!stricmp(CurrentArea, zeiger->areadata.areatag))
               zeiger->flWork |= ulMask;
            break;

         case FINDAREAS_ALL:
            zeiger->flWork |= ulMask;
            break;

         case FINDAREAS_CUSTOMN:
            if (AreaInAreaSet(pFindJob->pchAreas, zeiger->areadata.areatag))
               zeiger->flWork |= ulMask;
            break;

         case FINDAREAS_TYPE:
            if (zeiger->areadata.areatype == AREATYPE_LOCAL)
            {
               if (zeiger->areadata.ulAreaOpt & AREAOPT_FROMCFG)
                  ulThisType=FINDAREAS_LOCAL;
               else
                  ulThisType=FINDAREAS_PRIV;
            }
            else
               if (zeiger->areadata.areatype == AREATYPE_ECHO)
                  ulThisType=FINDAREAS_ECHO;
               else
                  ulThisType=FINDAREAS_NM;

            if (pFindJob->ulWAreas & ulThisType)
               zeiger->flWork |= ulMask;
            break;

         default:
            break;
      }

      zeiger = zeiger->next;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UnmarkAllAreas                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht Flags bei allen Areas                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAreaList: Area-Liste                                          */
/*            ulMask: zu loeschende Flags                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int UnmarkAllAreas(PAREALIST pAreaList, ULONG ulMask)
{
   AREADEFLIST *zeiger=pAreaList->pFirstArea;

   while (zeiger) /* Areas abklappern */
   {
      zeiger->flWork &= ~ulMask;

      zeiger = zeiger->next;
   }

   return 0;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: FindText                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht nach Text in einer Area                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pFindJob: Zeiger auf Suchjob                                   */
/*            pAreaDef: Zu durchsuchende Area                                */
/*            pCompareFunc: Zeiger auf Vergleichsfunktion                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int FindText(PFINDJOB pFindJob, AREADEFLIST *pAreaDef, CompareFunc pCompareFunc)
{
   extern AREALIST arealiste;
   extern char CurrentArea[LEN_AREATAG+1];
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;
   MSGHEADER Header;
   FTNMESSAGE Message;
   ULONG ulMsgNum;
   BOOL FoundIt=FALSE;
   FINDRESULTLIST ResultList;
   PFOUNDMSG pFoundMsg=NULL;
   PFOUNDMSG pFoundList=NULL;

   memset(&Message, 0, sizeof(Message));

   if (!MSG_OpenArea(&arealiste, pAreaDef->areadata.areatag, miscoptions.lastreadoffset, &driveremap))
   {
      /* Area-Tag anzeigen */
      SendMsg(client, WORKM_FINDAREA, pAreaDef->areadata.areatag, NULL);

      memset(&ResultList, 0, sizeof(ResultList));
      strcpy(ResultList.pchAreaTag, pAreaDef->areadata.areatag);
      ResultList.ulFindType = MARKFLAG_FIND;

      for (ulMsgNum=1; ulMsgNum<=pAreaDef->maxmessages && !StopFind; ulMsgNum++)
      {
         FoundIt=FALSE;

         if (!(pFindJob->ulWhere & FINDWHERE_TEXT))
         {
            /* Nur Header */
            if (MSG_ReadHeader(&Header, &arealiste, pAreaDef->areadata.areatag, ulMsgNum))
               continue;
         }
         else
            /* gesamte Message lesen */
            if (MSG_ReadNum(&Message, &Header, &arealiste, pAreaDef->areadata.areatag, ulMsgNum))
               continue;

         if (!(pFindJob->ulHow & FINDHOW_UNREADONLY) || !(Header.ulAttrib & ATTRIB_READ))
         {
            if (pFindJob->ulWhere & FINDWHERE_FROM)
               FoundIt = pCompareFunc(Header.pchFromName, pFindJob->pchWhat, pFindJob);

            if (!FoundIt && (pFindJob->ulWhere & FINDWHERE_TO))
               FoundIt = pCompareFunc(Header.pchToName, pFindJob->pchWhat, pFindJob);

            if (!FoundIt && (pFindJob->ulWhere & FINDWHERE_SUBJ))
               FoundIt = pCompareFunc(Header.pchSubject, pFindJob->pchWhat, pFindJob);

            if (!FoundIt && (pFindJob->ulWhere & FINDWHERE_TEXT))
               FoundIt = pCompareFunc(Message.pchMessageText, pFindJob->pchWhat, pFindJob);

            if (FoundIt && hwndFindResults)
            {
               ULONG ulMsgID = MSG_MsgnToUid(&arealiste, pAreaDef->areadata.areatag, ulMsgNum);

               /* In Markerliste aufnehmen */
               if (MarkMessage(&MarkerList, pAreaDef->areadata.areatag, ulMsgID,
                               ulMsgNum, &Header, pFindJob->pchWhat, MARKFLAG_FIND, pFindJob->ulHow, pFindJob->ulWhere)==0)
               {
                  /* Ergebnis einketten */
                  if (pFoundMsg)
                  {
                     pFoundMsg->next = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundMsg->next;
                  }
                  else
                  {
                     pFoundList = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundList;
                  }
                  pFoundMsg->Header = Header;
                  pFoundMsg->ulMsgID = ulMsgID;
                  pFoundMsg->ulMsgNum = ulMsgNum;
                  pFoundMsg->ulHow    = pFindJob->ulHow;
                  pFoundMsg->ulWhere  = pFindJob->ulWhere;
                  pFoundMsg->pchFindText= pFindJob->pchWhat;
                  ResultList.ulFoundMsgs++;
               }
            }
         }

         /* Laufbalken ab und zu updaten */
         if (ulMsgNum % 10 == 0)
            SendMsg(client, WORKM_FINDPROGRESS,
                       MPFROMLONG(pAreaDef->maxmessages?(100 * ulMsgNum / pAreaDef->maxmessages):100),
                       NULL);
      }

      /* Suchergebnis in Dialog einfuegen */
      if (ResultList.ulFoundMsgs)
         SendMsg(client, WORKM_FINDAREAEND, &ResultList, pFoundList);

      while (pFoundList)
      {
         pFoundMsg = pFoundList;
         pFoundList = pFoundList->next;
         free(pFoundMsg);
      }

      /* Aufraeumen */
      MSG_ClearMessage(&Header, &Message);

      MSG_CloseArea(&arealiste, pAreaDef->areadata.areatag, FALSE, miscoptions.lastreadoffset, &driveremap);
   }
   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindPersmail                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt den Personal Mail Scan in einer Area durch           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pFindJob: Such-Job-Parameter                                   */
/*            pAreaDef: Zeiger auf Area-Definition                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int FindPersmail(PFINDJOB pFindJob, AREADEFLIST *pAreaDef)
{
   extern MISCOPTIONS miscoptions;
   extern USERDATAOPT userdaten;
   extern DRIVEREMAP driveremap;
   extern AREALIST arealiste;
   MSGHEADER Header;
   BOOL FoundIt=FALSE;
   ULONG ulMsgNum;
   FINDRESULTLIST ResultList;
   PFOUNDMSG pFoundMsg=NULL;
   PFOUNDMSG pFoundList=NULL;

   if (!MSG_OpenArea(&arealiste, pAreaDef->areadata.areatag, miscoptions.lastreadoffset, &driveremap))
   {
      /* Area-Tag anzeigen */
      SendMsg(client, WORKM_FINDAREA, pAreaDef->areadata.areatag, NULL);

      memset(&ResultList, 0, sizeof(ResultList));
      strcpy(ResultList.pchAreaTag, pAreaDef->areadata.areatag);
      ResultList.ulFindType = MARKFLAG_PERSMAIL;

      if (pFindJob->PersMailOpt.bAllMsgs)
         ulMsgNum=1;
      else
         ulMsgNum= pAreaDef->currentmessage+1;

      for ( ; ulMsgNum<=pAreaDef->maxmessages && !StopFind; ulMsgNum++)
      {
         FoundIt=FALSE;

         /* Laufbalken ab und zu updaten */
         if (ulMsgNum % 10 == 0)
            SendMsg(client, WORKM_FINDPROGRESS,
                       MPFROMLONG(pAreaDef->maxmessages?(100 * ulMsgNum / pAreaDef->maxmessages):100),
                       NULL);

         if (!MSG_ReadHeader(&Header, &arealiste, pAreaDef->areadata.areatag, ulMsgNum))
         {
            /* evtl. gelesene Messages uebergehen */
            if ((pFindJob->ulHow & FINDHOW_UNREADONLY) && (Header.ulAttrib & ATTRIB_READ))
               continue;

            if (pFindJob->PersMailOpt.bAllNames)
            {
               int i=0;
               while (i < MAX_USERNAMES && userdaten.username[i][0] &&
                      stricmp(Header.pchToName, userdaten.username[i]))
                  i++;
               if (i < MAX_USERNAMES && userdaten.username[i][0])
                  FoundIt = TRUE;
            }
            else
               FoundIt = Header.pchToName[0]?(!stricmp(Header.pchToName, pAreaDef->areadata.username)):FALSE;

            if (FoundIt && hwndFindResults)
            {
               ULONG ulMsgID = MSG_MsgnToUid(&arealiste, pAreaDef->areadata.areatag, ulMsgNum);

               /* In Markerliste aufnehmen */
               if (MarkMessage(&MarkerList, pAreaDef->areadata.areatag, ulMsgID,
                               ulMsgNum, &Header, NULL, MARKFLAG_PERSMAIL, pFindJob->ulHow, pFindJob->ulWhere)==0)
               {
                  /* Ergebnis einketten */
                  if (pFoundMsg)
                  {
                     pFoundMsg->next = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundMsg->next;
                  }
                  else
                  {
                     pFoundList = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundList;
                  }
                  pFoundMsg->Header = Header;
                  pFoundMsg->ulMsgID = ulMsgID;
                  pFoundMsg->ulMsgNum = ulMsgNum;
                  pFoundMsg->ulHow    = pFindJob->ulHow;
                  pFoundMsg->ulWhere  = pFindJob->ulWhere;
                  pFoundMsg->pchFindText= NULL;
                  ResultList.ulFoundMsgs++;
               }
            }
         }
      }
      /* Suchergebnis in Dialog einfuegen */
      if (ResultList.ulFoundMsgs)
         SendMsg(client, WORKM_FINDAREAEND, &ResultList, pFoundList);

      while (pFoundList)
      {
         pFoundMsg = pFoundList;
         pFoundList = pFoundList->next;
         free(pFoundMsg);
      }

      MSG_CloseArea(&arealiste, pAreaDef->areadata.areatag, FALSE, miscoptions.lastreadoffset, &driveremap);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindUnsent                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt die Suche nach ungesendeten Messages in einer Area   */
/*               durch                                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pFindJob: Such-Job-Parameter                                   */
/*            pAreaDef: Zeiger auf Area-Definition                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int FindUnsent(PFINDJOB pFindJob, AREADEFLIST *pAreaDef)
{
   extern MISCOPTIONS miscoptions;
   extern USERDATAOPT userdaten;
   extern DRIVEREMAP driveremap;
   extern AREALIST arealiste;
   MSGHEADER Header;
   ULONG ulMsgNum;
   FINDRESULTLIST ResultList;
   PFOUNDMSG pFoundMsg=NULL;
   PFOUNDMSG pFoundList=NULL;

   if (!MSG_OpenArea(&arealiste, pAreaDef->areadata.areatag, miscoptions.lastreadoffset, &driveremap))
   {
      /* Area-Tag anzeigen */
      SendMsg(client, WORKM_FINDAREA, pAreaDef->areadata.areatag, NULL);

      memset(&ResultList, 0, sizeof(ResultList));
      strcpy(ResultList.pchAreaTag, pAreaDef->areadata.areatag);
      ResultList.ulFindType = MARKFLAG_UNSENT;

      for (ulMsgNum=1 ; ulMsgNum<=pAreaDef->maxmessages && !StopFind; ulMsgNum++)
      {
         /* Laufbalken ab und zu updaten */
         if (ulMsgNum % 10 == 0)
            SendMsg(client, WORKM_FINDPROGRESS,
                       MPFROMLONG(pAreaDef->maxmessages?(100 * ulMsgNum / pAreaDef->maxmessages):100),
                       NULL);

         if (!MSG_ReadHeader(&Header, &arealiste, pAreaDef->areadata.areatag, ulMsgNum))
         {
            /* evtl. gesendete Messages uebergehen */
            if (!(Header.ulAttrib & ATTRIB_LOCAL) ||
                (Header.ulAttrib & (ATTRIB_SENT | ATTRIB_SCANNED)))
               continue;

            if (hwndFindResults)
            {
               ULONG ulMsgID = MSG_MsgnToUid(&arealiste, pAreaDef->areadata.areatag, ulMsgNum);

               /* In Markerliste aufnehmen */
               if (MarkMessage(&MarkerList, pAreaDef->areadata.areatag, ulMsgID,
                               ulMsgNum, &Header, NULL, MARKFLAG_UNSENT, pFindJob->ulHow, pFindJob->ulWhere)==0)
               {
                  /* Ergebnis einketten */
                  if (pFoundMsg)
                  {
                     pFoundMsg->next = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundMsg->next;
                  }
                  else
                  {
                     pFoundList = calloc(1, sizeof(FOUNDMSG));
                     pFoundMsg = pFoundList;
                  }
                  pFoundMsg->Header = Header;
                  pFoundMsg->ulMsgID = ulMsgID;
                  pFoundMsg->ulMsgNum = ulMsgNum;
                  pFoundMsg->ulHow    = pFindJob->ulHow;
                  pFoundMsg->ulWhere  = pFindJob->ulWhere;
                  pFoundMsg->pchFindText= NULL;
                  ResultList.ulFoundMsgs++;
               }
            }
         }
      }
      /* Suchergebnis in Dialog einfuegen */
      if (ResultList.ulFoundMsgs)
         SendMsg(client, WORKM_FINDAREAEND, &ResultList, pFoundList);

      while (pFoundList)
      {
         pFoundMsg = pFoundList;
         pFoundList = pFoundList->next;
         free(pFoundMsg);
      }

      MSG_CloseArea(&arealiste, pAreaDef->areadata.areatag, FALSE, miscoptions.lastreadoffset, &driveremap);
   }

   return 0;
}

static int SensSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob)
{
   pFindJob = pFindJob;

   return (int) strstr(pchHaystack, pchNeedle);
}

static int CaseSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob)
{
   pFindJob = pFindJob;

   return (int) stristr(pchHaystack, pchNeedle);
}

static int FuzzySearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob)
{
   char *pchStart=pchHaystack;
   char *pchEnd=NULL;
   int howclose;

   Fuzz_init(pchNeedle, pchHaystack, pFindJob->ulFuzzyLevel);
   Fuzz_next(&pchStart, &pchEnd, &howclose);
   Fuzz_term();

   if (pchStart)
      return 1;
   else
      return 0;
}

static int RegexSearch(char *pchHaystack, char *pchNeedle, PFINDJOB pFindJob)
{
   pchNeedle = pchNeedle;

   return !regexec(pFindJob->pOptData, pchHaystack, 0, NULL, 0);
}

static int PrepareRegexSearch(PFINDJOB pFindJob)
{
   pFindJob->pOptData = malloc(sizeof(regex_t));
   if (regcomp(pFindJob->pOptData, pFindJob->pchWhat, REG_EXTENDED|REG_NEWLINE|REG_ICASE))
   {
      free(pFindJob->pOptData);
      return FALSE;
   }
   return TRUE;
}

static int TerminateRegexSearch(PFINDJOB pFindJob)
{
   regfree(pFindJob->pOptData);
   free(pFindJob->pOptData);
   return TRUE;
}

/*-------------------------------- Modulende --------------------------------*/

