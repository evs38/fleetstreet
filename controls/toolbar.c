/*---------------------------------------------------------------------------+
 | Titel: TOOLBAR.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Variable Toolbar von FleetStreet                                       |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "..\dump\pmassert.h"
#include "toolbar.h"
#include "toolbar_int.h"

/*--------------------------------- Defines ---------------------------------*/

/* #define FULLFUNC */

#define EXTRA_BYTES   sizeof(PTOOLBARDATA)
#define STARTID       10000

/* Notification-Codes der neuen Buttons */

#define BN_CONTEXTMENU    10
#define BN_BEGINDRAG      11
#define BN_DRAGOVER       12
#define BN_DRAGLEAVE      13
#define BN_DROP           14
#define BN_DROPHELP       15

/* interne Messages */
#define TBIM_ITEMDROPPED  (WM_USER+100)

#define TBID_SCROLL      1

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/
static MRESULT (* EXPENTRY OldButtonProc)(HWND, ULONG, MPARAM, MPARAM);

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY ToolbarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static BOOL CreateToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, PTBCTLDATA pCtlData, PCREATESTRUCT pCreateStruct);
static void DestroyToolbar(HWND hwnd, PTOOLBARDATA pToolbarData);
static void PaintToolbar(HWND hwnd, PTOOLBARDATA pToolbarData);
static BOOL SetWindowParams(HWND hwnd, PTOOLBARDATA pToolbarData, PWNDPARAMS pWndParams);
static BOOL QueryWindowParams(HWND hwnd, PTOOLBARDATA pToolbarData, PWNDPARAMS pWndParams);
static void SizeToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, SHORT sNewX);
static void ScrollToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, SHORT sSliderPos, SHORT sCmd);
static void SetScrollParams(PTOOLBARDATA pToolbarData);

static ULONG AddItem(HWND hwnd, PTOOLBARDATA pToolbarData, PTOOLBARITEM pNewItem, ULONG ulFlags);
static BOOL DeleteAllItems(HWND hwnd, PTOOLBARDATA pToolbarData);
static void RepositionButtons(HWND hwnd, PTOOLBARDATA pToolbarData);
static void RepositionButtonsVert(HWND hwnd, PTOOLBARDATA pToolbarData);
static void RepositionButtonsHorz(HWND hwnd, PTOOLBARDATA pToolbarData);
static void CalcRequiredSize(HWND hwnd, PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY, LONG lNewX);
static void CalcRequiredSizeVert(PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY);
static void CalcRequiredSizeHorz(PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY, LONG lNewX);
static BOOL EnableCmdItems(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulCommandID, BOOL bNewState);
static PTBITEMLIST FindItem(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID);
static ULONG QueryFirstItem(PTOOLBARDATA pToolbarData, PTOOLBARITEM pDestBuff);
static ULONG QueryNextItem(PTOOLBARDATA pToolbarData, PTOOLBARITEM pDestBuff);
static BOOL QueryItemData(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PTOOLBARITEM pDestBuff);
static BOOL SetItemData(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PTOOLBARITEM pNewData);
static BOOL IsCmdEnabled(PTOOLBARDATA pToolbarData, ULONG ulCommandID);
static MRESULT EXPENTRY NewButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);

#ifdef FULLFUNC
static BOOL DeleteItem(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID);
static PVOID QueryItemParams(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID);
static HBITMAP QueryItemBitmap(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID);
static BOOL BeginButtonDrag(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID);
static void DiscardObject(HWND hwnd, PTOOLBARDATA pToolbarData, PDRAGINFO pDraginfo);
static MRESULT DragOver(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PDRAGINFO pDraginfo);
static void TBDrop(HWND hwnd, ULONG ulButtonID, PDRAGINFO pDraginfo);
static void TBPerformDrop(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PDRAGINFO pDraginfo);
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterToolbar                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Registriert die Fensterklasse f〉 die Toolbar               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anchor-Block der Anwendung                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:   Erfolg                                             */
/*                FALSE:  Fehlschlag                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL EXPENTRY RegisterToolbar(HAB hab)
{
   if (WinRegisterClass(hab,
                        WC_TOOLBAR,
                        ToolbarProc,
                        CS_SYNCPAINT | CS_SIZEREDRAW,
                        EXTRA_BYTES))
      return TRUE;
   else
      return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ToolbarProc                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur der Toolbar-Klasse                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ToolbarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PTOOLBARDATA pToolbarData = (PTOOLBARDATA) WinQueryWindowULong(hwnd, QWL_USER);
   PTBITEMLIST pItem;

   switch(msg)
   {
      /* --- Standard-Window-Messages --- */
      case WM_CREATE:
         pToolbarData = calloc(1, sizeof(TOOLBARDATA));
         PMASSERT(pToolbarData != NULL, "Out of memory");
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pToolbarData);
         return (MRESULT) CreateToolbar(hwnd, pToolbarData, (PTBCTLDATA) mp1, (PCREATESTRUCT) mp2);

      case WM_DESTROY:
         DestroyToolbar(hwnd, pToolbarData);
         free(pToolbarData);
         break;

      case WM_SETWINDOWPARAMS:
         return (MRESULT) SetWindowParams(hwnd, pToolbarData, (PWNDPARAMS) mp1);

      case WM_QUERYWINDOWPARAMS:
         return (MRESULT) QueryWindowParams(hwnd, pToolbarData, (PWNDPARAMS) mp1);

      case WM_SIZE:
         SizeToolbar(hwnd, pToolbarData, SHORT1FROMMP(mp2));
         break;

      case WM_MOVE:
         break;

      case WM_PAINT:
         PaintToolbar(hwnd, pToolbarData);
         break;

      case WM_PRESPARAMCHANGED:
         /* neu Zeichnen */
         WinInvalidateRect(hwnd, NULL, TRUE);
         break;

      case WM_CONTROLPOINTER:
         pItem = FindItem(hwnd, pToolbarData, SHORT1FROMMP(mp1));
         if (pItem)
         {
            /* Weiterleiten an Owner */
            return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), msg, MPFROMSHORT(pItem->TBItem.ulCommandID), mp2);
         }
         else
            break;

      case WM_COMMAND:
         pItem = FindItem(hwnd, pToolbarData, SHORT1FROMMP(mp1));
         if (pItem)
         {
            /* Weiterleiten an Owner */
            WinPostMsg(WinQueryWindow(hwnd, QW_OWNER), msg,
                       MPFROM2SHORT(pItem->TBItem.ulCommandID, SHORT1FROMMP(mp1)), mp2);
         }
         break;

      case WM_CONTROL:
         switch(SHORT2FROMMP(mp1)) /* Annahme: es gibt nur Buttons */
         {
            case BN_CONTEXTMENU:
               return Notify(hwnd, TBN_CONTEXTMENU, MPFROMLONG(SHORT1FROMMP(mp1)));

#ifdef FULLFUNC
            case BN_BEGINDRAG:
               return (MRESULT) BeginButtonDrag(hwnd, pToolbarData, (ULONG) SHORT1FROMMP(mp1));

            case BN_DRAGOVER:
               return DragOver(hwnd, pToolbarData, SHORT1FROMMP(mp1), (PDRAGINFO) mp2);

            case BN_DROP:
               TBDrop(hwnd, SHORT1FROMMP(mp1), (PDRAGINFO) mp2);
               break;
#endif

            default:
               break;
         }
         break;

      case WM_HSCROLL:
         ScrollToolbar(hwnd, pToolbarData, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
         break;

      case WM_HELP:
         if (mp1 && (USHORT)mp1 != TBID_SCROLL)
         {
            pItem = FindItem(hwnd, pToolbarData, SHORT1FROMMP(mp1));
            if (pItem)
            {
               /* Weiterleiten an Owner */
               WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_HELP,
                          MPFROMSHORT(pItem->TBItem.ulCommandID), mp2);
            }
         }
         else
            WinPostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_HELP,
                       MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)), mp2);
         return (MRESULT) FALSE;

      case WM_MOUSEMOVE:
         WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROLPOINTER,
                    MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                    NULL);
         break;

      case WM_CONTEXTMENU:
         Notify(hwnd, TBN_CONTEXTMENU, NULL);
         break;

