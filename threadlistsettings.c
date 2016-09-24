/*---------------------------------------------------------------------------+
 | Titel: THREADLISTSETTINGS.C                                               |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 06.08.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Threadlist-Settings-Notebook                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include "main.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "areadlg.h"
#include "controls\clrsel.h"
#include "dialogids.h"
#include "resids.h"
#include "utility.h"
#include "threadlistsettings.h"

/*--------------------------------- Defines ---------------------------------*/

#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;
extern THREADLISTOPTIONS threadlistoptions;
extern DIRTYFLAGS dirtyflags;

/*----------------------- interne Funktionsprototypen -----------------------*/

static void InsertThreadListSettingsPages(HWND notebook);
static MRESULT EXPENTRY ThreadsSetupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY ThreadColorsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadListSettingsProc                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fenster-Prozedur der Threadlist-Settings                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ThreadListSettingsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   HWND notebook=NULLHANDLE;
   MRESULT resultbuf;

   switch(msg)
   {
      case WM_INITDLG:
         notebook=WinWindowFromID(hwnd, IDD_THRLISTSETTINGS+1);
         InsertThreadListSettingsPages(notebook);
         RestoreWinPos(hwnd, &threadlistoptions.SettingsPos, TRUE, TRUE);
         WinSetWindowULong(hwnd, QWL_USER, 1);
         break;

      case WM_DESTROY:
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_THRLISTSETTINGS+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (WinQueryWindowULong(hwnd, QWL_USER))
            SaveWinPos(hwnd, (PSWP) mp1, &threadlistoptions.SettingsPos, &dirtyflags.threadsdirty);
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(hwnd, msg, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=255;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=190;

         return resultbuf;

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
/* Funktionsname: InsertThreadListSettingsPages                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt die Settings-Pages in das Notebook fuer die           */
/*               Threadlist-Settings ein.                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: notebook: Window-handle des Notebooks                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertThreadListSettingsPages(HWND notebook)
{
   SetNotebookParams(notebook, 80);

   InsertOnePage(notebook, IDD_THREADSETUP,   IDST_TAB_THREADS,  ThreadsSetupProc, NULL);
   InsertOnePage(notebook, IDD_THRLISTCOLORS, IDST_TAB_THREADCOLORS, ThreadColorsProc, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadsSetupProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Threadlist-Setup                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ThreadsSetupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   BOOL booltemp;
   ULONG ulTemp;

   switch(message)
   {
      case WM_INITDLG:
         WinCheckButton(parent, IDD_THREADSETUP+4, threadlistoptions.shownames);
         WinCheckButton(parent, IDD_THREADSETUP+8, threadlistoptions.compact);
         switch(threadlistoptions.dspmode)
         {
            case DSPTHREADS_ALL:
               WinCheckButton(parent, IDD_THREADSETUP+5, TRUE);
               WinCheckButton(parent, IDD_THREADSETUP+6, FALSE);
               WinCheckButton(parent, IDD_THREADSETUP+7, FALSE);
               break;

            case DSPTHREADS_WITHUNREAD:
               WinCheckButton(parent, IDD_THREADSETUP+5, FALSE);
               WinCheckButton(parent, IDD_THREADSETUP+6, TRUE);
               WinCheckButton(parent, IDD_THREADSETUP+7, FALSE);
               break;

            case DSPTHREADS_UNREADONLY:
               WinCheckButton(parent, IDD_THREADSETUP+5, FALSE);
               WinCheckButton(parent, IDD_THREADSETUP+6, FALSE);
               WinCheckButton(parent, IDD_THREADSETUP+7, TRUE);
               break;

            default:
               break;
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         booltemp=WinQueryButtonCheckstate(parent, IDD_THREADSETUP+4);
         if (threadlistoptions.shownames != booltemp)
         {
            threadlistoptions.shownames = booltemp;
            dirtyflags.threadsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_THREADSETUP+8);
         if (threadlistoptions.compact != booltemp)
         {
            threadlistoptions.compact = booltemp;
            dirtyflags.threadsdirty=TRUE;
         }
         if (WinQueryButtonCheckstate(parent, IDD_THREADSETUP+5))
            ulTemp=DSPTHREADS_ALL;
         if (WinQueryButtonCheckstate(parent, IDD_THREADSETUP+6))
            ulTemp=DSPTHREADS_WITHUNREAD;
         if (WinQueryButtonCheckstate(parent, IDD_THREADSETUP+7))
            ulTemp=DSPTHREADS_UNREADONLY;
         if (threadlistoptions.dspmode != ulTemp)
         {
            threadlistoptions.dspmode = ulTemp;
            dirtyflags.threadsdirty=TRUE;
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadColorsProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Threadlist-Farben-Setup                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ThreadColorsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

   switch (msg)
   {
      case WM_INITDLG:
         /* Farben im Value-Set initialisieren */
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SETITEM,
                           MPFROM2SHORT(1, 1),
                           MPFROMLONG(threadlistoptions.lBackClr));
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SETITEM,
                           MPFROM2SHORT(2, 1),
                           MPFROMLONG(threadlistoptions.lReadClr));
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SETITEM,
                           MPFROM2SHORT(3, 1),
                           MPFROMLONG(threadlistoptions.lUnreadClr));
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SETITEM,
                           MPFROM2SHORT(4, 1),
                           MPFROMLONG(threadlistoptions.lPersonalClr));

         /* Erstes Element im VS auswaehlen */
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SELECTITEM,
                           MPFROM2SHORT(1, 1), NULL);

         /* Fadenkreuz im Color-Wheel entsprechend setzen */
         WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+3, CLSM_SETRGB,
                           &threadlistoptions.lBackClr,
                           NULL);
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_THRLISTCOLORS+3:  /* Color-Wheel */
               if (SHORT2FROMMP(mp1) == CLSN_RGB)
               {
                  MRESULT selected;

                  /* selektiertes Item im Value-Set abfragen */
                  selected=WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_QUERYSELECTEDITEM,
                                             NULL, NULL);

                  /* Farbe updaten */
                  WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4, VM_SETITEM,
                                    selected, mp2);

                  /* Farbe in Settings eintragen */
                  switch(SHORT1FROMMR(selected))
                  {
                     case 1:
                        threadlistoptions.lBackClr= (LONG) mp2;
                        break;

                     case 2:
                        threadlistoptions.lReadClr= (LONG) mp2;
                        break;

                     case 3:
                        threadlistoptions.lUnreadClr= (LONG) mp2;
                        break;

                     case 4:
                        threadlistoptions.lPersonalClr= (LONG) mp2;
                        break;

                     default:
                        break;
                  }

                  /* Dirty-Flag setzen */
                  dirtyflags.threadsdirty=TRUE;
               }
               break;

            case IDD_THRLISTCOLORS+4:  /* Value-Set */
               if (SHORT2FROMMP(mp1) == VN_SELECT)
               {
                  ULONG ulColor;

                  /* neue Selektion abfragen */
                  ulColor=(ULONG)WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+4,
                                                   VM_QUERYITEM, mp2, NULL);

                  /* Fadenkreuz setzen */
                  WinSendDlgItemMsg(hwnd, IDD_THRLISTCOLORS+3,
                                    CLSM_SETRGB, &ulColor, NULL);
               }
               break;

            default:
               break;
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/

