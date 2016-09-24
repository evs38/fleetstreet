/*---------------------------------------------------------------------------+
 | Titel: FINDDLG.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 04.08.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Find-Dialoge fuer Fleet Street                                        |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPI
#define INCL_WIN
#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "handlemsg\handlemsg.h"
#include "util\fltutil.h"
#include "finddlg.h"
#include "utility.h"
#include "areadlg.h"
#include "findexec.h"
#include "markmanage.h"
#include "msglist.h"

/*--------------------------------- Defines ---------------------------------*/

typedef struct {
         HPOINTER icon;
         BOOL AutoScroll;
         char pchTArea[20];
         char pchTNumber[20];
         char pchTFrom[20];
         char pchTSubject[20];
         char pchCnrTitle[100];
         HSWITCH hSwitch;
         POINTL minsize;
         HWND hwndPopup;
         HWND hwndFolderPopup;
         BOOL bKeyboard;
         BOOL bForeground;
         RESULTRECORD *pPopupRecord;
         ULONG ulView;
         ULONG ulDisable;
         BOOL bNoUpdate;
      } RESULTSDATA, *PRESULTSDATA;

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L    /* 2.1-spezifisch !!! */
#endif

/*---------------------------- Globale Variablen ----------------------------*/

extern HWND client, frame;
extern HWND hwndhelp;
extern HMODULE hmodLang;
extern AREALIST arealiste;
extern WINDOWPOSITIONS windowpositions;

extern HWND hwndFindResults;
extern HWND hwndFindDlg;
extern volatile BOOL DoingFind;
extern volatile BOOL StopFind;
extern int tidFind;
extern FINDJOB FindJob;

static PFNWP OldContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

static MRESULT EXPENTRY NewFContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static int GetJobParams(HWND hwndDlg, PFINDJOB pFindJob);
static MRESULT EXPENTRY PersmailProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void ResizeResults(HWND hwndDlg, PSWP pNewSize);
static void CleanupResults(HWND hwndContainer);
static int BookmarksPopup(HWND hwndDlg, PRESULTSDATA pResultsData, RESULTRECORD *pRecord);
static void SwitchResultsView(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulNewView);
static void FillResultsWindow(HWND hwndDlg, PRESULTSDATA pResultsData);
static void SetResultsTitle(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulMenuID);
static PWORKDATA CollectBookmarks(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulWork,
                                  PCHAR pchDestArea, PCHAR pchDestFile, ULONG ulOptions);
static void UpdateGotoButton(HWND hwndDlg, PRESULTSDATA pResultsData);
static SHORT _System SortResults(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage);
static void CheckBoxClick1(HWND parent);
static void CheckBoxClick2(HWND parent);
static void BacklogText(PFINDJOB pFindJob);
static void FillDropDown(HWND hwndDropDown, PFINDJOB pFindJob);

/*------------------------------ NewContainerProc ---------------------------*/
/* Neue Window-Prozedur f. Container, um OS/2-Bug zu umschiffen              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewFContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo(mp1);
         break;

      default:
         break;
   }
   return OldContainerProc(parent, message, mp1, mp2);
}


/*-------------------------------  FindProc  --------------------------------*/
/* Dialog-Prozedur des Such-Dialogs                                          */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY FindProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   AREALISTPAR AreaListPar;
   extern DIRTYFLAGS dirtyflags;

   switch(message)
   {
      case WM_INITDLG:
         /* Initialisieren */
         /* Text */

         FillDropDown(WinWindowFromID(parent, IDD_FINDTEXT+2), &FindJob);

         WinSetDlgItemText(parent, IDD_FINDTEXT+2, FindJob.pchBackTexts[0]);
         WinSendDlgItemMsg(parent, IDD_FINDTEXT+2, EM_SETSEL,
                           MPFROM2SHORT(0, 200), NULL);

         /* Where */
         if (FindJob.ulWhere & FINDWHERE_FROM)
            WinCheckButton(parent, IDD_FINDTEXT+5, TRUE);
         if (FindJob.ulWhere & FINDWHERE_TO)
            WinCheckButton(parent, IDD_FINDTEXT+6, TRUE);
         if (FindJob.ulWhere & FINDWHERE_SUBJ)
            WinCheckButton(parent, IDD_FINDTEXT+7, TRUE);
         if (FindJob.ulWhere & FINDWHERE_TEXT)
            WinCheckButton(parent, IDD_FINDTEXT+8, TRUE);

         /* WAreas */
         if (FindJob.ulWAreas & FINDAREAS_NM)
            WinCheckButton(parent, IDD_FINDTEXT+21, TRUE);
         if (FindJob.ulWAreas & FINDAREAS_ECHO)
            WinCheckButton(parent, IDD_FINDTEXT+22, TRUE);
         if (FindJob.ulWAreas & FINDAREAS_LOCAL)
            WinCheckButton(parent, IDD_FINDTEXT+23, TRUE);
         if (FindJob.ulWAreas & FINDAREAS_PRIV)
            WinCheckButton(parent, IDD_FINDTEXT+24, TRUE);

         switch(FindJob.ulWAreas & 0xf)
         {
            case FINDAREAS_CURRENT:
               WinCheckButton(parent, IDD_FINDTEXT+18, TRUE);
               break;

            case FINDAREAS_ALL:
               WinCheckButton(parent, IDD_FINDTEXT+19, TRUE);
               break;

            case FINDAREAS_CUSTOMN:
               WinCheckButton(parent, IDD_FINDTEXT+25, TRUE);
               break;

            case FINDAREAS_TYPE:
               WinCheckButton(parent, IDD_FINDTEXT+27, TRUE);
               break;

            default:
               WinCheckButton(parent, IDD_FINDTEXT+18, TRUE);
               break;
         }

         switch(FindJob.ulHow & FINDHOW_METHOD_MASK)
         {
            case FINDHOW_SENS:
               WinCheckButton(parent, IDD_FINDTEXT+10, TRUE);
               break;

            case FINDHOW_CASE:
               WinCheckButton(parent, IDD_FINDTEXT+11, TRUE);
               break;

            case FINDHOW_FUZZY:
               WinCheckButton(parent, IDD_FINDTEXT+12, TRUE);
               break;

            case FINDHOW_REGEX:
               WinCheckButton(parent, IDD_FINDTEXT+13, TRUE);
               break;

            case FINDHOW_PERSMAIL:
               WinCheckButton(parent, IDD_FINDTEXT+14, TRUE);
               break;

            case FINDHOW_UNSENT:
               WinCheckButton(parent, IDD_FINDTEXT+20, TRUE);
               break;

            default:
               break;
         }
         if (FindJob.ulHow & FINDHOW_UNREADONLY)
            WinCheckButton(parent, IDD_FINDTEXT+29, TRUE);

         /* Controls aktivieren oder deaktivieren */
         CheckBoxClick1(parent);
         CheckBoxClick2(parent);

         /* Fuzzy-Levels initialisieren */
         WinSendDlgItemMsg(parent, IDD_FINDTEXT+15, SPBM_SETLIMITS,
                           MPFROMLONG(5), MPFROMLONG(1));
         WinSendDlgItemMsg(parent, IDD_FINDTEXT+15, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(FindJob.ulFuzzyLevel), NULL);

         WinAssociateHelpInstance(hwndhelp, parent);
         RestoreWinPos(parent, &windowpositions.findpos, FALSE, TRUE);
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         QueryWinPos(parent, &(windowpositions.findpos));
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch(SHORT1FROMMP(mp1))
            {
               PFINDJOB pFindJob;

               case DID_OK:
                  switch(GetJobParams(parent, &FindJob))
                  {
                     case 0: /* OK */
                        if ((FindJob.ulWAreas & 0x0F) == FINDAREAS_CUSTOMN &&
                            !FindJob.pchAreas)
                        {
                           MessageBox(parent, IDST_MSG_FINDNOAREAS, 0,
                                      IDD_FINDNOAREAS, MB_OK | MB_ERROR);
                           return (MRESULT) FALSE;
                        }

                        pFindJob=malloc(sizeof(FINDJOB));
                        memcpy(pFindJob, &FindJob, sizeof(FINDJOB));
                        if (FindJob.pchAreas)
                           pFindJob->pchAreas = strdup(FindJob.pchAreas);

                        tidFind=_beginthread(FindThread2, NULL, 0x20000, pFindJob);
                        dirtyflags.finddirty=TRUE;
                        break;

                     case 1:
                        MessageBox(parent, IDST_MSG_FINDNOPARTS, 0,
                                   IDD_FINDNOPARTS, MB_OK | MB_ERROR);
                        return (MRESULT) FALSE;

                     case 2:
                        MessageBox(parent, IDST_MSG_FINDNOAREAS, 0,
                                   IDD_FINDNOAREAS, MB_OK | MB_ERROR);
                        return (MRESULT) FALSE;

                     case 3:
                        MessageBox(parent, IDST_MSG_FINDNOTEXT, 0,
                                   IDD_FINDNOTEXT, MB_OK | MB_ERROR);
                        return (MRESULT) FALSE;

                     default:
                        return (MRESULT) FALSE;
                  }

                  if (DoingFind)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     return (MRESULT) FALSE;
                  }
                  break;

               case DID_CANCEL:
                  break;

               case IDD_FINDTEXT+16: /* Configure */
                  FindJob.PersMailOpt.cb=sizeof(PERSMAILOPT);
                  WinDlgBox(HWND_DESKTOP, parent, PersmailProc, hmodLang,
                            IDD_PERSMAILOPT, &FindJob.PersMailOpt);
                  return (MRESULT) FALSE;

               case IDD_FINDTEXT+28: /* Select */
                  AreaListPar.cb=sizeof(AREALISTPAR);
                  AreaListPar.idTitle=0; /* @@ */
                  if (FindJob.pchAreas)
                     AreaListPar.pchString=strdup(FindJob.pchAreas);
                  else
                     AreaListPar.pchString=NULL;
                  AreaListPar.ulIncludeTypes = INCLUDE_ALL;
                  AreaListPar.bExtendedSel = TRUE;
                  AreaListPar.bChange = FALSE;
                  if (WinDlgBox(HWND_DESKTOP, parent, AreaListProc, hmodLang,
                                IDD_AREALIST, &AreaListPar) == DID_OK && AreaListPar.pchString)
                  {
                     if (FindJob.pchAreas)
                        free(FindJob.pchAreas);
                     FindJob.pchAreas = AreaListPar.pchString;
                     dirtyflags.finddirty=TRUE;
                  }
                  return (MRESULT) FALSE;

               default:
                  return (MRESULT) FALSE;
            }
         }
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_FINDTEXT+18:
            case IDD_FINDTEXT+19:
            case IDD_FINDTEXT+25:
            case IDD_FINDTEXT+27:
               CheckBoxClick1(parent);
               break;

            case IDD_FINDTEXT+10:
            case IDD_FINDTEXT+14:
            case IDD_FINDTEXT+13:
            case IDD_FINDTEXT+12:
            case IDD_FINDTEXT+11:
            case IDD_FINDTEXT+20:
               CheckBoxClick2(parent);
               break;

            default:
               break;
         }
         break;

      case WM_CHAR:
         if (!DoingFind)
            WinEnableControl(parent, DID_OK, TRUE);
         else
            WinEnableControl(parent, DID_OK, FALSE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

static void CheckBoxClick1(HWND parent)
{
   if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+27))
   {
      WinEnableControl(parent, IDD_FINDTEXT+21, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+22, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+23, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+24, TRUE);
   }
   else
   {
      WinEnableControl(parent, IDD_FINDTEXT+21, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+22, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+23, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+24, FALSE);
   }

   if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+25))
      WinEnableControl(parent, IDD_FINDTEXT+28, TRUE);
   else
      WinEnableControl(parent, IDD_FINDTEXT+28, FALSE);

   if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+18) ||
       WinQueryButtonCheckstate(parent, IDD_FINDTEXT+25))
      WinEnableControl(parent, IDD_FINDTEXT+26, FALSE);
   else
      WinEnableControl(parent, IDD_FINDTEXT+26, TRUE);

   return;
}