#ifdef FULLFUNC
      case DM_DRAGOVER:
         return DragOver(hwnd, pToolbarData, 0 /* ButtonID */, (PDRAGINFO) mp1);

      case DM_DRAGLEAVE:
         return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                           MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), TBN_DRAGLEAVE),
                           mp1);

      case DM_DROP:
         TBDrop(hwnd, 0, (PDRAGINFO) mp1);
         return (MRESULT) FALSE;

      case DM_DROPHELP:
         return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                           MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), TBN_DROPHELP),
                           mp1);

      case DM_ENDCONVERSATION:
         DrgFreeDraginfo(pToolbarData->pDraginfo);
         pToolbarData->pDraginfo=NULL;
         break;

      case DM_DISCARDOBJECT:
         DiscardObject(hwnd, pToolbarData, (PDRAGINFO) mp1);
         return (MRESULT) DRR_SOURCE;
#endif

#if 0
      case WM_BUTTON1DOWN:
      case WM_BUTTON2DOWN:
      case WM_BUTTON3DOWN:
         WinSetFocus(HWND_DESKTOP, hwnd);
         break;
#endif

      case WM_ADJUSTWINDOWPOS:
         {
            PSWP pSWP=(PSWP) mp1;

            CalcRequiredSize(hwnd, pToolbarData, &pSWP->cx, &pSWP->cy, pSWP->cx);
         }
         return 0;

      /* --- Toolbar-Messages --- */
      case TBM_ADDITEM:    /* mp1: PTOOLBARITEM pNewItem; mp2: ULONG ulPosition */
         return (MRESULT) AddItem(hwnd, pToolbarData, (PTOOLBARITEM) mp1, (ULONG) mp2);

#ifdef FULLFUNC
      case TBM_DELETEITEM: /* mp1: ULONG ulButtonID */
         return (MRESULT) DeleteItem(hwnd, pToolbarData, (ULONG) mp1);
#endif

      case TBM_DELETEALLITEMS:
         return (MRESULT) DeleteAllItems(hwnd, pToolbarData);

      case TBM_QUERYITEMDATA: /* mp1: ULONG ulButtonID; mp2: PTOOLBARITEM pDestBuff */
         return (MRESULT) QueryItemData(hwnd, pToolbarData, (ULONG) mp1, (PTOOLBARITEM) mp2);

      case TBM_SETITEMDATA: /* mp1: ULONG ulButtonID; mp2: PTOOLBARITEM pNewData */
         return (MRESULT) SetItemData(hwnd, pToolbarData, (ULONG) mp1, (PTOOLBARITEM) mp2);

      case TBM_QUERYITEMCOUNT:
         return (MRESULT) pToolbarData->ulItemCount;

      case TBM_ENABLECMD: /* mp1: ULONG ulCommandID */
         return (MRESULT) EnableCmdItems(hwnd, pToolbarData, (ULONG) mp1, TRUE);

      case TBM_DISABLECMD: /* mp1: ULONG ulCommandID */
         return (MRESULT) EnableCmdItems(hwnd, pToolbarData, (ULONG) mp1, FALSE);

      case TBM_ISCMDENABLED: /* mp1: ULONG ulCommandID */
         return (MRESULT) IsCmdEnabled(pToolbarData, (ULONG) mp1);

#ifdef FULLFUNC
      case TBM_QUERYITEMPARAMS: /* mp1: ULONG ulButtonID */
         return (MRESULT) QueryItemParams(hwnd, pToolbarData, (ULONG) mp1);
#endif

      case TBM_SETCHANGED: /* mp1: BOOL bNewFlag */
         pToolbarData->bDirty = (BOOL) mp1;
         return (MRESULT) TRUE;

      case TBM_QUERYCHANGED:
         return (MRESULT) pToolbarData->bDirty;

      case TBM_QUERYREQHEIGHT:
         {
            LONG lY, lX;

            CalcRequiredSize(hwnd, pToolbarData, &lX, &lY, pToolbarData->lWinWidth);
            return (MRESULT) lY;
         }

      case TBM_QUERYFIRSTITEM: /* mp1: PTOOLBARITEM pDestBuff */
         return (MRESULT) QueryFirstItem(pToolbarData, (PTOOLBARITEM) mp1);

      case TBM_QUERYNEXTITEM: /* mp1: PTOOLBARITEM pDestBuff */
         return (MRESULT) QueryNextItem(pToolbarData, (PTOOLBARITEM) mp1);

#ifdef FULLFUNC
      case TBM_QUERYITEMBITMAP: /* mp1: ULONG ulButtonID */
         return (MRESULT) QueryItemBitmap(hwnd, pToolbarData, (ULONG) mp1);
#endif

#ifdef FULLFUNC
      case TBIM_ITEMDROPPED: /* mp1: PDRAGINFO; mp2: ulButtonID */
         TBPerformDrop(hwnd, pToolbarData, (ULONG) mp2, (PDRAGINFO) mp1);
         break;
#endif

      default:
         break;
   }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateToolbar                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Initialisiert die Toolbar                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            pCtlData: Control-Daten aus Message                            */
