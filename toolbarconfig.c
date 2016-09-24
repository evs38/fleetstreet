/*---------------------------------------------------------------------------+
 | Titel: TOOLBARCONFIG.C                                                    |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 27.09.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Konfiguration der FleetStreet-Toolbar                                 |
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
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "utility.h"
#include "resids.h"
#include "dialogids.h"
#include "controls\toolbar.h"

#include "toolbarconfig.h"

/*--------------------------------- Defines ---------------------------------*/

#define BUTTONTYPE "FleetStreet Button"
#define BUTTONMECH "DRM_FLEET"
#define BUTTONFORMAT "DRF_BUTTON"
#define BUTTONRMF  "<" BUTTONMECH "," BUTTONFORMAT ">"

#define TBCM_REFRESHLISTS  (WM_USER+1)

/*---------------------------------- Typen ----------------------------------*/

/* Gesamt-Daten */

typedef struct
{
   ULONG ulButtonID;        /* IDB_* */
   ULONG ulBitmapID;        /* IDBMP_* (gro疇 Darstellung) */
   ULONG ulSmallBitmapID;   /* IDBMP_* (kleine Darstellung) */
   ULONG ulStringID;        /* IDST_* (Erklвung */
   HBITMAP hbmButton;
} BUTTONLIST, *PBUTTONLIST;

/* Selektionsfenster */

#define LEN_BUTTONTEXT   50

typedef struct buttonselect
{
   struct buttonselect *next;
   struct buttonselect *prev;
   ULONG   ulButtonID;                       /* 0 = Spacer */
   CHAR    pchButtonText[LEN_BUTTONTEXT+1];
   HBITMAP hBitmap;
} BUTTONSELECT, *PBUTTONSELECT;

typedef struct
{
   MINIRECORDCORE RecordCore;
   HBITMAP hbmButton;
   PCHAR   pchButtonText;
   PBUTTONSELECT pButton;
} BUTTONRECORD, *PBUTTONRECORD;

typedef struct
{
   char pchTitleDisp[50];
   char pchTitleAvail[50];
   LONG lMinX;
   LONG lMinY;
   BOOL bNotify;
   PTOOLBARCONFIG pConfig;
   PBUTTONSELECT pDisp;
   PBUTTONSELECT pAvail;
} TBCDATA, *PTBCDATA;

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HWND hwndhelp;

static HBITMAP hbmSeparator;

