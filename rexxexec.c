/*---------------------------------------------------------------------------+
 | Titel: REXXEXEC.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 10.08.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Rexx-Ausfuehrung                                                        |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#define INCL_PM
#include <os2.h>
#define INCL_REXXSAA
#include <rexxsaa.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "rexxexec.h"
#include "mainwindow.h"
#include "savemsg.h"
#include "utility.h"
#include "fltv7\fltv7.h"
#include "lookups.h"
#include "ccmanage.h"
#include "controls\editwin.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "util\addrcnv.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

#define ERRORMSGFILE    "rex.msg"
#define ERROREXPLFILE   "rexh.msg"
#define REXXENVNAME     "FLEETSTREET"

#define STDIN   0
#define STDOUT  1
#define STDERR  2

/*---------------------------------- Typen ----------------------------------*/

typedef struct _STRINGLIST {
            struct _STRINGLIST *next;
            PCHAR pchLine;
         } STRINGLIST, *PSTRINGLIST;

typedef struct {
            HMTX        hmtxQueueAccess;
            HEV         hevQueueAdd;
            PSTRINGLIST pLines;
         } IOQUEUE, *PIOQUEUE;

typedef struct {
            ULONG   ulStatus;
            BOOL    bNotify;
         } MONITORDATA, *PMONITORDATA;

#define REXXSTATUS_WAITFORSTART   0
#define REXXSTATUS_RUNNING        1
#define REXXSTATUS_ENDED          2

typedef struct {
            USHORT cb;
            PCHAR pchErrorText;
            PCHAR pchErrorTitle;
         } REXXERRORPAR, *PREXXERRORPAR;

typedef struct {
            PCHAR               pchFuncName;
            RexxFunctionHandler *pFuncAddr;
         } FUNCTIONTABLE;

/*---------------------------- Globale Variablen ----------------------------*/

extern SCRIPTLIST scriptlist;
extern HWND client, hwndhelp;
extern HMODULE hmodLang;
extern HAB anchor;

extern HWND hwndMonitor;
extern PRXSCRIPT pExecScript;
extern int tidRexxExec;

static HFILE   stdin_r;
static HFILE   stdin_w;
static HFILE   stdout_r;
static HFILE   stdout_w;
static HFILE   stderr_w;
static IOQUEUE stdinqueue;

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY ScriptMonitorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void _Optlink stdinpipe(PVOID pData);
static void _Optlink stdoutpipe(PVOID pData);
static void makepipe(PHFILE read, HFILE rspot, PHFILE write, HFILE wspot, ULONG psize);
static void setinherit(HFILE handle, BOOL inh);
static MRESULT EXPENTRY RexxErrorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
INT _System RexxInitExit(LONG lExitNumber, LONG lSubfunction, PEXIT pParams);
INT _System RexxTermExit(LONG lExitNumber, LONG lSubfunction, PEXIT pParams);
void _Optlink CallRexx(PVOID pParam);
static char *FormatRexxError(char *pchText);
static char *FormatRexxTitle(char *pchText);
static void RexxVarUserNames(void);
static void RexxVarUserAddresses(void);
static void RexxVarCC(PCCLIST pList);
static void SetRexxVar(const char *pchName, const char *pchValue);
static int QueryRexxVar(const char *pchName, char *pchBuffer, ULONG ulBufLen);
static void RexxVarMessageHeader(MSGHEADER *pHeader);
static void RexxVarMessageText(FTNMESSAGE *pMessage, ULONG ulCursor);
static void RexxVarSeenbys(FTNMESSAGE *pMessage);
static void RexxVarKludges(FTNMESSAGE *pMessage);
static void RegisterFunctions(void);
static void DeregisterFunctions(void);