static void CheckBoxClick2(HWND parent)
{
   if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+14) ||
       WinQueryButtonCheckstate(parent, IDD_FINDTEXT+20))
   {
      WinEnableControl(parent, IDD_FINDTEXT+2, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+5, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+6, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+7, FALSE);
      WinEnableControl(parent, IDD_FINDTEXT+8, FALSE);
      if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+14))
         WinEnableControl(parent, IDD_FINDTEXT+16, TRUE);
      else
         WinEnableControl(parent, IDD_FINDTEXT+16, FALSE);
   }
   else
   {
      WinEnableControl(parent, IDD_FINDTEXT+2, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+5, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+6, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+7, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+8, TRUE);
      WinEnableControl(parent, IDD_FINDTEXT+16, FALSE);
   }

   if (WinQueryButtonCheckstate(parent, IDD_FINDTEXT+12))
      WinEnableControl(parent, IDD_FINDTEXT+15, TRUE);
   else
      WinEnableControl(parent, IDD_FINDTEXT+15, FALSE);

   return;
}

static void FillDropDown(HWND hwndDropDown, PFINDJOB pFindJob)
{
   int i=0;

   while (i < NUM_BACKTEXTS && pFindJob->pchBackTexts[i][0])
   {
      WinSendMsg(hwndDropDown, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                 pFindJob->pchBackTexts[i]);
      i++;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetJobParams                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Suchjob-Parameter aus dem Dialog                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window                                         */
/*            pFindJob: Zeiger auf Suchjob-Struktur                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  OK                                                      */
/*                1  Keine zu durchsuchenden Message-Teile angegeben         */
/*                2  Keine Area-Typen angegeben                              */
/*                3  Kein Suchtext angegeben                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int GetJobParams(HWND hwndDlg, PFINDJOB pFindJob)
{
   /* What */
   WinQueryDlgItemText(hwndDlg, IDD_FINDTEXT+2, 200, pFindJob->pchWhat);

   BacklogText(pFindJob);

   /* Where */
   pFindJob->ulWhere=0;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+5))
      pFindJob->ulWhere |= FINDWHERE_FROM;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+6))
      pFindJob->ulWhere |= FINDWHERE_TO;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+7))
      pFindJob->ulWhere |= FINDWHERE_SUBJ;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+8))
      pFindJob->ulWhere |= FINDWHERE_TEXT;

   /* How */
   pFindJob->ulHow = 0;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+10))
      pFindJob->ulHow = FINDHOW_SENS;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+11))
      pFindJob->ulHow = FINDHOW_CASE;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+12))
      pFindJob->ulHow = FINDHOW_FUZZY;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+13))
      pFindJob->ulHow = FINDHOW_REGEX;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+14))
      pFindJob->ulHow = FINDHOW_PERSMAIL;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+20))
      pFindJob->ulHow = FINDHOW_UNSENT;

   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+29))
      pFindJob->ulHow |= FINDHOW_UNREADONLY;

   /* Areas */
   pFindJob->ulWAreas = 0;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+18))
      pFindJob->ulWAreas = FINDAREAS_CURRENT;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+19))
      pFindJob->ulWAreas = FINDAREAS_ALL;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+27))
      pFindJob->ulWAreas = FINDAREAS_TYPE;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+25))
      pFindJob->ulWAreas = FINDAREAS_CUSTOMN;

   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+21))
      pFindJob->ulWAreas |= FINDAREAS_NM;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+22))
      pFindJob->ulWAreas |= FINDAREAS_ECHO;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+23))
      pFindJob->ulWAreas |= FINDAREAS_LOCAL;
   if (WinQueryButtonCheckstate(hwndDlg, IDD_FINDTEXT+24))
      pFindJob->ulWAreas |= FINDAREAS_PRIV;

   /* Fuzzy-Level */
   WinSendDlgItemMsg(hwndDlg, IDD_FINDTEXT+15, SPBM_QUERYVALUE,
                     &pFindJob->ulFuzzyLevel, MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
   if (pFindJob->ulFuzzyLevel > 5)
      pFindJob->ulFuzzyLevel = 5;
   if (pFindJob->ulFuzzyLevel < 1)
      pFindJob->ulFuzzyLevel = 1;

   /* Pruefungen */
   if ((pFindJob->ulHow & FINDHOW_METHOD_MASK) != FINDHOW_PERSMAIL &&
       (pFindJob->ulHow & FINDHOW_METHOD_MASK) != FINDHOW_UNSENT   &&
       pFindJob->ulWhere == 0)
      return 1;  /* Keine Message-Teile */

   if ((pFindJob->ulWAreas & 0x0F) == FINDAREAS_TYPE &&
       (pFindJob->ulWAreas & 0xF0) == 0)
      return 2;  /* Keine Area-Typen */

   if ((pFindJob->ulHow & FINDHOW_METHOD_MASK) != FINDHOW_PERSMAIL &&
       (pFindJob->ulHow & FINDHOW_METHOD_MASK) != FINDHOW_UNSENT &&
       pFindJob->pchWhat[0]==0)
      return 3;  /* kein Text */

   return 0;
}

