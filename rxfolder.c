/*---------------------------------------------------------------------------+
 | Titel: RXFOLDER.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 02.08.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Rexx-Script-Folder und Script-Settings                                  |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "dialogids.h"
#include "areaman\areaman.h"
#include "utility.h"
#include "setupdlg.h"
#include "rxfolder.h"
#include "controls\editwin.h"
#include "util\fltutil.h"
#include "rexxexec.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif

#define SCRIPTDRAGTYPE "FleetStreet Script," DRT_TEXT
#define SCRIPTRMF      "<DRM_FLEET,DRF_SCRIPT>,(DRM_OS2FILE,DRM_DISCARD,DRM_PRINT)x(DRF_UNKNOWN)"
#define EMPTYSCRIPTRMF "<DRM_FLEET,DRF_SCRIPT>,<DRM_DISCARD,DRF_UNKNOWN>"

#define NUM_PAGES_SCRIPT  2
#define NUM_PAGES_RXSETTINGS  1
#if 0
#define RGB_GREY         0x00cccccc
#define TAB_FONT         "8.Helv"
#endif

#define MAX_NUM_QUICKACCESS    10

#define RXM_CHECKQUICKACCESS   (WM_USER+1)

/*---------------------------------- Typen ----------------------------------*/

typedef struct {
             MINIRECORDCORE RecordCore;
             HWND           hwndSettings;
             HWND           hwndMonitor;
             PRXSCRIPT      pScript;
          } SCRIPTRECORD, *PSCRIPTRECORD;

typedef struct {
             HWND          hwndFolderPopup;
             HWND          hwndScriptPopup;
             HWND          hwndSubMenu;
             HPOINTER      hptrScript;
             HPOINTER      hptrScriptFolder;
             BOOL          bNotify;
             HSWITCH       hSwitch;
             PSCRIPTRECORD pPopupRecord;
             BOOL          bForeground;
          } SCRIPTFOLDERDATA, *PSCRIPTFOLDERDATA;

typedef struct {
             USHORT    cb;
             PRXSCRIPT pScript;
          } OPENSCRIPT, *POPENSCRIPT;

typedef struct {
           PRXSCRIPT    pScript;
           HWND         notebook;
           NBPAGE       PageTable[NUM_PAGES_SCRIPT];
           BOOL         bNotify;
        } SCRIPTBOOKDATA, *PSCRIPTBOOKDATA;

typedef struct {
           HWND         notebook;
           BOOL         bNotify;
           NBPAGE       PageTable[NUM_PAGES_RXSETTINGS];
        } RXFOLDERSETTINGSDATA, *PRXFOLDERSETTINGSDATA;

/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;
extern HWND hwndRxFolder, client;
extern SCRIPTLIST scriptlist;
extern REXXHOOKS rexxhooks;
extern int tidRexxExec;
extern PRXSCRIPT pExecScript;
extern HWND hwndMonitor;

static PFNWP OldContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/
static MRESULT EXPENTRY NewScriptContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static int FillScriptFolder(HWND hwndCnr, HPOINTER hptr);
static void ScriptSettingsClosed(HWND hwndCnr, ULONG ulScriptID);
static void ScriptMonitorClosed(HWND hwndCnr, HWND hwndMonitor);
static int OpenScriptSettings(HWND hwndCnr, PSCRIPTRECORD pRecord);
static PSCRIPTRECORD AddNewScript(HWND hwndCnr, HPOINTER hptr);
static int HaveScriptName(PVOID pScriptList, char *pchName);
static int DeleteScript(HWND hwndCnr, PSCRIPTRECORD pRecord);
static void CleanupScriptFolder(HWND hwndCnr);
static void InitScriptDrag(HWND hwnd, PCNRDRAGINIT pInit);
static int OpenScript(HWND hwndCnr, PSCRIPTRECORD pRecord);
static int OpenScriptEdit(PSCRIPTRECORD pRecord);
static void NewScriptName(HWND hwndCnr, ULONG ulScriptID);
static MRESULT RxFolderDragOver(HWND hwndCnr, PCNRDRAGINFO pCnrDrag);
static void RxFolderDrop(HWND hwndCnr, PCNRDRAGINFO pCnrDrag, PSCRIPTFOLDERDATA pData);

static MRESULT EXPENTRY ScriptSettingsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void InsertScriptPages(HWND notebook, NBPAGE *Table);
static MRESULT EXPENTRY RxGeneralProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY RxMonitorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static BOOL QuickAccessPossible(BOOL SelfQuickAccess);
static void BroadcastToScripts(HWND hwndCnr, ULONG msg, MPARAM mp1, MPARAM mp2);

static MRESULT EXPENTRY RxFolderSettingsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void InsertRxSettingsPages(HWND notebook, NBPAGE *Table);
static MRESULT EXPENTRY RxHooksProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static SHORT EXPENTRY SortRexxFolder(PSCRIPTRECORD p1, PSCRIPTRECORD p2, PVOID pData);