static ULONG _System FSSetText(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSLookupAddress(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSLookupName(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSSetEntryField(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSCls(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSSetHeader(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSEncodeLine(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);
static ULONG _System FSDecodeLine(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr);

static const FUNCTIONTABLE FunctionTable[] =
{
 {"FSSetText",       &FSSetText},
 {"FSLookupAddress", &FSLookupAddress},
 {"FSLookupName",    &FSLookupName},
 {"FSSetEntryField", &FSSetEntryField},
 {"FSCls",           &FSCls},
 {"FSSetHeader",     &FSSetHeader},
 {"FSDecodeLine",    &FSDecodeLine},
 {"FSEncodeLine",    &FSEncodeLine},
 {NULL, NULL}
};

/*---------------------------------------------------------------------------*/
/* Funktionsname: StartRexxScript                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet mit der Ausfuehrung des Scripts                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ulScriptID: Script-ID                                          */
/*            phwndMonitor: Empfangspuffer f. Monitor-Window-Handle          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 Script gestartet                                         */
/*                1 Fehler, Script nicht gestartet                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:-                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int StartRexxScript(ULONG ulScriptID, PHWND phwndMonitor)
{
   PRXSCRIPT pScript=scriptlist.pScripts;
   HWND hwndNewMonitor = NULLHANDLE;

   if (tidRexxExec)
   {
      return 1;
   }

   while (pScript && pScript->ulScriptID != ulScriptID)
      pScript = pScript->next;

   if (!pScript)
   {
      return 1;   /* unbekannte ID */
   }

   if (!pScript->pchPathName[0])
   {
      return 1;   /* kein Pfadname */
   }

   pExecScript = pScript;

   if (!(pScript->ulFlags & REXXFLAG_NOMONITOR))
   {
#if 1
      /* Monitor-Fenster erzeugen */
      hwndNewMonitor = WinLoadDlg(HWND_DESKTOP, client, ScriptMonitorProc,
                                  hmodLang, IDD_RXMONITOR, NULL);

      if (!hwndNewMonitor)
      {
         return 1;
      }
#endif

      if (phwndMonitor)
         *phwndMonitor = hwndNewMonitor;

   }

   hwndMonitor = hwndNewMonitor;

   SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
   tidRexxExec = _beginthread(CallRexx, NULL, 100000, pScript);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterFunctions                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Registriert alle Funktionen der Funktionstabelle            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:-                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void RegisterFunctions(void)
{
   int i=0;

   while (FunctionTable[i].pchFuncName)
   {
      RexxRegisterFunctionExe(FunctionTable[i].pchFuncName,
                              (PFN)FunctionTable[i].pFuncAddr);
      i++;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeregisterFunctions                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: De-Registriert alle Funktionen der Funktionstabelle         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:-                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DeregisterFunctions(void)
{
   int i=0;

   while (FunctionTable[i].pchFuncName)
   {
      RexxDeregisterFunction(FunctionTable[i].pchFuncName);
      i++;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScriptMonitorProc                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure des Script-Monitors                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:-                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ScriptMonitorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PMONITORDATA pMonitorData = (PMONITORDATA) WinQueryWindowULong(hwnd, QWL_USER);
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;

   switch(msg)
   {
      case WM_INITDLG:
         /* Instanzdaten */
         pMonitorData = calloc(1, sizeof(MONITORDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pMonitorData);

         /* Eingabelaenge */
         WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(500), NULL);

         WinSubclassWindow(WinWindowFromID(hwnd, IDD_RXMONITOR+2), FileEntryProc);

         SetBackground(WinWindowFromID(hwnd, IDD_RXMONITOR+1), &windowcolors.monitorback);
         SetForeground(WinWindowFromID(hwnd, IDD_RXMONITOR+1), &windowcolors.monitorfore);
         SetFont(WinWindowFromID(hwnd, IDD_RXMONITOR+1), windowfonts.monitorfont);

         RestoreWinPos(hwnd, &pExecScript->MonitorPos, FALSE, TRUE);
         pMonitorData->bNotify = TRUE;
         break;

      case WM_CLOSE:
         if (pMonitorData->ulStatus != REXXSTATUS_ENDED)
            return (MRESULT) FALSE;
         else
         {
            WinPostMsg(client, REXXM_CLOSE, (MPARAM) hwnd, NULL);
            break;
         }

      case WM_DESTROY:
         if (!WinRequestMutexSem(stdinqueue.hmtxQueueAccess, SEM_INDEFINITE_WAIT))
         {
            if (stdinqueue.pLines)
            {
               /* Queue aufraeumen */

               PSTRINGLIST pString = stdinqueue.pLines;
               PSTRINGLIST pNextString;

               while(pString)
               {
                  pNextString = pString->next;
                  free(pString->pchLine);
                  free(pString);
                  pString = pNextString;
               }
            }

            DosReleaseMutexSem(stdinqueue.hmtxQueueAccess);
         }
         QueryBackground(WinWindowFromID(hwnd, IDD_RXMONITOR+1), &windowcolors.monitorback);
         QueryForeground(WinWindowFromID(hwnd, IDD_RXMONITOR+1), &windowcolors.monitorfore);
         QueryFont(WinWindowFromID(hwnd, IDD_RXMONITOR+1), windowfonts.monitorfont);
         free(pMonitorData);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pMonitorData && pMonitorData->bNotify)
         {
            if (SaveWinPos(hwnd, (PSWP) mp1, &pExecScript->MonitorPos, &pExecScript->bDirty))
               scriptlist.bDirty=TRUE;
         }
         break;

      case REXXM_STARTSCRIPT:
         pMonitorData->ulStatus = REXXSTATUS_RUNNING;
         WinEnableControl(hwnd, IDD_RXMONITOR+2, TRUE);
         WinEnableControl(hwnd, IDD_RXMONITOR+4, TRUE);
         SetFocusControl(hwnd, IDD_RXMONITOR+2);
         return (MRESULT) NULL;

      case REXXM_STOPSCRIPT:
         pMonitorData->ulStatus = REXXSTATUS_ENDED;
         WinEnableControl(hwnd, IDD_RXMONITOR+2, FALSE);
         WinEnableControl(hwnd, IDD_RXMONITOR+4, FALSE);
         WinEnableControl(hwnd, IDD_RXMONITOR+3, TRUE);
         if (pExecScript->ulFlags & REXXFLAG_AUTOCLOSE)
            WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
         return (MRESULT) NULL;

      case REXXM_CLS:
         WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+1, LM_DELETEALL, NULL, NULL);
         return (MRESULT) NULL;

      case REXXM_ERROR:
         DisplayRexxError(hwnd, (PCHAR) mp1, (PCHAR) mp2);
         pMonitorData->ulStatus = REXXSTATUS_ENDED;
         WinEnableControl(hwnd, IDD_RXMONITOR+2, FALSE);
         WinEnableControl(hwnd, IDD_RXMONITOR+4, FALSE);
         WinEnableControl(hwnd, IDD_RXMONITOR+3, TRUE);
         return (MRESULT) NULL;

      case REXXM_OUTLINE:  /* mp1 ist PSTRINGLIST */
         if (mp1)
         {
            SHORT sTop;
            ULONG ulNumLines=0;
            PSTRINGLIST pHelp=(PSTRINGLIST) mp1, pHelp2;

            while(pHelp)
            {
               while (WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+1, LM_INSERTITEM,
                                        MPFROMSHORT(LIT_END), pHelp->pchLine) == (MRESULT) LIT_MEMERROR)
               {
                  /* zu wenig Speicher, erstes wieder loeschen */
                  WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+1, LM_DELETEITEM,
                                    NULL, NULL);
               }
               ulNumLines++;
               free(pHelp->pchLine);

               pHelp2 = pHelp;
               pHelp = pHelp->next;
               free(pHelp2);
            }

            if (ulNumLines)
            {
               /* ulNumLines nach unten scrollen */
               sTop = (SHORT) WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+1, LM_QUERYTOPINDEX,
                                                NULL, NULL);
               WinSendDlgItemMsg(hwnd, IDD_RXMONITOR+1, LM_SETTOPINDEX,
                                 MPFROMSHORT(sTop+ulNumLines), NULL);
            }
         }
         return (MRESULT) NULL;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            PID pid;
            TID tid;

            case DID_OK:
               WinAlarm(HWND_DESKTOP, WA_NOTE);
               return (MRESULT) FALSE;

            case DID_CANCEL:
               return (MRESULT) FALSE;

            case IDD_RXMONITOR+3:  /* Close-Button */
               WinPostMsg(client, REXXM_CLOSE, (MPARAM) hwnd, NULL);
               break;

            case IDD_RXMONITOR+4:  /* Stop-Button */
               WinQueryWindowProcess(hwnd, &pid, &tid);
               RexxSetHalt(pid, tidRexxExec);
               WinEnableControl(hwnd, IDD_RXMONITOR+4, FALSE);
               return (MRESULT) FALSE;

            default:
               return (MRESULT) FALSE;
         }
         break;

      case WM_CHAR:
         if (!(SHORT1FROMMP(mp1) & KC_KEYUP) &&
             (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             (SHORT2FROMMP(mp2) == VK_NEWLINE ||
              SHORT2FROMMP(mp2) == VK_ENTER) &&
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(hwnd, IDD_RXMONITOR+2) &&
             pMonitorData->ulStatus == REXXSTATUS_RUNNING)
         {
            /* Text aus Zeile holen */
            PCHAR pchText=NULL;
            LONG lLen;
            PSTRINGLIST pListItem;

            lLen= WinQueryDlgItemTextLength(hwnd, IDD_RXMONITOR+2)+1;
            pchText=malloc(lLen);
            *pchText=0;
            WinQueryDlgItemText(hwnd, IDD_RXMONITOR+2, lLen, pchText);
            WinSetDlgItemText(hwnd, IDD_RXMONITOR+2, "");

            /* Neues Item erzeugen */
            pListItem = malloc(sizeof(STRINGLIST));
            pListItem->next = NULL;
            pListItem->pchLine = pchText;

            /* Item in Queue einhaengen */
            if (!WinRequestMutexSem(stdinqueue.hmtxQueueAccess, SEM_INDEFINITE_WAIT))
            {
               if (!stdinqueue.pLines)
               {
                  stdinqueue.pLines = pListItem;
               }
               else
               {
                  PSTRINGLIST pHelp=stdinqueue.pLines;

                  while(pHelp->next)
                     pHelp = pHelp->next;

                  pHelp->next = pListItem;
               }

               DosPostEventSem(stdinqueue.hevQueueAdd);
               DosReleaseMutexSem(stdinqueue.hmtxQueueAccess);
            }
            return (MRESULT) TRUE;
         }
         else
            break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

void DisplayRexxError(HWND hwndOwner, char *pchText, char *pchTitle)
{
   if (pchText)
   {
      REXXERRORPAR ErrorPar;

      ErrorPar.cb = sizeof(ErrorPar);
      ErrorPar.pchErrorText = pchText;
      ErrorPar.pchErrorTitle = pchTitle;

      WinDlgBox(HWND_DESKTOP, hwndOwner, RexxErrorProc, hmodLang,
                IDD_REXXERROR, &ErrorPar);
   }

   if (pchText)
      free(pchText);
   if (pchTitle)
      free(pchTitle);

   return;
}

int CreatePipes(void)
{
   LONG         rc;                   /* return code                         */

   DosClose(STDIN);                   /* close stdin, stdout, stderr         */
   DosClose(STDOUT);
   DosClose(STDERR);

   DosCreateMutexSem(NULL, &stdinqueue.hmtxQueueAccess, 0, FALSE);
   DosCreateEventSem(NULL, &stdinqueue.hevQueueAdd, 0, FALSE);

   /* Since we closed all open file handles, we are free to use any *
    * handles we want.                                                        */
   /* First, standard input                                                   */

   makepipe(&stdin_r, STDIN, &stdin_w, 4, 16384);
   setinherit(stdin_r, TRUE);
   setinherit(stdin_w, FALSE);

   /* Next, standard output                                                   */

   makepipe(&stdout_r, 5, &stdout_w, STDOUT, 16384);
   setinherit(stdout_w, TRUE);
   setinherit(stdout_r, FALSE);

   /* And, finally, standard error                                            */
   /* Just dup the standard output and handle it once.                        */
   stderr_w = STDERR;

   if (rc = DosDupHandle(stdout_w, &stderr_w))
   {
      return  rc;
   }

   /* Pipe-Threads starten */

   _beginthread(stdoutpipe, NULL, 16384, NULL);
   _beginthread(stdinpipe, NULL, 16384, NULL);

   return rc;
}

static void _Optlink stdinpipe(PVOID pData)
{
   ULONG written;
   ULONG ulCount;
   PSTRINGLIST pLine;

   INSTALLEXPT("stdin");

   pData = pData;

   while (TRUE)
   {
      /* Auf Eingabe in Queue warten */
      DosResetEventSem(stdinqueue.hevQueueAdd, &ulCount);
      if (DosWaitEventSem(stdinqueue.hevQueueAdd, SEM_INDEFINITE_WAIT))
         break;

      /* Queue belegen */
      if (!DosRequestMutexSem(stdinqueue.hmtxQueueAccess, SEM_INDEFINITE_WAIT))
      {
         while (stdinqueue.pLines)
         {
            pLine = stdinqueue.pLines;
            DosWrite(stdin_w, pLine->pchLine, strlen(pLine->pchLine), &written);
            DosWrite(stdin_w, "\r\n", 2, &written);
            free(pLine->pchLine);
            stdinqueue.pLines = pLine->next;
            free(pLine);
         }

         DosReleaseMutexSem(stdinqueue.hmtxQueueAccess);
      }
      else
         break;
   }

   DEINSTALLEXPT;

   return;
}

static void _Optlink stdoutpipe(PVOID pData)
{
   UCHAR pchLineBuffer[200]="";
   UCHAR pchReadBuffer[200]="";
   int LinePos=0;
   PSTRINGLIST pNewList=NULL;
   PSTRINGLIST pLastList=NULL;
   ULONG nread;
   int i;

   INSTALLEXPT("stdout");

   pData = pData;

   while (TRUE)
   {
      if (pNewList) /* Liste noch posten */
         if (hwndMonitor)
            while(!WinPostMsg(hwndMonitor, REXXM_OUTLINE, pNewList, NULL))
               DosSleep(1L);
         else
         {
            /* Liste wieder freigeben */
            while (pNewList)
            {
               free(pNewList->pchLine);
               pLastList = pNewList->next;
               free(pNewList);
               pNewList = pLastList;
            }
         }
      pNewList=NULL;
      nread=0;
      while (nread==0) /* lesen */
         DosRead(stdout_r, pchReadBuffer, sizeof(pchReadBuffer), &nread);

      for (i=0; i<nread; i++) /* gelesenes abklappern */
      {
         switch(pchReadBuffer[i])
         {
            case '\n':     /* Zeilenende, neuen Listeneintrag */
               pchLineBuffer[LinePos]=0;
               LinePos=0;
               if (pNewList)
               {
                  pLastList->next = malloc(sizeof(STRINGLIST));
                  pLastList = pLastList->next;
               }
               else
               {
                  pNewList = malloc(sizeof(STRINGLIST));
                  pLastList = pNewList;
               }
               pLastList->next=NULL;
               pLastList->pchLine = strdup(pchLineBuffer);
               break;

            case '\r': /* ignorieren */
               break;

            default:  /* kopieren */
               pchLineBuffer[LinePos++]=pchReadBuffer[i];
               if (LinePos == 199)
               {
                  /* Pufferueberlauf */
                  pchLineBuffer[199]=0;
                  LinePos=0;

                  if (pNewList)
                  {
                     pLastList->next = malloc(sizeof(STRINGLIST));
                     pLastList = pLastList->next;
                  }
                  else
                  {
                     pNewList = malloc(sizeof(STRINGLIST));
                     pLastList = pNewList;
                  }
                  pLastList->next=NULL;
                  pLastList->pchLine = strdup(pchLineBuffer);
               }
               break;
         }
      }
   }

   DEINSTALLEXPT;
}

static void makepipe(PHFILE read, HFILE rspot, PHFILE write, HFILE wspot, ULONG psize)
{
   HFILE rh,wh;                         /* read and write handles for pipe     */
   HFILE newh = 0xFFFF;                 /* we want to get a new handle         */

   /* Create the pipe                                                          */
   DosCreatePipe(&rh, &wh, psize);

   if (rh != rspot)
   {

      if (wh == rspot)
      {
         if (rh != wspot)
            newh = wspot;

         DosDupHandle(wh, &newh);
         wh = newh;
      }

      /* Dup requested read handle to the read handle we got                   */
      DosDupHandle(rh, &rspot);

      /* Close the original read handle we received                            */
      DosClose(rh);
      rh = rspot;
   }

   if (wh != wspot)
   {
      DosDupHandle(wh, &wspot);

      /* Close original write handle we received                               */
      DosClose(wh);
      wh = wspot;
   }

   *read = rh;
   *write = wh;

   return;
}

static void setinherit(HFILE handle, BOOL inh)
{
   ULONG  state;                        /* Variable to change current state of */
                                        /* the file handle                     */

   DosQueryFHState(handle, &state);

   if (inh)
      state &= ~OPEN_FLAGS_NOINHERIT;
   else
      state |= OPEN_FLAGS_NOINHERIT;

   DosSetFHState(handle, state);

   return;
}


static void FlushPipes(void)
{
   DosResetBuffer(stdin_r);
   DosResetBuffer(stdin_w);
   DosResetBuffer(stdout_r);
   DosResetBuffer(stdout_w);
   DosResetBuffer(stderr_w);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RexxErrorProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur f. Rexx-Fehlermeldungen                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY RexxErrorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_INITDLG:
         if (mp2)
         {
            PREXXERRORPAR pErrorPar = (PREXXERRORPAR) mp2;
            WinSetDlgItemText(hwnd, IDD_REXXERROR+3, pErrorPar->pchErrorText);
            if (pErrorPar->pchErrorTitle)
               WinSetWindowText(hwnd, pErrorPar->pchErrorTitle);
         }
         WinAlarm(HWND_DESKTOP, WA_ERROR);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetRexxErrorMsg                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Fehlermeldung   aus dem Message-File               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pMsg: Zeiger auf den Meldungspuffer                            */
/*            ulMsgNr: Message-Nummer                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int GetRexxErrorMsg(PCHAR pMsg, PCHAR pchTitle, ULONG ulMsgNr)
{
   ULONG retlen=0;

   memset(pMsg, 0, 1000);
   memset(pchTitle, 0, 100);

   switch (DosGetMessage(NULL, 0, pMsg, 1000, ulMsgNr,
           ERROREXPLFILE, &retlen))
   {
      case NO_ERROR:
         FormatRexxError(pMsg);
         break;

      case ERROR_FILE_NOT_FOUND:
      case ERROR_FILENAME_EXCED_RANGE:
      case ERROR_MR_UN_ACC_MSGF:
      case ERROR_MR_INV_MSGF_FORMAT:
      case ERROR_MR_INV_IVCOUNT:
      case ERROR_MR_UN_PERFORM:
         break;

      case ERROR_MR_MID_NOT_FOUND:
         sprintf(pMsg, "Error #%d", ulMsgNr);
         break;

      default:
         break;
   }

   switch (DosGetMessage(NULL, 0, pchTitle, 100, ulMsgNr,
           ERRORMSGFILE, &retlen))
   {
      case NO_ERROR:
         FormatRexxTitle(pchTitle);
         break;

      case ERROR_FILE_NOT_FOUND:
      case ERROR_FILENAME_EXCED_RANGE:
      case ERROR_MR_UN_ACC_MSGF:
      case ERROR_MR_INV_MSGF_FORMAT:
      case ERROR_MR_INV_IVCOUNT:
      case ERROR_MR_UN_PERFORM:
         break;

      case ERROR_MR_MID_NOT_FOUND:
         break;

      default:
         break;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FormatRexxError                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Formatiert die Fehlermeldung um                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pMsg: Zeiger auf den Meldungspuffer                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf Meldung                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char *FormatRexxError(char *pchText)
{
   char *pchSrc = pchText;
   char *pchDest = pchText;

   while (*pchSrc)
   {
      switch(*pchSrc)
      {
         case '\r':
            pchSrc++;
            break;

         case '\n':
            pchSrc++;
            if (*pchSrc != ' ')
            {
               *pchDest++ = '\n';
               *pchDest++ = *pchSrc;
               if (*pchSrc)
                  pchSrc++;
            }
            else
            {
               *pchDest++ = ' ';
               while (*pchSrc == ' ')
                  pchSrc++;
            }
            break;

         default:
            *pchDest++ = *pchSrc++;
            break;
      }
   }
   *pchDest=0;
   return pchText;
}

static char *FormatRexxTitle(char *pchText)
{
   char *pchSrc = pchText;
   char *pchDest = pchText;

#if 0
   while (*pchSrc && *pchSrc != '%')
      pchSrc++;

   if (*pchSrc)
   {
      pchSrc++;
      if (*pchSrc)
      {
         pchSrc++;
         while(*pchSrc && *pchSrc != '%')
            *pchDest++ = *pchSrc++;
         *pchDest = 0;
      }
   }
#else
   while(*pchSrc)
   {
      if (*pchSrc == '%')
      {
         pchSrc++;
         pchSrc++;
      }
      else
         *pchDest++ = *pchSrc++;
   }
   *pchDest = 0;
#endif
   return pchText;
}

#if 0
/*---------------------------------------------------------------------------*/
/* Funktionsname: PszToRxstring                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wandelt einen C-String in einen Rexx-String um              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pRxString: Zeiger auf den Rexx-String                          */
/*            pchString: Zeiger auf den C-String                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf den erzeugten Rexx-String                       */
/*                NULL bei Fehler                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Speicher fuer den Rexx-String wird mit DosAllocMem belegt      */
/*            falls notwendig                                                */
/*---------------------------------------------------------------------------*/

PRXSTRING PszToRxstring(PRXSTRING pRxString, PCHAR pchString)
{
   int len;

   if (!pchString)
      return NULL;

   len=strlen(pchString);

   if ((!pRxString->strptr) || (pRxString->strlength < len))
   {
      if (pRxString->strptr)
         DosFreeMem(pRxString->strptr);

      if (DosAllocMem((PVOID)&pRxString->strptr, len,
                   PAG_COMMIT | PAG_READ | PAG_WRITE))
         return NULL;
   }
   pRxString->strlength=len;
   strncpy(pRxString->strptr, pchString, len);

   return pRxString;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RxstringToPsz                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wandelt einen Rexx-String in einen 0-terminierten           */
/*               C-String um                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pRxString: Zeiger auf den Rexx-String                          */
/*            ppchString: Zeiger auf den erzeugten String                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf den erzeugten String                            */
/*                NULL bei Fehler                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Speicher fuer den erzeugten String wird mit malloc belegt      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PCHAR RxstringToPsz(PRXSTRING pRxString, PCHAR *ppchString)
{
   if (!pRxString->strptr)
      return NULL;
   if (!(*ppchString=malloc(pRxString->strlength+1)))
      return NULL;

   strncpy(*ppchString, pRxString->strptr, pRxString->strlength);
   *ppchString[pRxString->strlength]='\0';

   return *ppchString;
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: RexxInitExit                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Exit-Handler fuer INIT                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: lExitNumber: Exit-Nummer                                       */
/*            lSubfunction: Sub-Funktion                                     */
/*            pParams:      Exit-Parameter                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: RXEXIT_HANDLED: Exit behandelt                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

INT _System RexxInitExit(LONG lExitNumber, LONG lSubfunction, PEXIT pParams)
{
   extern ECHOTOSSOPT echotossoptions;
   extern PATHNAMES pathnames;
   extern char CurrentAddress[LEN_5DADDRESS+1];
   extern char CurrentName[LEN_USERNAME+1];
   extern char NewArea[LEN_AREATAG+1];
   extern char CurrentArea[LEN_AREATAG+1];
   extern int  CurrentStatus;
   extern MSGHEADER CurrentHeader;
   extern FTNMESSAGE CurrentMessage;
   extern AREALIST arealiste;
   FTNMESSAGE Message;
   MSGHEADER Header;
   AREADEFLIST *pAreaDef;
   char *pchTemp;

   pParams = pParams;
   if (lExitNumber == RXINI &&
       lSubfunction == RXINIEXT)
   {
      RegisterFunctions();
      RexxVarUserNames();
      RexxVarUserAddresses();
      SetRexxVar("FleetSetup.Echotoss", echotossoptions.pchEchoToss);
      SetRexxVar("FleetSetup.Tosser", pathnames.squishcfg);
      SetRexxVar("FleetStatus.Area.Tag", CurrentArea);
      if (CurrentStatus == PROGSTATUS_EDITING)
         SetRexxVar("FleetStatus.DestArea", NewArea);
      pAreaDef = AM_FindArea(&arealiste, CurrentArea);
      if (pAreaDef)
      {
         SetRexxVar("FleetStatus.Area.Desc", pAreaDef->areadata.areadesc);
         SetRexxVar("FleetStatus.Area.File", pAreaDef->areadata.pathfile);
         switch(pAreaDef->areadata.areaformat)
         {
            case AREAFORMAT_FTS:
               pchTemp = "*.MSG";
               break;
            case AREAFORMAT_SQUISH:
               pchTemp = "Squish";
               break;
            case AREAFORMAT_JAM:
               pchTemp = "JAM";
               break;
            default:
               pchTemp = "Unknown";
               break;
         }
         SetRexxVar("FleetStatus.Area.Format", pchTemp);

         switch(pAreaDef->areadata.areatype)
         {
            case AREATYPE_ECHO:
               pchTemp = "Echo";
               break;

            case AREATYPE_NET:
               pchTemp = "Net";
               break;

            case AREATYPE_LOCAL:
               if (pAreaDef->areadata.ulAreaOpt & AREAOPT_FROMCFG)
                  pchTemp = "Local";
               else
                  pchTemp = "Private";
               break;
         }
         SetRexxVar("FleetStatus.Area.Type", pchTemp);
      }
      SetRexxVar("FleetStatus.Name", CurrentName);
      SetRexxVar("FleetStatus.Address", CurrentAddress);
      switch(CurrentStatus)
      {
         char pchTemp[50];
         extern char *pchXPostList;
         extern ULONG ulCCSelected;
         extern PCCLIST pQuickCCList;
         IPT iptCursor;

         case PROGSTATUS_NOSETUP:
            SetRexxVar("FleetStatus.Mode", "No Setup");
            RexxVarMessageHeader(&CurrentHeader);
            RexxVarMessageText(&CurrentMessage, 0);
            break;

         case PROGSTATUS_EDITING:
            strcpy(pchTemp, "Edit ");
            if (pchXPostList)
               strcat(pchTemp, "XPost");
            else
               if (ulCCSelected || pQuickCCList)
               {
                  strcat(pchTemp, "CCopy");

                  if (ulCCSelected)
                  {
                     PCCLIST pList;
                     extern CCANCHOR ccanchor;

                     pList = QueryCCList(&ccanchor, ulCCSelected);
                     if (pList)
                        RexxVarCC(pList);
                  }
                  else
                     RexxVarCC(pQuickCCList);
               }
               else
                  strcat(pchTemp, "Single");
            SetRexxVar("FleetStatus.Mode", pchTemp);
            memset(&Message, 0, sizeof(Message));
            memcpy(&Header, &CurrentHeader, sizeof(Header));

            GetMsgContents(client, &Message, &Header,
                           AM_FindArea(&arealiste, NewArea)->areadata.areatype != AREATYPE_NET);
            RexxVarMessageHeader(&Header);
            iptCursor=(IPT)WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_QUERYSEL,
                                             MPFROMSHORT(MLFQS_CURSORSEL), NULL);
            RexxVarMessageText(&Message, iptCursor);
            MSG_ClearMessage(&Header, &Message);
            break;

         case PROGSTATUS_READING:
            SetRexxVar("FleetStatus.Mode", "Read");
            RexxVarMessageHeader(&CurrentHeader);
            RexxVarMessageText(&CurrentMessage, 0);
            RexxVarKludges(&CurrentMessage);
            RexxVarSeenbys(&CurrentMessage);
            break;

         case PROGSTATUS_CLEANUP:
            SetRexxVar("FleetStatus.Mode", "Cleanup");
            RexxVarMessageHeader(&CurrentHeader);
            RexxVarMessageText(&CurrentMessage, 0);
            break;

         default:
            SetRexxVar("FleetStatus.Mode", "Unknown");
            break;
      }

      if (CurrentStatus == PROGSTATUS_CLEANUP)
      {
         extern BOOL MailEntered[3];
         char pchTemp[20]="";

         if (MailEntered[AREATYPE_NET])
            strcpy(pchTemp, "Net");
         if (MailEntered[AREATYPE_ECHO])
            if (pchTemp[0])
               strcat(pchTemp, " Echo");
            else
               strcat(pchTemp, "Echo");
         if (MailEntered[AREATYPE_LOCAL])
            if (pchTemp[0])
               strcat(pchTemp, " Local");
            else
               strcat(pchTemp, "Local");

         SetRexxVar("NewMail", pchTemp);
      }

      if (hwndMonitor)
         SetRexxVar("FleetStatus.Monitor", "1");
      else
         SetRexxVar("FleetStatus.Monitor", "0");

      SendMsg(client, REXXM_STARTSCRIPT, NULL, NULL);
   }

   return RXEXIT_HANDLED;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RexxTermExit                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Exit-Handler fuer TERM                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: lExitNumber: Exit-Nummer                                       */
/*            lSubfunction: Sub-Funktion                                     */
/*            pParams:      Exit-Parameter                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: RXEXIT_HANDLED: Exit behandelt                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

INT _System RexxTermExit(LONG lExitNumber, LONG lSubfunction, PEXIT pParams)
{
   pParams = pParams;
   if (lExitNumber == RXTER &&
       lSubfunction == RXTEREXT)
   {
      DeregisterFunctions();
   }
   return RXEXIT_HANDLED;
}

static void RexxVarCC(PCCLIST pList)
{
   PCCENTRY pEntry=pList->pEntries;
   char pchTemp[30];
   ULONG ulCount=0;

   while (pEntry)
   {
      ulCount++;
      sprintf(pchTemp, "FleetCCopy.%d.Name", ulCount);
      SetRexxVar(pchTemp, pEntry->pchName);
      sprintf(pchTemp, "FleetCCopy.%d.Address", ulCount);
      SetRexxVar(pchTemp, pEntry->pchAddress);

      pEntry=pEntry->next;
   }

   /* Anzahl */
   sprintf(pchTemp, "%d", ulCount);
   SetRexxVar("FleetCCopy.0", pchTemp);

   return;
}

static void RexxVarUserNames(void)
{
   extern USERDATAOPT userdaten;

   int i;
   PSHVBLOCK pVarBlock;
   PSHVBLOCK pHelp;
   char pchTemp[50];

   pVarBlock=calloc(MAX_USERNAMES+1, sizeof(SHVBLOCK));

   /* User-Namen */
   i=0;
   while (i < MAX_USERNAMES && userdaten.username[i][0])
   {
      pVarBlock[i].shvnext = &(pVarBlock[i+1]);
      sprintf(pchTemp, "FleetSetup.Names.%d", i+1);
      pVarBlock[i].shvname.strptr=strdup(pchTemp);
      pVarBlock[i].shvname.strlength=strlen(pchTemp);
      pVarBlock[i].shvnamelen = pVarBlock[i].shvname.strlength;

      pVarBlock[i].shvvalue.strptr = userdaten.username[i];
      pVarBlock[i].shvvalue.strlength = strlen(userdaten.username[i]);
      pVarBlock[i].shvvaluelen = pVarBlock[i].shvvalue.strlength;

      pVarBlock[i].shvcode = RXSHV_SYSET;

      i++;
   }
   /* 0-Record */
   pVarBlock[i].shvnext = NULL;
   sprintf(pchTemp, "FleetSetup.Names.%d", 0);
   pVarBlock[i].shvname.strptr=strdup(pchTemp);
   pVarBlock[i].shvname.strlength=strlen(pchTemp);
   pVarBlock[i].shvnamelen = pVarBlock[i].shvname.strlength;

   sprintf(pchTemp, "%d", i);
   pVarBlock[i].shvvalue.strptr = pchTemp;
   pVarBlock[i].shvvalue.strlength = strlen(pchTemp);
   pVarBlock[i].shvvaluelen = pVarBlock[i].shvvalue.strlength;

   pVarBlock[i].shvcode = RXSHV_SYSET;

   /* Variablen setzen */
   RexxVariablePool(pVarBlock);

   /* aufrеmen */
   pHelp = pVarBlock;
   while (pHelp)
   {
      free(pHelp->shvname.strptr);
      pHelp = pHelp->shvnext;
   }
   free(pVarBlock);

   return;
}

static void RexxVarUserAddresses(void)
{
   extern USERDATAOPT userdaten;

   int i;
   PSHVBLOCK pVarBlock;
   PSHVBLOCK pHelp;
   char pchTemp[50];

   pVarBlock=calloc(MAX_ADDRESSES+1, sizeof(SHVBLOCK));

   /* User-Adressen */
   i=0;
   while (i < MAX_ADDRESSES && userdaten.address[i][0])
   {
      pVarBlock[i].shvnext = &(pVarBlock[i+1]);
      sprintf(pchTemp, "FleetSetup.Addresses.%d", i+1);
      pVarBlock[i].shvname.strptr=strdup(pchTemp);
      pVarBlock[i].shvname.strlength=strlen(pchTemp);
      pVarBlock[i].shvnamelen = pVarBlock[i].shvname.strlength;

      pVarBlock[i].shvvalue.strptr = userdaten.address[i];
      pVarBlock[i].shvvalue.strlength = strlen(userdaten.address[i]);
      pVarBlock[i].shvvaluelen = pVarBlock[i].shvvalue.strlength;

      pVarBlock[i].shvcode = RXSHV_SYSET;

      i++;
   }
   /* 0-Record */
   pVarBlock[i].shvnext = NULL;
   sprintf(pchTemp, "FleetSetup.Addresses.%d", 0);
   pVarBlock[i].shvname.strptr=strdup(pchTemp);
   pVarBlock[i].shvname.strlength=strlen(pchTemp);
   pVarBlock[i].shvnamelen = pVarBlock[i].shvname.strlength;

   sprintf(pchTemp, "%d", i);
   pVarBlock[i].shvvalue.strptr = pchTemp;
   pVarBlock[i].shvvalue.strlength = strlen(pchTemp);
   pVarBlock[i].shvvaluelen = pVarBlock[i].shvvalue.strlength;

   pVarBlock[i].shvcode = RXSHV_SYSET;

   /* Variablen setzen */
   RexxVariablePool(pVarBlock);

   /* aufrеmen */
   pHelp = pVarBlock;
   while (pHelp)
   {
      free(pHelp->shvname.strptr);
      pHelp = pHelp->shvnext;
   }
   free(pVarBlock);

   return;
}

static void RexxVarKludges(FTNMESSAGE *pMessage)
{
   char pchName[50];
   char *pchCont=NULL;
   int i=0;
   PKLUDGE pKludge=NULL;

   while (pKludge = MSG_FindKludge(pMessage, KLUDGE_ANY, pKludge))
   {
      if (pKludge->ulKludgeType == KLUDGE_OTHER)
         pchCont = strdup(pKludge->pchKludgeText);
      else
      {
         size_t len=0;

         len=strlen(MSG_QueryKludgeName(pKludge->ulKludgeType));
         len+= 1 + strlen(pKludge->pchKludgeText);

         pchCont = malloc(len+1);

         strcpy(pchCont, MSG_QueryKludgeName(pKludge->ulKludgeType));
         strcat(pchCont, " ");
         strcat(pchCont, pKludge->pchKludgeText);
      }

      i++;
      sprintf(pchName, "FleetMsg.Kludges.%d", i);
      SetRexxVar(pchName, pchCont);
      free(pchCont);
   }

   /* Anzahl */
   _itoa(i, pchName, 10);
   SetRexxVar("FleetMsg.Kludges.0", pchName);

   return;
}

static void RexxVarSeenbys(FTNMESSAGE *pMessage)
{
   char pchName[50];
   char *pchDup, *pchTemp;
   int i=0;

   if (pMessage->pchSeenPath)
   {
      pchDup = strdup(pMessage->pchSeenPath);

      /* SOH umwandeln (v. PATH) */
      pchTemp = pchDup;
      while (*pchTemp)
      {
         if (*pchTemp == 1)
            *pchTemp = '@';
         pchTemp++;
      }

      pchTemp = strtok(pchDup, "\r\n");
      while (pchTemp)
      {
         i++;
         sprintf(pchName, "FleetMsg.Seenbys.%d", i);
         SetRexxVar(pchName, pchTemp);

         pchTemp = strtok(NULL, "\r\n");
      }

      free(pchDup);
   }

   /* Anzahl */
   _itoa(i, pchName, 10);
   SetRexxVar("FleetMsg.Seenbys.0", pchName);

   return;
}

static void SetRexxVar(const char *pchName, const char *pchValue)
{
   SHVBLOCK VarBlock;

   VarBlock.shvnext=NULL;

   MAKERXSTRING(VarBlock.shvname, pchName, strlen(pchName));
   VarBlock.shvnamelen = VarBlock.shvname.strlength;
   MAKERXSTRING(VarBlock.shvvalue, pchValue, strlen(pchValue));
   VarBlock.shvvaluelen = VarBlock.shvvalue.strlength;

#if 1
   VarBlock.shvcode = RXSHV_SYSET;
#else
   VarBlock.shvcode = RXSHV_SET;
#endif

   RexxVariablePool(&VarBlock);

   return;
}

static int QueryRexxVar(const char *pchName, char *pchBuffer, ULONG ulBufLen)
{
   SHVBLOCK VarBlock;

   if (ulBufLen < 1 || !pchBuffer || !pchName)
      return -1;

   VarBlock.shvnext=NULL;

   MAKERXSTRING(VarBlock.shvname, pchName, strlen(pchName));
   VarBlock.shvnamelen = VarBlock.shvname.strlength;
   VarBlock.shvvalue.strptr=NULL;
   VarBlock.shvvaluelen = 0;

   VarBlock.shvcode = RXSHV_SYFET;

   if (RexxVariablePool(&VarBlock))
      return -1;

   pchBuffer[ulBufLen-1]=0;
   if (ulBufLen-1 <= VarBlock.shvvalue.strlength)
      memcpy(pchBuffer, VarBlock.shvvalue.strptr, ulBufLen-1);
   else
   {
      memcpy(pchBuffer, VarBlock.shvvalue.strptr, VarBlock.shvvalue.strlength);
      pchBuffer[VarBlock.shvvalue.strlength]=0;
   }

   DosFreeMem(VarBlock.shvvalue.strptr);

   return 0;
}

static void RexxVarMessageHeader(MSGHEADER *pHeader)
{
   static char flagsbuf[200];

   MSG_AttribToText(pHeader->ulAttrib, flagsbuf);
   SetRexxVar("FleetMsg.Header.Attrib", flagsbuf);

   SetRexxVar("FleetMsg.Header.From", pHeader->pchFromName);
   SetRexxVar("FleetMsg.Header.To",   pHeader->pchToName);
   SetRexxVar("FleetMsg.Header.Subj", pHeader->pchSubject);
   SetRexxVar("FleetMsg.Header.FromAddress", NetAddrToString(flagsbuf, &pHeader->FromAddress));
   SetRexxVar("FleetMsg.Header.ToAddress", NetAddrToString(flagsbuf, &pHeader->ToAddress));
   SetRexxVar("FleetMsg.Header.DateWritten", StampToString(flagsbuf, &pHeader->StampWritten));
   SetRexxVar("FleetMsg.Header.DateReceived", StampToString(flagsbuf, &pHeader->StampArrived));

   return;
}

static void RexxVarMessageText(FTNMESSAGE *pMessage, ULONG ulCursor)
{
   char pchNameTemp[50];
   char *pchTextDup;
   char *pchHelp;
   char *pchHelp2;
   int i=0;

   ULONG ulHelp=0;
   ULONG ulPara=1;
   ULONG ulOffs=1;

   /* Cursorposition ermitteln */
   while (ulHelp < ulCursor && pMessage->pchMessageText[ulHelp])
   {
      if (pMessage->pchMessageText[ulHelp] == '\n')
      {
         ulPara++;
         ulOffs=1;
      }
      else
         ulOffs++;
      ulHelp++;
   }

   sprintf(pchNameTemp, "%d %d", ulPara, ulOffs);
   SetRexxVar("FleetStatus.Cursor", pchNameTemp);

   if (!pMessage->pchMessageText)
   {
      SetRexxVar("FleetMsg.Text.0", "0");
      return;
   }
   else
   {
      pchTextDup = strdup(pMessage->pchMessageText);
      pchHelp = pchTextDup;

      do
      {
         pchHelp2 = pchHelp;
         while (*pchHelp && *pchHelp != '\n')
            pchHelp++;
         if (*pchHelp)
            *pchHelp++ = 0;

         i++;
         sprintf(pchNameTemp, "FleetMsg.Text.%d", i);
         SetRexxVar(pchNameTemp, pchHelp2);
      } while(*pchHelp);
      sprintf(pchNameTemp, "%d", i);
      SetRexxVar("FleetMsg.Text.0", pchNameTemp);
      free(pchTextDup);
      return;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CallRexx                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Threadfunktion zur Ausfuehrung des Rexx-Scripts             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf Script                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void _Optlink CallRexx(PVOID pParam)
{
   PRXSCRIPT pScript = (PRXSCRIPT) pParam;
   RXSYSEXIT Exits[] = {
                        {"ScriptInit", RXINI},
                        {"ScriptTerm", RXTER},
                        {NULL,         RXENDLST}
                       };
   LONG rc;
   SHORT ReturnCode;
   RXSTRING ReturnString= {0, NULL};
   HMQ hmq;
   HAB hab;

   INSTALLEXPT("CallRexx");

   hab=WinInitialize(0);
   hmq=WinCreateMsgQueue(hab, 100);
   WinCancelShutdown(hmq, TRUE);

   FlushPipes();

   RexxRegisterExitExe("ScriptInit", RexxInitExit, NULL);
   RexxRegisterExitExe("ScriptTerm", RexxTermExit, NULL);

   rc = RexxStart(0,                    /* Arg count */
                  NULL,                 /* Arg list */
                  pScript->pchPathName, /* Name */
                  NULL,                 /* Instore */
                  REXXENVNAME,          /* environment name */
                  RXSUBROUTINE,         /* call type */
                  Exits,                /* Exits */
                  &ReturnCode,
                  &ReturnString);

   if (ReturnString.strptr)
      DosFreeMem(ReturnString.strptr);

   RexxDeregisterExit("ScriptInit", NULL);
   RexxDeregisterExit("ScriptTerm", NULL);

   if (rc)
   {
      if (rc < 0)
      {
         char *pchErrorDesc=malloc(1000);
         char *pchErrorTitle=malloc(100);
         *pchErrorDesc=0;
         *pchErrorTitle=0;

         GetRexxErrorMsg(pchErrorDesc, pchErrorTitle, -rc);

         SendMsg(client, REXXM_ERROR, pchErrorDesc, pchErrorTitle);
      }
      else
      {
         char *pchErrorText=malloc(50);

         sprintf(pchErrorText, "System error %d", rc);
         SendMsg(client, REXXM_ERROR, pchErrorText, NULL);
      }
   }
   else
      SendMsg(client, REXXM_STOPSCRIPT, NULL, NULL);

   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);

   DEINSTALLEXPT;

   return;
}

static ULONG _System FSSetText(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   extern FTNMESSAGE CurrentMessage;
   extern BOOL bTemplateProcessed;

   char *pchTempName;
   SHVBLOCK VarBlock;
   ULONG ulNum=0;
   PCHAR pchRest=NULL;
   ULONG i;
   PCHAR pchNewText=NULL, pchHelp;
   ULONG ulReqSpace=0;
   PSTRINGLIST pStringList=NULL, pLast=NULL;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc != 1)
      return 40;

   /* Stem.0 abfragen */
   pchTempName=calloc(1, strlen(argv[0].strptr)+20);
   sprintf(pchTempName, "%s.0", argv[0].strptr);

   memset(&VarBlock, 0, sizeof(VarBlock));
   VarBlock.shvnext=NULL;
   VarBlock.shvname.strptr= pchTempName;
   VarBlock.shvname.strlength = strlen(pchTempName);
   VarBlock.shvnamelen=VarBlock.shvname.strlength;
   VarBlock.shvcode = RXSHV_SYFET;

   if (RexxVariablePool(&VarBlock))
   {
      free(pchTempName);
      return 40;
   }
   free(pchTempName);

   pchTempName = calloc(1, VarBlock.shvvalue.strlength+1);
   strncpy(pchTempName,VarBlock.shvvalue.strptr, VarBlock.shvvalue.strlength);
   DosFreeMem(VarBlock.shvvalue.strptr);

   /* numerischen Wert auslesen */
   ulNum=strtoul(pchTempName, &pchRest, 10);
   if (*pchRest)
   {
      free(pchTempName);
      return 40;
   }
   free(pchTempName);

   if (ulNum > 0)
   {
      /* Alle Variablen abfragen */
      pchTempName=malloc(strlen(argv[0].strptr)+20);

      for (i=0; i<ulNum; i++)
      {
         memset(&VarBlock, 0, sizeof(VarBlock));

         VarBlock.shvname.strptr = pchTempName;
         sprintf(pchTempName, "%s.%d", argv[0].strptr, i+1);
         VarBlock.shvname.strlength = strlen(pchTempName);
         VarBlock.shvnamelen = VarBlock.shvname.strlength;
         VarBlock.shvcode = RXSHV_SYFET;

         if (!RexxVariablePool(&VarBlock))
         {
            /* Wert kopieren */
            if (pStringList)
            {
               pLast->next = malloc(sizeof(STRINGLIST));
               pLast= pLast->next;
            }
            else
               pStringList = pLast = malloc(sizeof(STRINGLIST));
            pLast->next=NULL;
            pLast->pchLine = calloc(1, VarBlock.shvvalue.strlength+1);
            memcpy(pLast->pchLine, VarBlock.shvvalue.strptr, VarBlock.shvvalue.strlength);
            DosFreeMem(VarBlock.shvvalue.strptr);
            ulReqSpace += VarBlock.shvvalue.strlength+2;
         }
         else
         {
            /* Fehler */
            /* bisherige Liste freigeben */
            while (pStringList)
            {
               pLast = pStringList->next;
               free(pStringList->pchLine);
               free(pStringList);
               pStringList = pLast;
            }
            free(pchTempName);
            return 40;
         }
      }
      free(pchTempName);

      /* String f. neuen Text bereitstellen */
      pchNewText = malloc(ulReqSpace);
      *pchNewText=0;
      pchHelp = pchNewText;

      /* Liste zusammenkopieren */
      while (pStringList)
      {
         strcpy(pchHelp, pStringList->pchLine);
         pchHelp = strchr(pchHelp, 0);
         *pchHelp++ = '\n';
         pLast = pStringList->next;
         free(pStringList->pchLine);
         free(pStringList);
         pStringList = pLast;
      }
      *pchHelp = 0;

   }
   else
      pchNewText=calloc(1, 1);

   /* in pchNewText steht nun der neue Text */
   if (CurrentMessage.pchMessageText)
      free(CurrentMessage.pchMessageText);

   CurrentMessage.pchMessageText = pchNewText;
   CurrentMessage.pchSeenPath=NULL;
   WinSetDlgItemText(client, IDML_MAINEDIT, "");
   DisplayMsgText(client, &CurrentMessage);

   /* Text wurde geaendert, weitere Template-Verarbeitung macht keinen
      Sinn mehr, deshalb abschalten. */
   bTemplateProcessed = TRUE;

   strcpy(pRetStr->strptr, "OK");
   pRetStr->strlength = 2;

   return 0;
}

static ULONG _System FSLookupName(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   PNODEDATA pNodeData=NULL;
   extern PDOMAINS domains;
   char pchErrDomain[LEN_DOMAIN+1];
   char *pchVarName;
   char pchTemp[40];
   char *pchLookupName;
   int iRet, i;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc != 2)
      return 40;

   pchLookupName = calloc(1, LEN_USERNAME+1);
   strncpy(pchLookupName, argv[0].strptr, LEN_USERNAME);

   iRet = LookupNodelists(pchLookupName, domains, &pNodeData, pchErrDomain);

   if (iRet == 0)
   {
      strcpy(pRetStr->strptr, "NotFound");
      pRetStr->strlength = strlen(pRetStr->strptr);
   }
   else
      if (iRet < 0)
      {
         strcpy(pRetStr->strptr, "Error");
         pRetStr->strlength = strlen(pRetStr->strptr);
      }
      else
      {
         pchVarName = malloc(argv[1].strlength + 20);
         sprintf(pchVarName, "%s.0", argv[1].strptr);
         _itoa(iRet, pchTemp, 10);
         SetRexxVar(pchVarName, pchTemp);

         for (i=0; i < iRet; i++)
         {
            sprintf(pchVarName, "%s.%d.Address", argv[1].strptr, i+1);
            NetAddrToString(pchTemp, (PFTNADDRESS) &pNodeData[i].Address);
            SetRexxVar(pchVarName, pchTemp);

            sprintf(pchVarName, "%s.%d.Name", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, pNodeData[i].SysopName);

            sprintf(pchVarName, "%s.%d.System", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, pNodeData[i].SystemName);

            sprintf(pchVarName, "%s.%d.Phone", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, pNodeData[i].PhoneNr);

            sprintf(pchVarName, "%s.%d.Location", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, pNodeData[i].Location);

            sprintf(pchVarName, "%s.%d.Password", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, pNodeData[i].Password);

            sprintf(pchVarName, "%s.%d.Modem", argv[1].strptr, i+1);
            _itoa(pNodeData[i].ModemType, pchTemp, 10);
            SetRexxVar(pchVarName, pchTemp);

            sprintf(pchVarName, "%s.%d.Baud", argv[1].strptr, i+1);
            _itoa(pNodeData[i].BaudRate, pchTemp, 10);
            SetRexxVar(pchVarName, pchTemp);

            sprintf(pchVarName, "%s.%d.UserCost", argv[1].strptr, i+1);
            _itoa(pNodeData[i].UserCost, pchTemp, 10);
            SetRexxVar(pchVarName, pchTemp);

            sprintf(pchVarName, "%s.%d.CallCost", argv[1].strptr, i+1);
            _itoa(pNodeData[i].CallCost, pchTemp, 10);
            SetRexxVar(pchVarName, pchTemp);

            sprintf(pchVarName, "%s.%d.Flags", argv[1].strptr, i+1);
            SetRexxVar(pchVarName, NLFlagsToString(&pNodeData[i], pchTemp));
         }
         free(pNodeData);
         free(pchVarName);

         strcpy(pRetStr->strptr, "OK");
         pRetStr->strlength= 2;
      }

   free(pchLookupName);

   return 0;
}

static ULONG _System FSLookupAddress(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   PNODEDATA pNodeData=NULL;
   extern PDOMAINS domains;
   char pchErrDomain[LEN_DOMAIN+1];
   char *pchVarName;
   char pchTemp[40];
   char *pchLookupAddress;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc != 2)
      return 40;

   pchLookupAddress = calloc(1, LEN_5DADDRESS+1);
   strncpy(pchLookupAddress, argv[0].strptr, LEN_5DADDRESS);

   switch(LookupAddress(pchLookupAddress, domains, &pNodeData, pchErrDomain))
   {
      case 0:
         strcpy(pRetStr->strptr, "NotFound");
         pRetStr->strlength = strlen(pRetStr->strptr);
         break;

      case 1:
         pchVarName = malloc(argv[1].strlength + 15);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Address");
         SetRexxVar(pchVarName, pchLookupAddress);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Name");
         SetRexxVar(pchVarName, pNodeData->SysopName);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".System");
         SetRexxVar(pchVarName, pNodeData->SystemName);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Phone");
         SetRexxVar(pchVarName, pNodeData->PhoneNr);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Location");
         SetRexxVar(pchVarName, pNodeData->Location);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Password");
         SetRexxVar(pchVarName, pNodeData->Password);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Modem");
         _itoa(pNodeData->ModemType, pchTemp, 10);
         SetRexxVar(pchVarName, pchTemp);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Baud");
         _itoa(pNodeData->BaudRate, pchTemp, 10);
         SetRexxVar(pchVarName, pchTemp);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".UserCost");
         _itoa(pNodeData->UserCost, pchTemp, 10);
         SetRexxVar(pchVarName, pchTemp);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".CallCost");
         _itoa(pNodeData->CallCost, pchTemp, 10);
         SetRexxVar(pchVarName, pchTemp);
         strcpy(pchVarName, argv[1].strptr);
         strcat(pchVarName, ".Flags");
         SetRexxVar(pchVarName, NLFlagsToString(pNodeData, pchTemp));
         free(pNodeData);
         free(pchVarName);

         strcpy(pRetStr->strptr, "OK");
         pRetStr->strlength= 2;
         break;

      default:
         strcpy(pRetStr->strptr, "Error");
         pRetStr->strlength = strlen(pRetStr->strptr);
         break;
   }

   free(pchLookupAddress);

   return 0;
}

static ULONG _System FSSetEntryField(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc != 1)
      return 40;

   if (hwndMonitor)
   {
      WinSetDlgItemText(hwndMonitor, IDD_RXMONITOR+2, argv[0].strptr);
      strcpy(pRetStr->strptr, "OK");
   }
   else
   {
      strcpy(pRetStr->strptr, "NoMonitor");
   }
   pRetStr->strlength = strlen(pRetStr->strptr);

   return 0;
}