/* alle definierten Buttons */
static BUTTONLIST ButtonList[]=
{
 /* ButtonID,      BitmapID,          BitmapID (klein),       StringID */
 {IDB_OK,          IDBMP_OK,          IDBMP_OK_S,             IDST_HLP_OK, NULLHANDLE},
 {IDB_CANCEL,      IDBMP_CANCEL,      IDBMP_CANCEL_S,         IDST_HLP_CANCEL, NULLHANDLE},
 {IDB_PREVMSG,     IDBMP_PREVMSG,     IDBMP_PREVMSG_S,        IDST_HLP_PREVMSG, NULLHANDLE},
 {IDB_NEXTMSG,     IDBMP_NEXTMSG,     IDBMP_NEXTMSG_S,        IDST_HLP_NEXTMSG, NULLHANDLE},
 {IDB_PREVREPLY,   IDBMP_PREVREPLY,   IDBMP_PREVREPLY_S,      IDST_HLP_PREVREPLY, NULLHANDLE},
 {IDB_NEXTREPLY,   IDBMP_NEXTREPLY,   IDBMP_NEXTREPLY_S,      IDST_HLP_NEXTREPLY, NULLHANDLE},
 {IDB_FIRSTMSG,    IDBMP_FIRSTMSG,    IDBMP_FIRSTMSG_S,       IDST_HLP_FIRSTMSG, NULLHANDLE},
 {IDB_LASTMSG,     IDBMP_LASTMSG,     IDBMP_LASTMSG_S,        IDST_HLP_LASTMSG, NULLHANDLE},
 {IDB_FIND,        IDBMP_FIND,        IDBMP_FIND_S,           IDST_HLP_FIND, NULLHANDLE},
 {IDB_MSGTREE,     IDBMP_MSGTREE,     IDBMP_MSGTREE_S,        IDST_HLP_MSGTREE, NULLHANDLE},
 {IDB_NEWMSG,      IDBMP_NEWMSG,      IDBMP_NEWMSG_S,         IDST_HLP_NEWMSG, NULLHANDLE},
 {IDB_REPLY,       IDBMP_REPLY,       IDBMP_REPLY_S,          IDST_HLP_REPLY, NULLHANDLE},
 {IDB_IMPORT,      IDBMP_IMPORT,      IDBMP_IMPORT_S,         IDST_HLP_IMPORT, NULLHANDLE},
 {IDB_EXPORT,      IDBMP_EXPORT,      IDBMP_EXPORT_S,         IDST_HLP_EXPORT, NULLHANDLE},
 {IDB_DELMSG,      IDBMP_DELMSG,      IDBMP_DELMSG_S,         IDST_HLP_DELMSG, NULLHANDLE},
 {IDB_EDITMSG,     IDBMP_EDITMSG,     IDBMP_EDITMSG_S,        IDST_HLP_EDITMSG, NULLHANDLE},
 {IDB_AREA,        IDBMP_AREA,        IDBMP_AREA_S,           IDST_HLP_AREA, NULLHANDLE},
 {IDB_MSGLIST,     IDBMP_MSGLIST,     IDBMP_MSGLIST_S,        IDST_HLP_MSGLIST, NULLHANDLE},
 {IDB_PRINTMSG,    IDBMP_PRINTMSG,    IDBMP_PRINTMSG_S,       IDST_HLP_PRINTMSG, NULLHANDLE},
 {IDB_SHOWKLUDGES, IDBMP_SHOWKLUDGES, IDBMP_SHOWKLUDGES_S,    IDST_HLP_SHOWKLUDGES, NULLHANDLE},
 {IDB_HOMEMSG,     IDBMP_HOMEMSG,     IDBMP_HOMEMSG_S,        IDST_HLP_HOMEMSG, NULLHANDLE},
 {IDB_NEXTAREA,    IDBMP_NEXTAREA,    IDBMP_NEXTAREA_S,       IDST_HLP_NEXTAREA, NULLHANDLE},
 {IDB_BOOKMARKS,   IDBMP_BOOKMARKS,   IDBMP_BOOKMARKS_S,      IDST_HLP_WI_RESULTS, NULLHANDLE},
 {IDB_HELP,        IDBMP_HELP,        IDBMP_HELP_S,           IDST_HLP_HE_GENERAL, NULLHANDLE},
 {IDB_CUT,         IDBMP_CUT,         IDBMP_CUT_S,            IDST_HLP_ED_CUT, NULLHANDLE},
 {IDB_COPY,        IDBMP_COPY,        IDBMP_COPY_S,           IDST_HLP_ED_COPY, NULLHANDLE},
 {IDB_PASTE,       IDBMP_PASTE,       IDBMP_PASTE_S,          IDST_HLP_ED_PASTE, NULLHANDLE},
 {IDB_COPYMSG,     IDBMP_COPYMSG,     IDBMP_COPYMSG_S,        IDST_HLP_MS_COPY, NULLHANDLE},
 {IDB_SHELL,       IDBMP_SHELL,       IDBMP_SHELL_S,          IDST_HLP_FI_SHELL, NULLHANDLE},
 {IDB_SCRIPTS,     IDBMP_SCRIPTS,     IDBMP_SCRIPTS_S,        IDST_HLP_RXSCRIPTS, NULLHANDLE},
 {IDB_MOVEMSG,     IDBMP_MOVEMSG,     IDBMP_MOVEMSG_S,        IDST_HLP_MS_MOVE, NULLHANDLE},
 {IDB_BROWSER,     IDBMP_BROWSER,     IDBMP_BROWSER_S,        IDST_HLP_SPCBROWSER, NULLHANDLE},
 {IDB_FORWARD,     IDBMP_FORWARD,     IDBMP_FORWARD_S,        IDST_HLP_MS_FORWARD, NULLHANDLE},
 {IDB_CATCHUP,     IDBMP_CATCHUP,     IDBMP_CATCHUP_S,        IDST_HLP_MS_MARK, NULLHANDLE},
 {IDB_REQUEST,     IDBMP_REQUEST,     IDBMP_REQUEST_S,        IDST_HLP_MS_REQ, NULLHANDLE}
};

#define SIZEOFBUTTONLIST (sizeof(ButtonList)/sizeof(ButtonList[1]))