/*---------------------------------------------------------------------------*/
/* Funktionsname: ScriptFolderProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure des Script-Folders                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ScriptFolderProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS  windowfonts;
   extern GENERALOPT generaloptions;
   extern HWND hwndhelp, client;
   PSCRIPTFOLDERDATA pFolderData = (PSCRIPTFOLDERDATA) WinQueryWindowULong(hwnd, QWL_USER);
   CNRINFO cnrinfo;

   switch (message)
   {
      case WM_INITDLG:
         /* Instanzdaten anfordern */
         pFolderData = malloc(sizeof(SCRIPTFOLDERDATA));
         memset(pFolderData, 0, sizeof(SCRIPTFOLDERDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pFolderData);

         OldContainerProc = WinSubclassWindow(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                                              NewScriptContainerProc);

         /* Icon laden */
         pFolderData->hptrScript = LoadIcon(IDIC_RXSCRIPT);
         pFolderData->hptrScriptFolder = LoadIcon(IDIC_SCRIPTFOLDER);
         SendMsg(hwnd, WM_SETICON, (MPARAM) pFolderData->hptrScriptFolder, NULL);

         /* Switch-Entry */
         pFolderData->hSwitch=AddToWindowList(hwnd);

         /* Menues laden */
         pFolderData->hwndScriptPopup = WinLoadMenu(HWND_OBJECT,
                                                    hmodLang, IDM_RXF_POPUP);
         pFolderData->hwndFolderPopup = WinLoadMenu(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                                                    hmodLang, IDM_RXF_POPUP2);
         /* Conditional Cascade style setzen */
         if (pFolderData->hwndScriptPopup)
         {
            MENUITEM MenuItem;

            SendMsg(pFolderData->hwndScriptPopup, MM_QUERYITEM,
                       MPFROM2SHORT(IDM_RXF_OPEN, TRUE),
                       &MenuItem);
            if (MenuItem.hwndSubMenu)
            {
               ULONG ulStyle;

               ulStyle = WinQueryWindowULong(MenuItem.hwndSubMenu, QWL_STYLE);
               WinSetWindowULong(MenuItem.hwndSubMenu, QWL_STYLE,
                                 ulStyle | MS_CONDITIONALCASCADE);
               pFolderData->hwndSubMenu = MenuItem.hwndSubMenu;
            }
         }

         if (pFolderData->hwndFolderPopup)
            ReplaceSysMenu(hwnd, pFolderData->hwndFolderPopup, 1);

         if (scriptlist.ulFlags & SCRIPTS_FOREGROUND)
         {
            pFolderData->bForeground = TRUE;
            WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_RXF_FGROUND, TRUE);
            WinSetOwner(hwnd, client);
         }
         else
         {
            pFolderData->bForeground = FALSE;
            WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_RXF_FGROUND, FALSE);
            WinSetOwner(hwnd, HWND_DESKTOP);
         }

         /* Farben und Font setzen */
         SetBackground(WinWindowFromID(hwnd, IDD_RXFOLDER+1), &windowcolors.scriptback);
         SetForeground(WinWindowFromID(hwnd, IDD_RXFOLDER+1), &windowcolors.scriptfore);
         SetFont(WinWindowFromID(hwnd, IDD_RXFOLDER+1), windowfonts.scriptfont);

         /* Sortierfunktion */
         cnrinfo.cb = sizeof(cnrinfo);
         cnrinfo.pSortRecord = (PVOID) SortRexxFolder;
         WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_PSORTRECORD));

         /* Icons einfuegen */
         FillScriptFolder(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                          pFolderData->hptrScript);
         RestoreWinPos(hwnd, &scriptlist.FolderPos, TRUE, TRUE);
         pFolderData->bNotify = TRUE;
         break;

      case WM_DESTROY:
         /* Farben und Font */
         CleanupScriptFolder(WinWindowFromID(hwnd, IDD_RXFOLDER+1));
         RemoveFromWindowList(pFolderData->hSwitch);
         QueryBackground(WinWindowFromID(hwnd, IDD_RXFOLDER+1), &windowcolors.scriptback);
         QueryForeground(WinWindowFromID(hwnd, IDD_RXFOLDER+1), &windowcolors.scriptfore);
         QueryFont(WinWindowFromID(hwnd, IDD_RXFOLDER+1), windowfonts.scriptfont);
         if (pFolderData->hptrScript)
            WinDestroyPointer(pFolderData->hptrScript);
         if (pFolderData->hptrScriptFolder)
            WinDestroyPointer(pFolderData->hptrScriptFolder);
         if (pFolderData->hwndScriptPopup)
            WinDestroyWindow(pFolderData->hwndScriptPopup);
         if (pFolderData->hwndFolderPopup)
            WinDestroyWindow(pFolderData->hwndFolderPopup);

         if (pFolderData->bForeground)
         {
            if (!(scriptlist.ulFlags & SCRIPTS_FOREGROUND))
            {
               scriptlist.ulFlags |= SCRIPTS_FOREGROUND;
               scriptlist.bDirty = TRUE;
            }
         }
         else
         {
            if (scriptlist.ulFlags & SCRIPTS_FOREGROUND)
            {
               scriptlist.ulFlags &= ~SCRIPTS_FOREGROUND;
               scriptlist.bDirty = TRUE;
            }
         }
         free(pFolderData);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_RXFOLDER+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               PCNREDITDATA pEdit;
               PSCRIPTRECORD pRecord;
               PNOTIFYRECORDENTER pEnter;

               case CN_ENTER:
                  pEnter = (PNOTIFYRECORDENTER) mp2;
                  if (pEnter->pRecord)
                  {
                     pRecord = (PSCRIPTRECORD) pEnter->pRecord;
                     if (pRecord->pScript->pchPathName[0])
                     {
                        if (!tidRexxExec && !hwndMonitor)
                           OpenScript(pEnter->hwndCnr, pRecord);
                     }
                     else
                        OpenScriptSettings(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                                           pRecord);
                  }
                  break;

               case CN_REALLOCPSZ:
                  pEdit = (PCNREDITDATA) mp2;
                  pRecord = (PSCRIPTRECORD) pEdit->pRecord;
                  free (pRecord->pScript->pchScriptName);
                  pRecord->pScript->pchScriptName = malloc(pEdit->cbText+1);
                  pRecord->pScript->pchScriptName[0] = '\0';
                  pRecord->RecordCore.pszIcon = pRecord->pScript->pchScriptName;
                  pRecord->pScript->bDirty=TRUE;
                  scriptlist.bDirty = TRUE;
                  return (MRESULT) TRUE;

               case CN_ENDEDIT:
                  /* Settings offen ? */
                  pEdit = (PCNREDITDATA) mp2;
                  if (((PSCRIPTRECORD)pEdit->pRecord)->hwndSettings)
                     SendMsg(((PSCRIPTRECORD)pEdit->pRecord)->hwndSettings,
                                RXSET_NEWNAME,
                                MPFROMLONG(((PSCRIPTRECORD)pEdit->pRecord)->pScript->ulScriptID),
                                NULL);
                  WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, CM_SORTRECORD,
                                    (MPARAM) SortRexxFolder, NULL);
                  SendMsg(client, RXM_UPDATEMENU, NULL, NULL);
                  break;

               case CN_INITDRAG:
                  InitScriptDrag(hwnd, (PCNRDRAGINIT) mp2);
                  break;

               case CN_CONTEXTMENU:
                  pFolderData->pPopupRecord = (PSCRIPTRECORD) mp2;
                  if (pFolderData->pPopupRecord)
                  {
                     /* Popup-Menue eines Scripts */
                     RECTL rcl;
                     POINTL ptl;
                     QUERYRECORDRECT QRecord;

                     if (pFolderData->pPopupRecord->RecordCore.flRecordAttr & CRA_SELECTED)
                     {
                        PSCRIPTRECORD pRecord;
                        HWND hwndCnr = WinWindowFromID(hwnd, IDD_RXFOLDER+1);
                        BOOL bScriptOpen=FALSE, bScriptEmpty=FALSE;
                        int i=0;

                        pRecord=NULL;
                        while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                                    pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                                    MPFROMLONG(CRA_SELECTED)))
                        {
                           if (pRecord->hwndMonitor)
                              bScriptOpen=TRUE;
                           else
                              if (!pFolderData->pPopupRecord->pScript->pchPathName[0])
                                 bScriptEmpty=TRUE;
                           if (++i == 2)  /* verhindern, da· mehrere Scripte gestartet werden */
                              bScriptEmpty=TRUE;
                        }
                        if (bScriptOpen)
                        {
                           WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_DELETE, FALSE);
                           SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                      MPFROMSHORT(IDM_RXF_OPEN_SCR), NULL);
                           WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SCR, TRUE);
                           WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SET, FALSE);
                        }
                        else
                        {
                           WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_DELETE, TRUE);
                           if (!bScriptEmpty)
                           {
                              SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                         MPFROMSHORT(IDM_RXF_OPEN_SCR), NULL);
                              WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SET, FALSE);
                              WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_EDIT, TRUE);
                           }
                           else
                           {
                              SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                         MPFROMSHORT(IDM_RXF_OPEN_SET), NULL);
                              WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SCR, FALSE);
                              WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_EDIT, FALSE);
                           }
                        }
                     }
                     else
                     {
                        /* nur ein Record */
                        if (pFolderData->pPopupRecord->hwndMonitor)
                        {
                           WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_DELETE, FALSE);
                           SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                      MPFROMSHORT(IDM_RXF_OPEN_SCR), NULL);
                           WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SCR, TRUE);
                           WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SET, FALSE);
                        }
                        else
                        {
                           WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_DELETE, TRUE);
                           if (pFolderData->pPopupRecord->pScript->pchPathName[0])
                           {
                              SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                         MPFROMSHORT(IDM_RXF_OPEN_SCR), NULL);
                              WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SET, FALSE);
                              WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_EDIT, TRUE);
                           }
                           else
                           {
                              SendMsg(pFolderData->hwndSubMenu, MM_SETDEFAULTITEMID,
                                         MPFROMSHORT(IDM_RXF_OPEN_SET), NULL);
                              WinCheckMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_SCR, FALSE);
                              WinEnableMenuItem(pFolderData->hwndScriptPopup, IDM_RXF_OPEN_EDIT, FALSE);
                           }
                        }
                     }

                     QRecord.cb = sizeof(QUERYRECORDRECT);
                     QRecord.pRecord = (PRECORDCORE) pFolderData->pPopupRecord;
                     QRecord.fRightSplitWindow = FALSE;
                     QRecord.fsExtent = CMA_ICON;
                     WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, CM_QUERYRECORDRECT,
                                       &rcl, &QRecord);
                     ptl.x = rcl.xRight;
                     ptl.y = rcl.yBottom;
                     WinMapWindowPoints(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                                        HWND_DESKTOP, &ptl, 1);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pFolderData->hwndScriptPopup,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  else
                  {
                     /* Popup-Menue des Folders */
                     POINTL ptl;

                     WinQueryPointerPos(HWND_DESKTOP, &ptl);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pFolderData->hwndFolderPopup,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  break;

               case CN_HELP:
                  SendMsg(hwnd, WM_HELP, MPFROMSHORT(IDD_RXFOLDER+1), NULL);
                  break;

               case CN_DROP:
                  RxFolderDrop(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (PCNRDRAGINFO) mp2, pFolderData);
                  break;

               case CN_DRAGOVER:
                  return RxFolderDragOver(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (PCNRDRAGINFO) mp2);

               default:
                  break;
            }
         }
         break;

      case DM_DISCARDOBJECT:
         if (mp1)
         {
            ULONG ulNum, i;
            PDRAGITEM pItem;
            PSCRIPTRECORD pRecord;
            HWND hwndCnr= WinWindowFromID(hwnd, IDD_RXFOLDER+1);

            DrgAccessDraginfo((PDRAGINFO) mp1);
            ulNum = DrgQueryDragitemCount((PDRAGINFO) mp1);
            for (i=0; i<ulNum; i++)
            {
               pItem = DrgQueryDragitemPtr((PDRAGINFO) mp1, i);

               pRecord = NULL;
               while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                           pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                           MPFROMLONG(CRA_SOURCE)))
               {
                  if (pRecord->pScript->ulScriptID == pItem->ulItemID)
                     break;
               }
               if (pRecord)
                  DeleteScript(WinWindowFromID(hwnd, IDD_RXFOLDER+1), pRecord);
            }
            DrgFreeDraginfo((PDRAGINFO) mp1);
         }
         else
            WinAlarm(HWND_DESKTOP, WA_NOTE);
         return (MRESULT) DRR_SOURCE;

      case DM_PRINTOBJECT:
         return (MRESULT) DRR_TARGET;

      case WM_MENUEND:
         if ((HWND) mp2 == pFolderData->hwndScriptPopup ||
             (HWND) mp2 == pFolderData->hwndFolderPopup)
         {
            /* Emphasis wegnehmen */
            if (pFolderData->pPopupRecord)
               RemoveSourceEmphasis(WinWindowFromID(hwnd, IDD_RXFOLDER+1));
            else
               WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, CM_SETRECORDEMPHASIS, NULL, MPFROM2SHORT(FALSE, CRA_SOURCE));
            if ( (HWND) mp2 == pFolderData->hwndFolderPopup)
               ResetMenuStyle(pFolderData->hwndFolderPopup, hwnd);
         }
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pFolderData->hwndFolderPopup)
            pFolderData->pPopupRecord=NULL;
         if ((HWND) mp2 == pFolderData->hwndScriptPopup ||
             (HWND) mp2 == pFolderData->hwndFolderPopup)
         {
            /* Emphasis setzen */
            if (pFolderData->pPopupRecord)
            {
               ApplySourceEmphasis(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (PRECORDCORE) pFolderData->pPopupRecord);
            }
            else
               WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, CM_SETRECORDEMPHASIS,
                                 NULL,
                                 MPFROM2SHORT(TRUE, CRA_SOURCE));
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTEXTMENU:
      case WM_TEXTEDIT:
         if (!SHORT1FROMMP(mp1) &&
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(hwnd, IDD_RXFOLDER+1))
         {
            WinSendDlgItemMsg(hwnd, IDD_RXFOLDER+1, message,
                              mp1, mp2);
         }
         break;

      case WM_CLOSE:
         WinPostMsg(client, RXF_CLOSE, NULL, NULL);
         break;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(hwnd, IDD_RXFOLDER+1), FALSE);
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
            WinShowWindow(WinWindowFromID(hwnd, IDD_RXFOLDER+1), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_RXFOLDER+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pFolderData && pFolderData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &scriptlist.FolderPos, &scriptlist.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            PSCRIPTRECORD pNewScript;
            PSCRIPTRECORD pRecord;
            HWND hwndCnr;

            case IDM_RXF_OPEN_SET:
               hwndCnr = WinWindowFromID(hwnd, IDD_RXFOLDER+1);
               if (pFolderData->pPopupRecord->RecordCore.flRecordAttr & CRA_SELECTED)
               {
                  pRecord=NULL;
                  while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                              MPFROMLONG(CRA_SELECTED)))
                  {
                     OpenScriptSettings(hwndCnr, pRecord);
                  }
               }
               else
                  OpenScriptSettings(hwndCnr, pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_RXF_OPEN_SCR:
               if (pFolderData->pPopupRecord->pScript->pchPathName[0] &&
                   !tidRexxExec && !hwndMonitor)
                  OpenScript(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                             pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_RXF_OPEN_EDIT:
               if (pFolderData->pPopupRecord->pScript->pchPathName[0] &&
                   !tidRexxExec && !hwndMonitor)
                  OpenScriptEdit(pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_RXF_CREATE:
               if (pNewScript = AddNewScript(WinWindowFromID(hwnd, IDD_RXFOLDER+1),
                                             pFolderData->hptrScript))
                  OpenScriptSettings(WinWindowFromID(hwnd, IDD_RXFOLDER+1), pNewScript);
               return (MRESULT) FALSE;

            case IDM_RXF_DELETE:
               if (generaloptions.safety & SAFETY_CHANGESETUP)
                  if (MessageBox(hwnd, IDST_MSG_DELSCRIPT, IDST_TITLE_DELSCRIPT,
                                 IDD_DELSCRIPT, MB_YESNO| MB_QUERY| MB_DEFBUTTON2) != MBID_YES)
                     return (MRESULT) FALSE;
               /* zaehlen */
               hwndCnr = WinWindowFromID(hwnd, IDD_RXFOLDER+1);
               if (pFolderData->pPopupRecord->RecordCore.flRecordAttr & CRA_SELECTED)
               {
                  ULONG ulNum=0;
                  int i=0;
                  PSCRIPTRECORD *ppRecordArray;

                  pRecord=NULL;
                  while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                              MPFROMLONG(CRA_SELECTED)))
                     ulNum++;

                  if (ulNum)
                  {
                     ppRecordArray = calloc(ulNum, sizeof(PSCRIPTRECORD));
                     pRecord=NULL;
                     while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                                 MPFROMLONG(CRA_SELECTED)))
                        ppRecordArray[i++]=pRecord;

                     for (i=0; i< ulNum; i++)
                        DeleteScript(hwndCnr, ppRecordArray[i]);
                     free(ppRecordArray);
                  }
               }
               else
                  DeleteScript(hwndCnr, pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_RXF_FGROUND:
               if (pFolderData->bForeground)
               {
                  pFolderData->bForeground = FALSE;
                  WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_RXF_FGROUND, FALSE);
                  WinSetOwner(hwnd, HWND_DESKTOP);
               }
               else
               {
                  pFolderData->bForeground = TRUE;
                  WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_RXF_FGROUND, TRUE);
                  WinSetOwner(hwnd, client);
               }
               return (MRESULT) FALSE;

            case IDM_RXF_SETTINGS:
               WinDlgBox(HWND_DESKTOP, hwnd, RxFolderSettingsProc, hmodLang,
                         IDD_RXFOLDERSETTINGS, NULL);
               return (MRESULT) FALSE;

            case DID_CANCEL:
               break;

            default:
               return (MRESULT) FALSE;
         }
         return (MRESULT) FALSE;

      case RXSET_CLOSE:
         ScriptSettingsClosed(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (ULONG) mp1);
         break;

      case RXSET_NEWNAME:
         NewScriptName(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (ULONG) mp1);
         break;

      case RXM_CHECKQUICKACCESS:
         BroadcastToScripts(WinWindowFromID(hwnd, IDD_RXFOLDER+1), message, mp1, mp2);
         break;

      case REXXM_CLOSE:
         ScriptMonitorClosed(WinWindowFromID(hwnd, IDD_RXFOLDER+1), (HWND) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

static SHORT EXPENTRY SortRexxFolder(PSCRIPTRECORD p1, PSCRIPTRECORD p2, PVOID pData)
{
   pData = pData;

   return stricmp(p1->pScript->pchScriptName, p2->pScript->pchScriptName);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NewScriptContainerProc                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Neue Window-Prozedur des Script-Folder-Containers           */
/*               wg. Container-Bug bei Drag-Over                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewScriptContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo(mp1);
         break;

      case WM_BUTTON2DOWN:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldContainerProc(parent, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FillScriptFolder                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuellt den Script-Folder mit allen Script-Objekten          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            hptr: Zu verwendendes Icon                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 0   OK                                                     */
/*                sonst Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int FillScriptFolder(HWND hwndCnr, HPOINTER hptr)
{
   RECORDINSERT RecordInsert;
   PSCRIPTRECORD pRecord, pFirstRecord;
   PRXSCRIPT pScript;

   if (scriptlist.ulNumScripts == 0)
      return 0;

   /* Records vom Container anfordern */
   pFirstRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                             MPFROMLONG(sizeof(SCRIPTRECORD) - sizeof(MINIRECORDCORE)),
                             MPFROMLONG(scriptlist.ulNumScripts));
   pRecord = pFirstRecord;

   pScript = scriptlist.pScripts;
   while (pRecord)
   {
      pRecord->hwndSettings = NULLHANDLE;
      pRecord->hwndMonitor  = NULLHANDLE;
      pRecord->pScript      = pScript;

      pRecord->RecordCore.flRecordAttr = 0;
      pRecord->RecordCore.pszIcon = pScript->pchScriptName;

      pRecord->RecordCore.hptrIcon = hptr;

      pRecord = (PSCRIPTRECORD) pRecord->RecordCore.preccNextRecord;
      pScript = pScript->next;
   }

   /* Records einfuegen */
   RecordInsert.cb = sizeof(RECORDINSERT);
   RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
   RecordInsert.pRecordParent = NULL;
   RecordInsert.fInvalidateRecord = TRUE;
   RecordInsert.zOrder = CMA_TOP;
   RecordInsert.cRecordsInsert = scriptlist.ulNumScripts;

   SendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScriptSettingsClosed                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Settings-Notebook eines Scripts wurde geschlossen           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            ulScriptID: Script-ID des geschlossenen Scripts                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ScriptSettingsClosed(HWND hwndCnr, ULONG ulScriptID)
{
   PSCRIPTRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->pScript->ulScriptID == ulScriptID)
      {
         if (pRecord->hwndSettings)
            WinDestroyWindow(pRecord->hwndSettings);
         pRecord->hwndSettings = NULLHANDLE;

         if (!pRecord->hwndMonitor)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                       MPFROM2SHORT(FALSE, CRA_INUSE));

         SendMsg(hwndCnr, CM_SORTRECORD, (MPARAM) SortRexxFolder, NULL);
         SendMsg(client, RXM_UPDATEMENU, NULL, NULL);
         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScriptMonitorClosed                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Monitor eines Scripts wurde geschlossen                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            hwndMonitor: Window-Handle des Monitors                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ScriptMonitorClosed(HWND hwndCnr, HWND hwndMonitor)
{
   PSCRIPTRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndMonitor == hwndMonitor)
      {
         if (!pRecord->hwndSettings)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                       MPFROM2SHORT(FALSE, CRA_INUSE));
         pRecord->hwndMonitor = NULLHANDLE;

         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: BroadcastToScripts                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Leitet eine Message an alle offenen Scripts weiter          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            msg: Message                                                   */
/*            mp1, mp2: Message-Parameter                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void BroadcastToScripts(HWND hwndCnr, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PSCRIPTRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndSettings)
         SendMsg(pRecord->hwndSettings, msg, mp1, mp2);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NewScriptName                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Script-Name hat sich geÑndert                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            ulScriptID: Script-ID des  Scripts                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void NewScriptName(HWND hwndCnr, ULONG ulScriptID)
{
   PSCRIPTRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->pScript->ulScriptID == ulScriptID)
      {
         if (pRecord->hwndSettings)
            SendMsg(pRecord->hwndSettings, RXSET_NEWNAME, MPFROMLONG(ulScriptID), NULL);

         pRecord->RecordCore.pszIcon = pRecord->pScript->pchScriptName;
         SendMsg(hwndCnr, CM_INVALIDATERECORD, &pRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
         SendMsg(hwndCnr, CM_SORTRECORD, (MPARAM) SortRexxFolder, NULL);

         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenScriptSettings                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet Settings-Notebook eines Scripts                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            pRecord: Record des Scripts                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int OpenScriptSettings(HWND hwndCnr, PSCRIPTRECORD pRecord)
{
   if (pRecord->hwndSettings)
      SetFocus(pRecord->hwndSettings);
   else
   {
      OPENSCRIPT OpenScript;

      /* in-use-emphasis setzen */
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                 MPFROM2SHORT(TRUE, CRA_INUSE));

      OpenScript.cb = sizeof(OpenScript);
      OpenScript.pScript = pRecord->pScript;

      pRecord->hwndSettings = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, ScriptSettingsProc,
                                         hmodLang, IDD_RXSETTINGS, &OpenScript);
   }
   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddNewScript                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt ein neues Script                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            hptr: Zu verwendendes Icon                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: NULL Fehler                                                */