/*            pCreateStruct: Initialisierungs-Struktur aus Message           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Fehler beim Erzeugen                                 */
/*                FALSE: alles OK                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CreateToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, PTBCTLDATA pCtlData, PCREATESTRUCT pCreateStruct)
{
   LONG lColor;

   /* Presentation-Parameter vorbereiten */

   /* Hintergrund grau */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
   WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &lColor);

   /* Rahmen */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWFRAME, 0);
   WinSetPresParam(hwnd, PP_BORDERCOLOR, sizeof(LONG), &lColor);

   /* ID-Startwert */
   pToolbarData->ulButtonID = STARTID;

   /* Geometrie-Werte */
   if (pCtlData)
   {
      pToolbarData->lButtonSpacing = pCtlData->lButtonSpacing;
      pToolbarData->lExtraSpacing  = pCtlData->lExtraSpacing;
      pToolbarData->lBorderX       = pCtlData->lBorderX;
      pToolbarData->lBorderY       = pCtlData->lBorderY;
   }
   else
   {
      pToolbarData->lButtonSpacing = BUTTONSPACING;
      pToolbarData->lExtraSpacing  = EXTRASPACING;
      pToolbarData->lBorderX       = BORDERSIZE_X;
      pToolbarData->lBorderY       = BORDERSIZE_Y;
   }

   pToolbarData->lWinWidth = pCreateStruct->cx;

   /* Scrollbar erzeugen */
   pToolbarData->lScrollHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL)*2/3;
   pToolbarData->hwndScroll = WinCreateWindow(hwnd,
                                              WC_SCROLLBAR,
                                              NULL,
                                              SBS_HORZ /*|
                                              SBS_THUMBSIZE*/,
                                              0, 0,
                                              pCreateStruct->cx,
                                              pToolbarData->lScrollHeight,
                                              hwnd,
                                              HWND_TOP,
                                              TBID_SCROLL,
                                              NULL, NULL);

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DestroyToolbar                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Raeumt die Toolbar beim Destroy auf                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DestroyToolbar(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   PTBITEMLIST pList=pToolbarData->pItemList;
   PTBITEMLIST pTemp;

   /* @@ */
   hwnd=hwnd;

   while (pList)
   {
      /* Button zerstoeren */
      if (pList->hwndButton)
         WinDestroyWindow(pList->hwndButton);

      /* optionale Parameter loeschen */
      if (pList->TBItem.pItemParams)
         free(pList->TBItem.pItemParams);

      pTemp=pList;
      pList = pList->next;
      free(pTemp);
   }

   if (pToolbarData->hwndScroll)
      WinDestroyWindow(pToolbarData->hwndScroll);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PaintToolbar                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Zeichnet die Toolbar                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void PaintToolbar(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   HPS hps;
   RECTL rectl;
   RECTL rclWindow;
   POINTL pointl;
   LONG lBackColor = 0L;
   LONG lBorderColor =0L;
   ULONG ulStyle = WinQueryWindowULong(hwnd, QWL_STYLE);

   /* @@ */
   pToolbarData = pToolbarData;

   hps=WinBeginPaint(hwnd, NULLHANDLE, &rectl);
   WinQueryWindowRect(hwnd, &rclWindow);

   /* Farben vorbereiten */
   WinQueryPresParam(hwnd,
                     PP_BACKGROUNDCOLOR,
                     PP_BACKGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBackColor,
                     QPF_ID2COLORINDEX);

   WinQueryPresParam(hwnd,
                     PP_BORDERCOLOR,
                     PP_BORDERCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBorderColor,
                     QPF_ID2COLORINDEX);

   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   /* Hintergrund loeschen */
   WinFillRect(hps, &rectl, lBackColor);

   if (ulStyle & TBS_BORDER)
   {
      /* Rahmen zeichnen */
      GpiSetColor(hps, lBorderColor);

      pointl.x=rclWindow.xLeft;
      pointl.y=rclWindow.yBottom;
      GpiMove(hps, &pointl);

      pointl.x=rclWindow.xRight-1;
      pointl.y=rclWindow.yTop-1;
      GpiBox(hps, DRO_OUTLINE, &pointl, 0, 0);
   }

   WinEndPaint(hps);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetWindowParams                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Window-Parameter neu                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            pWndParams: Zweiger auf Window-Parameter-Struktur der Message  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:   Erfolg                                             */
/*                FALSE:  Fehler                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Window-Text wird nicht unterstuetzt.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetWindowParams(HWND hwnd, PTOOLBARDATA pToolbarData, PWNDPARAMS pWndParams)
{
   if (!pWndParams)
      return FALSE;

   if (pWndParams->fsStatus & WPM_CTLDATA)
   {
      pToolbarData->lButtonSpacing = ((PTBCTLDATA)pWndParams->pCtlData)->lButtonSpacing;
      pToolbarData->lExtraSpacing  = ((PTBCTLDATA)pWndParams->pCtlData)->lExtraSpacing;
      pToolbarData->lBorderX       = ((PTBCTLDATA)pWndParams->pCtlData)->lBorderX;
      pToolbarData->lBorderY       = ((PTBCTLDATA)pWndParams->pCtlData)->lBorderY;
      RepositionButtons(hwnd, pToolbarData);
   }

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryWindowParams                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fragt die Window-Parameter ab                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            pWndParams: Zweiger auf Window-Parameter-Struktur der Message  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:   Erfolg                                             */
/*                FALSE:  Fehler                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Window-Text wird nicht unterstuetzt.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL QueryWindowParams(HWND hwnd, PTOOLBARDATA pToolbarData, PWNDPARAMS pWndParams)
{
   hwnd=hwnd; /* @@ */

   if (!pWndParams)
      return FALSE;

   if (pWndParams->fsStatus & WPM_CTLDATA)
   {
      ((PTBCTLDATA)pWndParams->pCtlData)->lButtonSpacing = pToolbarData->lButtonSpacing;
      ((PTBCTLDATA)pWndParams->pCtlData)->lExtraSpacing  = pToolbarData->lExtraSpacing;
      ((PTBCTLDATA)pWndParams->pCtlData)->lBorderX       = pToolbarData->lBorderX;
      ((PTBCTLDATA)pWndParams->pCtlData)->lBorderY       = pToolbarData->lBorderY;
      ((PTBCTLDATA)pWndParams->pCtlData)->cb = sizeof(TBCTLDATA);

      pWndParams->fsStatus &= ~WPM_CTLDATA;
   }
   if (pWndParams->fsStatus & WPM_CBCTLDATA)
   {
      pWndParams->cbCtlData = sizeof(TBCTLDATA);

      pWndParams->fsStatus &= ~WPM_CBCTLDATA;
   }

   if (pWndParams->fsStatus)
      return FALSE;
   else
      return TRUE;
}

static void SizeToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, SHORT sNewX)
{
   /* Scrollbar neu setzen */
   WinSetWindowPos(pToolbarData->hwndScroll,
                   NULLHANDLE,
                   0, 0,
                   sNewX,
                   pToolbarData->lScrollHeight,
                   SWP_SIZE);

   pToolbarData->lWinWidth = sNewX;

   RepositionButtons(hwnd, pToolbarData);

   return;
}

static void ScrollToolbar(HWND hwnd, PTOOLBARDATA pToolbarData, SHORT sSliderPos, SHORT sCmd)
{
   switch(sCmd)
   {
      case SB_LINELEFT:
         pToolbarData->lScrollOffset-=pToolbarData->lAvgWidth;
         if (pToolbarData->lScrollOffset < 0)
            pToolbarData->lScrollOffset = 0;
         break;

      case SB_LINERIGHT:
         pToolbarData->lScrollOffset+=pToolbarData->lAvgWidth;
         if (pToolbarData->lScrollOffset+pToolbarData->lWinWidth > pToolbarData->lTotlWidth)
            pToolbarData->lScrollOffset = pToolbarData->lTotlWidth - pToolbarData->lWinWidth;
         break;

      case SB_PAGELEFT:
         pToolbarData->lScrollOffset-=pToolbarData->lWinWidth;
         if (pToolbarData->lScrollOffset < 0)
            pToolbarData->lScrollOffset = 0;
         break;

      case SB_PAGERIGHT:
         pToolbarData->lScrollOffset+=pToolbarData->lWinWidth;
         if (pToolbarData->lScrollOffset+pToolbarData->lWinWidth > pToolbarData->lTotlWidth)
            pToolbarData->lScrollOffset = pToolbarData->lTotlWidth - pToolbarData->lWinWidth;
         break;

      case SB_SLIDERPOSITION:
         pToolbarData->lScrollOffset = sSliderPos;
         break;

      default:
         break;
   }
   RepositionButtons(hwnd, pToolbarData);
   return;
}

