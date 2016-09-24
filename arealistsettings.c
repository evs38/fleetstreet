/*---------------------------------------------------------------------------+
 | Titel: AREALISTSETTINGS.C                                                 |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 22.02.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Settings-Notebook der Area-Liste                                      |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

#pragma strings(readonly)
/*----------------------------- Header-Dateien ------------------------------*/
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "resids.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "areadlg.h"
#include "utility.h"
#include "controls\clrsel.h"

#include "arealistsettings.h"

/*--------------------------------- Defines ---------------------------------*/

#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern AREALISTOPTIONS arealistoptions;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static void InsertAreaListSettingsPages(HWND notebook, PALSETTINGSPARAM pParam);
static MRESULT EXPENTRY AreaListSettViewProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AreaListSettColProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

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

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaListSettingsProc                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Area-List-Settings                      */
/*                                                                           */
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

MRESULT EXPENTRY AreaListSettingsProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   HWND notebook=NULLHANDLE;
   MRESULT resultbuf;

   switch(message)
   {
      case WM_INITDLG:
         notebook=WinWindowFromID(hwnd, IDD_AREALISTSETTINGS+1);
         InsertAreaListSettingsPages(notebook, (PALSETTINGSPARAM) mp2);
         RestoreWinPos(hwnd, &arealistoptions.SettingsPos, TRUE, TRUE);
         WinSetWindowULong(hwnd, QWL_USER, 1);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_AREALISTSETTINGS+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (WinQueryWindowULong(hwnd, QWL_USER))
         {
            extern DIRTYFLAGS dirtyflags;

            SaveWinPos(hwnd, (PSWP) mp1, &arealistoptions.SettingsPos, &dirtyflags.alsettingsdirty);
         }
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(hwnd, message, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=255;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=190;

         return resultbuf;

      case WM_DESTROY:
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
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertAreaListSettingsPages                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt die Settings-Pages in das Notebook fuer die           */
/*               Arealisten-Settings ein.                                    */
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

static void InsertAreaListSettingsPages(HWND notebook, PALSETTINGSPARAM pParam)
{
   SetNotebookParams(notebook, 80);

   InsertOnePage(notebook, IDD_AL_SETTINGS_VIEW,   IDST_TAB_AL_VIEW,   AreaListSettViewProc, pParam);
   InsertOnePage(notebook, IDD_AL_SETTINGS_COLORS, IDST_TAB_AL_COLORS, AreaListSettColProc,  pParam);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaListSettViewProc                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Area-List-View-Settings                 */
/*                                                                           */
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

static MRESULT EXPENTRY AreaListSettViewProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern DIRTYFLAGS dirtyflags;
   PAREALISTOPTIONS pAreaListOptions= (PAREALISTOPTIONS)WinQueryWindowULong(hwnd, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pAreaListOptions=((PALSETTINGSPARAM) mp2)->pAreaListOptions;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pAreaListOptions);

         if (pAreaListOptions->ulFlags & AREALISTFLAG_SHOWTAGS)
            WinCheckButton(hwnd, IDD_AL_SETTINGS_VIEW+8, TRUE);
         else
            WinCheckButton(hwnd, IDD_AL_SETTINGS_VIEW+7, TRUE);
         return (MRESULT) TRUE;

      case WM_CONTROL:
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         if (WinQueryButtonCheckstate(hwnd, IDD_AL_SETTINGS_VIEW+8) && !(pAreaListOptions->ulFlags & AREALISTFLAG_SHOWTAGS))
         {
            pAreaListOptions->ulFlags |= AREALISTFLAG_SHOWTAGS;
            dirtyflags.alsettingsdirty=TRUE;
         }
         else
            if (WinQueryButtonCheckstate(hwnd, IDD_AL_SETTINGS_VIEW+7) && (pAreaListOptions->ulFlags & AREALISTFLAG_SHOWTAGS))
            {
               pAreaListOptions->ulFlags &= ~AREALISTFLAG_SHOWTAGS;
               dirtyflags.alsettingsdirty=TRUE;
            }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaListSettColProc                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Area-List-Color-Settings                */
/*                                                                           */
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

static MRESULT EXPENTRY AreaListSettColProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PAREALISTOPTIONS pAreaListOptions= (PAREALISTOPTIONS)WinQueryWindowULong(hwnd, QWL_USER);
   extern DIRTYFLAGS dirtyflags;

   switch (message)
   {
      case WM_INITDLG:
         pAreaListOptions=((PALSETTINGSPARAM) mp2)->pAreaListOptions;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pAreaListOptions);

         /* Farben im Value-Set initialisieren */
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(1, 1),
                           MPFROMLONG(pAreaListOptions->lBackColor));
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(2, 1),
                           MPFROMLONG(pAreaListOptions->lNetAreaColor));
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(3, 1),
                           MPFROMLONG(pAreaListOptions->lEchoAreaColor));
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(4, 1),
                           MPFROMLONG(pAreaListOptions->lLocalAreaColor));

         /* Erstes Element im VS auswaehlen */
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SELECTITEM,
                           MPFROM2SHORT(1, 1), NULL);

         /* Fadenkreuz im Color-Wheel entsprechend setzen */
         WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+3, CLSM_SETRGB,
                           &pAreaListOptions->lBackColor,
                           NULL);
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_AL_SETTINGS_COLORS+3:  /* Color-Wheel */
               if (SHORT2FROMMP(mp1) == CLSN_RGB)
               {
                  MRESULT selected;

                  /* selektiertes Item im Value-Set abfragen */
                  selected=WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_QUERYSELECTEDITEM,
                                             NULL, NULL);

                  /* Farbe updaten */
                  WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4, VM_SETITEM,
                                    selected, mp2);

                  /* Farbe in Settings eintragen */
                  switch(SHORT1FROMMR(selected))
                  {
                     case 1:
                        pAreaListOptions->lBackColor= (LONG) mp2;
                        break;

                     case 2:
                        pAreaListOptions->lNetAreaColor= (LONG) mp2;
                        break;

                     case 3:
                        pAreaListOptions->lEchoAreaColor= (LONG) mp2;
                        break;

                     case 4:
                        pAreaListOptions->lLocalAreaColor= (LONG) mp2;
                        break;

                     default:
                        break;
                  }

                  /* Dirty-Flag setzen */
                  dirtyflags.alsettingsdirty=TRUE;
               }
               break;

            case IDD_AL_SETTINGS_COLORS+4:  /* Value-Set */
               if (SHORT2FROMMP(mp1) == VN_SELECT)
               {
                  ULONG ulColor;

                  /* neue Selektion abfragen */
                  ulColor=(ULONG)WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+4,
                                                   VM_QUERYITEM, mp2, NULL);

                  /* Fadenkreuz setzen */
                  WinSendDlgItemMsg(hwnd, IDD_AL_SETTINGS_COLORS+3,
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
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/