static void BacklogText(PFINDJOB pFindJob)
{
   int i=0;

   if (!pFindJob->pchWhat[0]) /* bei leerem Text nix machen */
      return;

   /* Text in gemerkten suchen */
   while (i < NUM_BACKTEXTS && strcmp(pFindJob->pchWhat, pFindJob->pchBackTexts[i]))
      i++;

   if (i < NUM_BACKTEXTS)
   {
      /* gefunden */
      if (i > 0)
      {
         /* mit erstem austauschen */
         strcpy(pFindJob->pchBackTexts[i], pFindJob->pchBackTexts[0]);
         strcpy(pFindJob->pchBackTexts[0], pFindJob->pchWhat);
      }
   }
   else
   {
      /* nicht gefunden, vorne einfuegen */
      memmove(pFindJob->pchBackTexts[1], pFindJob->pchBackTexts[0],
              (NUM_BACKTEXTS-1)* sizeof(pFindJob->pchBackTexts[0]));
      strcpy(pFindJob->pchBackTexts[0], pFindJob->pchWhat);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PersmailProc                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur f. Personal-Mail-Scan-Optionen              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Win-Proc                                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY PersmailProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern DIRTYFLAGS dirtyflags;
   PPERSMAILOPT pPersMailOpt = (PPERSMAILOPT) WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         pPersMailOpt = (PPERSMAILOPT) mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pPersMailOpt);

         if (pPersMailOpt->bAllMsgs)
         {
            WinCheckButton(hwnd, IDD_PERSMAILOPT+2, TRUE);
            SetFocusControl(hwnd, IDD_PERSMAILOPT+2);
         }
         else
         {
            WinCheckButton(hwnd, IDD_PERSMAILOPT+1, TRUE);
            SetFocusControl(hwnd, IDD_PERSMAILOPT+1);
         }

         if (pPersMailOpt->bAllNames)
            WinCheckButton(hwnd, IDD_PERSMAILOPT+4, TRUE);
         else
            WinCheckButton(hwnd, IDD_PERSMAILOPT+5, TRUE);

         /* Fensterposition */
         RestoreWinPos(hwnd, &windowpositions.persmailpos, FALSE, TRUE);

         return (MRESULT) TRUE;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)== DID_OK)
         {
            if (WinQueryButtonCheckstate(hwnd, IDD_PERSMAILOPT+2))
               pPersMailOpt->bAllMsgs = TRUE;
            else
               pPersMailOpt->bAllMsgs = FALSE;

            if (WinQueryButtonCheckstate(hwnd, IDD_PERSMAILOPT+4))
               pPersMailOpt->bAllNames = TRUE;
            else
               pPersMailOpt->bAllNames = FALSE;

            dirtyflags.finddirty=TRUE;
         }
         break;

      case WM_DESTROY:
         /* Fensterposition */
         QueryWinPos(hwnd, &windowpositions.persmailpos);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*------------------------------ FindResultsProc  ---------------------------*/