static void SetScrollParams(PTOOLBARDATA pToolbarData)
{
   SHORT sScrollerBottom;

   sScrollerBottom=pToolbarData->lTotlWidth - pToolbarData->lWinWidth;

   WinSendMsg(pToolbarData->hwndScroll, SBM_SETSCROLLBAR,
                     MPFROMSHORT(pToolbarData->lScrollOffset),
                     MPFROM2SHORT(0, sScrollerBottom));

   WinSendMsg(pToolbarData->hwndScroll, SBM_SETTHUMBSIZE,
                     MPFROM2SHORT(pToolbarData->lWinWidth, pToolbarData->lTotlWidth),
                     NULL);

   return;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: AddItem                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt ein neues Item in die Liste ein                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            pAddItem: Zeiger auf das hinzuzufuegende Item                  */
/*            ulFlags: Neue Position des Items                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Neue interne ID                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG AddItem(HWND hwnd, PTOOLBARDATA pToolbarData, PTOOLBARITEM pAddItem, ULONG ulFlags)
{
   PTBITEMLIST pNewItem, pInsertItem;
   char pchTemp[20]="#";
   SWP swp;

   /* neuen Listeneintrag erzeugen */
   if (ulFlags == ADDITEM_LAST)
   {
      pNewItem = calloc(1, sizeof(TBITEMLIST));
      PMASSERT(pNewItem != NULL, "Out of memory");
      pNewItem->prev = pToolbarData->pItemListLast;
      if (pToolbarData->pItemListLast)
         pToolbarData->pItemListLast->next = pNewItem;
      pToolbarData->pItemListLast = pNewItem;
      if (!pToolbarData->pItemList)
         pToolbarData->pItemList = pNewItem;
   }
   else
      if (ulFlags == ADDITEM_FIRST)
      {
         pNewItem = calloc(1, sizeof(TBITEMLIST));
         PMASSERT(pNewItem != NULL, "Out of memory");
         pNewItem->next = pToolbarData->pItemList;
         if (pToolbarData->pItemList)
            pToolbarData->pItemList->prev = pNewItem;
         pToolbarData->pItemList = pNewItem;
         if (!pToolbarData->pItemListLast)
            pToolbarData->pItemListLast = pNewItem;
      }
      else
      {
         /* Item suchen */
         pInsertItem= FindItem(hwnd, pToolbarData, ulFlags);
         if (pInsertItem)
         {
            pNewItem = calloc(1, sizeof(TBITEMLIST));
            PMASSERT(pNewItem != NULL, "Out of memory");
            pNewItem->next = pInsertItem;
            pNewItem->prev = pInsertItem->prev;
            if (pInsertItem->prev)
               pInsertItem->prev->next = pNewItem;
            pInsertItem->prev = pNewItem;
            if (pToolbarData->pItemList == pInsertItem)
               pToolbarData->pItemList = pNewItem;
         }
         else
            return 0; /* nicht gefunden */
      }
   /* Daten uebernehmen */
   memcpy(&pNewItem->TBItem, pAddItem, sizeof(TOOLBARITEM));

   /* Item-Parameter kopieren */
   if (pAddItem->ulParamSize && pAddItem->pItemParams)
   {
      pNewItem->TBItem.pItemParams= malloc(pAddItem->ulParamSize);
      PMASSERT(pNewItem->TBItem.pItemParams != NULL, "Out of memory");
      memcpy(pNewItem->TBItem.pItemParams, pAddItem->pItemParams, pAddItem->ulParamSize);
   }
   else
   {
      pNewItem->TBItem.ulParamSize=0;
      pNewItem->TBItem.pItemParams=NULL;
   }

   /* ID vergeben und hochzaehlen */
   pNewItem->ulInternalID=pToolbarData->ulButtonID;
   pToolbarData->ulButtonID++;

   /* Window erzeugen */
   _itoa(pAddItem->ulBitmapID, pchTemp+1, 10);
   pNewItem->hwndButton = WinCreateWindow(hwnd,
                                          WC_BUTTON,
                                          pchTemp,
                                          ((pAddItem->ulFlags & TBITEM_DISABLED)?WS_DISABLED:0) |
                                          BS_PUSHBUTTON |
                                          BS_BITMAP |
                                          BS_NOPOINTERFOCUS |
                                          BS_AUTOSIZE,
                                          0, 0,
                                          0, 0,
                                          hwnd,
                                          HWND_TOP,
                                          pNewItem->ulInternalID,
                                          NULL, NULL);
   WinQueryWindowPos(pNewItem->hwndButton, &swp);
   pNewItem->lCreateCX = swp.cx;
   pNewItem->lCreateCY = swp.cy;

   pToolbarData->ulItemCount++;
   RepositionButtons(hwnd, pToolbarData);
   OldButtonProc=WinSubclassWindow(pNewItem->hwndButton, NewButtonProc);

   pToolbarData->pCurrentQuery = NULL; /* Query zuruecksetzen */
   pToolbarData->bDirty = TRUE;

   return pNewItem->ulInternalID;
}

#ifdef FULLFUNC
/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteItem                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht das angegebene Item                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:   Item geloescht                                     */
/*                FALSE:  Item nicht gefunden                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL DeleteItem(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID)
{
   PTBITEMLIST pItem;

   pItem = FindItem(hwnd, pToolbarData, ulButtonID);
   if (pItem)
   {
      /* Button zerstoeren */
      if (pItem->hwndButton)
         WinDestroyWindow(pItem->hwndButton);

      /* optionale Daten freigeben */
      if (pItem->TBItem.pItemParams)
         free(pItem->TBItem.pItemParams);

      /* Knoten aushaengen */
      if (pItem->prev)
         pItem->prev->next = pItem->next;
      if (pItem->next)
         pItem->next->prev = pItem->prev;
      if (pToolbarData->pItemList == pItem)
         pToolbarData->pItemList = pItem->next;
      if (pToolbarData->pItemListLast == pItem)
         pToolbarData->pItemListLast = pItem->prev;

      /* Knoten freigeben */
      free(pItem);

      pToolbarData->ulItemCount--;
      pToolbarData->pCurrentQuery=NULL;
      pToolbarData->bDirty = TRUE;
      RepositionButtons(hwnd, pToolbarData);
      return TRUE;
   }
   else
      return FALSE;
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteAllItems                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht alle Items                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:   Item geloescht                                     */
/*                FALSE:  Item nicht gefunden                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL DeleteAllItems(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   PTBITEMLIST pItem, pTemp;

   pItem = pToolbarData->pItemList;
   while (pItem)
   {
      /* Button zerstoeren */
      if (pItem->hwndButton)
         WinDestroyWindow(pItem->hwndButton);

      /* optionale Daten freigeben */
      if (pItem->TBItem.pItemParams)
         free(pItem->TBItem.pItemParams);

      /* Knoten freigeben */
      pTemp = pItem->next;
      free(pItem);
      pItem = pTemp;
   }

   pToolbarData->pItemList=NULL;
   pToolbarData->pItemListLast=NULL;
   pToolbarData->ulItemCount = 0;
   pToolbarData->pCurrentQuery=NULL;
   pToolbarData->bDirty = TRUE;
   RepositionButtons(hwnd, pToolbarData);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RepositionButtons                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Positioniert die Buttons neu                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void RepositionButtons(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   if (WinQueryWindowULong(hwnd, QWL_STYLE) & TBS_VERTICAL)
      RepositionButtonsVert(hwnd, pToolbarData);
   else
      RepositionButtonsHorz(hwnd, pToolbarData);
}