/*                sonst Zeiger auf neuen Record                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PSCRIPTRECORD AddNewScript(HWND hwndCnr, HPOINTER hptr)
{
   PRXSCRIPT pScript, pNewScript, pLastScript=NULL;
   RECORDINSERT RecordInsert;
   PSCRIPTRECORD pNewRecord;
   ULONG ulNewID=0;

   /* Ende der Templatekette suchen */
   pLastScript = pScript = scriptlist.pScripts;
   while (pScript)
   {
      if (pScript->ulScriptID > ulNewID)
         ulNewID = pScript->ulScriptID;
      pLastScript = pScript;
      pScript = pScript->next;
   }
   ulNewID++;

   /* neues Skript erzeugen */
   pNewScript = malloc(sizeof(RXSCRIPT));
   memset(pNewScript, 0, sizeof(RXSCRIPT));

   /* Name */
   pNewScript->pchScriptName = malloc(100);
   CreateUniqueName(IDST_RX_NEWSCRIPT, &scriptlist, HaveScriptName, 100, pNewScript->pchScriptName);

   /* hinten anhaengen */
   pNewScript->next = NULL;
   pNewScript->prev = pLastScript;
   if (pLastScript)
      pLastScript->next = pNewScript;
   if (!scriptlist.pScripts)
      scriptlist.pScripts = pNewScript;

   scriptlist.ulNumScripts++;
   scriptlist.bDirty = TRUE;

   /* Default-Daten setzen */
   pNewScript->bDirty = TRUE;
   pNewScript->ulScriptID = ulNewID;

   /* Record vom Container anfordern */
   pNewRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(SCRIPTRECORD) - sizeof(MINIRECORDCORE)),
                           MPFROMLONG(1));

   if (pNewRecord)
   {
      pNewRecord->hwndSettings = NULLHANDLE;
      pNewRecord->hwndMonitor  = NULLHANDLE;
      pNewRecord->pScript      = pNewScript;

      pNewRecord->RecordCore.flRecordAttr = 0;
      pNewRecord->RecordCore.pszIcon = pNewScript->pchScriptName;
      pNewRecord->RecordCore.hptrIcon = hptr;

      /* Record einfuegen */
      RecordInsert.cb = sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = NULL;
      RecordInsert.fInvalidateRecord = TRUE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = 1;

      SendMsg(hwndCnr, CM_INSERTRECORD, pNewRecord, &RecordInsert);

      return pNewRecord;
   }
   else
      return NULL;
}