static ULONG _System FSCls(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;
   argv=argv;

   if (argc != 0)
      return 40;

   if (hwndMonitor)
   {
      WinPostMsg(hwndMonitor, REXXM_CLS, NULL, NULL);
      strcpy(pRetStr->strptr, "OK");
   }
   else
   {
      strcpy(pRetStr->strptr, "NoMonitor");
   }
   pRetStr->strlength = strlen(pRetStr->strptr);

   return 0;
}

static ULONG _System FSSetHeader(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   char pchFromName[LEN_USERNAME+1];
   char pchToName[LEN_USERNAME+1];
   char pchSubject[LEN_SUBJECT+1];
   char pchFromAddr[LEN_5DADDRESS+1];
   char pchToAddr[LEN_5DADDRESS+1];
   char *pchVarName;
   ULONG ret=0;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc != 1)
      return 40;

   pchVarName=malloc(argv[0].strlength + 30);

   sprintf(pchVarName, "%s.%s", argv[0].strptr, "From");
   if (!QueryRexxVar(pchVarName, pchFromName, LEN_USERNAME+1))
   {
      sprintf(pchVarName, "%s.%s", argv[0].strptr, "To");
      if (!QueryRexxVar(pchVarName, pchToName, LEN_USERNAME+1))
      {
         sprintf(pchVarName, "%s.%s", argv[0].strptr, "FromAddress");
         if (!QueryRexxVar(pchVarName, pchFromAddr, LEN_5DADDRESS+1))
         {
            sprintf(pchVarName, "%s.%s", argv[0].strptr, "ToAddress");
            if (!QueryRexxVar(pchVarName, pchToAddr, LEN_5DADDRESS+1))
            {
               sprintf(pchVarName, "%s.%s", argv[0].strptr, "Subj");
               if (!QueryRexxVar(pchVarName, pchSubject, LEN_SUBJECT+1))
               {
                  extern MSGHEADER CurrentHeader;

                  /* Alles da, Header updaten */
                  strcpy(CurrentHeader.pchFromName, pchFromName);
                  strcpy(CurrentHeader.pchToName,   pchToName);
                  strcpy(CurrentHeader.pchSubject,  pchSubject);
                  StringToNetAddr(pchFromAddr, &CurrentHeader.FromAddress, NULL);
                  StringToNetAddr(pchToAddr,   &CurrentHeader.ToAddress, NULL);

                  WinSetDlgItemText(client, IDML_MAINEDIT, "");
                  DisplayMessage(TRUE);
                  /*UpdateDisplay(FALSE, TRUE);*/
               }
               else
                  ret=40;
            }
            else
               ret=40;
         }
         else
            ret=40;
      }
      else
         ret=40;
   }
   else
      ret=40;

   free(pchVarName);

   if (ret)
      strcpy(pRetStr->strptr, "Error");
   else
      strcpy(pRetStr->strptr, "OK");

   pRetStr->strlength = strlen(pRetStr->strptr);

   return ret;
}