static void RepositionButtonsHorz(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   int i=0;
   LONG lX=pToolbarData->lBorderX;
   LONG lMaxHeight=0;
   PTBITEMLIST pList=pToolbarData->pItemList;
   PSWP pSwpArray;
   BOOL bScrollVisible=FALSE;

   pToolbarData->lTotlWidth=0;

   if (pToolbarData->ulItemCount)
   {
      pSwpArray=malloc(pToolbarData->ulItemCount * sizeof(SWP));
      PMASSERT(pSwpArray != NULL, "Out of memory");
      for (i=0; i<pToolbarData->ulItemCount; i++)
      {
         if (pList->lCreateCY > lMaxHeight)
            lMaxHeight = pList->lCreateCY;
         pToolbarData->lTotlWidth += pList->lCreateCX;
         if (i>0 && (pList->TBItem.ulFlags & TBITEM_SPACER))
            pToolbarData->lTotlWidth += pToolbarData->lExtraSpacing;
         pList=pList->next;
      }
      pToolbarData->lTotlWidth += 2*pToolbarData->lBorderX;
      pToolbarData->lAvgWidth = pToolbarData->lTotlWidth / pToolbarData->ulItemCount / 2;

      if (pToolbarData->lTotlWidth > pToolbarData->lWinWidth)
      {
         /* zu breit, Scrollbar zeigen */
         WinShowWindow(pToolbarData->hwndScroll, TRUE);
         bScrollVisible = TRUE;
      }
      else
      {
         /* breit genug, Scrollbar verstecken */
         WinShowWindow(pToolbarData->hwndScroll, FALSE);
         pToolbarData->lScrollOffset=0;
      }

      pList=pToolbarData->pItemList;
      lX -= pToolbarData->lScrollOffset;
      for (i=0; i<pToolbarData->ulItemCount; i++)
      {
         pSwpArray[i].cy = lMaxHeight;
         pSwpArray[i].cx = pList->lCreateCX;
         if (i>0 && (pList->TBItem.ulFlags & TBITEM_SPACER))
            lX += pToolbarData->lExtraSpacing;
         pSwpArray[i].x = lX;
         pSwpArray[i].y = pToolbarData->lBorderY +
                          (bScrollVisible?pToolbarData->lScrollHeight:0);
         lX += pList->lCreateCX;
         pSwpArray[i].fl = SWP_SIZE | SWP_MOVE | SWP_SHOW;
         pSwpArray[i].hwnd = pList->hwndButton;
         pList=pList->next;
      }
      WinSetMultWindowPos(WinQueryAnchorBlock(hwnd),
                          pSwpArray,
                          pToolbarData->ulItemCount);
      free(pSwpArray);
      pToolbarData->lMaxHeight = lMaxHeight;
      SetScrollParams(pToolbarData);
   }
   else
   {
      pToolbarData->lMaxHeight = 0;
      WinShowWindow(pToolbarData->hwndScroll, FALSE);
      pToolbarData->lScrollOffset=0;
   }
   return;
}

static void RepositionButtonsVert(HWND hwnd, PTOOLBARDATA pToolbarData)
{
   int i=0;
   LONG lX=pToolbarData->lBorderX, lY=pToolbarData->lBorderY;
   LONG lMaxWidth=0;
   PTBITEMLIST pList=pToolbarData->pItemList;
   PSWP pSwpArray;
   SWP WinPos;

   pToolbarData->lTotlWidth=0;

   WinQueryWindowPos(hwnd, &WinPos);

   if (pToolbarData->ulItemCount)
   {
      pSwpArray=malloc(pToolbarData->ulItemCount * sizeof(SWP));
      PMASSERT(pSwpArray != NULL, "Out of memory");
      for (i=0; i<pToolbarData->ulItemCount; i++)
      {
         if (pList->lCreateCX > lMaxWidth)
            lMaxWidth = pList->lCreateCX;
         pList=pList->next;
      }

      /* breit genug, Scrollbar verstecken */
      WinShowWindow(pToolbarData->hwndScroll, FALSE);
      pToolbarData->lScrollOffset=0;

      pList=pToolbarData->pItemList;
      lY = WinPos.cy - pToolbarData->lBorderY;
      for (i=0; i<pToolbarData->ulItemCount; i++)
      {
         if ((i%2) == 0)
            pSwpArray[i].cy = pList->lCreateCY;
         else
            if (pList->lCreateCY > pSwpArray[i-1].cy)
            {
               pSwpArray[i].cy = pList->lCreateCY;
               pSwpArray[i-1].cy = pList->lCreateCY;
               pSwpArray[i-1].y = lY-pSwpArray[i].cy;
            }
            else
            {
               pSwpArray[i].cy = pSwpArray[i-1].cy;
            }
         pSwpArray[i].y = lY-pSwpArray[i].cy;
         pSwpArray[i].cx = lMaxWidth;
         pSwpArray[i].x = lX;
         pSwpArray[i].fl = SWP_SIZE | SWP_MOVE | SWP_SHOW;
         pSwpArray[i].hwnd = pList->hwndButton;

         if (i%2)
         {
            lX = pToolbarData->lBorderX;
            lY -= pSwpArray[i].cy;
         }
         else
            lX += lMaxWidth;

         pList=pList->next;
      }
      WinSetMultWindowPos(WinQueryAnchorBlock(hwnd),
                          pSwpArray,
                          pToolbarData->ulItemCount);
      free(pSwpArray);
      SetScrollParams(pToolbarData);
   }
   else
   {
      pToolbarData->lMaxHeight = 0;
      WinShowWindow(pToolbarData->hwndScroll, FALSE);
      pToolbarData->lScrollOffset=0;
   }
   return;
}

static void CalcRequiredSize(HWND hwnd, PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY, LONG lNewX)
{
   if (WinQueryWindowULong(hwnd, QWL_STYLE) & TBS_VERTICAL)
      CalcRequiredSizeVert(pToolbarData, plX, plY);
   else
      CalcRequiredSizeHorz(pToolbarData, plX, plY, lNewX);
}