/* Dialog-Prozedur fuer den Find-Ergebnis-Dialog                             */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY FindResultsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWFONTS windowfonts;
   extern WINDOWCOLORS windowcolors;
   extern RESULTSOPTIONS resultsoptions;
   extern DIRTYFLAGS dirtyflags;
   extern BOOL bSaveResults;
   PFIELDINFO pFieldInfo, pfirstFieldInfo;
   FIELDINFOINSERT fieldInfoInsert;
   CNRINFO cnrinfo;
   RECORDINSERT RecordInsert;
   RESULTRECORD *pResultRecord, *pFirstRecord;
   SWP swp;
   MRESULT resultbuf;
   PFINDRESULTLIST pResultList=NULL;
   PFOUNDMSG pFoundMsg=NULL;
   PRESULTSDATA pResultsData = (PRESULTSDATA) WinQueryWindowULong(parent, QWL_USER);

   switch(message)
   {
      case WM_INITDLG:
         pResultsData = calloc(1, sizeof(RESULTSDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pResultsData);

         OldContainerProc=WinSubclassWindow(WinWindowFromID(parent, IDD_FINDRESULTS+1),
                                            NewFContainerProc);

         /* Eintrag in Fensterliste */
         pResultsData->hSwitch=AddToWindowList(parent);

         /* Popup-Menues */
         pResultsData->hwndPopup=WinLoadMenu(WinWindowFromID(parent, IDD_FINDRESULTS+1),
                                              hmodLang, IDM_BM_POPUP);
         pResultsData->hwndFolderPopup=WinLoadMenu(HWND_DESKTOP,
                                                   hmodLang, IDM_BMF_POPUP);
         if (pResultsData->hwndFolderPopup)
            ReplaceSysMenu(parent, pResultsData->hwndFolderPopup, 1);

         /* minimale Fensterbreite berechnen (Slider rechts) */
         WinQueryWindowPos(WinWindowFromID(parent, IDD_FINDRESULTS+9), &swp);
         pResultsData->minsize.x=swp.x+swp.cx+10;
         pResultsData->minsize.y=160;

         pResultsData->icon=LoadIcon(IDB_FIND);
         SendMsg(parent, WM_SETICON, (MPARAM) pResultsData->icon, (MPARAM) 0);
         WinEnableControl(parent, IDD_FINDRESULTS+2, FALSE);
         SetFont(WinWindowFromID(parent, IDD_FINDRESULTS+1), windowfonts.findresultsfont);
         SetForeground(WinWindowFromID(parent, IDD_FINDRESULTS+1), &windowcolors.resultsfore);
         SetBackground(WinWindowFromID(parent, IDD_FINDRESULTS+1), &windowcolors.resultsback);

         pResultsData->AutoScroll=resultsoptions.bScroll;
         WinCheckButton(parent, IDD_FINDRESULTS+5, pResultsData->AutoScroll);

         /* Vordergrund */
         if (resultsoptions.keepinfront)
         {
            pResultsData->bForeground=TRUE;
            WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_FGROUND, TRUE);
            WinSetOwner(parent, client);
         }
         else
         {
            pResultsData->bForeground=FALSE;
            WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_FGROUND, FALSE);
            WinSetOwner(parent, HWND_DESKTOP);
         }

         if (bSaveResults)
            WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_SAVE, TRUE);

         /* Container initialisieren */
         pFieldInfo = WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1,
                           CM_ALLOCDETAILFIELDINFO, MPFROMLONG(4), NULL);
         pfirstFieldInfo = pFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_RIGHT |
                              CFA_FIREADONLY | CFA_SEPARATOR;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pResultsData->pchTArea;
         LoadString(IDST_FD_AREA, 20, pFieldInfo->pTitleData);
         pFieldInfo->offStruct = FIELDOFFSET(RESULTRECORD, pchAreaTag);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_ULONG | CFA_HORZSEPARATOR | CFA_LEFT |
                              CFA_FIREADONLY /*| CFA_SEPARATOR*/;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pResultsData->pchTNumber;
         LoadString(IDST_FD_NUMBER, 20, pFieldInfo->pTitleData);
         pFieldInfo->offStruct = FIELDOFFSET(RESULTRECORD, ulMsgNum);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT |
                              CFA_FIREADONLY | CFA_SEPARATOR;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pResultsData->pchTFrom;
         LoadString(IDST_FD_FROM, 20, pFieldInfo->pTitleData);
         pFieldInfo->offStruct = FIELDOFFSET(RESULTRECORD, pchFrom);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT |
                              CFA_FIREADONLY;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pResultsData->pchTSubject;
         LoadString(IDST_FD_SUBJECT, 20, pFieldInfo->pTitleData);
         pFieldInfo->offStruct = FIELDOFFSET(RESULTRECORD, pchSubj);


         fieldInfoInsert.cb = (ULONG)(sizeof(FIELDINFOINSERT));
         fieldInfoInsert.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
         fieldInfoInsert.cFieldInfoInsert = 4;
         fieldInfoInsert.fInvalidateFieldInfo = TRUE;

         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INSERTDETAILFIELDINFO,
                           MPFROMP(pfirstFieldInfo),
                           MPFROMP(&fieldInfoInsert));

         cnrinfo.cb=sizeof(CNRINFO);
         cnrinfo.pFieldInfoLast=pfirstFieldInfo->pNextFieldInfo;
         cnrinfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES | CA_CONTAINERTITLE |
                              CA_TITLESEPARATOR;
         cnrinfo.xVertSplitbar=resultsoptions.lSplitBar;
         cnrinfo.pSortRecord=(PVOID)SortResults;
         cnrinfo.pszCnrTitle = pResultsData->pchCnrTitle;
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_SETCNRINFO, &cnrinfo,
                            MPFROMLONG(CMA_FLWINDOWATTR | CMA_PFIELDINFOLAST |
                                       CMA_XVERTSPLITBAR | CMA_PSORTRECORD | CMA_CNRTITLE));

         /* View */
         SwitchResultsView(parent, pResultsData, ((PBOOKMARKSOPEN)mp2)->ulView);
         SetInitialAccel(parent);
         RestoreWinPos(parent, &windowpositions.findresultspos, TRUE, TRUE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch(SHORT1FROMMP(mp1))
            {
               case DID_OK:
               case DID_CANCEL:
                  if (DoingFind)
                  {
                     return (MRESULT) FALSE;
                  }
                  WinPostMsg(client, FM_FINDRESULTSCLOSE, NULL, NULL);
                  break;

               /* Clear-Button */
               case IDD_FINDRESULTS+4:
                  pResultsData->bNoUpdate=TRUE;
                  pResultRecord=NULL;
                  while (pResultRecord = WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYRECORD,
                                                           pResultRecord,
                                                           MPFROM2SHORT(pResultRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
                  {
                     extern MARKERLIST MarkerList;
                     MESSAGEID MessageID;

                     UnmarkMessage(&MarkerList, pResultRecord->pchAreaTag, pResultRecord->ulMsgID, pResultsData->ulView);
                     strcpy(MessageID.pchAreaTag, pResultRecord->pchAreaTag);
                     MessageID.ulMsgID = pResultRecord->ulMsgID;
                     if (pResultsData->ulView == MARKFLAG_MANUAL)
                        SendMsg(client, WORKM_MSGUNMARKED, &MessageID, NULL);
                     else
                        SendMsg(parent, WORKM_DELETED, &MessageID, NULL);
                  }
                  CleanupResults(WinWindowFromID(parent, IDD_FINDRESULTS+1));
                  WinEnableControl(parent, IDD_FINDRESULTS+2, FALSE);
                  pResultsData->bNoUpdate=FALSE;
                  return (MRESULT) FALSE;

               /* Go to-Button */
               case IDD_FINDRESULTS+2:
                  if (pResultRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1,
                               CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_SELECTED)))
                  {
                     MESSAGEID MessageID;
                     JUMPINFO JumpInfo;

                     SetFocusControl(client, IDML_MAINEDIT);

                     strcpy(MessageID.pchAreaTag, pResultRecord->pchAreaTag);
                     MessageID.ulMsgID = pResultRecord->ulMsgID;
                     JumpInfo.pchText = pResultRecord->pchText;
                     JumpInfo.ulHow   = pResultRecord->ulHow;
                     JumpInfo.ulWhere = pResultRecord->ulWhere;
                     SendMsg(client, FM_JUMPTOMESSAGE, &MessageID, &JumpInfo);
                  }
                  return (MRESULT) FALSE;

               /* Stop-Button */
               case IDD_FINDRESULTS+10:
                  WinEnableControl(parent, IDD_FINDRESULTS+10, FALSE);
                  StopFind=TRUE;
                  return (MRESULT) FALSE;

               default:
                  return (MRESULT) FALSE;
            }
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_MENU)
         {
            PWORKDATA pWorkData;
            AREALISTPAR AreaListPar;
            extern int tidWorker;
            extern BOOL bDoingWork;
            extern GENERALOPT generaloptions;
            extern char CurrentArea[LEN_AREATAG+1];
            extern PATHNAMES pathnames;
            extern ULONG ulExportOptions;

            switch(SHORT1FROMMP(mp1))
            {
               case IDM_BMFP_FGROUND:
                  if (pResultsData->bForeground)
                  {
                     pResultsData->bForeground=FALSE;
                     WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_FGROUND, FALSE);
                     WinSetOwner(parent, HWND_DESKTOP);
                  }
                  else
                  {
                     pResultsData->bForeground=TRUE;
                     WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_FGROUND, TRUE);
                     WinSetOwner(parent, client);
                  }
                  return (MRESULT) FALSE;

               case IDM_BMFP_VIEW_FIND:
                  SwitchResultsView(parent, pResultsData, MARKFLAG_FIND);
                  return (MRESULT) FALSE;

               case IDM_BMFP_VIEW_PERS:
                  SwitchResultsView(parent, pResultsData, MARKFLAG_PERSMAIL);
                  return (MRESULT) FALSE;

               case IDM_BMFP_VIEW_MARK:
                  SwitchResultsView(parent, pResultsData, MARKFLAG_MANUAL);
                  return (MRESULT) FALSE;

               case IDM_BMFP_VIEW_UNSENT:
                  SwitchResultsView(parent, pResultsData, MARKFLAG_UNSENT);
                  return (MRESULT) FALSE;

               case IDM_BMFP_SAVE:
                  bSaveResults = !bSaveResults;
                  WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_SAVE, bSaveResults);
                  dirtyflags.finddirty = TRUE;
                  return (MRESULT) FALSE;

               case IDM_BMP_DELETE:
                  if (generaloptions.safety & SAFETY_DELMSG)
                  {
                     if (MessageBox(parent, IDST_MSG_DELETE, IDST_TITLE_DELETE,
                                    IDD_DELETE, MB_YESNO | MB_ICONEXCLAMATION)!=MBID_YES)
                     {
                        return (MRESULT) FALSE;
                     }
                  }
                  if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_DELETE, NULL, NULL, 0))
                  {
                     bDoingWork=TRUE;
                     tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_EXPORT:
                  /* Filenamen holen */
                  if (GetExportName(parent, pathnames.lastexport, &ulExportOptions))
                  {
                     if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_EXPORT, NULL, pathnames.lastexport, 0))
                     {
                        pWorkData->ulExportOptions = ulExportOptions;
                        bDoingWork=TRUE;
                        tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                     }
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_PRINT:
                  if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_PRINT, NULL, NULL, 0))
                  {
                     bDoingWork=TRUE;
                     tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_COPY:
                  /* ZielArea holen */
                  AreaListPar.cb=sizeof(AREALISTPAR);
                  if (generaloptions.LastCopyArea[0])
                     AreaListPar.pchString=strdup(generaloptions.LastCopyArea);
                  else
                     AreaListPar.pchString=strdup(CurrentArea);
                  AreaListPar.idTitle = IDST_TITLE_AL_COPY;
                  AreaListPar.ulIncludeTypes = INCLUDE_ALL;
                  AreaListPar.bExtendedSel = FALSE;
                  AreaListPar.bChange      = FALSE;

                  if (WinDlgBox(HWND_DESKTOP, parent,
                                AreaListProc, hmodLang,
                                IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
                  {
                     ULONG ulOptions=0;
                     AREADEFLIST *pArea;

                     pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
                     if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
                        switch(MessageBox(parent, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                          IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                        {
                           case MBID_YES:
                              ulOptions = COPYMOVE_RESEND;
                              break;

                           default:
                              break;
                        }

                     if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_COPY, AreaListPar.pchString, NULL, ulOptions))
                     {
                        bDoingWork=TRUE;
                        tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                     }
                     else
                        free(AreaListPar.pchString);
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_MOVE:
                  /* ZielArea holen */
                  AreaListPar.cb=sizeof(AREALISTPAR);
                  if (generaloptions.LastMoveArea[0])
                     AreaListPar.pchString=strdup(generaloptions.LastMoveArea);
                  else
                     AreaListPar.pchString=strdup(CurrentArea);
                  AreaListPar.idTitle = IDST_TITLE_AL_MOVE;
                  AreaListPar.ulIncludeTypes = INCLUDE_ALL;
                  AreaListPar.bExtendedSel = FALSE;
                  AreaListPar.bChange      = FALSE;

                  if (WinDlgBox(HWND_DESKTOP, parent,
                                AreaListProc, hmodLang,
                                IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
                  {
                     ULONG ulOptions=0;
                     AREADEFLIST *pArea;

                     pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
                     if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
                        switch(MessageBox(parent, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                          IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                        {
                           case MBID_YES:
                              ulOptions = COPYMOVE_RESEND;
                              break;

                           default:
                              break;
                        }

                     if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_MOVE, AreaListPar.pchString, NULL, ulOptions))
                     {
                        bDoingWork=TRUE;
                        tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                     }
                     else
                        free(AreaListPar.pchString);
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_REMOVE:
                  if (pWorkData = CollectBookmarks(parent, pResultsData, 0, NULL, NULL, 0))
                  {
                     PWORKDATA pWorkData2;
                     MESSAGEID MessageID;
                     extern MARKERLIST MarkerList;

                     while (pWorkData)
                     {
                        int i;
                        strcpy(MessageID.pchAreaTag, pWorkData->pchSrcArea);

                        for (i=0; i<pWorkData->ulArraySize; i++)
                        {
                           UnmarkMessage(&MarkerList, pWorkData->pchSrcArea,
                                         pWorkData->MsgIDArray[i], pResultsData->ulView);

                           strcpy(MessageID.pchAreaTag, pWorkData->pchSrcArea);
                           MessageID.ulMsgID = pWorkData->MsgIDArray[i];
                           if (pResultsData->ulView == MARKFLAG_MANUAL)
                              SendMsg(client, WORKM_MSGUNMARKED, &MessageID, NULL);
                           else
                              SendMsg(parent, WORKM_DELETED, &MessageID, NULL);
                        }
                        pWorkData2=pWorkData;
                        pWorkData = pWorkData->next;
                        free(pWorkData2->MsgIDArray);
                        free(pWorkData2);
                     }
                  }
                  return (MRESULT) FALSE;

               case IDM_BMP_SELECTALL:
                  SelectAllRecords(WinWindowFromID(parent, IDD_FINDRESULTS+1));
                  return (MRESULT) FALSE;

               case IDM_BMP_SELECTNONE:
                  DeselectAllRecords(WinWindowFromID(parent, IDD_FINDRESULTS+1));
                  return (MRESULT) FALSE;

               default:
                  return (MRESULT) FALSE;
            }
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_ACCELERATOR)
         {
            PWORKDATA pWorkData;
            extern int tidWorker;
            extern BOOL bDoingWork;
            extern GENERALOPT generaloptions;

            switch(SHORT1FROMMP(mp1))
            {
               case IDA_DELMSG:
                  /* Cursor-Record feststellen */
                  pResultRecord = WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1,
                                                    CM_QUERYRECORDEMPHASIS,
                                                    MPFROMLONG(CMA_FIRST),
                                                    MPFROMLONG(CRA_CURSORED));

                  if (pResultRecord)
                  {
                     /* Abfrage */
                     if (generaloptions.safety & SAFETY_DELMSG)
                     {
                        if (MessageBox(parent, IDST_MSG_DELETE, IDST_TITLE_DELETE,
                                       IDD_DELETE, MB_YESNO | MB_ICONEXCLAMATION)!=MBID_YES)
                        {
                           return (MRESULT) FALSE;
                        }
                     }
                     pResultsData->pPopupRecord = pResultRecord;
                     if (pWorkData = CollectBookmarks(parent, pResultsData, WORK_DELETE, NULL, NULL, 0))
                     {
                        bDoingWork=TRUE;
                        tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                     }
                  }
                  return (MRESULT) FALSE;

               default:
                  return RedirectCommand(mp1, mp2);
            }
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_FINDRESULTS+1)
         {
            switch (SHORT2FROMMP(mp1))
            {
               /* Enter gedrueckt oder Doppelklick */
               case CN_ENTER:
                  if (((PNOTIFYRECORDENTER)mp2)->pRecord && !pResultsData->ulDisable)
                  {
                     MESSAGEID MessageID;
                     JUMPINFO JumpInfo;

                     pResultRecord = (RESULTRECORD*)((PNOTIFYRECORDENTER)mp2)->pRecord;
                     SetFocusControl(client, IDML_MAINEDIT);

                     strcpy(MessageID.pchAreaTag, pResultRecord->pchAreaTag);
                     MessageID.ulMsgID = pResultRecord->ulMsgID;
                     JumpInfo.pchText = pResultRecord->pchText;
                     JumpInfo.ulHow   = pResultRecord->ulHow;
                     JumpInfo.ulWhere = pResultRecord->ulWhere;
                     SendMsg(client, FM_JUMPTOMESSAGE, &MessageID, &JumpInfo);
                  }
                  break;

               case CN_HELP:
                  SendMsg(parent, WM_HELP, MPFROMSHORT(IDD_FINDRESULTS+1), NULL);
                  break;

               case CN_CONTEXTMENU:
                  BookmarksPopup(parent, pResultsData, (RESULTRECORD*) mp2);
                  break;

               default:
                  break;
            }
         }
         if (SHORT1FROMMP(mp1)==IDD_FINDRESULTS+5)
            pResultsData->AutoScroll=WinQueryButtonCheckstate(parent, IDD_FINDRESULTS+5);

         if (SHORT1FROMMP(mp1)==IDD_FINDRESULTS+6)
         {
            if (WinQueryButtonCheckstate(parent, IDD_FINDRESULTS+6))
               WinSetOwner(parent, client);
            else
               WinSetOwner(parent, HWND_DESKTOP);
         }
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent, message, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=pResultsData->minsize.x;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=pResultsData->minsize.y;
         return resultbuf;

      case WM_CLOSE:
         if (DoingFind)
         {
            return 0;
         }
         WinPostMsg(client, FM_FINDRESULTSCLOSE, NULL, NULL);
         break;

      case WM_DESTROY:
         RemoveFromWindowList(pResultsData->hSwitch);
         CleanupResults(WinWindowFromID(parent, IDD_FINDRESULTS+1));
         WinDestroyPointer(pResultsData->icon);
         if (pResultsData->hwndPopup)
            WinDestroyWindow(pResultsData->hwndPopup);
         if (pResultsData->hwndFolderPopup)
            WinDestroyWindow(pResultsData->hwndFolderPopup);
         QueryWinPos(parent, &windowpositions.findresultspos);
         QueryFont(WinWindowFromID(parent, IDD_FINDRESULTS+1), windowfonts.findresultsfont);
         QueryForeground(WinWindowFromID(parent, IDD_FINDRESULTS+1), &windowcolors.resultsfore);
         QueryBackground(WinWindowFromID(parent, IDD_FINDRESULTS+1), &windowcolors.resultsback);
         if (resultsoptions.bScroll != pResultsData->AutoScroll)
         {
            resultsoptions.bScroll = pResultsData->AutoScroll;
            dirtyflags.resultsdirty=TRUE;
         }
         if (resultsoptions.keepinfront != pResultsData->bForeground)
         {
            resultsoptions.keepinfront = pResultsData->bForeground;
            dirtyflags.resultsdirty=TRUE;
         }
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYCNRINFO,
                           &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
         if (resultsoptions.lSplitBar != cnrinfo.xVertSplitbar)
         {
            resultsoptions.lSplitBar = cnrinfo.xVertSplitbar;
            dirtyflags.resultsdirty=TRUE;
         }
         free(pResultsData);
         break;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
         {
            WinShowWindow(WinWindowFromID(parent, IDD_FINDRESULTS+2), FALSE);
         }
         if (((PSWP)mp1)->fl & SWP_RESTORE)
            WinShowWindow(WinWindowFromID(parent, IDD_FINDRESULTS+2), TRUE);

         if (((PSWP)mp1)->fl & SWP_SIZE)
            ResizeResults(parent, (PSWP)mp1);
         break;

      case WM_MENUEND:
         if ((HWND) mp2 != pResultsData->hwndPopup &&
             (HWND) mp2 != pResultsData->hwndFolderPopup)
            break;
         pResultsData->bKeyboard=FALSE;

         if ((HWND) mp2 == pResultsData->hwndFolderPopup)
            ResetMenuStyle(pResultsData->hwndFolderPopup, parent);

         /* Source-Emphasis loeschen */
         pResultRecord=(RESULTRECORD *)WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1,
                     CM_QUERYRECORDEMPHASIS, (MPARAM) CMA_FIRST,
                     MPFROMSHORT(CRA_SOURCE));
         while (pResultRecord)
         {
            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_SETRECORDEMPHASIS,
                        pResultRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
            pResultRecord=(RESULTRECORD *)WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1,
                        CM_QUERYRECORDEMPHASIS, pResultRecord,
                        MPFROMSHORT(CRA_SOURCE));
         }
         if (pResultsData->pPopupRecord==NULL)
            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_SETRECORDEMPHASIS,
                              NULL, MPFROM2SHORT(FALSE, CRA_SOURCE));
         break;

      case WM_CONTEXTMENU:
         if (WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(parent, IDD_FINDRESULTS+1) &&
             !SHORT1FROMMP(mp1))
         {
            pResultsData->bKeyboard = TRUE;
            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, message, mp1, mp2);
         }
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pResultsData->hwndFolderPopup)
            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_SETRECORDEMPHASIS,
                              NULL, MPFROM2SHORT(TRUE, CRA_SOURCE));
         break;

      case WORKM_STARTFIND:
         WinEnableControl(parent, IDD_FINDRESULTS+10, TRUE);
         WinEnableControl(parent, DID_OK, FALSE);
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+9, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                           MPFROMLONG(0));
         WinSetDlgItemText(parent, IDD_FINDRESULTS+8, "");
         SetFocusControl(parent, IDD_FINDRESULTS+1);
         SwitchResultsView(parent, pResultsData, (ULONG) mp1);
         break;

      case WORKM_STOPFIND:
         WinEnableControl(parent, IDD_FINDRESULTS+10, FALSE);
         WinEnableControl(parent, DID_OK, TRUE);
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+9, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                           MPFROMLONG(0));
         WinSetDlgItemText(parent, IDD_FINDRESULTS+8, "");
         WinSetWindowPos(parent, HWND_TOP,
                         0, 0,
                         0, 0,
                         SWP_ZORDER);
         break;

      case WORKM_FINDAREA:
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+9, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                           MPFROMLONG(0));
         WinSetDlgItemText(parent, IDD_FINDRESULTS+8, (PCHAR) mp1);
         break;

      case WORKM_FINDPROGRESS:
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+9, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                           MPFROMLONG(((SHORT)mp1)/5));
         break;

      case WORKM_FINDAREAEND:
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+9, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                           MPFROMLONG(19));

         /* Ergebnisse einfuegen */
         pResultList= (PFINDRESULTLIST) mp1;
         pFoundMsg=   (PFOUNDMSG) mp2;

         if (pResultList->ulFindType & pResultsData->ulView)
         {
            pFirstRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_ALLOCRECORD,
                                     MPFROMLONG(sizeof(RESULTRECORD)-sizeof(MINIRECORDCORE)),
                                     MPFROMLONG(pResultList->ulFoundMsgs));
            pResultRecord = pFirstRecord;
            while (pFoundMsg)
            {
               pResultRecord->pchAreaTag=malloc(LEN_AREATAG+1);
               strcpy(pResultRecord->pchAreaTag, pResultList->pchAreaTag);
               pResultRecord->ulMsgNum = pFoundMsg->ulMsgNum;
               pResultRecord->ulMsgID  = pFoundMsg->ulMsgID;
               pResultRecord->pchFrom=malloc(LEN_USERNAME+1);
               strcpy(pResultRecord->pchFrom, pFoundMsg->Header.pchFromName);
               pResultRecord->pchSubj=malloc(LEN_SUBJECT+1);
               strcpy(pResultRecord->pchSubj, pFoundMsg->Header.pchSubject);
               if (pFoundMsg->pchFindText)
                  pResultRecord->pchText=strdup(pFoundMsg->pchFindText);
               else
                  pResultRecord->pchText=NULL;
               pResultRecord->ulFlags = pResultList->ulFindType;
               pResultRecord->ulHow   = pFoundMsg->ulHow;
               pResultRecord->ulWhere = pFoundMsg->ulWhere;

               pResultRecord = (RESULTRECORD*) pResultRecord->RecordCore.preccNextRecord;
               pFoundMsg = pFoundMsg->next;
            }

            RecordInsert.cb=sizeof(RECORDINSERT);
            RecordInsert.pRecordOrder=(RECORDCORE *)CMA_END;
            RecordInsert.pRecordParent=NULL;
            RecordInsert.zOrder=CMA_TOP;
            RecordInsert.fInvalidateRecord=FALSE;
            RecordInsert.cRecordsInsert=pResultList->ulFoundMsgs;

            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INSERTRECORD,
                              pFirstRecord, &RecordInsert);
            WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INVALIDATERECORD,
                              NULL, NULL);