#define ENC(c) (c?(((c) & 077) + ' '):'`')
#define DEC(c) ((c=='`')?0:((c) - ' '))

static ULONG _System FSEncodeLine(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   ULONG ret=0;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc == 1)
   {
      if (argv[0].strlength)
      {
         int i=0, j=1;

         pRetStr->strptr[0] = ENC(argv[0].strlength);

         while (i < argv[0].strlength)
         {
            pRetStr->strptr[j]=ENC( argv[0].strptr[i] >> 2);
            pRetStr->strptr[j+1]=ENC( (argv[0].strptr[i] << 4) & 060 | (argv[0].strptr[i+1] >> 4) & 017);
            pRetStr->strptr[j+2]=ENC( (argv[0].strptr[i+1] << 2) & 074 | (argv[0].strptr[i+2] >> 6) & 03);
            pRetStr->strptr[j+3]=ENC( argv[0].strptr[i+2] & 077);

            i+=3;
            j+=4;
         }
         pRetStr->strlength=j;
      }
   }
   else
      ret=40;

   return ret;
}

static ULONG _System FSDecodeLine(PUCHAR pchFuncName, ULONG argc, PRXSTRING argv, PSZ pchQueueName, PRXSTRING pRetStr)
{
   ULONG ret=0;

   pchFuncName = pchFuncName;
   pchQueueName = pchQueueName;

   if (argc == 1)
   {
      if (argv[0].strlength)
      {
        int i=0, j=1;
        int numbytes=0;

        numbytes = DEC(argv[0].strptr[0]);

        while (j < argv[0].strlength &&
               i < numbytes)
        {
           pRetStr->strptr[i]  = (DEC(argv[0].strptr[j]) << 2) | (DEC(argv[0].strptr[j+1]) >> 4);
           pRetStr->strptr[i+1]= (DEC(argv[0].strptr[j+1]) << 4) | (DEC(argv[0].strptr[j+2]) >> 2);
           pRetStr->strptr[i+2]= (DEC(argv[0].strptr[j+2]) << 6) | DEC(argv[0].strptr[j+3]);

           i+=3;
           j+=4;
        }
        pRetStr->strlength=numbytes;
      }
   }
   else
      ret=40;

   return ret;
}

/*-------------------------------- Modulende --------------------------------*/