static int HaveScriptName(PVOID pScriptList, char *pchName)
{
   PRXSCRIPT pScript = ((PSCRIPTLIST)pScriptList)->pScripts;

   while (pScript)
      if (!strcmp(pScript->pchScriptName, pchName))
         return TRUE;
      else
         pScript = pScript->next;

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteScript                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht ein Script-Objekt                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            pRecord: Zeiger auf Record                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int DeleteScript(HWND hwndCnr, PSCRIPTRECORD pRecord)
{
   if (pRecord->hwndMonitor)
      return -1;

   if (tidRexxExec && pRecord->pScript == pExecScript)
      return -1;

   /* offenes Notebook schliessen */
   if (pRecord->hwndSettings)
      WinDestroyWindow(pRecord->hwndSettings);

   /* Record im Container loeschen */
   SendMsg(hwndCnr, CM_REMOVERECORD, &pRecord, MPFROM2SHORT(1, CMA_INVALIDATE));

   /* Felder freigeben */
   if (pRecord->pScript->pchScriptName)
      free(pRecord->pScript->pchScriptName);

   /* Skript selbst loeschen */
   if (scriptlist.pScripts == pRecord->pScript)
      scriptlist.pScripts = scriptlist.pScripts->next;

   if (pRecord->pScript->next)
      pRecord->pScript->next->prev = pRecord->pScript->prev;
   if (pRecord->pScript->prev)
      pRecord->pScript->prev->next = pRecord->pScript->next;
   free(pRecord->pScript);

   scriptlist.ulNumScripts--;
   scriptlist.bDirty = TRUE;

   /* endgueltig aus Container entfernen */
   SendMsg(hwndCnr, CM_FREERECORD, &pRecord, MPFROMLONG(1));

   SendMsg(client, RXM_UPDATEMENU, NULL, NULL);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupScriptFolder                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht alle Objekte aus dem Script-Folder                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CleanupScriptFolder(HWND hwndCnr)
{
   PSCRIPTRECORD pRecord = NULL;

   /* alle offenen Templates schliessen */
   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndSettings)
         WinDestroyWindow(pRecord->hwndSettings);
   }

   /* Folder leeren */
   SendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InitScriptDrag                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drag-Init f. Scripts                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Window-Handle                                     */
/*            pInit: Init-Struktur v. Container                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InitScriptDrag(HWND hwnd, PCNRDRAGINIT pInit)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage[3];
   char *pchTemp, *pchTemp2;
   PSCRIPTRECORD pRecord;
   HWND hwndCnr = WinWindowFromID(hwnd, IDD_RXFOLDER+1);
   ULONG ulNum=0;
   int i=0;

   if (!pInit->pRecord)
      return;

   if (pInit->pRecord->flRecordAttr & CRA_SELECTED)
   {
      /* alle selektierten */
      pRecord = NULL;
      while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                  MPFROMLONG(CRA_SELECTED)))
      {
         ulNum++;
         if (pRecord->hwndMonitor)
            return;

         if (tidRexxExec && pRecord->pScript == pExecScript)
            return;
      }
      pDraginfo = DrgAllocDraginfo(ulNum);
      pDraginfo->usOperation=DO_DEFAULT;
      pDraginfo->hwndSource=hwnd;

      pRecord = NULL;
      while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                  MPFROMLONG(CRA_SELECTED)))
      {
         /* Source emphasis */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

         /* Drag-Item vorbereiten*/
         dItem.hwndItem=hwnd;
         dItem.ulItemID= pRecord->pScript->ulScriptID;
         dItem.hstrType=DrgAddStrHandle(SCRIPTDRAGTYPE);
         pchTemp = strdup(pRecord->pScript->pchPathName);
         pchTemp2 = strrchr(pchTemp, '\\');
         if (pchTemp2)
         {
            dItem.hstrSourceName=DrgAddStrHandle(pchTemp2+1);
            dItem.hstrTargetName=DrgAddStrHandle(pchTemp2+1);
            pchTemp2++;
            *pchTemp2='\0';
            dItem.hstrContainerName=DrgAddStrHandle(pchTemp);
         }
         else
         {
            dItem.hstrSourceName=DrgAddStrHandle(pchTemp);
            dItem.hstrTargetName=DrgAddStrHandle(pchTemp);
            dItem.hstrContainerName = NULLHANDLE;
         }
         free(pchTemp);

         if (pRecord->pScript->pchPathName[0])
            dItem.hstrRMF=DrgAddStrHandle(SCRIPTRMF);
         else
            dItem.hstrRMF=DrgAddStrHandle(EMPTYSCRIPTRMF);

         if (pRecord->hwndSettings)
            dItem.fsControl= DC_OPEN;
         else
            dItem.fsControl= 0;
         dItem.fsSupportedOps=DO_COPYABLE;
         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), i);
         i++;
      }
      for (i=0; i<ulNum && i<3; i++)
      {
         /* Drag-Image vorbereiten */
         dImage[i].cb=sizeof(DRAGIMAGE);
         dImage[i].hImage=pInit->pRecord->hptrIcon;
         dImage[i].fl=DRG_ICON;
         dImage[i].cxOffset=i*10;
         dImage[i].cyOffset=i*10;
      }

      DrgDrag(hwnd, pDraginfo, dImage, (ulNum<3)?ulNum:3, VK_ENDDRAG, NULL);
      DrgFreeDraginfo(pDraginfo);

      /* Source emphasis wegnehmen */
      RemoveSourceEmphasis(hwndCnr);
   }
   else
   {
      /* nur einer */
      pRecord = (PSCRIPTRECORD) pInit->pRecord;

      if (pRecord->hwndMonitor)
         return;

      if (tidRexxExec && pRecord->pScript == pExecScript)
         return;

      pDraginfo = DrgAllocDraginfo(1);
      pDraginfo->usOperation=DO_DEFAULT;
      pDraginfo->hwndSource=hwnd;

      /* Source emphasis */
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

      /* Drag-Item vorbereiten*/
      dItem.hwndItem=hwnd;
      dItem.ulItemID= pRecord->pScript->ulScriptID;
      dItem.hstrType=DrgAddStrHandle(SCRIPTDRAGTYPE);
      pchTemp = strdup(pRecord->pScript->pchPathName);
      pchTemp2 = strrchr(pchTemp, '\\');
      if (pchTemp2)
      {
         dItem.hstrSourceName=DrgAddStrHandle(pchTemp2+1);
         dItem.hstrTargetName=DrgAddStrHandle(pchTemp2+1);
         pchTemp2++;
         *pchTemp2='\0';
         dItem.hstrContainerName=DrgAddStrHandle(pchTemp);
      }
      else
      {
         dItem.hstrSourceName=DrgAddStrHandle(pchTemp);
         dItem.hstrTargetName=DrgAddStrHandle(pchTemp);
         dItem.hstrContainerName = NULLHANDLE;
      }
      free(pchTemp);

      if (pRecord->pScript->pchPathName[0])
         dItem.hstrRMF=DrgAddStrHandle(SCRIPTRMF);
      else
         dItem.hstrRMF=DrgAddStrHandle(EMPTYSCRIPTRMF);

      if (pRecord->hwndSettings)
         dItem.fsControl= DC_OPEN;
      else
         dItem.fsControl= 0;
      dItem.fsSupportedOps=DO_COPYABLE;
      DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

      /* Drag-Image vorbereiten */
      dImage[0].cb=sizeof(DRAGIMAGE);
      dImage[0].hImage=pInit->pRecord->hptrIcon;
      dImage[0].fl=DRG_ICON;
      dImage[0].cxOffset=i*10;
      dImage[0].cyOffset=i*10;

      DrgDrag(hwnd, pDraginfo, dImage, 1, VK_ENDDRAG, NULL);
      DrgFreeDraginfo(pDraginfo);

      /* Source emphasis wegnehmen */
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RxFolderDragOver                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drag-Over des Rexx-Folders                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            pCnrDrag: Drag-Infos vom Container                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT RxFolderDragOver(HWND hwndCnr, PCNRDRAGINFO pCnrDrag)
{
   USHORT usDrop=DOR_NEVERDROP;
   USHORT usDefaultOp=DO_COPY;
   PID pid1, pid2;
   TID tid;

   hwndCnr = hwndCnr;

   if (!pCnrDrag->pRecord)
   {
      DrgAccessDraginfo(pCnrDrag->pDragInfo);
      if (pCnrDrag->pDragInfo->usOperation < DO_UNKNOWN)
      {
         WinQueryWindowProcess(hwndCnr, &pid1, &tid);
         WinQueryWindowProcess(pCnrDrag->pDragInfo->hwndSource, &pid2, &tid);
         if (pid1 != pid2)
         {
            PDRAGITEM pdItem;
            ULONG ulNumItems;
            ULONG i;

            ulNumItems = DrgQueryDragitemCount(pCnrDrag->pDragInfo);

            for (i=0; i<ulNumItems; i++)
            {
               pdItem = DrgQueryDragitemPtr(pCnrDrag->pDragInfo, i);
               if (DrgVerifyRMF(pdItem, "DRM_OS2FILE", NULL) &&
                   pdItem->hstrSourceName &&
                   !(pdItem->fsControl & (DC_CONTAINER | DC_PREPARE | DC_GROUP)))
                  usDrop = DOR_DROP;
            }
         }
         else
            usDrop = DOR_NODROPOP;
      }
      else
         usDrop = DOR_NODROPOP;

      DrgFreeDraginfo(pCnrDrag->pDragInfo);
   }
   else
      usDrop = DOR_NODROP;

   return MRFROM2SHORT(usDrop, usDefaultOp);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RxFolderDrop                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drop      des Rexx-Folders                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            pCnrDrag: Drag-Infos vom Container                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void RxFolderDrop(HWND hwndCnr, PCNRDRAGINFO pCnrDrag, PSCRIPTFOLDERDATA pData)
{
   if (!pCnrDrag->pRecord)
   {
      DrgAccessDraginfo(pCnrDrag->pDragInfo);
      if (pCnrDrag->pDragInfo->usOperation < DO_UNKNOWN)
      {
         PDRAGITEM pdItem;
         ULONG ulNumItems;
         ULONG i;

         ulNumItems = DrgQueryDragitemCount(pCnrDrag->pDragInfo);

         for (i=0; i<ulNumItems; i++)
         {
            pdItem = DrgQueryDragitemPtr(pCnrDrag->pDragInfo, i);
            if (DrgVerifyRMF(pdItem, "DRM_OS2FILE", NULL) &&
                pdItem->hstrSourceName &&
                !(pdItem->fsControl & (DC_CONTAINER | DC_PREPARE | DC_GROUP)))
            {
               /* Script aus diesem Filenamen erzeugen */
               PSCRIPTRECORD pNewScript;
               char *pchTemp;

               pNewScript = AddNewScript(hwndCnr, pData->hptrScript);
               if (pNewScript)
               {
                  ULONG ulLen;

                  if (pdItem->hstrContainerName)
                     DrgQueryStrName(pdItem->hstrContainerName, LEN_PATHNAME+1, pNewScript->pScript->pchPathName);

                  pchTemp = pNewScript->pScript->pchPathName;
                  while(*pchTemp)
                     pchTemp++;
                  if (pdItem->hstrSourceName)
                     DrgQueryStrName(pdItem->hstrSourceName, LEN_PATHNAME+1, pchTemp);

                  if (pdItem->hstrTargetName)
                  {
                     ulLen= DrgQueryStrNameLen(pdItem->hstrTargetName);
                     free(pNewScript->pScript->pchScriptName);
                     pNewScript->pScript->pchScriptName = malloc(ulLen+1);
                     DrgQueryStrName(pdItem->hstrTargetName, ulLen+1, pNewScript->pScript->pchScriptName);
                     pNewScript->RecordCore.pszIcon = pNewScript->pScript->pchScriptName;
                     SendMsg(hwndCnr, CM_INVALIDATERECORD, &pNewScript,
                                MPFROM2SHORT(1, CMA_TEXTCHANGED));
                  }

                  OpenScriptSettings(hwndCnr, pNewScript);
               }
            }
         }
      }
      DrgDeleteDraginfoStrHandles(pCnrDrag->pDragInfo);
      DrgFreeDraginfo(pCnrDrag->pDragInfo);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenScript                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Laesst ein Script laufen                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window-Handle                               */
/*            pRecord: Zeiger auf Container-Record                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 1 Fehler                                                   */
/*                0 OK                                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int OpenScript(HWND hwndCnr, PSCRIPTRECORD pRecord)
{
   if (pRecord->hwndMonitor)
      SetFocus(pRecord->hwndMonitor);
   else
   {
      if (!(pRecord->pScript->ulFlags & REXXFLAG_NOMONITOR))
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                    MPFROM2SHORT(TRUE, CRA_INUSE));

      if (StartRexxScript(pRecord->pScript->ulScriptID, &pRecord->hwndMonitor))
      {
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                    MPFROM2SHORT(FALSE, CRA_INUSE));
         return 1;
      }
   }

   return 0;
}

static int OpenScriptEdit(PSCRIPTRECORD pRecord)
{
   HOBJECT hobj;

   hobj = WinQueryObject(pRecord->pScript->pchPathName);
   if (hobj)
   {
      WinSetObjectData(hobj, "OPEN=DEFAULT");
      return 0;
   }
   else
      return 1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScriptSettingsProc                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur f. Script-Settings                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ScriptSettingsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   POPENSCRIPT pOpenScript;
   PSCRIPTBOOKDATA pBookData = (PSCRIPTBOOKDATA) WinQueryWindowULong(hwnd, QWL_USER);
   MRESULT resultbuf;
   ULONG ulPageID;
   HWND hwndPage;

   switch(msg)
   {
      case WM_INITDLG:
         /* Instanzdaten */
         pBookData = malloc(sizeof(SCRIPTBOOKDATA));
         memset(pBookData, 0, sizeof(SCRIPTBOOKDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pBookData);

         pBookData->notebook = WinWindowFromID(hwnd, IDD_RXSETTINGS+1);

         /* Leere Seiten einfuegen */
         InsertScriptPages(pBookData->notebook, pBookData->PageTable);

         pOpenScript = (POPENSCRIPT) mp2;
         pBookData->pScript = pOpenScript->pScript;

         /* Titel */
         WinSetWindowText(hwnd, pBookData->pScript->pchScriptName);

         /* erste Seite gleich anzeigen */
         LoadPage(pBookData->notebook, &(pBookData->PageTable[0]), pOpenScript);

         RestoreWinPos(hwnd, &pBookData->pScript->SettingsPos, TRUE, TRUE);
         pBookData->bNotify = TRUE;
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         free(pBookData);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_RXSETTINGS+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pBookData && pBookData->bNotify)
         {
            if (SaveWinPos(hwnd, (PSWP) mp1, &pBookData->pScript->SettingsPos, &pBookData->pScript->bDirty))
               scriptlist.bDirty = TRUE;
         }
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(hwnd, msg, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=490;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=350;

         return resultbuf;

      case WM_CLOSE:
         WinPostMsg(hwndRxFolder, RXSET_CLOSE,
                    MPFROMLONG(pBookData->pScript->ulScriptID), NULL);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_RXSETTINGS+1)
            if (SHORT2FROMMP(mp1)==BKN_PAGESELECTED)
            {
               int i=0;
               /* Seitenwechsel */
               /* neue Seite in Seiten-Tabelle suchen */

               while (i<NUM_PAGES_SCRIPT)
               {
                  if (pBookData->PageTable[i].PageID == ((PPAGESELECTNOTIFY)mp2)->ulPageIdNew)
                     break;
                  else
                     i++;
               }

               /* Seite ggf. Laden */
               if (i<NUM_PAGES_SCRIPT && pBookData->PageTable[i].hwndPage==NULLHANDLE)
               {
                  OPENSCRIPT OpenScript;

                  OpenScript.cb = sizeof(OpenScript);
                  OpenScript.pScript = pBookData->pScript;
                  LoadPage(pBookData->notebook, &(pBookData->PageTable[i]), &OpenScript);
               }
            }
         break;

      case RXSET_NEWNAME:
         WinSetWindowText(hwnd, pBookData->pScript->pchScriptName);
         ulPageID = (ULONG) SendMsg(pBookData->notebook, BKM_QUERYPAGEID,
                                       MPFROMLONG(0),
                                       MPFROM2SHORT(BKA_FIRST, BKA_MAJOR));
         if (ulPageID)
         {
            hwndPage = (HWND) SendMsg(pBookData->notebook, BKM_QUERYPAGEWINDOWHWND,
                                         MPFROMLONG(ulPageID), NULL);
            if (hwndPage)
               SendMsg(hwndPage, RXSET_NEWNAME, mp1, mp2);
         }
         break;

      case RXM_CHECKQUICKACCESS:
         ulPageID = (ULONG) SendMsg(pBookData->notebook, BKM_QUERYPAGEID,
                                       MPFROMLONG(0),
                                       MPFROM2SHORT(BKA_FIRST, BKA_MAJOR));
         if (ulPageID)
         {
            hwndPage = (HWND) SendMsg(pBookData->notebook, BKM_QUERYPAGEWINDOWHWND,
                                         MPFROMLONG(ulPageID), NULL);
            if (hwndPage)
               SendMsg(hwndPage, msg, mp1, mp2);
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertScriptPages                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Seiten in das Notebook ein                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: notebook: Window-Handle des Notebooks                          */
/*            Table:    Seitentabelle                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertScriptPages(HWND notebook, NBPAGE *Table)
{
   SetNotebookParams(notebook, 120);

   /* Leere Seiten einfuegen, Tabelle fuellen */
   InsertEmptyPage(notebook, IDST_TAB_RXGENERAL, &(Table[0]));
   Table[0].resID=IDD_RXSET_GENERAL;
   Table[0].DlgProc=RxGeneralProc;
   InsertEmptyPage(notebook, IDST_TAB_RXMONITOR, &(Table[1]));
   Table[1].resID=IDD_RXSET_MONITOR;
   Table[1].DlgProc=RxMonitorProc;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RxGeneralProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. General-Optionen                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY RxGeneralProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PRXSCRIPT pScript = (PRXSCRIPT) WinQueryWindowULong(hwnd, QWL_USER);
   ULONG ulLen;
   char *pchTemp;
   BOOL bTemp;

   switch(msg)
   {
      case WM_INITDLG:
         pScript = ((POPENSCRIPT)mp2)->pScript;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pScript);

         /* Script-Name */
         WinSendDlgItemMsg(hwnd, IDD_RXSET_GENERAL+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(100), NULL);
         WinSetDlgItemText(hwnd, IDD_RXSET_GENERAL+3, pScript->pchScriptName);

         /* Script-File */
         WinSubclassWindow(WinWindowFromID(hwnd, IDD_RXSET_GENERAL+5),
                           FileEntryProc);
         WinSendDlgItemMsg(hwnd, IDD_RXSET_GENERAL+5, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(hwnd, IDD_RXSET_GENERAL+5, pScript->pchPathName);

         /* Quick-Access */
         if (pScript->ulFlags & REXXFLAG_QUICKACCESS)
            WinCheckButton(hwnd, IDD_RXSET_GENERAL+7, TRUE);
         else
            if (!QuickAccessPossible(FALSE))
               WinEnableControl(hwnd, IDD_RXSET_GENERAL+7, FALSE);
         break;

      case WM_DESTROY:
         /* Script-Name */
         ulLen = WinQueryDlgItemTextLength(hwnd, IDD_RXSET_GENERAL+3);

         pchTemp = malloc(ulLen+1);
         pchTemp[0]='\0';

         WinQueryDlgItemText(hwnd, IDD_RXSET_GENERAL+3, ulLen+1, pchTemp);
         if (strcmp(pchTemp, pScript->pchScriptName))
         {
            free(pScript->pchScriptName);
            pScript->pchScriptName=pchTemp;
            pScript->bDirty = TRUE;
            scriptlist.bDirty = TRUE;
         }
         else
            free(pchTemp);

         /* Pfadname */
         pchTemp = malloc(LEN_PATHNAME+1);
         WinQueryDlgItemText(hwnd, IDD_RXSET_GENERAL+5, LEN_PATHNAME+1, pchTemp);
         if (strcmp(pchTemp, pScript->pchPathName))
         {
            strcpy(pScript->pchPathName, pchTemp);
            pScript->bDirty = TRUE;
            scriptlist.bDirty = TRUE;
         }
         free(pchTemp);

         /* Quick-Access */
         bTemp = WinQueryButtonCheckstate(hwnd, IDD_RXSET_GENERAL+7);
         if (bTemp && !(pScript->ulFlags & REXXFLAG_QUICKACCESS))
         {
            pScript->ulFlags |= REXXFLAG_QUICKACCESS;
            pScript->bDirty = TRUE;
            scriptlist.bDirty = TRUE;
         }
         else
            if (!bTemp && (pScript->ulFlags & REXXFLAG_QUICKACCESS))
            {
               pScript->ulFlags &= ~REXXFLAG_QUICKACCESS;
               pScript->bDirty = TRUE;
               scriptlist.bDirty = TRUE;
            }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_RXSET_GENERAL+3)
         {
            if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            {
               ulLen = WinQueryDlgItemTextLength(hwnd, IDD_RXSET_GENERAL+3);

               pchTemp = malloc(ulLen+1);
               pchTemp[0]='\0';

               WinQueryDlgItemText(hwnd, IDD_RXSET_GENERAL+3, ulLen+1, pchTemp);
               if (strcmp(pchTemp, pScript->pchScriptName))
               {
                  free(pScript->pchScriptName);
                  pScript->pchScriptName=pchTemp;
                  pScript->bDirty = TRUE;
                  scriptlist.bDirty = TRUE;
                  SendMsg(hwndRxFolder, RXSET_NEWNAME, MPFROMLONG(pScript->ulScriptID), NULL);
               }
               else
                  free(pchTemp);

            }
         }
         if (SHORT1FROMMP(mp1) == IDD_RXSET_GENERAL+7)
         {
            if (SHORT2FROMMP(mp1) == BN_CLICKED ||
                SHORT2FROMMP(mp1) == BN_DBLCLICKED)
            {
               bTemp = WinQueryButtonCheckstate(hwnd, IDD_RXSET_GENERAL+7);
               if (bTemp)
                 pScript->ulFlags |= REXXFLAG_QUICKACCESS;
               else
                 pScript->ulFlags &= ~REXXFLAG_QUICKACCESS;
               pScript->bDirty = TRUE;
               scriptlist.bDirty = TRUE;

               bTemp = QuickAccessPossible(FALSE);
               if (hwndRxFolder)
                  SendMsg(hwndRxFolder, RXM_CHECKQUICKACCESS,
                             MPFROMLONG(bTemp), NULL);

            }
         }
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDD_RXSET_GENERAL+6) /* Locate-Button */
         {
            char pathnamebuffer[LEN_PATHNAME+1];

            WinQueryDlgItemText(hwnd, IDD_RXSET_GENERAL+5,
                                LEN_PATHNAME+1, pathnamebuffer);
            if (GetPathname(hwnd, pathnamebuffer)==DID_OK)
            {
               WinSetDlgItemText(hwnd, IDD_RXSET_GENERAL+5, pathnamebuffer);
            }
         }
         return (MRESULT) FALSE;

      case RXSET_NEWNAME:
         WinSetDlgItemText(hwnd, IDD_RXSET_GENERAL+3, pScript->pchScriptName);
         break;

      case RXM_CHECKQUICKACCESS:
         bTemp = WinQueryButtonCheckstate(hwnd, IDD_RXSET_GENERAL+7);
         if (!bTemp && !mp1)
            WinEnableControl(hwnd, IDD_RXSET_GENERAL+7, FALSE);
         else
            WinEnableControl(hwnd, IDD_RXSET_GENERAL+7, TRUE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RxMonitorProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Monitor-Optionen                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY RxMonitorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PRXSCRIPT pScript = (PRXSCRIPT) WinQueryWindowULong(hwnd, QWL_USER);
   BOOL bTemp;

   switch(msg)
   {
      case WM_INITDLG:
         pScript = ((POPENSCRIPT)mp2)->pScript;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pScript);

         if (pScript->ulFlags & REXXFLAG_NOMONITOR)
         {
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+1, FALSE);
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+2, TRUE);
            WinEnableControl(hwnd, IDD_RXSET_MONITOR+3, FALSE);
         }
         else
         {
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+1, TRUE);
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+2, FALSE);
            WinEnableControl(hwnd, IDD_RXSET_MONITOR+3, TRUE);
         }
         if (pScript->ulFlags & REXXFLAG_AUTOCLOSE)
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+3, TRUE);
         else
            WinCheckButton(hwnd, IDD_RXSET_MONITOR+3, FALSE);
         SetFocusControl(hwnd, IDD_RXSET_MONITOR+1);
         return (MRESULT) TRUE;

      case WM_DESTROY:
         bTemp = WinQueryButtonCheckstate(hwnd, IDD_RXSET_MONITOR+2);
         if (bTemp && !(pScript->ulFlags & REXXFLAG_NOMONITOR))
         {
            pScript->ulFlags |= REXXFLAG_NOMONITOR;
            pScript->bDirty = TRUE;
            scriptlist.bDirty = TRUE;
         }
         else
            if (!bTemp && (pScript->ulFlags & REXXFLAG_NOMONITOR))
            {
               pScript->ulFlags &= ~REXXFLAG_NOMONITOR;
               pScript->bDirty = TRUE;
               scriptlist.bDirty = TRUE;
            }

         bTemp = WinQueryButtonCheckstate(hwnd, IDD_RXSET_MONITOR+3);
         if (bTemp && !(pScript->ulFlags & REXXFLAG_AUTOCLOSE))
         {
            pScript->ulFlags |= REXXFLAG_AUTOCLOSE;
            pScript->bDirty = TRUE;
            scriptlist.bDirty = TRUE;
         }
         else
            if (!bTemp && (pScript->ulFlags & REXXFLAG_AUTOCLOSE))
            {
               pScript->ulFlags &= ~REXXFLAG_AUTOCLOSE;
               pScript->bDirty = TRUE;
               scriptlist.bDirty = TRUE;
            }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_RXSET_MONITOR+1 ||
             SHORT1FROMMP(mp1) == IDD_RXSET_MONITOR+2)
         {
            if (WinQueryButtonCheckstate(hwnd, IDD_RXSET_MONITOR+1))
               WinEnableControl(hwnd, IDD_RXSET_MONITOR+3, TRUE);
            else
               WinEnableControl(hwnd, IDD_RXSET_MONITOR+3, FALSE);
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QuickAccessPossible                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Prueft, ob ein weiteres Script mit Quick-Access versehen    */
/*               werden kann                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: SelfQuickAccess: Das Script selbst ist schon mit Quick-Access  */
/*                             versehen                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: TRUE  Weiteres Quick-Access moeglich                       */
/*                FALSE Weiteres Quick-Access nicht moeglich                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL QuickAccessPossible(BOOL SelfQuickAccess)
{
   int i=0;
   PRXSCRIPT pScript;

   if (SelfQuickAccess)
      return TRUE;

   pScript = scriptlist.pScripts;

   while(pScript)
   {
      if (pScript->ulFlags & REXXFLAG_QUICKACCESS)
         i++;
      pScript = pScript->next;
   }

   if (i >= MAX_NUM_QUICKACCESS)
      return FALSE;
   else
      return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UpdateRexxMenu                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuellt das Rexx-Menue neu mit den Quick-Access-Entraegen    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndSubMenu: Window-Handle des Rexx-Menues                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void UpdateRexxMenu(HWND hwndSubMenu)
{
   MENUITEM MenuItem;
   int iCount;
   PRXSCRIPT pScript = scriptlist.pScripts;
   PCHAR pchMenuText;
   PCHAR pchSrc, pchDst;
   char pchAccelPrefix[20]="";

   LoadString(IDST_RX_ACCEL, sizeof(pchAccelPrefix), pchAccelPrefix);

   /* Alle Eintraege loeschen */
   for (iCount = IDM_RXQUICK1; iCount <= IDM_RXQUICK10; iCount++)
      SendMsg(hwndSubMenu, MM_DELETEITEM, MPFROM2SHORT(iCount, FALSE), NULL);

   /* Eintraege fuellen */
   iCount = IDM_RXQUICK1;
   while (pScript && iCount <= IDM_RXQUICK10)
   {
      if (pScript->ulFlags & REXXFLAG_QUICKACCESS)
      {
         MenuItem.iPosition   = MIT_END;
         MenuItem.afStyle     = MIS_TEXT;
         MenuItem.afAttribute = 0;
         MenuItem.id          = iCount;
         MenuItem.hwndSubMenu = NULLHANDLE;
         MenuItem.hItem       = 0;

         /* Menue-Text vorbereiten */
         pchMenuText=malloc(strlen(pScript->pchScriptName)+20);
         pchDst=pchMenuText;
         pchSrc=pScript->pchScriptName;
         while(*pchSrc)
         {
            if (*pchSrc == '\r')
               *pchDst++= ' ';
            else
               if (*pchSrc != '\n')
                  *pchDst++ = *pchSrc;
            pchSrc++;
         }
         *pchDst=*pchSrc;
         strcat(pchMenuText, pchAccelPrefix);
         pchDst = strchr(pchMenuText, 0);
         if (iCount == IDM_RXQUICK10)
            strcpy(pchDst, "0");
         else
            _itoa(iCount-IDM_RXQUICK1+1, pchDst, 10);

         SendMsg(hwndSubMenu, MM_INSERTITEM, &MenuItem, pchMenuText);
         free(pchMenuText);
         iCount++;
      }
      pScript = pScript->next;
   }

   return;
}