static void CalcRequiredSizeVert(PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY)
{
   int i=0;
   LONG lMaxHeight=0, lTotlHeight=0;
   LONG lMaxWidth=0;
   PTBITEMLIST pList=pToolbarData->pItemList;

   *plX=0;
   *plY=0;

   for (i=0; i<pToolbarData->ulItemCount; i++)
   {
      if ((i%2) == 0)
         lMaxHeight= pList->lCreateCY;
      else
      {
         if (pList->lCreateCY > pList->prev->lCreateCY)
            lMaxHeight = pList->lCreateCY;
         lTotlHeight += lMaxHeight;
      }
      if (pList->lCreateCX > lMaxWidth)
         lMaxWidth = pList->lCreateCX;
      pList=pList->next;
   }

   *plX = 2*lMaxWidth + 2*pToolbarData->lBorderX;
   *plY = lTotlHeight + 2*pToolbarData->lBorderY;

   return;
}

static void CalcRequiredSizeHorz(PTOOLBARDATA pToolbarData, PLONG plX, PLONG plY, LONG lNewX)
{
   int i=0;
   LONG lMaxHeight=0;
   PTBITEMLIST pList=pToolbarData->pItemList;

   *plX=0;
   *plY=0;

   for (i=0; i<pToolbarData->ulItemCount; i++)
   {
      if (pList->lCreateCY > lMaxHeight)
         lMaxHeight = pList->lCreateCY;
      *plX += pList->lCreateCX;
      if (i>0 && (pList->TBItem.ulFlags & TBITEM_SPACER))
         *plX += pToolbarData->lExtraSpacing;
      pList=pList->next;
   }

   *plX += 2*pToolbarData->lBorderX;
   *plY = lMaxHeight + 2*pToolbarData->lBorderY;

   if (*plX > lNewX)
      *plY+=pToolbarData->lScrollHeight;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: EnableCmdItems                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Enabled oder Disabled alle Buttons einer Kategorie          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Toolbar-Window-Handle                                    */
/*            pToolbarData: Window-Daten                                     */
/*            ulCommandID: ID der Button-Kategorie                           */
/*            bNewState: Neuer Enable-Status                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:  mind. ein Button gefunden                           */
/*                FALSE: kein Button gefunden                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL EnableCmdItems(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulCommandID, BOOL bNewState)
{
   BOOL bFound=FALSE;
   PTBITEMLIST pList = pToolbarData->pItemList;

   hwnd = hwnd;

   while(pList)
   {
      if (pList->TBItem.ulCommandID == ulCommandID)
      {
         WinEnableWindow(pList->hwndButton, bNewState);
         bFound = TRUE;
      }
      pList = pList->next;
   }

   return bFound;
}

static BOOL IsCmdEnabled(PTOOLBARDATA pToolbarData, ULONG ulCommandID)
{
   PTBITEMLIST pList = pToolbarData->pItemList;

   while (pList && pList->TBItem.ulCommandID != ulCommandID)
      pList = pList->next;

   if (pList)
   {
      if (WinIsWindowEnabled(pList->hwndButton))
         return TRUE;
      else
         return FALSE;
   }
   else
      return FALSE;
}
/*---------------------------------------------------------------------------*/
/* Funktionsname: FindItem                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht ein Item anhand der internen ID                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Toolbar-Window-Handle                                    */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf Listen-Item                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PTBITEMLIST FindItem(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID)
{
   PTBITEMLIST pList = pToolbarData->pItemList;

   hwnd=hwnd;

   while (pList && pList->ulInternalID != ulButtonID)
      pList = pList->next;

   return pList;
}

#ifdef FULLFUNC
/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryItemParams                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt Zeiger auf optionale Parameter zurueck                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Toolbar-Window-Handle                                    */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: NULL: Button nicht gefunden, oder keine Parameter          */
/*                sonst: Zeiger auf Parameter                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PVOID QueryItemParams(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID)
{
   PTBITEMLIST pList = FindItem(hwnd, pToolbarData, ulButtonID);

   if (pList)
      return pList->TBItem.pItemParams;
   else
      return NULL;
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryFirstItem                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt die Item-Daten des ersten Items zurueck                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pToolbarData: Window-Daten                                     */
/*            pDestBuffer: Zielpuffer                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TBQUERY_ERROR: Kein Zielpuffer                             */
/*                TBQUERY_NOMORE: keine Items                                */
/*                TBQUERY_OK: Item erfolgreich geliefert                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG QueryFirstItem(PTOOLBARDATA pToolbarData, PTOOLBARITEM pDestBuff)
{
   if (!pDestBuff)
      return TBQUERY_ERROR;

   if (pToolbarData->ulItemCount == 0)
      return TBQUERY_NOMORE;
   else
   {
      pToolbarData->pCurrentQuery = pToolbarData->pItemList;
      memcpy(pDestBuff, &pToolbarData->pItemList->TBItem, sizeof(TOOLBARITEM));
      return TBQUERY_OK;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryNextItem                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt die Item-Daten des naechsten Items in der Abfrage      */
/*               zurueck.                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pToolbarData: Window-Daten                                     */
/*            pDestBuffer: Zielpuffer                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TBQUERY_ERROR: QueryFirst wurde nicht aufgerufen;          */
/*                               Kein Puffer angegeben;                      */
/*                               Keine Items;                                */
/*                TBQUERY_OK:    Item erfolgreich geliefert                  */
/*                TBQUERY_NOMORE: keine weiteren Items                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG QueryNextItem(PTOOLBARDATA pToolbarData, PTOOLBARITEM pDestBuff)
{
   if (!pDestBuff)
      return TBQUERY_ERROR;

   if (pToolbarData->ulItemCount == 0)
      return TBQUERY_ERROR;

   if (!pToolbarData->pCurrentQuery)
      return TBQUERY_ERROR;

   if (pToolbarData->pCurrentQuery->next)
   {
      pToolbarData->pCurrentQuery = pToolbarData->pCurrentQuery->next;
      memcpy(pDestBuff, &pToolbarData->pCurrentQuery->TBItem, sizeof(TOOLBARITEM));
      return TBQUERY_OK;
   }
   else
      return TBQUERY_NOMORE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryItemData                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Kopiert die Item-Daten eines Items                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*            pDestBuff: Zielpuffer                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:  Erfolg                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Optionale Item-Parameter werden nicht kopiert, nur die         */
/*            Laenge und der Pointer                                         */
/*---------------------------------------------------------------------------*/

static BOOL QueryItemData(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PTOOLBARITEM pDestBuff)
{
   PTBITEMLIST pList = FindItem(hwnd, pToolbarData, ulButtonID);

   if (pList && pDestBuff)
   {
      memcpy(pDestBuff, &pList->TBItem, sizeof(TOOLBARITEM));
      return TRUE;
   }
   else
      return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetItemData                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Daten eines Items neu                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*            pDestBuff: Zielpuffer                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE:  Erfolg                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Bislang werden nur die Flags neu gesetzt                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetItemData(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PTOOLBARITEM pNewData)
{
   PTBITEMLIST pList;

   if (ulButtonID)
      pList = FindItem(hwnd, pToolbarData, ulButtonID);
   else
      pList = pToolbarData->pCurrentQuery;

   if (pList && pNewData)
   {
      BTNCDATA ButtonData;
      HPS hps;
      BITMAPINFOHEADER Header;
      WNDPARAMS WndParams={WPM_CTLDATA, 0, NULL, 0, NULL, sizeof(ButtonData), &ButtonData};

      pList->TBItem.ulFlags = pNewData->ulFlags;
      pList->TBItem.ulBitmapID = pNewData->ulBitmapID;

      WinSendMsg(pList->hwndButton, WM_QUERYWINDOWPARAMS, &WndParams, 0);

      hps=WinGetPS(pList->hwndButton);
      if (ButtonData.hImage)
         GpiDeleteBitmap(ButtonData.hImage);
      ButtonData.hImage=GpiLoadBitmap(hps, NULLHANDLE, pList->TBItem.ulBitmapID, 0, 0);
      GpiQueryBitmapParameters(ButtonData.hImage, &Header);
      WinReleasePS(hps);

      WinSendMsg(pList->hwndButton, WM_SETWINDOWPARAMS, &WndParams, 0);
      pList->lCreateCX = Header.cx+10;
      pList->lCreateCY = Header.cy+10;

      WinSetWindowPos(pList->hwndButton,
                      NULLHANDLE,
                      0, 0,
                      pList->lCreateCX,
                      pList->lCreateCY,
                      SWP_SIZE);

      RepositionButtons(hwnd, pToolbarData);

      /* pToolbarData->pCurrentQuery = NULL; Query zuruecksetzen */
      pToolbarData->bDirty = TRUE;

      return TRUE;
   }
   else
      return FALSE;
}

#ifdef FULLFUNC
/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryItemBitmap                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert das Bitmap-Handle eines Items zurueck               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne Button-ID                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: NULLHANDLE: Item nicht gefunden                            */
/*                sonst:      Bitmap-Handle                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static HBITMAP QueryItemBitmap(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID)
{
   PTBITEMLIST pList = FindItem(hwnd, pToolbarData, ulButtonID);

   if (pList)
   {
      BTNCDATA ButtonData;
      WNDPARAMS WndParams={WPM_CTLDATA, 0, NULL, 0, NULL, sizeof(ButtonData), &ButtonData};

      WinSendMsg(pList->hwndButton, WM_QUERYWINDOWPARAMS, &WndParams, 0);

      return ButtonData.hImage;
   }
   else
      return NULLHANDLE;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: BeginButtonDrag                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet den Drag eines Items                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne ID des Buttons, der gedragged werden soll  */
/*            pPoints: Start-Position                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Drag ausgefuehrt                                     */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL BeginButtonDrag(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID)
{
   PTBITEMLIST pItem = FindItem(hwnd, pToolbarData, ulButtonID);
   DRAGITEM DragItem;
   PDRAGINFO pDraginfo;
   DRAGIMAGE DragImage;
   char pchTemp[40];

   if (!pItem)
      return FALSE;

   if (!(pItem->TBItem.ulFlags & TBITEM_DRAGABLE))
      return FALSE;

   if (!(pDraginfo = DrgAllocDraginfo(1)))
      return FALSE;

   pDraginfo->usOperation = DO_DEFAULT;
   pDraginfo->hwndSource = hwnd;

   DragItem.hwndItem = hwnd;
   DragItem.ulItemID = ulButtonID;
   DragItem.hstrType = DrgAddStrHandle(TBDRAGTYPE);
   if (pItem->TBItem.ulFlags & TBITEM_DELETEABLE)
      DragItem.hstrRMF  = DrgAddStrHandle(TBDRAGRMFDEL);
   else
      DragItem.hstrRMF  = DrgAddStrHandle(TBDRAGRMF);
   DragItem.hstrContainerName=DrgAddStrHandle("a");
   sprintf(pchTemp, "%d,%d", pItem->TBItem.ulCommandID, pItem->TBItem.ulBitmapID);
   DragItem.hstrSourceName=DrgAddStrHandle(pchTemp);
   DragItem.hstrTargetName=DrgAddStrHandle(pchTemp);
   DragItem.cxOffset=0;
   DragItem.cyOffset=0;
   DragItem.fsControl=0;
   DragItem.fsSupportedOps=DO_COPYABLE;

   DrgSetDragitem(pDraginfo, &DragItem, sizeof(DragItem), 0);

   DragImage.cb=sizeof(DragImage);
   DragImage.hImage = QueryItemBitmap(hwnd, pToolbarData, ulButtonID);
   DragImage.fl = DRG_BITMAP;
   DragImage.cxOffset = 1;
   DragImage.cyOffset = 1;

   pToolbarData->pDraginfo = pDraginfo;
#if 1
   DrgDrag(hwnd, pDraginfo, &DragImage, 1, VK_ENDDRAG, NULL);
#else
   if (DrgDrag(hwnd, pDraginfo, &DragImage, 1, VK_ENDDRAG, NULL)==NULLHANDLE)
   {
      DrgAccessDraginfo(pDraginfo);
      DrgDeleteDraginfoStrHandles(pDraginfo);
      DrgFreeDraginfo(pDraginfo);
   }
#endif

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DiscardObject                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht ein Item bei Drop auf Shredder                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            pDraginfo: Drag-Info aus der Message                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DiscardObject(HWND hwnd, PTOOLBARDATA pToolbarData, PDRAGINFO pDraginfo)
{
   PDRAGITEM pDItem;

   DrgAccessDraginfo(pDraginfo);
   pDItem = DrgQueryDragitemPtr(pDraginfo, 0);
   DeleteItem(hwnd, pToolbarData, pDItem->ulItemID);
   DrgFreeDraginfo(pDraginfo);

   Notify(hwnd, TBN_ITEMDELETED, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DragOver                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt, ob ein Drop moeglich ist                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne ID des Buttons, auf den gedroppt werden    */
/*                        soll (0= freie Flaeche)                            */
/*            pDraginfo: Drag-Info aus der Message                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: wie DM_DROP                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT DragOver(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PDRAGINFO pDraginfo)
{
   PDRAGITEM pDItem;
   USHORT usDrop=DOR_NODROP;
   USHORT usDefOp = DO_MOVE;

   /* @@ */
   pToolbarData=pToolbarData;

   DrgAccessDraginfo(pDraginfo);

   pDItem = DrgQueryDragitemPtr(pDraginfo, 0);

   if (DrgVerifyTrueType(pDItem, TBDRAGTYPE))
   {
      if (DrgVerifyNativeRMF(pDItem, TBDRAGRMF))
      {
         if (hwnd == pDItem->hwndItem)
         {
            /* bei gleichem Window, kein Drag auf freie Flaeche, und kein Drag auf gleiches Item */
            if (ulButtonID && pDItem->ulItemID != ulButtonID)
            {
               if (pDraginfo->usOperation == DO_DEFAULT ||
                   pDraginfo->usOperation == DO_UNKNOWN ||
                   pDraginfo->usOperation == DO_COPY    ||
                   pDraginfo->usOperation == DO_MOVE    )
               {
                  usDrop=DOR_DROP;

                  if (pDraginfo->usOperation == DO_DEFAULT ||
                      pDraginfo->usOperation == DO_UNKNOWN)
                     usDefOp=DO_MOVE;
               }
               else
                  usDefOp=DOR_NODROPOP;
            }
            else
            {
               if (pDraginfo->usOperation == DO_COPY)
               {
                  usDrop=DOR_DROP;
                  usDefOp=DO_COPY;
               }
               else
                  usDefOp=DOR_NODROPOP;
            }
         }
         else
         {
            /* anderes Fenster, Drop erlaubt */
            usDrop = DOR_DROP;
            if (pDraginfo->usOperation == DO_DEFAULT ||
                pDraginfo->usOperation == DO_UNKNOWN ||
                pDraginfo->usOperation == DO_COPY)
            {
               usDrop = DOR_DROP;
               usDefOp = DO_COPY;
            }
            else
               usDrop = DOR_NODROPOP;
         }
      }
   }
   DrgFreeDraginfo(pDraginfo);

   return MRFROM2SHORT(usDrop, usDefOp);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TBDrop                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet DM_DROP                                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Toolbar                                */
/*            pToolbarData: Window-Daten                                     */
/*            ulButtonID: interne ID des Buttons, ueber dem der Drop pass.   */
/*            pDraginfo: Drag-Info aus der Message                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TBDrop(HWND hwnd, ULONG ulButtonID, PDRAGINFO pDraginfo)
{

   DrgAccessDraginfo(pDraginfo);

   WinPostMsg(hwnd, TBIM_ITEMDROPPED, pDraginfo, MPFROMLONG(ulButtonID));

   return;
}

static void TBPerformDrop(HWND hwnd, PTOOLBARDATA pToolbarData, ULONG ulButtonID, PDRAGINFO pDraginfo)
{
   PDRAGITEM pDItem;
   PTBITEMLIST pDraggedItem;
   PTBITEMLIST pDropItem;

   pDItem = DrgQueryDragitemPtr(pDraginfo, 0);
   if (hwnd == pDItem->hwndItem)
   {
      /* eigenes Fenster */
      if (pDraginfo->usOperation == DO_COPY)
      {
         pDraggedItem = FindItem(hwnd, pToolbarData, pDItem->ulItemID);

         if (pDraggedItem)
         {
            /* Vor dem Drop-Item einfuegen */
            AddItem(hwnd, pToolbarData, &pDraggedItem->TBItem, ulButtonID);

            /* Notify */
            Notify(hwnd, TBN_ITEMADDED, NULL);
         }
      }
      else
         if (pDraginfo->usOperation == DO_MOVE)
         {
            pDraggedItem = FindItem(hwnd, pToolbarData, pDItem->ulItemID);
            if (pDraggedItem)
            {
               pDropItem = FindItem(hwnd, pToolbarData, ulButtonID);
               if (pDropItem)
               {
                  /* Move = Umhaengen */
                  /* Aushaengen */
                  if (pDraggedItem->prev)
                     pDraggedItem->prev->next = pDraggedItem->next;
                  if (pDraggedItem->next)
                     pDraggedItem->next->prev = pDraggedItem->prev;
                  if (pToolbarData->pItemList == pDraggedItem)
                     pToolbarData->pItemList = pDraggedItem->next;
                  if (pToolbarData->pItemListLast == pDraggedItem)
                     pToolbarData->pItemListLast = pDraggedItem->prev;

                  /* vor Drop-Item einhaengen */
                  pDraggedItem->prev = pDropItem->prev;
                  pDraggedItem->next = pDropItem;
                  if (pDropItem->prev)
                     pDropItem->prev->next = pDraggedItem;
                  pDropItem->prev = pDraggedItem;
                  if (pToolbarData->pItemList == pDropItem)
                     pToolbarData->pItemList = pDraggedItem;

                  /* Neu Zeichnen */
                  RepositionButtons(hwnd, pToolbarData);
                  pToolbarData->bDirty=TRUE;
                  pToolbarData->pCurrentQuery = NULL; /* Query zuruecksetzen */

                  /* Notify */
                  Notify(hwnd, TBN_ITEMMOVED, NULL);
               }
            }
         }
   }
   else
   {
      /* anderes Fenster */
      if (pDraginfo->usOperation == DO_COPY ||
          pDraginfo->usOperation == DO_MOVE )
      {
         /* Item-Daten auslesen */
         TOOLBARITEM NewItem;
         char *pchNext=NULL;
         char pchTemp[40];

         NewItem.ulFlags = TBITEM_DELETEABLE | TBITEM_DRAGABLE;
         DrgQueryStrName(pDItem->hstrSourceName, sizeof(pchTemp), pchTemp);
         NewItem.ulCommandID = strtoul(pchTemp, &pchNext, 10);
         if (pchNext)
         {
            pchNext++;
            NewItem.ulBitmapID = strtoul(pchNext, NULL, 10);
         }
         else
            NewItem.ulBitmapID =0;
         NewItem.ulParamSize=0;
         NewItem.pItemParams=NULL;

         /* hinzufuegen */
         AddItem(hwnd, pToolbarData, &NewItem, ulButtonID); /* hinter Item oder hinten */
         /* Notify */
         Notify(hwnd, TBN_ITEMADDED, NULL);
      }
   }

   if (hwnd != pDItem->hwndItem)
   {
      /* Item bestaetigen */
      DrgSendTransferMsg(pDItem->hwndItem, DM_ENDCONVERSATION,
                         MPFROMLONG(pDItem->ulItemID),
                         MPFROMLONG(DMFL_TARGETSUCCESSFUL));
      DrgDeleteDraginfoStrHandles(pDraginfo);
      DrgFreeDraginfo(pDraginfo);
   }
   else
   /* Item bestaetigen */
   DrgSendTransferMsg(pDItem->hwndItem, DM_ENDCONVERSATION,
                      MPFROMLONG(pDItem->ulItemID),
                      MPFROMLONG(DMFL_TARGETSUCCESSFUL));

   return;
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: NewButtonProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Neue Window-Prozedur fuer die Buttons in der Toolbar        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_CONTEXTMENU:
         return Notify(hwnd, BN_CONTEXTMENU, NULL);

      case WM_BEGINDRAG:
         return Notify(hwnd, BN_BEGINDRAG, mp1);

      case DM_DRAGOVER:
         return Notify(hwnd, BN_DRAGOVER, mp1);

      case DM_DRAGLEAVE:
         return Notify(hwnd, BN_DRAGLEAVE, mp1);

      case DM_DROP:
         Notify(hwnd, BN_DROP, mp1);
         return (MRESULT) FALSE;

      case DM_DROPHELP:
         return Notify(hwnd, BN_DROPHELP, mp1);

      default:
         break;
   }
   return OldButtonProc(hwnd, msg, mp1, mp2);
}

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd,QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}

/*-------------------------------- Modulende --------------------------------*/

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