#if 0
            if (pResultsData->AutoScroll)
            {
               QUERYRECORDRECT qrecord;

               /* Auto-Scroll */
               qrecord.cb=sizeof(QUERYRECORDRECT);
               qrecord.pRecord=(RECORDCORE *)pResultRecord;
               qrecord.fRightSplitWindow=FALSE;
               qrecord.fsExtent=CMA_TEXT;
               WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYRECORDRECT,
                                 &rectl, &qrecord);
               WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_SCROLLWINDOW,
                                 MPFROMSHORT(CMA_VERTICAL),
                                 MPFROMLONG(rectl.yTop-rectl.yBottom));
            }
#endif
            UpdateGotoButton(parent, pResultsData);
         }
         break;

      case WORKM_DELETED:
      case WORKM_MSGUNMARKED:
         if (pResultsData->bNoUpdate)
            break;
         if (message == WORKM_MSGUNMARKED && pResultsData->ulView != MARKFLAG_MANUAL)
            break;

         pResultRecord=NULL;
         while (pResultRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYRECORD,
                                                pResultRecord,
                                                MPFROM2SHORT(pResultRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
         {
            if (pResultRecord->ulMsgID == ((PMESSAGEID)mp1)->ulMsgID &&
                !stricmp(pResultRecord->pchAreaTag, ((PMESSAGEID)mp1)->pchAreaTag))
            {
               if (pResultRecord->pchFrom)
                  free(pResultRecord->pchFrom);
               if (pResultRecord->pchSubj)
                  free(pResultRecord->pchSubj);
               if (pResultRecord->pchAreaTag)
                  free(pResultRecord->pchAreaTag);
               if (pResultRecord->pchText)
                  free(pResultRecord->pchText);

               WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_REMOVERECORD,
                                 &pResultRecord, MPFROM2SHORT(1, CMA_FREE|CMA_INVALIDATE));
               UpdateGotoButton(parent, pResultsData);
               break;
            }
         }
         break;

      case WORKM_CHANGED:
         pResultRecord=NULL;
         while (pResultRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYRECORD,
                                                pResultRecord,
                                                MPFROM2SHORT(pResultRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
         {
            if (pResultRecord->ulMsgID == ((PMESSAGEID)mp1)->ulMsgID &&
                !stricmp(pResultRecord->pchAreaTag, ((PMESSAGEID)mp1)->pchAreaTag))
            {
               strcpy(pResultRecord->pchFrom, ((PMSGHEADER)mp2)->pchFromName);
               strcpy(pResultRecord->pchSubj, ((PMSGHEADER)mp2)->pchSubject);

               WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INVALIDATERECORD,
                                 &pResultRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               break;
            }
         }
         break;

      case WORKM_MSGMARKED:
         if (pResultsData->ulView != MARKFLAG_MANUAL)
            break;

         pResultRecord=NULL;
         while (pResultRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_QUERYRECORD,
                                                pResultRecord,
                                                MPFROM2SHORT(pResultRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
         {
            if (pResultRecord->ulMsgID == ((PMESSAGEID)mp1)->ulMsgID &&
                !stricmp(pResultRecord->pchAreaTag, ((PMESSAGEID)mp1)->pchAreaTag))
               /* schon vorhanden */
               return (MRESULT) FALSE;
         }
         /* neuen Record */
         pResultRecord=WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_ALLOCRECORD,
                                  MPFROMLONG(sizeof(RESULTRECORD)-sizeof(MINIRECORDCORE)),
                                  MPFROMLONG(1));
         pResultRecord->pchAreaTag=malloc(LEN_AREATAG+1);
         strcpy(pResultRecord->pchAreaTag, ((PMESSAGEID)mp1)->pchAreaTag);
         pResultRecord->ulMsgNum = 0; /*@@*/
         pResultRecord->ulMsgID  = ((PMESSAGEID)mp1)->ulMsgID;
         pResultRecord->pchFrom=malloc(LEN_USERNAME+1);
         strcpy(pResultRecord->pchFrom, ((PMSGHEADER)mp2)->pchFromName);
         pResultRecord->pchSubj=malloc(LEN_SUBJECT+1);
         strcpy(pResultRecord->pchSubj, ((PMSGHEADER)mp2)->pchSubject);
         pResultRecord->pchText=NULL;
         pResultRecord->ulFlags = MARKFLAG_MANUAL;
         pResultRecord->ulHow = 0;
         pResultRecord->ulWhere = 0;

         RecordInsert.cb=sizeof(RECORDINSERT);
         RecordInsert.pRecordOrder=(RECORDCORE *)CMA_END;
         RecordInsert.pRecordParent=NULL;
         RecordInsert.zOrder=CMA_TOP;
         RecordInsert.fInvalidateRecord=FALSE;
         RecordInsert.cRecordsInsert=1;

         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INSERTRECORD,
                           pResultRecord, &RecordInsert);
         WinSendDlgItemMsg(parent, IDD_FINDRESULTS+1, CM_INVALIDATERECORD,
                           NULL, NULL);
         UpdateGotoButton(parent, pResultsData);
         break;

      case WORKM_DISABLEVIEWS:
         pResultsData->ulDisable++;
         UpdateGotoButton(parent, pResultsData);
         break;

      case WORKM_ENABLEVIEWS:
         if (pResultsData->ulDisable)
         {
            pResultsData->ulDisable--;
            UpdateGotoButton(parent, pResultsData);
         }
         break;

      case WORKM_SWITCHACCELS:
         SwitchAccels(parent, (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

static void ResizeResults(HWND hwndDlg, PSWP pNewSize)
{
   POINTL size;
   POINTL pos;
   SWP swp, swp2;
   RECTL rectl;
   extern HAB anchor;

   rectl.xLeft=0;
   rectl.xRight=pNewSize->cx;
   rectl.yBottom=0;
   rectl.yTop=pNewSize->cy;

   CalcClientRect(anchor, hwndDlg, &rectl);

   size.x=rectl.xRight-rectl.xLeft;
   size.y=rectl.yTop-rectl.yBottom;
   WinMapDlgPoints(hwndDlg, &size, 1, FALSE);

   /* Container */
   pos.x=5;                 /* linke untere Ecke */
   pos.y=23;
   size.x=pos.x+size.x-10;  /* rechte obere Ecke */
   size.y=pos.y+size.y-55;
   WinMapDlgPoints(hwndDlg, &pos, 1, TRUE);
   WinMapDlgPoints(hwndDlg, &size, 1, TRUE);
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1),
                   NULLHANDLE,
                   rectl.xLeft/* /2 */, pos.y,
                   rectl.xRight-rectl.xLeft, rectl.yTop-pos.y-30,
                   SWP_MOVE | SWP_SIZE);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1), &swp);
   /* "Area" */
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+7),
                   NULLHANDLE,
                   swp.x, swp.y+swp.cy,
                   0, 0,
                   SWP_MOVE);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+7), &swp);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+9), &swp2);
   /* Laufbalken */
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+9),
                   NULLHANDLE,
                   pNewSize->cx - swp2.cx - 10, swp.y,
                   0, 0,
                   SWP_MOVE);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+9), &swp2);
   /* Area-Name */
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+8),
                   NULLHANDLE,
                   swp.x+swp.cx, swp.y,
                   swp2.x - swp.x - swp.cx, swp.cy,
                   SWP_MOVE | SWP_SIZE);

   return;
}