/* Default-Konfiguration */
static BUTTONCONFIG DefaultConfig[]=
{
 {IDB_HOMEMSG,     0},
 {IDB_NEXTAREA,    0},
 {IDB_PREVMSG,     TBITEM_SPACER},
 {IDB_NEXTMSG,     0},
 {IDB_PREVREPLY,   TBITEM_SPACER},
 {IDB_NEXTREPLY,   0},
 {IDB_FIRSTMSG,    TBITEM_SPACER},
 {IDB_LASTMSG,     0},
 {IDB_OK,          TBITEM_SPACER},
 {IDB_CANCEL,      0},
 {IDB_NEWMSG,      TBITEM_SPACER},
 {IDB_EDITMSG,     0},
 {IDB_REPLY,       0},
 {IDB_DELMSG,      TBITEM_SPACER},
 {IDB_COPYMSG,     0},
 {IDB_MOVEMSG,     0},
 {IDB_FORWARD,     0},
 {IDB_PRINTMSG,    TBITEM_SPACER},
 {IDB_IMPORT,      0},
 {IDB_EXPORT,      0},
 {IDB_FIND,        TBITEM_SPACER},
 {IDB_MSGLIST,     0},
 {IDB_MSGTREE,     0},
 {IDB_AREA,        0},
 {IDB_SHOWKLUDGES, 0},
 {IDB_BOOKMARKS,   0},
 {IDB_SCRIPTS,     0},
 {IDB_BROWSER,     0},
 {IDB_REQUEST,     0},
 {IDB_CATCHUP,     0},
 {IDB_SHELL,       TBITEM_SPACER},
 {IDB_CUT,         TBITEM_SPACER},
 {IDB_COPY,        0},
 {IDB_PASTE,       0},
 {IDB_HELP,        TBITEM_SPACER}
};

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static void CreateBitmaps(HWND hwndDlg);
static void DestroyBitmaps(void);
static PBUTTONSELECT ConfigToButtonSelect(PTOOLBARCONFIG pToolbarConfig);
static PBUTTONSELECT AddButtonToSelect(PBUTTONSELECT *ppFirst, PBUTTONSELECT pCurrent, ULONG ulButtonID);
static BOOL ButtonInSelection(PBUTTONSELECT pButtons, ULONG ulButtonID);
static void InitButtonCnr(HWND hwndCnr, PCHAR pchTitleText);
static void FillButtonCnr(HWND hwndCnr, PBUTTONSELECT pButtonSelect);
static PBUTTONSELECT BuildAvailList(PBUTTONSELECT pSelected);
static void FreeButtonList(PBUTTONSELECT pList);
static void AddToDisp(HWND hwndDlg, PTBCDATA pTBCData);
static void RemoveFromDisp(HWND hwndDlg, PTBCDATA pTBCData);
static void ButtonSelectToConfig(PBUTTONSELECT pButtons, PTOOLBARCONFIG pToolbarConfig);
static void InitButtonDrag(HWND hwndDlg, PCNRDRAGINIT pDragInit);
static MRESULT DragAfter(HWND hwndDlg, PCNRDRAGINFO pCnrDrag);
static void DropButton(HWND hwndDlg, PCNRDRAGINFO pCnrDrag, PTBCDATA pTBCData);
static PBUTTONLIST FindButton(ULONG ulButtonID);