static MRESULT EXPENTRY RxFolderSettingsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   MRESULT resultbuf;
   PRXFOLDERSETTINGSDATA pData = (PRXFOLDERSETTINGSDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         pData = calloc(1, sizeof(RXFOLDERSETTINGSDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pData);

         pData->notebook = WinWindowFromID(hwnd, IDD_RXFOLDERSETTINGS+1);

         /* Leere Seiten einfuegen */
         InsertRxSettingsPages(pData->notebook, pData->PageTable);

         /* erste Seite gleich anzeigen */
         LoadPage(pData->notebook, &(pData->PageTable[0]), NULL);

         RestoreWinPos(hwnd, &scriptlist.FolderSettingsPos, TRUE, TRUE);
         pData->bNotify=TRUE;
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_RXFOLDERSETTINGS+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pData && pData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &scriptlist.FolderSettingsPos, &scriptlist.bDirty);
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(hwnd, msg, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=490;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=350;

         return resultbuf;

      case WM_DESTROY:
         free(pData);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

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

static void InsertRxSettingsPages(HWND notebook, NBPAGE *Table)
{
   SetNotebookParams(notebook, 120);

   /* Leere Seiten einfuegen, Tabelle fuellen */
   InsertEmptyPage(notebook, IDST_TAB_RXHOOKS, &(Table[0]));
   Table[0].resID=IDD_RXHOOKS;
   Table[0].DlgProc=RxHooksProc;

   return;
}

static MRESULT EXPENTRY RxHooksProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   int i;
   SHORT sSel1=0, sSel2=0;
   SHORT sItem;

   switch(msg)
   {
      case WM_INITDLG:
         for (i=IDD_RXHOOKS+2; i<= IDD_RXHOOKS+4; i+=2)
         {
            HWND hwndList=WinWindowFromID(hwnd, i);
            PRXSCRIPT pScript = scriptlist.pScripts;
            SHORT sItem;
            char pchTemp[50];

            LoadString(IDST_RX_NOHOOK, 50, pchTemp);
            WinInsertLboxItem(hwndList, LIT_END, pchTemp);

            while (pScript)
            {
               sItem = WinInsertLboxItem(hwndList, LIT_END, pScript->pchScriptName);

               SendMsg(hwndList, LM_SETITEMHANDLE, MPFROMSHORT(sItem),
                          MPFROMLONG(pScript->ulScriptID));

               if (i == IDD_RXHOOKS+2 && pScript->ulScriptID == rexxhooks.ulExitID)
                  sSel1 = sItem;
               if (i == IDD_RXHOOKS+4 && pScript->ulScriptID == rexxhooks.ulPreSaveID)
                  sSel2 = sItem;

               pScript = pScript->next;
            }
         }
         WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+2, LM_SELECTITEM, MPFROMSHORT(sSel1),
                           MPFROMSHORT(TRUE));
         WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+4, LM_SELECTITEM, MPFROMSHORT(sSel2),
                           MPFROMSHORT(TRUE));
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         sItem = (USHORT)WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
         if (sItem >= 0)
         {
            ULONG ulID;

            ulID = (ULONG) WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+2, LM_QUERYITEMHANDLE,
                                             MPFROMSHORT(sItem), NULL);
            if (ulID != rexxhooks.ulExitID)
            {
               extern DIRTYFLAGS dirtyflags;

               rexxhooks.ulExitID = ulID;
               dirtyflags.hooksdirty = TRUE;
            }
         }
         sItem = (USHORT)WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+4, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
         if (sItem >= 0)
         {
            ULONG ulID;

            ulID = (ULONG) WinSendDlgItemMsg(hwnd, IDD_RXHOOKS+4, LM_QUERYITEMHANDLE,
                                             MPFROMSHORT(sItem), NULL);
            if (ulID != rexxhooks.ulPreSaveID)
            {
               extern DIRTYFLAGS dirtyflags;

               rexxhooks.ulPreSaveID = ulID;
               dirtyflags.hooksdirty = TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/