/*------------------------------ CleanupResults   ---------------------------*/
/* Loescht alle Records aus der Ergebnisliste                                */
/*---------------------------------------------------------------------------*/

static void CleanupResults(HWND hwndContainer)
{
   RESULTRECORD *pRecord=NULL;

   while(pRecord=(RESULTRECORD *)SendMsg(hwndContainer, CM_QUERYRECORD,
                                  pRecord, MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->pchFrom)
         free(pRecord->pchFrom);
      if (pRecord->pchSubj)
         free(pRecord->pchSubj);
      if (pRecord->pchAreaTag)
         free(pRecord->pchAreaTag);
      if (pRecord->pchText)
         free(pRecord->pchText);
   }
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL,
              MPFROM2SHORT(0, CMA_FREE|CMA_INVALIDATE));
   return;
}

static void UpdateGotoButton(HWND hwndDlg, PRESULTSDATA pResultsData)
{
   CNRINFO cnrinfo;

   WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_QUERYCNRINFO,
                     &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
   if (cnrinfo.cRecords && !pResultsData->ulDisable)
      WinEnableControl(hwndDlg, IDD_FINDRESULTS+2, TRUE);
   else
      WinEnableControl(hwndDlg, IDD_FINDRESULTS+2, FALSE);

   return;
}