/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void CreateBitmaps(HWND hwndDlg)
{
   int i;
   HPS hps;

   hps = WinGetPS(hwndDlg);
   if (hps)
   {
      for (i=0; i<SIZEOFBUTTONLIST; i++)
         ButtonList[i].hbmButton = GpiLoadBitmap(hps, NULLHANDLE, ButtonList[i].ulSmallBitmapID, 0, 0);

      hbmSeparator = GpiLoadBitmap(hps, NULLHANDLE, IDBMP_SEPARATOR, 0, 0);

      WinReleasePS(hps);
   }
   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void DestroyBitmaps(void)
{
   int i;

   for (i=0; i<SIZEOFBUTTONLIST; i++)
      GpiDeleteBitmap(ButtonList[i].hbmButton);

   GpiDeleteBitmap(hbmSeparator);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static PBUTTONLIST FindButton(ULONG ulButtonID)
{
   int i=0;

   while (i < SIZEOFBUTTONLIST &&
          ButtonList[i].ulButtonID != ulButtonID)
      i++;

   if (i < SIZEOFBUTTONLIST)
      return &(ButtonList[i]);
   else
      return NULL;
}


/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static PBUTTONSELECT ConfigToButtonSelect(PTOOLBARCONFIG pToolbarConfig)
{
   PBUTTONSELECT pFirst=NULL, pCurrent=NULL;
   int i=0;

   for (i=0; i<pToolbarConfig->ulNumButtons; i++)
   {
      if (pToolbarConfig->pButtons[i].ulFlags & TBITEM_SPACER)
      {
         pCurrent = AddButtonToSelect(&pFirst, pCurrent, 0);
      }
      pCurrent = AddButtonToSelect(&pFirst, pCurrent, pToolbarConfig->pButtons[i].ulButtonID);
   }

   return pFirst;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void ButtonSelectToConfig(PBUTTONSELECT pButtons, PTOOLBARCONFIG pToolbarConfig)
{
   ULONG ulNum=0;
   PBUTTONSELECT pTemp=pButtons;
   ULONG ulFlags=0;

   /* Alte Buttons freigeben */

   if (pToolbarConfig->pButtons)
   {
      free(pToolbarConfig->pButtons);
      pToolbarConfig->pButtons=NULL;
      pToolbarConfig->ulNumButtons = 0;
   }

   pToolbarConfig->bDirty = TRUE;

   /* Buttons zaehlen */
   while (pTemp)
   {
      if (pTemp->ulButtonID)
         ulNum++;
      pTemp = pTemp->next;
   }

   if (ulNum)
   {
      pToolbarConfig->pButtons = calloc(ulNum, sizeof(BUTTONCONFIG));
      pToolbarConfig->ulNumButtons = ulNum;

      pTemp = pButtons;
      ulNum=0;

      while (pTemp)
      {
         if (pTemp->ulButtonID)
         {
            pToolbarConfig->pButtons[ulNum].ulButtonID = pTemp->ulButtonID;
            pToolbarConfig->pButtons[ulNum].ulFlags = ulFlags;

            ulFlags=0;

            ulNum++;
         }
         else
            ulFlags = TBITEM_SPACER;

         pTemp = pTemp->next;
      }
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static PBUTTONSELECT AddButtonToSelect(PBUTTONSELECT *ppFirst, PBUTTONSELECT pCurrent, ULONG ulButtonID)
{
   /* Speicher besorgen */
   if (*ppFirst)
   {
      pCurrent->next = calloc(1, sizeof(BUTTONSELECT));
      pCurrent->next->prev = pCurrent;
      pCurrent = pCurrent->next;
   }
   else
      pCurrent = *ppFirst = calloc(1, sizeof(BUTTONSELECT));

   /* Daten laden */
   pCurrent->ulButtonID = ulButtonID;

   if (ulButtonID)
   {
      /* passenden Text u. Bitmap laden */
      PBUTTONLIST pButtonDef = FindButton(ulButtonID);

      if (pButtonDef)
      {
         LoadString(pButtonDef->ulStringID, sizeof(pCurrent->pchButtonText),
                    pCurrent->pchButtonText);

         /* Bitmap */
         pCurrent->hBitmap = pButtonDef->hbmButton;
      }
   }
   else
   {
      /* Text u. Bitmap f. Spacer laden */
      LoadString(IDST_TBC_SEPARATOR, sizeof(pCurrent->pchButtonText),
                 pCurrent->pchButtonText);

      /* Bitmap */
      pCurrent->hBitmap = hbmSeparator;
   }

   return pCurrent;
}


/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static BOOL ButtonInSelection(PBUTTONSELECT pButtons, ULONG ulButtonID)
{
   while (pButtons && pButtons->ulButtonID != ulButtonID)
      pButtons = pButtons->next;

   if (pButtons)
      return TRUE;
   else
      return FALSE;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

MRESULT EXPENTRY ToolbarConfigProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PTBCDATA pTBCData = WinQueryWindowPtr(hwnd, QWL_USER);
   SWP swp;
   MRESULT mrbuffer;

   switch(msg)
   {
      case WM_INITDLG:
         pTBCData = calloc(1, sizeof(TBCDATA));
         WinSetWindowPtr(hwnd, QWL_USER, pTBCData);

         pTBCData->pConfig = (PTOOLBARCONFIG) mp2;

         CreateBitmaps(hwnd);

         LoadString(IDST_TBC_DISP, sizeof(pTBCData->pchTitleDisp), pTBCData->pchTitleDisp);
         LoadString(IDST_TBC_AVAIL, sizeof(pTBCData->pchTitleAvail), pTBCData->pchTitleAvail);

         InitButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+1), pTBCData->pchTitleDisp);
         InitButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+2), pTBCData->pchTitleAvail);

         /* Mindestgroesse ermitteln */
         WinQueryWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+6), &swp);
         pTBCData->lMinX = swp.x + swp.cx +10;
         WinQueryWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+3), &swp);
         pTBCData->lMinY = swp.y + swp.cy +50;

         /* Disp einfuegen */
         pTBCData->pDisp = ConfigToButtonSelect(pTBCData->pConfig);

         /* Avail einfuegen */
         pTBCData->pAvail = BuildAvailList(pTBCData->pDisp);
         FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+1), pTBCData->pDisp);
         FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+2), pTBCData->pAvail);

         RestoreWinPos(hwnd, &pTBCData->pConfig->SelectPos, TRUE, TRUE);
         pTBCData->bNotify = TRUE;
         break;

      case WM_DESTROY:
         FreeButtonList(pTBCData->pDisp);
         FreeButtonList(pTBCData->pAvail);
         free(pTBCData);
         DestroyBitmaps();
         break;

      case WM_QUERYTRACKINFO:
         mrbuffer = WinDefDlgProc(hwnd, msg, mp1, mp2);
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x = pTBCData->lMinX;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y = pTBCData->lMinY;
         return mrbuffer;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MINIMIZE|SWP_MAXIMIZE|SWP_RESTORE))
         {
            RECTL rclClient;

            rclClient.xLeft=0;
            rclClient.xRight=((PSWP)mp1)->cx;
            rclClient.yBottom=0;
            rclClient.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, hwnd, &rclClient);

            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+3), &swp);
            rclClient.yBottom += swp.y + swp.cy;

            WinSetWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+1),
                            NULLHANDLE,
                            0, 0,
                            (rclClient.xRight - rclClient.xLeft) /2,
                            (rclClient.yTop - rclClient.yBottom),
                            SWP_SIZE);

            WinSetWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+2),
                            NULLHANDLE,
                            rclClient.xLeft + (rclClient.xRight - rclClient.xLeft)/2 + 1,
                            rclClient.yBottom,
                            (rclClient.xRight - rclClient.xLeft) /2,
                            (rclClient.yTop - rclClient.yBottom),
                            SWP_SIZE | SWP_MOVE);

            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+3), &swp);
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+3),
                            NULLHANDLE,
                            rclClient.xLeft + (rclClient.xRight - rclClient.xLeft)/2 - swp.cx/2,
                            swp.y,
                            0, 0,
                            SWP_MOVE);
            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+4), &swp);
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+4),
                            NULLHANDLE,
                            rclClient.xLeft + (rclClient.xRight - rclClient.xLeft)/2 - swp.cx/2,
                            swp.y,
                            0, 0,
                            SWP_MOVE);

         }
         break;

      case WM_WINDOWPOSCHANGED:
         if (pTBCData && pTBCData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &pTBCData->pConfig->SelectPos, &pTBCData->pConfig->bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               ButtonSelectToConfig(pTBCData->pDisp, pTBCData->pConfig);
               break;

            case DID_CANCEL:
               break;

            case IDD_TOOLBARCONFIG+3: /* Hinzufuegen */
               AddToDisp(hwnd, pTBCData);
               return 0;

            case IDD_TOOLBARCONFIG+4: /* Entfernen */
               RemoveFromDisp(hwnd, pTBCData);
               return 0;

            case IDD_TOOLBARCONFIG+5: /* Default */
               /* @@ Sicherheitsabfrage */

               /* alte Konfiguration freigeben */
               if (pTBCData->pConfig->pButtons)
               {
                  free(pTBCData->pConfig->pButtons);
                  pTBCData->pConfig->pButtons = NULL;
                  pTBCData->pConfig->ulNumButtons = 0;
               }

               /* Default-Konfiguration */
               pTBCData->pConfig->pButtons = malloc(sizeof(DefaultConfig));
               memcpy(pTBCData->pConfig->pButtons, DefaultConfig, sizeof(DefaultConfig));
               pTBCData->pConfig->ulNumButtons = sizeof(DefaultConfig)/sizeof(DefaultConfig[0]);

               pTBCData->pConfig->bDirty = TRUE;

               /* Alte Selektionen freigeben */
               FreeButtonList(pTBCData->pDisp);
               FreeButtonList(pTBCData->pAvail);

               /* neue Selektion */
               pTBCData->pDisp = ConfigToButtonSelect(pTBCData->pConfig);

               /* Avail einfuegen */
               pTBCData->pAvail = BuildAvailList(pTBCData->pDisp);
               FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+1), pTBCData->pDisp);
               FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+2), pTBCData->pAvail);
               return 0;

            default:
               return 0;
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_TOOLBARCONFIG+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_INITDRAG:
                  InitButtonDrag(hwnd, (PCNRDRAGINIT) mp2);
                  break;

               case CN_DROP:
                  DropButton(hwnd, (PCNRDRAGINFO) mp2, pTBCData);
                  break;

               case CN_DRAGAFTER:
                  return DragAfter(hwnd, (PCNRDRAGINFO) mp2);

               default:
                  break;
            }
         }
         if (SHORT1FROMMP(mp1) == IDD_TOOLBARCONFIG+2)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_INITDRAG:
                  InitButtonDrag(hwnd, (PCNRDRAGINIT) mp2);
                  break;

               default:
                  break;
            }
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case TBCM_REFRESHLISTS:
         FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+1), pTBCData->pDisp);
         FillButtonCnr(WinWindowFromID(hwnd, IDD_TOOLBARCONFIG+2), pTBCData->pAvail);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void LoadDefaultToolbar(PTOOLBARCONFIG pConfig)
{
   /* alte Konfiguration freigeben */
   if (pConfig->pButtons)
   {
      free(pConfig->pButtons);
      pConfig->pButtons = NULL;
      pConfig->ulNumButtons = 0;
   }

   /* Default-Konfiguration */
   pConfig->pButtons = malloc(sizeof(DefaultConfig));
   memcpy(pConfig->pButtons, DefaultConfig, sizeof(DefaultConfig));
   pConfig->ulNumButtons = sizeof(DefaultConfig)/sizeof(DefaultConfig[0]);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void InitButtonCnr(HWND hwndCnr, PCHAR pchTitleText)
{
   CNRINFO cnrinfo;
   PFIELDINFO pField=NULL, pFirstField=NULL;
   FIELDINFOINSERT InfoInsert;

   pFirstField = SendMsg(hwndCnr, CM_ALLOCDETAILFIELDINFO, MPFROMSHORT(2), NULL);
   pField = pFirstField;

   if (pField)
   {
      pField->cb = sizeof(FIELDINFO);
      pField->flData = CFA_BITMAPORICON | CFA_HORZSEPARATOR | CFA_SEPARATOR;
      pField->flTitle = 0;
      pField->offStruct = FIELDOFFSET(BUTTONRECORD, hbmButton);

      pField = pField->pNextFieldInfo;

      pField->cb = sizeof(FIELDINFO);
      pField->flData = CFA_STRING | CFA_HORZSEPARATOR;
      pField->flTitle = 0;
      pField->offStruct = FIELDOFFSET(BUTTONRECORD, pchButtonText);

      InfoInsert.cb = sizeof(InfoInsert);
      InfoInsert.cFieldInfoInsert = 2;
      InfoInsert.pFieldInfoOrder = (PFIELDINFO) CMA_FIRST;
      InfoInsert.fInvalidateFieldInfo = TRUE;

      SendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO, pFirstField, &InfoInsert);
   }

   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.flWindowAttr = CV_DETAIL | CA_DRAWBITMAP | CA_ORDEREDTARGETEMPH |
                          CA_CONTAINERTITLE | CA_TITLEREADONLY | CA_TITLESEPARATOR;
   cnrinfo.pszCnrTitle = pchTitleText;
   cnrinfo.slBitmapOrIcon.cx = 15;
   cnrinfo.slBitmapOrIcon.cy = 15;

   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo,
           MPFROMLONG(CMA_FLWINDOWATTR | CMA_CNRTITLE | CMA_SLBITMAPORICON));

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void FillButtonCnr(HWND hwndCnr, PBUTTONSELECT pButtonSelect)
{
   ULONG ulNum=0;
   PBUTTONSELECT pTemp=pButtonSelect;
   PBUTTONRECORD pFirstRecord=NULL, pRecord=NULL;
   RECORDINSERT RecordInsert;

   SendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));

   /* zaehlen */
   while (pTemp)
   {
      ulNum++;
      pTemp = pTemp->next;
   }

   /* Records anfordern */
   pFirstRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                          MPFROMLONG(sizeof(BUTTONRECORD)-sizeof(MINIRECORDCORE)),
                          MPFROMSHORT(ulNum));
   pRecord = pFirstRecord;

   pTemp = pButtonSelect;
   while (pTemp && pRecord)
   {
      pRecord->pButton = pTemp;
      pRecord->pchButtonText = pTemp->pchButtonText;
      pRecord->hbmButton = pTemp->hBitmap;

      pTemp = pTemp->next;
      pRecord = (PBUTTONRECORD) pRecord->RecordCore.preccNextRecord;
   }

   if (ulNum)
   {
      RecordInsert.cb = sizeof(RecordInsert);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_FIRST;
      RecordInsert.pRecordParent = NULL;
      RecordInsert.fInvalidateRecord = TRUE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = ulNum;

      SendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);
   }
   else
      SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static PBUTTONSELECT BuildAvailList(PBUTTONSELECT pSelected)
{
   int i;
   PBUTTONSELECT pFirst=NULL, pCurrent=NULL;

   for (i=0; i<SIZEOFBUTTONLIST; i++)
   {
      if (!ButtonInSelection(pSelected, ButtonList[i].ulButtonID))
         pCurrent = AddButtonToSelect(&pFirst, pCurrent, ButtonList[i].ulButtonID);
   }

   /* Separator hinzufuegen */
   pCurrent = AddButtonToSelect(&pFirst, pCurrent, 0);

   return pFirst;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void FreeButtonList(PBUTTONSELECT pList)
{
   PBUTTONSELECT pTemp;

   while (pList)
   {
      pTemp = pList->next;

      free(pList);
      pList = pTemp;
   }
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void AddToDisp(HWND hwndDlg, PTBCDATA pTBCData)
{
   PBUTTONRECORD pButtonRecord;
   PBUTTONSELECT pNewButton;

   pButtonRecord = WinSendDlgItemMsg(hwndDlg, IDD_TOOLBARCONFIG+2, CM_QUERYRECORDEMPHASIS,
                                     MPFROMLONG(CMA_FIRST),
                                     MPFROMSHORT(CRA_CURSORED));
   if (pButtonRecord && pButtonRecord->pButton)
   {
      if (pTBCData->pDisp)
      {
         /* Am Ende anfuegen */
         pNewButton = pTBCData->pDisp;
         while (pNewButton->next)
            pNewButton = pNewButton->next;
         pNewButton->next = calloc(1, sizeof(BUTTONSELECT));
         pNewButton->next->prev = pNewButton;
         pNewButton = pNewButton->next;
      }
      else
         pTBCData->pDisp = pNewButton = calloc(1, sizeof(BUTTONSELECT));

      /* Daten kopieren */
      pNewButton->ulButtonID = pButtonRecord->pButton->ulButtonID;
      memcpy(pNewButton->pchButtonText, pButtonRecord->pButton->pchButtonText, LEN_BUTTONTEXT);
      pNewButton->hBitmap = pButtonRecord->pButton->hBitmap;

      if (pButtonRecord->pButton->ulButtonID)
      {
         /* in Quell-Liste loeschen */
         if (pButtonRecord->pButton->next)
            pButtonRecord->pButton->next->prev = pButtonRecord->pButton->prev;
         if (pButtonRecord->pButton->prev)
            pButtonRecord->pButton->prev->next = pButtonRecord->pButton->next;
         if (pButtonRecord->pButton == pTBCData->pAvail)
            pTBCData->pAvail = pButtonRecord->pButton->next;
         free(pButtonRecord->pButton);
      }

      /* neu anzeigen */
      FillButtonCnr(WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+1), pTBCData->pDisp);
      FillButtonCnr(WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+2), pTBCData->pAvail);
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void RemoveFromDisp(HWND hwndDlg, PTBCDATA pTBCData)
{
   PBUTTONRECORD pButtonRecord;
   PBUTTONSELECT pNewButton;

   pButtonRecord = WinSendDlgItemMsg(hwndDlg, IDD_TOOLBARCONFIG+1, CM_QUERYRECORDEMPHASIS,
                                     MPFROMLONG(CMA_FIRST),
                                     MPFROMSHORT(CRA_CURSORED));
   if (pButtonRecord && pButtonRecord->pButton)
   {
      if (pButtonRecord->pButton->ulButtonID)
      {
         if (pTBCData->pAvail)
         {
            /* Am Ende anfuegen */
            pNewButton = pTBCData->pAvail;
            while (pNewButton->next)
               pNewButton = pNewButton->next;
            pNewButton->next = calloc(1, sizeof(BUTTONSELECT));
            pNewButton->next->prev = pNewButton;
            pNewButton = pNewButton->next;
         }
         else
            pTBCData->pAvail = pNewButton = calloc(1, sizeof(BUTTONSELECT));

         /* Daten kopieren */
         pNewButton->ulButtonID = pButtonRecord->pButton->ulButtonID;
         memcpy(pNewButton->pchButtonText, pButtonRecord->pButton->pchButtonText, LEN_BUTTONTEXT);
         pNewButton->hBitmap = pButtonRecord->pButton->hBitmap;
      }

      /* in Quell-Liste loeschen */
      if (pButtonRecord->pButton->next)
         pButtonRecord->pButton->next->prev = pButtonRecord->pButton->prev;
      if (pButtonRecord->pButton->prev)
         pButtonRecord->pButton->prev->next = pButtonRecord->pButton->next;
      if (pButtonRecord->pButton == pTBCData->pDisp)
         pTBCData->pDisp = pButtonRecord->pButton->next;
      free(pButtonRecord->pButton);

      /* neu anzeigen */
      FillButtonCnr(WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+1), pTBCData->pDisp);
      FillButtonCnr(WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+2), pTBCData->pAvail);
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void InitButtonDrag(HWND hwndDlg, PCNRDRAGINIT pDragInit)
{
   PBUTTONRECORD pDragButton;
   PDRAGINFO pDraginfo;
   DRAGITEM DragItem;
   DRAGIMAGE DragImage;

   if (!pDragInit || !pDragInit->pRecord)
      return; /* nur echte Records */

   pDragButton = (PBUTTONRECORD) pDragInit->pRecord;

   pDraginfo = DrgAllocDraginfo(1); /* ein Item */

   /* Item vorbereiten */
   memset(&DragItem, 0, sizeof(DragItem));
   DragItem.hwndItem = pDragInit->hwndCnr;
   DragItem.ulItemID = (ULONG) pDragButton->pButton;
   DragItem.hstrType = DrgAddStrHandle(BUTTONTYPE);
   DragItem.hstrRMF  = DrgAddStrHandle(BUTTONRMF);
   DragItem.fsSupportedOps = DO_MOVEABLE;

   DrgSetDragitem(pDraginfo, &DragItem, sizeof(DragItem), 0);

   /* Image vorbereiten */
   memset(&DragImage, 0, sizeof(DragImage));

   DragImage.cb = sizeof(DragImage);
   DragImage.hImage = pDragButton->hbmButton;
   DragImage.fl = DRG_BITMAP;

   /* Drag */
   SendMsg(pDragInit->hwndCnr, CM_SETRECORDEMPHASIS, pDragButton, MPFROM2SHORT(TRUE, CRA_SOURCE));
   if (!DrgDrag(hwndDlg, pDraginfo, &DragImage, 1, VK_ENDDRAG, NULL))
      WinPostMsg(hwndDlg, TBCM_REFRESHLISTS, NULL, NULL);

   DrgFreeDraginfo(pDraginfo);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static MRESULT DragAfter(HWND hwndDlg, PCNRDRAGINFO pCnrDrag)
{
   USHORT usDefOp = DO_MOVE;
   USHORT usDrop = DOR_NODROP;
   PBUTTONRECORD pAfterRecord;
   PDRAGITEM pDragItem;

   pAfterRecord = (PBUTTONRECORD) pCnrDrag->pRecord;

   DrgAccessDraginfo(pCnrDrag->pDragInfo);

   pDragItem = DrgQueryDragitemPtr(pCnrDrag->pDragInfo, 0);

   if (DrgVerifyType(pDragItem, BUTTONTYPE))
      if (DrgVerifyRMF(pDragItem, BUTTONMECH, BUTTONFORMAT))
         if (hwndDlg == pCnrDrag->pDragInfo->hwndSource)
            if (pDragItem->hwndItem != WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+1) ||
                pAfterRecord)
               if (pCnrDrag->pDragInfo->usOperation == DO_MOVE ||
                   pCnrDrag->pDragInfo->usOperation == DO_DEFAULT)
                  usDrop = DOR_DROP;
               else
                  usDrop = DOR_NODROPOP;

   DrgFreeDraginfo(pCnrDrag->pDragInfo);

   return MRFROM2SHORT(usDrop, usDefOp);
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void DropButton(HWND hwndDlg, PCNRDRAGINFO pCnrDrag, PTBCDATA pTBCData)
{
   PBUTTONRECORD pDropRecord;
   PDRAGITEM pDragItem;
   PBUTTONSELECT pSrcButton, pTargetButton=NULL, pNewButton;

   pDropRecord = (PBUTTONRECORD) pCnrDrag->pRecord;

   DrgAccessDraginfo(pCnrDrag->pDragInfo);

   pDragItem = DrgQueryDragitemPtr(pCnrDrag->pDragInfo, 0);
   pSrcButton = (PBUTTONSELECT) pDragItem->ulItemID;
   if (pDropRecord && pDropRecord != (PBUTTONRECORD) CMA_FIRST)
      pTargetButton = pDropRecord->pButton;

   if (pDragItem->hwndItem == WinWindowFromID(hwndDlg, IDD_TOOLBARCONFIG+1))
   {
      /* Drag innerhalb d. linken Containers */
      if (pSrcButton != pTargetButton &&
          pSrcButton->prev != pTargetButton)
      {
         /* Position hat sich geaendert */
         /* Src aushaengen */
         if (pSrcButton->next)
            pSrcButton->next->prev = pSrcButton->prev;
         if (pSrcButton->prev)
            pSrcButton->prev->next = pSrcButton->next;
         if (pTBCData->pDisp == pSrcButton)
            pTBCData->pDisp = pSrcButton->next;

         /* hinter Target einhaengen */
         pSrcButton->next = pTargetButton->next;
         pSrcButton->prev = pTargetButton;
         if (pTargetButton->next)
            pTargetButton->next->prev = pSrcButton;
         pTargetButton->next = pSrcButton;

         WinPostMsg(hwndDlg, TBCM_REFRESHLISTS, NULL, NULL);
      }
   }
   else
   {
      /* Drag v. rechts nach links */
      if (!pTargetButton)
      {
         if (pDropRecord == (PBUTTONRECORD) CMA_FIRST)
         {
            /* am Anfang einhaengen */
            pNewButton = calloc(1, sizeof(BUTTONSELECT));
            pNewButton->next = pTBCData->pDisp;
            if (pNewButton->next)
               pNewButton->next->prev = pNewButton;
            pTBCData->pDisp = pNewButton;
         }
         else
            /* Am Ende einhaengen */
            if (pTBCData->pDisp)
            {
               pNewButton = pTBCData->pDisp;
               while (pNewButton->next)
                  pNewButton = pNewButton->next;
               pNewButton->next = calloc(1, sizeof(BUTTONSELECT));
               pNewButton->next->prev = pNewButton;
               pNewButton = pNewButton->next;
            }
            else
               pNewButton = pTBCData->pDisp = calloc(1, sizeof(BUTTONSELECT));
      }
      else
      {
         /* Bei best. Stelle einfuegen */
         pNewButton = calloc(1, sizeof(BUTTONSELECT));
         pNewButton->prev = pTargetButton;
         pNewButton->next = pTargetButton->next;
         if (pTargetButton->next)
            pTargetButton->next->prev = pNewButton;
         pTargetButton->next = pNewButton;
      }

      /* Daten kopieren */
      pNewButton->ulButtonID = pSrcButton->ulButtonID;
      pNewButton->hBitmap = pSrcButton->hBitmap;
      memcpy(pNewButton->pchButtonText, pSrcButton->pchButtonText, LEN_BUTTONTEXT);

      if (pSrcButton->ulButtonID)
      {
         /* Non-Spacer in Src. loeschen */
         if (pSrcButton->next)
            pSrcButton->next->prev = pSrcButton->prev;
         if (pSrcButton->prev)
            pSrcButton->prev->next = pSrcButton->next;
         if (pTBCData->pAvail == pSrcButton)
            pTBCData->pAvail = pSrcButton->next;
         free(pSrcButton);
      }
      WinPostMsg(hwndDlg, TBCM_REFRESHLISTS, NULL, NULL);
   }

   DrgDeleteDraginfoStrHandles(pCnrDrag->pDragInfo);
   DrgFreeDraginfo(pCnrDrag->pDragInfo);

   return;
}

void RefreshToolbar(HWND hwndToolbar, PTOOLBARCONFIG pConfig, BOOL bSmallIcons)
{
   int i;
   TOOLBARITEM Item;
   PBUTTONLIST pButtonDef;

   SendMsg(hwndToolbar, TBM_DELETEALLITEMS, NULL, NULL);
   for (i=0; i<pConfig->ulNumButtons; i++)
   {
      if (pButtonDef = FindButton(pConfig->pButtons[i].ulButtonID))
      {
         Item.ulCommandID = pConfig->pButtons[i].ulButtonID;
         Item.ulParamSize=0;
         Item.pItemParams = NULL;
         Item.ulFlags = pConfig->pButtons[i].ulFlags;
         if (bSmallIcons)
            Item.ulBitmapID = pButtonDef->ulSmallBitmapID;
         else
            Item.ulBitmapID = pButtonDef->ulBitmapID;

         SendMsg(hwndToolbar, TBM_ADDITEM, &Item, MPFROMLONG(ADDITEM_LAST));
      }
   }

   return;
}

ULONG QueryBitmap(ULONG ulButtonID, BOOL bSmall)
{
   PBUTTONLIST pButtonDef;

   pButtonDef = FindButton(ulButtonID);

   if (pButtonDef)
   {
      if (bSmall)
         return pButtonDef->ulSmallBitmapID;
      else
         return pButtonDef->ulBitmapID;
   }
   else
      return 0;

}
/*-------------------------------- Modulende --------------------------------*/