static int BookmarksPopup(HWND hwndDlg, PRESULTSDATA pResultsData, RESULTRECORD *pRecord)
{
   POINTL ptlPointer;
   extern BOOL bDoingWork;

   WinQueryPointerPos(HWND_DESKTOP, &ptlPointer);

   if (pRecord)
   {
      if (pResultsData->ulDisable || bDoingWork)
         return 1;
      if (pRecord->RecordCore.flRecordAttr & CRA_SELECTED)
      {
         /* Alle selektierten */
         RESULTRECORD *pRecord2=NULL;

         while (pRecord2 = WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_QUERYRECORDEMPHASIS,
                                             MPFROMP(pRecord2?pRecord2:(RESULTRECORD*)CMA_FIRST),
                                             MPFROMSHORT(CRA_SELECTED)))
         {
            WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_SETRECORDEMPHASIS,
                              pRecord2, MPFROM2SHORT(TRUE, CRA_SOURCE));
         }
      }
      else
         WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_SETRECORDEMPHASIS,
                           pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));
      if (pResultsData->bKeyboard)
      {
         QUERYRECORDRECT qrecord;
         RECTL rcl;
         SWP swp;

         qrecord.cb = sizeof(QUERYRECORDRECT);
         qrecord.pRecord = (PRECORDCORE) pRecord;
         qrecord.fRightSplitWindow = TRUE;
         qrecord.fsExtent = CMA_TEXT;
         WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_QUERYRECORDRECT,
                           &rcl, &qrecord);
         ptlPointer.y = (rcl.yBottom + rcl.yTop)/2;
         WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1),
                           &swp);
         ptlPointer.x = swp.cx/2;
         WinMapWindowPoints(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1),
                            HWND_DESKTOP, &ptlPointer, 1);
      }
      WinPopupMenu(HWND_DESKTOP, hwndDlg, pResultsData->hwndPopup,
                   ptlPointer.x, ptlPointer.y,
                   0,
                   PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }
   else
   {
      if (pResultsData->bKeyboard)
      {
         SWP swp;

         WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1),
                           &swp);
         ptlPointer.x = swp.cx/2;
         ptlPointer.y = swp.cy/2;
         WinMapWindowPoints(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1),
                            HWND_DESKTOP, &ptlPointer, 1);
      }
      WinPopupMenu(HWND_DESKTOP, hwndDlg, pResultsData->hwndFolderPopup,
                   ptlPointer.x, ptlPointer.y,
                   0,
                   PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }

   pResultsData->pPopupRecord = pRecord;

   return 0;
}

static void SwitchResultsView(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulNewView)
{
   ULONG ulMenuID=0;

   if (pResultsData->ulView == ulNewView)
      return;

   pResultsData->ulView = ulNewView;

   switch(ulNewView)
   {
      case MARKFLAG_FIND:
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_FIND, TRUE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_PERS, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_MARK, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_UNSENT, FALSE);
         ulMenuID = IDM_BMFP_VIEW_FIND;
         break;

      case MARKFLAG_PERSMAIL:
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_FIND, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_PERS, TRUE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_MARK, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_UNSENT, FALSE);
         ulMenuID = IDM_BMFP_VIEW_PERS;
         break;

      case MARKFLAG_MANUAL:
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_FIND, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_PERS, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_MARK, TRUE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_UNSENT, FALSE);
         ulMenuID = IDM_BMFP_VIEW_MARK;
         break;

      case MARKFLAG_UNSENT:
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_FIND, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_PERS, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_MARK, FALSE);
         WinCheckMenuItem(pResultsData->hwndFolderPopup, IDM_BMFP_VIEW_UNSENT, TRUE);
         ulMenuID = IDM_BMFP_VIEW_UNSENT;
         break;
   }
   SetResultsTitle(hwndDlg, pResultsData, ulMenuID);

   FillResultsWindow(hwndDlg, pResultsData);

   return;
}

static void SetResultsTitle(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulMenuID)
{
   char *pchSrc, *pchDest;
   CNRINFO cnrinfo;

   WinSendMsg(pResultsData->hwndFolderPopup, MM_QUERYITEMTEXT,
              MPFROM2SHORT(ulMenuID, sizeof(pResultsData->pchCnrTitle)),
              pResultsData->pchCnrTitle);
   /* Prefix loeschen */
   pchSrc = pchDest = pResultsData->pchCnrTitle;
   while(*pchSrc)
      if (*pchSrc != '~')
         *pchDest++ = *pchSrc++;
      else
         pchSrc++;
   *pchDest = 0;

   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.pszCnrTitle = pResultsData->pchCnrTitle;
   WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_SETCNRINFO, &cnrinfo,
                     MPFROMLONG(CMA_CNRTITLE));

   return;
}

static void FillResultsWindow(HWND hwndDlg, PRESULTSDATA pResultsData)
{
   extern MARKERLIST MarkerList;
   PMARKERBUCKET pBucket=NULL;
   PMARKERAREA   pArea=NULL;
   RESULTRECORD *pRecord, *pFirstRecord;
   RECORDINSERT RecordInsert;

   CleanupResults(WinWindowFromID(hwndDlg, IDD_FINDRESULTS+1));

   WinRequestMutexSem(MarkerList.hmtxAccess, SEM_INDEFINITE_WAIT);

   /* Alle Areas abklappern */
   pArea = MarkerList.pAreas;
   while(pArea)
   {
      /* Items zaehlen */
      ULONG ulItemCount=0;

      pBucket = pArea->pBuckets;
      while(pBucket)
      {
         int i;

         for (i=0; i<pBucket->ulCountItems; i++)
            if (pBucket->aItems[i].ulFlags & pResultsData->ulView)
               ulItemCount++;

         pBucket = pBucket->next;
      }

      if (ulItemCount)
      {
         /* Items einfuegen */
         pFirstRecord=WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_ALLOCRECORD,
                                  MPFROMLONG(sizeof(RESULTRECORD)-sizeof(MINIRECORDCORE)),
                                  MPFROMLONG(ulItemCount));
         pRecord = pFirstRecord;

         pBucket = pArea->pBuckets;
         while(pBucket)
         {
            int i;

            for (i=0; i<pBucket->ulCountItems; i++)
               if (pBucket->aItems[i].ulFlags & pResultsData->ulView)
               {
                  pRecord->pchAreaTag=malloc(LEN_AREATAG+1);
                  strcpy(pRecord->pchAreaTag, pArea->pchAreaTag);
                  pRecord->ulMsgNum = pBucket->aItems[i].ulMsgNr;
                  pRecord->ulMsgID  = pBucket->aItems[i].ulMsgID;
                  pRecord->pchFrom=malloc(LEN_USERNAME+1);
                  strcpy(pRecord->pchFrom, pBucket->aItems[i].pchFrom);
                  pRecord->pchSubj=malloc(LEN_SUBJECT+1);
                  strcpy(pRecord->pchSubj, pBucket->aItems[i].pchSubj);
                  if (pBucket->aItems[i].pchFindText[0])
                     pRecord->pchText=strdup(pBucket->aItems[i].pchFindText);
                  else
                     pRecord->pchText=NULL;
                  pRecord->ulFlags = pBucket->aItems[i].ulFlags;
                  pRecord->ulHow   = pBucket->aItems[i].ulHow;
                  pRecord->ulWhere = pBucket->aItems[i].ulWhere;

                  pRecord = (RESULTRECORD*) pRecord->RecordCore.preccNextRecord;
               }
            pBucket = pBucket->next;
         }

         RecordInsert.cb=sizeof(RECORDINSERT);
         RecordInsert.pRecordOrder=(RECORDCORE *)CMA_END;
         RecordInsert.pRecordParent=NULL;
         RecordInsert.zOrder=CMA_TOP;
         RecordInsert.fInvalidateRecord=FALSE;
         RecordInsert.cRecordsInsert=ulItemCount;

         WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_INSERTRECORD,
                           pFirstRecord, &RecordInsert);
      }

      pArea=pArea->next;
   }
   WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_INVALIDATERECORD,
                     NULL, NULL);

   DosReleaseMutexSem(MarkerList.hmtxAccess);

   UpdateGotoButton(hwndDlg, pResultsData);

   return;
}

static PWORKDATA CollectBookmarks(HWND hwndDlg, PRESULTSDATA pResultsData, ULONG ulWork,
                                  PCHAR pchDestArea, PCHAR pchDestFile, ULONG ulOptions)
{
   char pchLastArea[LEN_AREATAG+1]="#";
   PWORKDATA pWorkData=NULL, pWorkData2=NULL;
   RESULTRECORD *pRecord=NULL;
   CNRINFO cnrinfo;
   ULONG ulIndex=0;

   if (pResultsData->pPopupRecord->RecordCore.flRecordAttr & CRA_SELECTED)
   {
      /* Anzahl der Records abfragen */
      WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_QUERYCNRINFO,
                        &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));

      while (pRecord = WinSendDlgItemMsg(hwndDlg, IDD_FINDRESULTS+1, CM_QUERYRECORDEMPHASIS,
                                         pRecord?pRecord:(PVOID)CMA_FIRST,
                                         MPFROMSHORT(CRA_SELECTED)))
      {
         if (stricmp(pchLastArea, pRecord->pchAreaTag))
         {
            /* Neue Area */
            strcpy(pchLastArea, pRecord->pchAreaTag);

            if (pWorkData)
            {
               pWorkData->next = calloc(1, sizeof(WORKDATA));
               pWorkData = pWorkData->next;
            }
            else
            {
               pWorkData = calloc(1, sizeof(WORKDATA));
               pWorkData2 = pWorkData;
            }

            pWorkData->flWorkToDo = ulWork;
            pWorkData->ulCopyMove = ulOptions;
            memcpy(pWorkData->pchSrcArea, pRecord->pchAreaTag, LEN_AREATAG);
            if (pchDestArea)
               strcpy(pWorkData->pchDestArea, pchDestArea);
            else
               strcpy(pWorkData->pchDestArea, pRecord->pchAreaTag);
            if (pchDestFile)
               strcpy(pWorkData->pchDestFile, pchDestFile);

            pWorkData->MsgIDArray=calloc(cnrinfo.cRecords, sizeof(ULONG));
            ulIndex=0;
         }

         /* ID eintragen */
         pWorkData->MsgIDArray[ulIndex++]=pRecord->ulMsgID;
         pWorkData->ulArraySize=ulIndex;
      }
   }
   else
   {
      /* nur einen Record */
      pWorkData2 = calloc(1, sizeof(WORKDATA));
      pWorkData2->flWorkToDo = ulWork;
      pWorkData2->ulCopyMove = ulOptions;
      memcpy(pWorkData2->pchSrcArea, pResultsData->pPopupRecord->pchAreaTag, LEN_AREATAG);
      if (pchDestArea)
         strcpy(pWorkData2->pchDestArea, pchDestArea);
      if (pchDestFile)
         strcpy(pWorkData2->pchDestFile, pchDestFile);

      pWorkData2->MsgIDArray=calloc(1, sizeof(ULONG));

      /* ID eintragen */
      pWorkData2->MsgIDArray[0]=pResultsData->pPopupRecord->ulMsgID;
      pWorkData2->ulArraySize=1;
   }

   return pWorkData2;
}

static SHORT _System SortResults(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
{
   int res=0;

   pStorage=pStorage;

   res = stricmp(((RESULTRECORD*)p1)->pchAreaTag, ((RESULTRECORD*)p2)->pchAreaTag);

   if (res)
      return (SHORT) res;
   else
   {
      if (((RESULTRECORD*)p1)->ulMsgNum < ((RESULTRECORD*)p2)->ulMsgNum)
         return -1;
      else
         return 1;
   }
}

MRESULT EXPENTRY SearchProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PSEARCHPAR pSearchPar = WinQueryWindowPtr(hwnd, QWL_USER);
   extern HWND hwndhelp;

   switch(message)
   {
      case WM_INITDLG:
         pSearchPar = (PSEARCHPAR) mp2;
         WinSetWindowPtr(hwnd, QWL_USER, pSearchPar);

         /* Text */
         WinSendDlgItemMsg(hwnd, IDD_SEARCH, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_SEARCHTEXT), NULL);
         WinSetDlgItemText(hwnd, IDD_SEARCH+2, pSearchPar->pchSearchText);
         if (pSearchPar->pchSearchText[0])
         {
            WinEnableControl(hwnd, DID_OK, TRUE);
            WinSendDlgItemMsg(hwnd, IDD_SEARCH+2, EM_SETSEL,
                              MPFROM2SHORT(0, LEN_SEARCHTEXT), NULL);
         }
         /* Flags */
         if (pSearchPar->ulSearchFlags & SEARCHFLAG_CASESENSITIVE)
            WinCheckButton(hwnd, IDD_SEARCH+3, TRUE);

         RestoreWinPos(hwnd, &pSearchPar->DlgPos, FALSE, TRUE);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == DID_OK)
         {
            WinQueryDlgItemText(hwnd, IDD_SEARCH+2, LEN_SEARCHTEXT+1, pSearchPar->pchSearchText);
            pSearchPar->ulSearchFlags=0;
            if (WinQueryButtonCheckstate(hwnd, IDD_SEARCH+3))
               pSearchPar->ulSearchFlags |= SEARCHFLAG_CASESENSITIVE;
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_SEARCH+2 &&
             SHORT2FROMMP(mp1) == EN_CHANGE)
         {
            if (WinQueryDlgItemTextLength(hwnd, IDD_SEARCH+2))
               WinEnableControl(hwnd, DID_OK, TRUE);
            else
               WinEnableControl(hwnd, DID_OK, FALSE);
         }
         break;

      case WM_DESTROY:
         QueryWinPos(hwnd, &pSearchPar->DlgPos);
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/

