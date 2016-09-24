/*---------------------------------------------------------------------------+
 | Titel: MLIST.C                                                            |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 15.06.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Message-List-Control f. FleetStreet                                     |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "..\main.h"       /* Stringlaengendefinitionen   */
#include "..\msgheader.h"
#include "..\structs.h"
#include "..\areaman\areaman.h"
#include "..\handlemsg\handlemsg.h"
#include "..\dump\pmassert.h"
#include "mlist.h"         /* externes Interface          */
#include "mlistprv.h"      /* Instanzdaten-Struktur       */

/*--------------------------------- Defines ---------------------------------*/

#define ALLOC_BLOCKSIZE      200   /* Anzahl der Items pro Allokationsblock */
#define EXTRA_BYTES            4   /* Anzahl der Bytes in den Window-Words  */

#ifndef ABS
#define ABS(x) (((x)>=0)?(x):-(x))
#endif

/*---------------------------------- Typen ----------------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY MsgListBoxProc(HWND hwnd, ULONG message, MPARAM mp1,
                                       MPARAM mp2);
static BOOL  DrawListItem(HWND hwnd, HPS hps, ULONG ulItemNum, PRECTL prclItem,
                          PMLISTRECORD pItem, PLBCTLDATA pCtlData);
static void  DrawList(HWND hwnd, PLBCTLDATA pCtlData);
static BOOL  CreateList(HWND hwnd, PCREATESTRUCT pCreate);
static BOOL  DestroyList(HWND hwnd, PLBCTLDATA pCtlData);
static LONG  AddItem(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord);
static ULONG AddArray(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord,
                      ULONG ulCountItems);
static BOOL  UpdateItem(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord,
                        LONG lItem);
static BOOL  DeleteItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lDelNr);
static BOOL  ClearList(HWND hwnd, PLBCTLDATA pCtlData);
static BOOL  SetColors(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLORS pColors);
static BOOL  QueryColors(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLORS pColors);
static BOOL  ScrollToItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void  CalcScrollBar(HWND hwnd, PLBCTLDATA pCtlData);
static void  CalcColumns(HWND hwnd, PLBCTLDATA pCtlData);
static void  SizeList(HWND hwnd, SHORT x, SHORT y, PLBCTLDATA pCtlData);
static LONG  FindUMsgID(HWND hwnd, PLBCTLDATA pCtlData, ULONG ulMsgID);
static void  ScrollList(HWND hwnd, PLBCTLDATA pCtlData, SHORT sSliderPos,
                        USHORT usCommand);
static LONG  GetFontHeight(HWND hwnd);
static LONG  ItemFromY(HWND hwnd, PLBCTLDATA pCtlData, LONG y);
static BOOL  SetColumns(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLUMNS pColumns);
static BOOL  QueryColumns(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLUMNS pColumns);
static BOOL  CharList(HWND hwnd, PLBCTLDATA pCtlData, USHORT usFlags,
                      USHORT usVK, USHORT usChar);
static BOOL  QueryItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem,
                       PMLISTRECORD pRecord);
static BOOL  CalcItemRect(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem,
                          PRECTL pDestRect);
static void  CreateCursor(HWND hwnd, PLBCTLDATA pCtlData);
static void  SetCursor(HWND hwnd, PLBCTLDATA pCtlData);
static void  ClickItem(HWND hwnd, PLBCTLDATA pCtlData, LONG x, LONG y,
                       SHORT flags);
static void  ShiftIntoView(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void  DeselectAll(HWND hwnd, PLBCTLDATA pCtlData);
static void  SelectAll(HWND hwnd, PLBCTLDATA pCtlData);
static void  MouseMove(HWND hwnd, PLBCTLDATA pCtlData, SHORT y, SHORT flags);
static LONG  QueryFirstSelected(HWND hwnd, PLBCTLDATA pCtlData);
static LONG  QueryNextSelected(HWND hwnd, PLBCTLDATA pCtlData, LONG lPrevItem);
static void  ListTimer(HWND hwnd, PLBCTLDATA pCtlData);
static void  DragUp(HWND hwnd, PLBCTLDATA pCtlData);
static void  DragDown(HWND hwnd, PLBCTLDATA pCtlData);
static void  OverSeparator(HWND hwnd, PLBCTLDATA pCtlData, SHORT x, SHORT y);
static void  TrackSeparator(HWND hwnd, PLBCTLDATA pCtlData);
static void  RecalcColumns(HWND hwnd, PLBCTLDATA pCtlData);
static BOOL  Emphasize(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static BOOL  SetPointer(HWND hwnd, PLBCTLDATA pCtlData);
static void ShiftSelect(HWND hwnd, PLBCTLDATA pCtlData, LONG lOldCrs, ULONG ulAction);
static void SelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void DeSelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);
static LONG QueryVisibleLines(PLBCTLDATA pCtlData);

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterMessageList                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Registriert die Fensterklasse                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anchor-Block                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Success                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL EXPENTRY RegisterMessageList(HAB hab)
{
   if (WinRegisterClass(hab,
                        WC_MSGLISTBOX,
                        MsgListBoxProc,
                        CS_CLIPCHILDREN | CS_SYNCPAINT,
                        EXTRA_BYTES))
      return TRUE;
   else
      return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MsgListBoxProc                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fensterprozedur der Fensterklasse                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            message: Message-ID                                            */
/*            mp1:  Parameter 1                                              */
/*            mp2:  Parameter 2                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MsgListBoxProc(HWND hwnd, ULONG message, MPARAM mp1,
                                       MPARAM mp2)
{
   PLBCTLDATA pCtlData = (PLBCTLDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch(message)
   {
      case WM_CREATE:
         return (MRESULT) CreateList(hwnd, (PCREATESTRUCT) mp2);

      case WM_DESTROY:
         return (MRESULT) DestroyList(hwnd, pCtlData);

      case WM_CHAR:
         if (CharList(hwnd, pCtlData, SHORT1FROMMP(mp1),
                      SHORT2FROMMP(mp2), CHAR1FROMMP(mp2)))
            return (MRESULT) TRUE;
         else
         {
            WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),
                       message, mp1, mp2);
            return (MRESULT) FALSE;
         }

      case WM_CONTEXTMENU:
         {
            LONG lItem;
            MLCONTEXT MLContext;

            if (!mp2)
            {
               POINTL ptl;

               /* Context per Maus */
               lItem = ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1));
               /*if (lItem == MLIT_NONE)
                  return (MRESULT) FALSE;*/

               ptl.x = SHORT1FROMMP(mp1);
               ptl.y = SHORT2FROMMP(mp1);
               WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1);
               MLContext.xContext = ptl.x;
               MLContext.yContext = ptl.y;
               MLContext.lItem = lItem;
            }
            else
            {
               RECTL rec;

               /* Context per Tastatur */
               if (pCtlData->lItemCount == 0)
               {
                  MLContext.lItem = MLIT_NONE;
                  WinQueryWindowRect(hwnd, &rec);
               }
               else
               {
                  MLContext.lItem = pCtlData->lCrsItem;
                  CalcItemRect(hwnd, pCtlData, MLContext.lItem, &rec);
               }
               WinMapWindowPoints(hwnd, HWND_DESKTOP, (PPOINTL) &rec, 2);
               MLContext.xContext = rec.xLeft + (rec.xRight - rec.xLeft)/2;
               MLContext.yContext = rec.yBottom + (rec.yTop - rec.yBottom)/2;
            }

            /* Notification senden */
            Notify(hwnd, MLIN_CONTEXTMENU, MPFROMP(&MLContext));

            return (MRESULT) TRUE;
         }

      case WM_BUTTON1DOWN:
         if (pCtlData->ulNrSeparator != SEPA_NONE)
            TrackSeparator(hwnd, pCtlData);
         else
         {
            ClickItem(hwnd, pCtlData, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1),
                      SHORT2FROMMP(mp2));
            WinSetFocus(HWND_DESKTOP, hwnd);
            if (ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1)) != MLIT_NONE)
            {
               WinSetCapture(HWND_DESKTOP, hwnd);
               pCtlData->bSwipe = TRUE;
               pCtlData->lPrevItem = ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1));
               pCtlData->lSwipeAnchor = pCtlData->lPrevItem;
            }
         }
         break;

      case WM_BUTTON2DOWN:
      case WM_BUTTON3DOWN:
         WinSetFocus(HWND_DESKTOP, hwnd);
         break;

      case WM_MOUSEMOVE:
         if (pCtlData->bSwipe)
            MouseMove(hwnd, pCtlData, SHORT2FROMMP(mp1), SHORT2FROMMP(mp2));
         else
         {
            OverSeparator(hwnd, pCtlData, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
            if (SetPointer(hwnd, pCtlData))
               return (MRESULT) TRUE;
         }
         break;

      case WM_BUTTON1UP:
         /* Capture ausschalten */
         if (WinQueryCapture(HWND_DESKTOP) == hwnd)
         {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pCtlData->bSwipe = FALSE;
            pCtlData->lPrevItem = MLIT_NONE;
            if (pCtlData->lSwipeAnchor != pCtlData->lCrsItem)
            {
              pCtlData->lAnchorItem = pCtlData->lSwipeAnchor;
              if (pCtlData->lCrsItem < pCtlData->lSwipeAnchor)
                 pCtlData->ulPrevKey = MKEY_UP;
              else
                 pCtlData->ulPrevKey = MKEY_DOWN;
            }
            pCtlData->lSwipeAnchor = MLIT_NONE;
         }

         /* Timer evtl. stoppen */
         if (pCtlData->bTimer)
         {
            pCtlData->bTimer = FALSE;
            WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL);
         }
         break;

      case WM_OPEN:
         {
            LONG lItem;

            lItem = ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1));

            /* Notification senden */
            if (lItem != MLIT_NONE)
               Notify(hwnd, MLIN_ENTER, MPFROMLONG(lItem));
         }
         break;

      case WM_VSCROLL:
         ScrollList(hwnd, pCtlData, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
         break;

      case WM_SIZE:
         SizeList(hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2), pCtlData);
         break;

      case WM_SYSCOLORCHANGE:
      case WM_PRESPARAMCHANGED:
         pCtlData->lLineHeight = GetFontHeight(hwnd);
         pCtlData->lVisibleLines = QueryVisibleLines(pCtlData);
         CalcScrollBar(hwnd, pCtlData);
         WinInvalidateRect(hwnd, NULL, TRUE);
         /* Notification */
         if (message == WM_PRESPARAMCHANGED)
            Notify(hwnd, MLIN_PPARAMCHANGED, NULL);
         break;

      case WM_PAINT:
         DrawList(hwnd, pCtlData);
         return (MRESULT) FALSE;

      case WM_SETFOCUS:
         if (mp2)
            CreateCursor(hwnd, pCtlData);
         else
            WinDestroyCursor(hwnd);
         break;

      case WM_TIMER:
         if (SHORT1FROMMP(mp1) == TID_AUTOSCROLL)
            ListTimer(hwnd, pCtlData);
         break;

#ifdef FULLFUNC
      case WM_BEGINDRAG:
         {
            LONG lItem;

            lItem = ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1));

            /* Notification senden */
            if (lItem != MLIT_NONE)
               Notify(hwnd, MLIN_BEGINDRAG, MPFROMLONG(lItem));
         }
         break;

      case WM_ENDDRAG:
         Notify(hwnd, MLIN_ENDDRAG, NULL);
         break;
#endif

      case MLIM_CLEARLIST:
         return (MRESULT) ClearList(hwnd, pCtlData);

      /* mp1 = PMLISTRECORD pNewRecord */
      case MLIM_ADDITEM:
         return (MRESULT) AddItem(hwnd, pCtlData, (PMLISTRECORD) mp1);

      /* mp1 = PMLISTRECORD pItemArray, mp2 = ULONG ulCountItems */
      case MLIM_ADDITEMARRAY:
         return (MRESULT) AddArray(hwnd, pCtlData, (PMLISTRECORD) mp1,
                                   (ULONG) mp2);

      /* mp1 = LONG lItemIndex */
      case MLIM_DELITEM:
         return (MRESULT) DeleteItem(hwnd, pCtlData, (LONG) mp1);

      /* mp1 = PMLISTRECORD pNewRecord, mp2 = LONG lItemIndex */
      case MLIM_UPDATEITEM:
         return (MRESULT) UpdateItem(hwnd, pCtlData, (PMLISTRECORD) mp1,
                                     (LONG) mp2);

      /* mp1 = PMLISTCOLORS pColors */
      case MLIM_SETCOLORS:
         return (MRESULT) SetColors(hwnd, pCtlData, (PMLISTCOLORS) mp1);

      /* mp1 = PMLISTCOLORS pColors */
      case MLIM_QUERYCOLORS:
         return (MRESULT) QueryColors(hwnd, pCtlData, (PMLISTCOLORS) mp1);

      /* mp1 = PMLISTCOLUMNS pColumns */
      case MLIM_SETCOLUMNS:
         return (MRESULT) SetColumns(hwnd, pCtlData, (PMLISTCOLUMNS) mp1);

      case MLIM_QUERYCOLUMNS:
         return (MRESULT) QueryColumns(hwnd, pCtlData, (PMLISTCOLUMNS) mp1);

      /* mp1 = LONG lItem */
      case MLIM_SCROLLTO:
         return (MRESULT) ScrollToItem(hwnd, pCtlData, (LONG) mp1);

      case MLIM_QUERYITEMCOUNT:
         return (MRESULT) pCtlData->lItemCount;

      /* mp1 = ULONG uMsgID */
      case MLIM_FINDUMSGID:
         return (MRESULT) FindUMsgID(hwnd, pCtlData, (ULONG) mp1);

      /* mp1 = LONG lItem, mp2 = PMLISTRECORD pBuffer */
      case MLIM_QUERYITEM:
         return (MRESULT) QueryItem(hwnd, pCtlData, (LONG) mp1,
                                    (PMLISTRECORD) mp2);

      case MLIM_QUERYFSELECT:
         return (MRESULT) QueryFirstSelected(hwnd, pCtlData);

      /* mp1 = LONG lPrevItem */
      case MLIM_QUERYNSELECT:
         return (MRESULT) QueryNextSelected(hwnd, pCtlData, (LONG) mp1);

      case MLIM_QUERYCRSITEM:
         if (pCtlData->lItemCount == 0)
            return (MRESULT) MLIT_NONE;
         else
            return (MRESULT) pCtlData->lCrsItem;

      /* mp1 = LONG lItem / MLIT_NONE*/
      case MLIM_EMPHASIZEITEM:
         return (MRESULT) Emphasize(hwnd, pCtlData, (LONG) mp1);

      case MLIM_SELECTALL:
         SelectAll(hwnd, pCtlData);
         WinInvalidateRect(hwnd, NULL, TRUE);
         return (MRESULT) FALSE;

      case MLIM_SELECTNONE:
         DeselectAll(hwnd, pCtlData);
         WinInvalidateRect(hwnd, NULL, TRUE);
         return (MRESULT) FALSE;

      case MLIM_SHIFTINTOVIEW:
         WinShowCursor(hwnd, FALSE);
         pCtlData->lCrsItem = (LONG) mp1;
         ShiftIntoView(hwnd, pCtlData, (LONG) mp1);
         CalcScrollBar(hwnd, pCtlData);
         SetCursor(hwnd, pCtlData);
         return (MRESULT) TRUE;

      default:
         break;
   }
   return WinDefWindowProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateList                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Initialisierung der Liste                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCreate: Zeiger auf CREATESTRUCT                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Fehler                                               */
/*                FALSE Erfolg                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CreateList(HWND hwnd, PCREATESTRUCT pCreate)
{
   PLBCTLDATA pCtlData;
   LONG lColor;

   /* Speicher f. Control-Data belegen */

   pCtlData = calloc(1, sizeof(LBCTLDATA));
   PMASSERT(pCtlData != NULL, "Out of memory");
   WinSetWindowULong(hwnd, QWL_USER, (ULONG) pCtlData);

   /* Daten initialisieren */
   pCtlData->lLineHeight = GetFontHeight(hwnd);
   pCtlData->lUnreadClr = RGB_RED;
   pCtlData->lFromClr = RGB_RED;
   pCtlData->lToClr = RGB_RED;
   pCtlData->lAnchorItem = MLIT_NONE;

   /* Breite der Scrollbar abfragen */
   pCtlData->cxScroll = WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);

   /* Scrollbar am rechten Rand erzeugen */
   pCtlData->hwndScroll = WinCreateWindow(hwnd, WC_SCROLLBAR,
                                          NULL,
                                          SBS_VERT | WS_VISIBLE,
                                          pCreate->cx - pCtlData->cxScroll,
                                          0,
                                          pCtlData->cxScroll,
                                          pCreate->cy,
                                          hwnd, HWND_TOP,
                                          MLID_VSCROLL,
                                          NULL,
                                          NULL);
   pCtlData->rclWindow.xLeft = 0;
   pCtlData->rclWindow.yBottom = 0;
   pCtlData->rclWindow.xRight = pCreate->cx;
   pCtlData->rclWindow.yTop = pCreate->cy;

   pCtlData->rclItems.xLeft = 0;
   pCtlData->rclItems.yBottom = 0;
   pCtlData->rclItems.xRight = pCreate->cx - pCtlData->cxScroll;
   pCtlData->rclItems.yTop = pCreate->cy;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &pCtlData->rclItems, -2, -2);

   /* Presentation Parameters */
   lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_OUTPUTTEXT, 0);
   WinSetPresParam(hwnd, PP_FOREGROUNDCOLOR, sizeof(lColor), &lColor);
   lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOW, 0);
   WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, sizeof(lColor), &lColor);

   pCtlData->lLineHeight = GetFontHeight(hwnd);
   pCtlData->lVisibleLines = QueryVisibleLines(pCtlData);

   /* Spaltenbreiten */
   pCtlData->ulNrPercent   =  50;
   pCtlData->ulFromPercent = 200;
   pCtlData->ulToPercent   = 200;
   pCtlData->ulSubjPercent = 300;
   pCtlData->ulStampWrittenPercent = 120;
   pCtlData->ulStampArrivedPercent = 120;

   /* Berechnungen */
   CalcColumns(hwnd, pCtlData);
   CalcScrollBar(hwnd, pCtlData);

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DestroyList                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Aufraeumarbeiten                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Success                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL DestroyList(HWND hwnd, PLBCTLDATA pCtlData)
{
   /* Items */
   if (pCtlData->pRecords)
      free(pCtlData->pRecords);

   /* Scrollbar */
   if (pCtlData->hwndScroll)
      WinDestroyWindow(pCtlData->hwndScroll);

   /* Control-Daten */
   free (pCtlData);

   /* Warnings vermeiden */
   hwnd = hwnd;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ClearList                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht alle Eintraege in der Listbox                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Success                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL ClearList(HWND hwnd, PLBCTLDATA pCtlData)
{
   /* Items entfernen */
   if (pCtlData->pRecords)
   {
      free(pCtlData->pRecords);
      pCtlData->pRecords = NULL;
      pCtlData->lItemCount = 0;
      pCtlData->lItemAlloc = 0;
      pCtlData->lCrsItem = 0;
      pCtlData->lTopItem = 0;
   }

   /* Scrollbar neu berechnen */
   CalcScrollBar(hwnd, pCtlData);

   /* neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddItem                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt einen Eintrag in die Liste ein                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            pRecord: Neuer Eintrag                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Item-Index                                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG AddItem(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord)
{
   RECTL rectlnew;

   if (pCtlData->lItemCount == pCtlData->lItemAlloc) /* Allok voll */
   {
      PMLISTRECORD pNewArray;

      pCtlData->lItemAlloc += ALLOC_BLOCKSIZE;

      pNewArray = malloc(pCtlData->lItemAlloc * sizeof(MLISTRECORD));
      PMASSERT(pNewArray != NULL, "Out of memory");
      if (pCtlData->pRecords)
      {
         memcpy(pNewArray, pCtlData->pRecords,
                pCtlData->lItemCount * sizeof(MLISTRECORD));
         free(pCtlData->pRecords);
      }
      pCtlData->pRecords = pNewArray;
   }

   /* Hinter das letzte Element anhaengen */
   memcpy(&(pCtlData->pRecords[pCtlData->lItemCount]), pRecord,
          sizeof(MLISTRECORD));

   /* Zaehler hoch */
   pCtlData->lItemCount++;

   if (pCtlData->lItemCount == 1)
   {
      /* erstes Element eingefuegt */
      /*pCtlData->pRecords[0].flRecFlags |= LISTFLAG_SELECTED;*/
      pCtlData->lCrsItem = 0;
   }

   /* Scrollbar anpassen */
   CalcScrollBar(hwnd, pCtlData);

   /* neu zeichnen */
   if (CalcItemRect(hwnd, pCtlData, pCtlData->lItemCount-1, &rectlnew))
      WinInvalidateRect(hwnd, &rectlnew, TRUE);

   return pCtlData->lItemCount-1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddArray                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt ein Feld von Eintraegen in die Liste ein              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            pRecord: Array der Eintraege                                   */
/*            ulCountItems: Anzahl der Array-Elemente                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Anzahl der eingefuegten Elemente                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG AddArray(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord,
                      ULONG ulCountItems)
{
   if (ulCountItems == 0 ||
       pRecord == NULL)
      return 0;  /* nix zu tun */

   if (pCtlData->lItemCount + ulCountItems > pCtlData->lItemAlloc)
   {
      /* Zu wenig Speicher, neu anfordern */
      PMLISTRECORD pNewArray;
      ULONG ulNewSize;

      ulNewSize = (pCtlData->lItemAlloc + ulCountItems);

      /* Aufrunden */
      if (ulNewSize % ALLOC_BLOCKSIZE)
         ulNewSize = ((ulNewSize / ALLOC_BLOCKSIZE) + 1) * ALLOC_BLOCKSIZE;
      else
         ulNewSize = (ulNewSize / ALLOC_BLOCKSIZE) * ALLOC_BLOCKSIZE;

      pCtlData->lItemAlloc = ulNewSize;

      pNewArray = malloc(pCtlData->lItemAlloc * sizeof(MLISTRECORD));
      PMASSERT(pNewArray != NULL, "Out of memory");
      if (pCtlData->pRecords)
      {
         memcpy(pNewArray, pCtlData->pRecords,
                pCtlData->lItemCount * sizeof(MLISTRECORD));
         free(pCtlData->pRecords);
      }
      pCtlData->pRecords = pNewArray;
   }

   /* Anhaengen */
   memcpy(&(pCtlData->pRecords[pCtlData->lItemCount]), pRecord,
          ulCountItems * sizeof(MLISTRECORD));

   if (pCtlData->lItemCount == 0)
   {
      /* erstes Element eingefuegt */
      /*pCtlData->pRecords[0].flRecFlags |= LISTFLAG_SELECTED;*/
      pCtlData->lCrsItem = 0;
   }

   /* Zaehler hoch */
   pCtlData->lItemCount += ulCountItems;

   /* Scrollbar anpassen */
   CalcScrollBar(hwnd, pCtlData);

   /* neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return ulCountItems;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UpdateItem                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Daten eines Eintrags neu                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            pRecord: Neuer Eintrag                                         */
/*            lItem: Item-Index                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Success                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL UpdateItem(HWND hwnd, PLBCTLDATA pCtlData, PMLISTRECORD pRecord,
                       LONG lItem)
{
   RECTL rectlnew;

   if (lItem < 0 ||
       lItem > (pCtlData->lItemCount-1))  /* Index fehlerhaft */
      return FALSE;

   /* Element updaten */
   memcpy(&(pCtlData->pRecords[lItem]), pRecord, sizeof(MLISTRECORD));

   /* neu zeichnen */
   if (CalcItemRect(hwnd, pCtlData, lItem, &rectlnew))
      WinInvalidateRect(hwnd, &rectlnew, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteItem                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht einen Eintrag aus der Liste                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lDelNr: Item-Index                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Success                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL DeleteItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lDelNr)
{
   RECTL rectl;
   LONG lVisibleLines;

   if (lDelNr < 0 ||
       lDelNr > (pCtlData->lItemCount-1))  /* Index fehlerhaft */
      return FALSE;

   /* Rect des zu l敗chenden Items ermitteln */
   CalcItemRect(hwnd, pCtlData, lDelNr, &rectl);

   if (lDelNr == (pCtlData->lItemCount-1)) /* letzter Eintrag */
      pCtlData->lItemCount--; /* einfach ausknipsen */
   else
   {
      /* von hinten nach vorne kopieren */
      memcpy(&(pCtlData->pRecords[lDelNr]), &(pCtlData->pRecords[lDelNr+1]),
             (pCtlData->lItemCount - lDelNr - 1) * sizeof(MLISTRECORD));

      pCtlData->lItemCount--; /* einen weniger */
   }

   if (lDelNr < pCtlData->lCrsItem)
      pCtlData->lCrsItem--;  /* Item vor Cursor geloescht, nachfuehren */

   /* Instanzdaten pruefen */
   if (pCtlData->lCrsItem > pCtlData->lItemCount)
      pCtlData->lCrsItem = pCtlData->lItemCount;

   if (pCtlData->lTopItem > pCtlData->lItemCount)
      pCtlData->lTopItem = pCtlData->lItemCount;

   /* Scrollbar neu setzen */
   CalcScrollBar(hwnd, pCtlData);

   /* Cursor neu setzen */
   SetCursor(hwnd, pCtlData);

   /* neu zeichnen */
   if (rectl.yTop > pCtlData->rclItems.yBottom)
   {
      /* Geloeschtes Item war sichtbar, bis zum Ende der Liste neu zeichnen */
      rectl.yBottom = pCtlData->rclItems.yBottom;
      if (rectl.yTop > pCtlData->rclItems.yTop)
         rectl.yTop = pCtlData->rclItems.yTop;
      WinInvalidateRect(hwnd, &rectl, TRUE);
   }

   lVisibleLines = pCtlData->lVisibleLines;

   if (pCtlData->lTopItem > 0 &&
       pCtlData->lTopItem + lVisibleLines > pCtlData->lItemCount)
   {
      /* am sichtbaren Ende geloescht, eine Zeile hochscrollen */
      pCtlData->lTopItem--;
      WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight),
                      &pCtlData->rclItems, &pCtlData->rclItems,
                      (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
      CalcScrollBar(hwnd, pCtlData);
   }

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetColors                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Farben des Controls neu                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            pColors: Zeiger auf neue Farben                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Success                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetColors(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLORS pColors)
{
   if (!pColors)
      return FALSE;

   /* Farben neu setzen */
   pCtlData->lUnreadClr = pColors->lUnreadClr;
   pCtlData->lFromClr   = pColors->lFromClr;
   pCtlData->lToClr     = pColors->lToClr;

   /* neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryColors                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fragt die Farben des Controls ab                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            pColors: Zeiger auf neue Farben                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Success                                              */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL QueryColors(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLORS pColors)
{
   if (!pColors)
      return FALSE;

   /* Farben neu setzen */
   pColors->lUnreadClr = pCtlData->lUnreadClr;
   pColors->lFromClr   = pCtlData->lFromClr;
   pColors->lToClr     = pCtlData->lToClr;

   /* Warnings vermeiden */
   hwnd = hwnd;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindUMsgID                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht das Item mit der angegebenen UMSGID                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            ulMsgID: gesuchte UMSGID                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MLIT_NONE: nicht gefunden                                  */
/*                sonst:     Item-Index                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG FindUMsgID(HWND hwnd, PLBCTLDATA pCtlData, ULONG ulMsgID)
{
   LONG i;

   if (pCtlData->pRecords)
      for (i=0; i< pCtlData->lItemCount; i++)
         if (pCtlData->pRecords[i].ulMsgID == ulMsgID)
            return i;

   /* Warnings vermeiden */
   hwnd = hwnd;

   return MLIT_NONE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CalcScrollBar                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet Daten des Scrollbars neu                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Setzt auch die Daten im Scrollbar-Control                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CalcScrollBar(HWND hwnd, PLBCTLDATA pCtlData)
{
   SHORT sVisibleLines, sScrollerBottom;

   sVisibleLines = pCtlData->lVisibleLines;

   sScrollerBottom= pCtlData->lItemCount - sVisibleLines;

   WinSendMsg(pCtlData->hwndScroll, SBM_SETSCROLLBAR,
                     MPFROMSHORT(pCtlData->lTopItem),
                     MPFROM2SHORT(0, sScrollerBottom));

   WinSendMsg(pCtlData->hwndScroll, SBM_SETTHUMBSIZE,
                     MPFROM2SHORT(sVisibleLines, pCtlData->lItemCount),
                     NULL);

   /* Warnings vermeiden */
   hwnd = hwnd;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetFontHeight                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt die Fonthoehe                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Font-Hoehe                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG GetFontHeight(HWND hwnd)
{
   FONTMETRICS fm;
   HPS hps;

   hps = WinGetPS(hwnd);
   GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
   WinReleasePS(hps);

   return fm.lMaxAscender*3/2;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CalcColumns                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet Spaltenbreiten neu                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle                                            */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CalcColumns(HWND hwnd, PLBCTLDATA pCtlData)
{
   ULONG ulWidth = (pCtlData->rclItems.xRight - pCtlData->rclItems.xLeft);

   pCtlData->ulNrWidth   = (pCtlData->ulNrPercent   * ulWidth) / 1000;
   if (pCtlData->ulNrWidth < 2)
      pCtlData->ulNrWidth = 2;
   pCtlData->ulFromWidth = (pCtlData->ulFromPercent * ulWidth) / 1000;
   if (pCtlData->ulFromWidth < 2)
      pCtlData->ulFromWidth = 2;
   pCtlData->ulToWidth   = (pCtlData->ulToPercent   * ulWidth) / 1000;
   if (pCtlData->ulToWidth < 2)
      pCtlData->ulToWidth = 2;
   pCtlData->ulSubjWidth = (pCtlData->ulSubjPercent * ulWidth) / 1000;
   if (pCtlData->ulSubjWidth < 2)
      pCtlData->ulSubjWidth = 2;
   pCtlData->ulStampWrittenWidth = (pCtlData->ulStampWrittenPercent * ulWidth) / 1000;
   if (pCtlData->ulStampWrittenWidth < 2)
      pCtlData->ulStampWrittenWidth = 2;
   pCtlData->ulStampArrivedWidth = (pCtlData->ulStampArrivedPercent * ulWidth) / 1000;
   if (pCtlData->ulStampArrivedWidth < 2)
      pCtlData->ulStampArrivedWidth = 2;

   /* Warnings vermeiden */
   hwnd = hwnd;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DrawList                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_PAINT                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DrawList(HWND hwnd, PLBCTLDATA pCtlData)
{
   HPS hps;
   ULONG ulLine=0;
   RECTL updrectl, drawrcl, intrcl;
   POINTL pointl;
   POINTL pt;
   LONG lBorderColor, lShadowColor, lBackColor;

   hps = WinBeginPaint(hwnd, NULLHANDLE, &updrectl);

   /* Fensterhintergrund */
   WinQueryPresParam(hwnd,
                     PP_BACKGROUNDCOLOR,
                     PP_BACKGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBackColor,
                     QPF_ID2COLORINDEX);


   /* Rahmen-Farben */
   lBorderColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWFRAME, 0);
   lShadowColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);

   /* Auf RGB schalten */
   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   /* Rahmen zeichnen */
   GpiSetColor(hps, lShadowColor);

   pointl.x = 1;
   pointl.y = 0;
   GpiMove(hps, &pointl);

   pointl.x = pCtlData->rclItems.xRight+1;
   pointl.y = pCtlData->rclItems.yTop;
   GpiBox(hps, DRO_OUTLINE, &pointl, 0, 0);

   GpiSetColor(hps, lBorderColor);

   pointl.x = 0;
   pointl.y = 1;
   GpiMove(hps, &pointl);

   pointl.x = pCtlData->rclItems.xRight;
   pointl.y = pCtlData->rclItems.yTop+1;
   GpiBox(hps, DRO_OUTLINE, &pointl, 0, 0);


   /* Alle Items zeichnen */
   ulLine = pCtlData->lTopItem;

   while (ulLine < pCtlData->lItemCount)
   {
      /* Rechteck dieses Items */
      drawrcl.yBottom = pCtlData->rclItems.yTop -
                        (ulLine - pCtlData->lTopItem + 1)* pCtlData->lLineHeight;
      drawrcl.yTop = drawrcl.yBottom + pCtlData->lLineHeight;
      drawrcl.xLeft = pCtlData->rclItems.xLeft;
      drawrcl.xRight = pCtlData->rclItems.xRight;

      if (drawrcl.yTop < pCtlData->rclItems.yBottom)  /* Ende des Fensters */
         break;

      if (drawrcl.yBottom <= pCtlData->rclItems.yBottom)
         drawrcl.yBottom = pCtlData->rclItems.yBottom;

      if (WinIntersectRect(WinQueryAnchorBlock(hwnd), &intrcl, &drawrcl, &updrectl))
         /* Item zeichnen */
         DrawListItem(hwnd, hps, ulLine + 1, &drawrcl,
                      &(pCtlData->pRecords[ulLine]), pCtlData);

      /* n�chstes Item */
      ulLine++;
   }

   /* Rest des Fensters loeschen */
   if (pCtlData->lItemCount)
   {
      CalcItemRect(hwnd, pCtlData, ulLine-1, &drawrcl);
      drawrcl.yTop = drawrcl.yBottom;
      drawrcl.yBottom = updrectl.yBottom;
      if (drawrcl.yBottom < pCtlData->rclItems.yBottom)
         drawrcl.yBottom = pCtlData->rclItems.yBottom;
      WinFillRect(hps, &drawrcl, lBackColor);
   }
   else
      WinFillRect(hps, &updrectl, lBackColor);

   /* Linien zeichnen */
   pt.y = pCtlData->rclItems.yBottom;
   pt.x = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth;
   GpiMove(hps, &pt);
   pt.y = pCtlData->rclItems.yTop;
   GpiLine(hps, &pt);

   pt.x += pCtlData->ulFromWidth;
   GpiMove(hps, &pt);
   pt.y = pCtlData->rclItems.yBottom;
   GpiLine(hps, &pt);

   pt.x += pCtlData->ulToWidth;
   GpiMove(hps, &pt);
   pt.y = pCtlData->rclItems.yTop;
   GpiLine(hps, &pt);

   pt.x += pCtlData->ulSubjWidth;
   GpiMove(hps, &pt);
   pt.y = pCtlData->rclItems.yBottom;
   GpiLine(hps, &pt);

   pt.x += pCtlData->ulStampWrittenWidth;
   GpiMove(hps, &pt);
   pt.y = pCtlData->rclItems.yTop;
   GpiLine(hps, &pt);

   /* Ende der Vorstellung */
   WinEndPaint(hps);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DrawListItem                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Zeichnet ein List-Item                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            hps: Presentation Space                                        */
/*            ulItemNum: Item-Nummer (1..n)                                  */
/*            prclItem: Item-Rectangle (Clip)                                */
/*            pItem: Zeiger auf Item-Daten                                   */
/*            pCtlData: Control-Daten                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Erfolg                                                */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL DrawListItem(HWND hwnd, HPS hps, ULONG ulItemNum, PRECTL prclItem,
                         PMLISTRECORD pItem, PLBCTLDATA pCtlData)
{
   char pchNumber[20];
   RECTL colrcl = {0, 0, 0, 0};
   LONG lBackColor, lForeColor;
   LONG lBackColorHi=0, lForeColorHi=0;
   char TimeStamp[50];

   /* Fensterhintergrund */
   WinQueryPresParam(hwnd,
                     PP_BACKGROUNDCOLOR,
                     PP_BACKGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBackColor,
                     QPF_ID2COLORINDEX);

   /* Schrift */
   WinQueryPresParam(hwnd,
                     PP_FOREGROUNDCOLOR,
                     PP_FOREGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lForeColor,
                     QPF_ID2COLORINDEX);

   /* Fensterhintergrund highlight */
   lBackColorHi = WinQuerySysColor(HWND_DESKTOP, SYSCLR_HILITEBACKGROUND, 0);

   /* Schrift highlight*/
   lForeColorHi = WinQuerySysColor(HWND_DESKTOP, SYSCLR_HILITEFOREGROUND, 0);

   /* Kontrollieren, ob geladen */
   if (!(pItem->flRecFlags & LISTFLAG_LOADED))
   {
      pItem->flRecFlags |= LISTFLAG_LOADED;

      /* Owner zum Laden auffordern */
      if (!Notify(hwnd, MLIN_LOADITEM, pItem))
      {
         /* Fehler beim Laden */
         pItem->flRecFlags |= LISTFLAG_ERROR;
         strcpy(pItem->pchFrom,     "*");
         strcpy(pItem->pchTo,       "*");
         strcpy(pItem->pchSubject,  "*");
         memset(&pItem->StampWritten, 0, sizeof(pItem->StampWritten));
         memset(&pItem->StampArrived, 0, sizeof(pItem->StampArrived));
      }
   }

   /* Hintergrund loeschen */
   if (pItem->flRecFlags & LISTFLAG_SELECTED)
      WinFillRect(hps, prclItem, lBackColorHi);
   else
      WinFillRect(hps, prclItem, lBackColor);

   /* Alle Felder zeichnen */
   /* Messagenummer */
   colrcl.xLeft   = prclItem->xLeft;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = colrcl.xLeft + pCtlData->ulNrWidth;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);


   itoa(ulItemNum, pchNumber, 10);
   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, pchNumber, &colrcl, lForeColorHi, lBackColorHi,
                  DT_RIGHT | DT_TOP);
   }
   else
   {
      if (pItem->flRecFlags & LISTFLAG_READ)
      {
         WinDrawText(hps, -1, pchNumber, &colrcl, lForeColor, lBackColor,
                     DT_RIGHT | DT_TOP);
      }
      else
      {
         WinDrawText(hps, -1, pchNumber, &colrcl, pCtlData->lUnreadClr,
                     lBackColor, DT_RIGHT | DT_TOP);
      }
   }

   /* From */
   colrcl.xLeft   = prclItem->xLeft + pCtlData->ulNrWidth;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = colrcl.xLeft + pCtlData->ulFromWidth;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);

   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, pItem->pchFrom, &colrcl, lForeColorHi, lBackColorHi,
                  DT_LEFT | DT_TOP);
   }
   else
   {
      if (pItem->flRecFlags & LISTFLAG_FROMME)
      {
         WinDrawText(hps, -1, pItem->pchFrom, &colrcl, pCtlData->lFromClr,
                     lBackColor, DT_LEFT | DT_TOP);
      }
      else
      {
         WinDrawText(hps, -1, pItem->pchFrom, &colrcl, lForeColor, lBackColor,
                     DT_LEFT | DT_TOP);
      }
   }

   /* To */
   colrcl.xLeft   = prclItem->xLeft + pCtlData->ulNrWidth + pCtlData->ulFromWidth;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = colrcl.xLeft + pCtlData->ulToWidth;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);

   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, pItem->pchTo, &colrcl, lForeColorHi, lBackColorHi,
                  DT_LEFT | DT_TOP);
   }
   else
   {
      if (pItem->flRecFlags & LISTFLAG_TOME)
      {
         WinDrawText(hps, -1, pItem->pchTo, &colrcl, pCtlData->lToClr,
                     lBackColor, DT_LEFT | DT_TOP);
      }
      else
      {
         WinDrawText(hps, -1, pItem->pchTo, &colrcl, lForeColor, lBackColor,
                     DT_LEFT | DT_TOP);
      }
   }

   /* Subject */
   colrcl.xLeft   = prclItem->xLeft + pCtlData->ulNrWidth +
                    pCtlData->ulFromWidth + pCtlData->ulToWidth;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = colrcl.xLeft + pCtlData->ulSubjWidth;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);

   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, pItem->pchSubject, &colrcl, lForeColorHi,
                  lBackColorHi, DT_LEFT | DT_TOP);
   }
   else
   {
      WinDrawText(hps, -1, pItem->pchSubject, &colrcl, lForeColor, lBackColor,
                  DT_LEFT | DT_TOP);
   }

   /* StampWritten */
   colrcl.xLeft   = prclItem->xLeft + pCtlData->ulNrWidth +
                    pCtlData->ulFromWidth + pCtlData->ulToWidth + pCtlData->ulSubjWidth;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = colrcl.xLeft + pCtlData->ulStampWrittenWidth;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);

   StampToString(TimeStamp, &pItem->StampWritten);

   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, TimeStamp, &colrcl, lForeColorHi,
                  lBackColorHi, DT_LEFT | DT_TOP);
   }
   else
   {
      WinDrawText(hps, -1, TimeStamp, &colrcl, lForeColor, lBackColor,
                  DT_LEFT | DT_TOP);
   }

   /* StampArrived */
   colrcl.xLeft   = prclItem->xLeft + pCtlData->ulNrWidth +
                    pCtlData->ulFromWidth + pCtlData->ulToWidth + pCtlData->ulSubjWidth + pCtlData->ulStampWrittenWidth;
   colrcl.yBottom = prclItem->yBottom;
   colrcl.yTop    = prclItem->yTop;
   colrcl.xRight  = prclItem->xRight;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &colrcl, -5, 0);

   StampToString(TimeStamp, &pItem->StampArrived);

   if (pItem->flRecFlags & LISTFLAG_SELECTED)
   {
      WinDrawText(hps, -1, TimeStamp, &colrcl, lForeColorHi,
                  lBackColorHi, DT_LEFT | DT_TOP);
   }
   else
   {
      WinDrawText(hps, -1, TimeStamp, &colrcl, lForeColor, lBackColor,
                  DT_LEFT | DT_TOP);
   }

   /* Source-Emphasis ? */
   if (pItem->flRecFlags & LISTFLAG_SOURCE)
   {
      /* Kringel aussenrum */
      POINTL pointl;
      LONG lOldClr;

      /* Backmix auf Invert */
      GpiSetBackMix(hps, BM_XOR);
      lOldClr = GpiQueryColor(hps);
      GpiSetColor(hps, lForeColor);

      pointl.x = prclItem->xLeft+1;
      pointl.y = prclItem->yBottom+1;
      GpiMove(hps, &pointl);

      pointl.x = prclItem->xRight-2;
      pointl.y = prclItem->yTop-2;
      GpiBox(hps, DRO_OUTLINE, &pointl, pCtlData->lLineHeight/3,
                                        pCtlData->lLineHeight/3);
      /* Backmix zurueck */
      GpiSetBackMix(hps, BM_DEFAULT);
      GpiSetColor(hps, lOldClr);
   }

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ItemFromY                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt von der Y-Position den Item-Index                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Control-Daten                                        */
/*            y: y-Koordinate                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MLIT_NONE: kein Item getroffen                             */
/*                sonst: Item-Index                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: y ist relativ zum Control-Window                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG ItemFromY(HWND hwnd, PLBCTLDATA pCtlData, LONG y)
{
   LONG lItem;

   /* Warnings vermeiden */
   hwnd = hwnd;

   if (y > pCtlData->rclItems.yTop ||
       y < pCtlData->rclItems.yBottom)
     return MLIT_NONE;

   lItem = ((pCtlData->rclItems.yTop - y)/
            pCtlData->lLineHeight) + (LONG)pCtlData->lTopItem;

   if (lItem >= pCtlData->lItemCount || lItem < 0)
      return MLIT_NONE;
   else
      return lItem;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScrollToItem                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scrollt zu dem angegebenen Item, setzt Cursor auf das Item  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Control-Daten                                        */
/*            lItem: Item-Index                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Erfolg                                               */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL ScrollToItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem)
{
   LONG lLinesToScroll=0;
   ULONG ulVisibleLines = pCtlData->lVisibleLines;

   if (lItem < 0 ||
       lItem >= pCtlData->lItemCount ||
       pCtlData->lItemCount == 0)
      return FALSE;

   if (ulVisibleLines < pCtlData->lItemCount)
      if (lItem + ulVisibleLines >= pCtlData->lItemCount)
         lLinesToScroll = lItem - pCtlData->lTopItem -
                          (lItem + ulVisibleLines - pCtlData->lItemCount);
      else
         lLinesToScroll = lItem - pCtlData->lTopItem;

   pCtlData->lTopItem += lLinesToScroll;

   pCtlData->lCrsItem = lItem;

   if (ABS(pCtlData->lLineHeight * lLinesToScroll) > 32000)
      WinInvalidateRect(hwnd, NULL, TRUE);
   else
      WinScrollWindow(hwnd, 0, (pCtlData->lLineHeight * lLinesToScroll),
                      &pCtlData->rclItems, &pCtlData->rclItems, (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
   SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);

   CalcScrollBar(hwnd, pCtlData);
   SetCursor(hwnd, pCtlData);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetColumns                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Spaltenbreiten neu                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Control-Daten                                        */
/*            pColumns: neue Spaltenbreiten                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Erfolg                                               */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetColumns(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLUMNS pColumns)
{
   if (!pColumns)
      return FALSE;

   /* Daten setzen */
   pCtlData->ulNrPercent   = pColumns->ulNrPercent;
   pCtlData->ulFromPercent = pColumns->ulFromPercent;
   pCtlData->ulToPercent   = pColumns->ulToPercent;
   pCtlData->ulSubjPercent = pColumns->ulSubjPercent;
   pCtlData->ulStampWrittenPercent = pColumns->ulStampWrittenPercent;
   pCtlData->ulStampArrivedPercent = pColumns->ulStampArrivedPercent;

   /* Breiten neu berechnen */
   CalcColumns(hwnd, pCtlData);

   /* neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryColumns                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fragt die Spaltenbreiten ab                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Control-Daten                                        */
/*            pColumns: Puffer f. Spaltenbreiten                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Erfolg                                               */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL  QueryColumns(HWND hwnd, PLBCTLDATA pCtlData, PMLISTCOLUMNS pColumns)
{
   hwnd = hwnd;

   if (!pColumns)
      return FALSE;

   /* Daten setzen */
   pColumns->ulNrPercent   = pCtlData->ulNrPercent;
   pColumns->ulFromPercent = pCtlData->ulFromPercent;
   pColumns->ulToPercent   = pCtlData->ulToPercent;
   pColumns->ulSubjPercent = pCtlData->ulSubjPercent;
   pColumns->ulStampWrittenPercent = pCtlData->ulStampWrittenPercent;
   pColumns->ulStampArrivedPercent = pCtlData->ulStampArrivedPercent;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SizeList                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Passt an die neue Fenstergroesse an                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            x: neue Breite                                                 */
/*            y: neue Hoehe                                                  */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SizeList(HWND hwnd, SHORT x, SHORT y, PLBCTLDATA pCtlData)
{
   ULONG ulVisibleLines;

   /* Scrollbar neu positionieren */

   WinSetWindowPos(pCtlData->hwndScroll,
                   NULLHANDLE,
                   x - pCtlData->cxScroll,
                   0,
                   pCtlData->cxScroll,
                   y,
                   SWP_MOVE | SWP_SIZE);

   /* neue Fenstergroesse */
   pCtlData->rclWindow.xLeft   = 0;
   pCtlData->rclWindow.yBottom = 0;
   pCtlData->rclWindow.xRight  = x;
   pCtlData->rclWindow.yTop    = y;

   pCtlData->rclItems.xLeft = 0;
   pCtlData->rclItems.yBottom = 0;
   pCtlData->rclItems.xRight = x - pCtlData->cxScroll;
   pCtlData->rclItems.yTop = y;
   WinInflateRect(WinQueryAnchorBlock(hwnd), &pCtlData->rclItems, -2, -2);

   /* Spalten neu berechnen */
   CalcColumns(hwnd, pCtlData);

   /* eventuell hochscrollen */
   ulVisibleLines = pCtlData->lVisibleLines = QueryVisibleLines(pCtlData);
   if (pCtlData->lItemCount > ulVisibleLines)
   {
      if (pCtlData->lTopItem + ulVisibleLines >= pCtlData->lItemCount)
         pCtlData->lTopItem -= pCtlData->lTopItem + ulVisibleLines -
                                pCtlData->lItemCount;
   }
   else
      pCtlData->lTopItem = 0;

   /* Scrollbar anpassen */
   CalcScrollBar(hwnd, pCtlData);

   CreateCursor(hwnd, pCtlData);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScrollList                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scrollt die Liste                                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            sSliderPos: Position des Sliders                               */
/*            usCommand: Art des Scrollens                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ScrollList(HWND hwnd, PLBCTLDATA pCtlData, SHORT sSliderPos,
                       USHORT usCommand)
{
   ULONG ulVisibleLines = pCtlData->lVisibleLines;

   switch(usCommand)
   {
      case SB_LINEUP:
         if (pCtlData->lTopItem > 0)
         {
            pCtlData->lTopItem--;
            WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_LINEDOWN:
         if (pCtlData->lTopItem < (pCtlData->lItemCount - ulVisibleLines))
         {
            pCtlData->lTopItem++;
            WinScrollWindow(hwnd, 0, pCtlData->lLineHeight,
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_PAGEUP:
         if (pCtlData->lTopItem > 0)
         {
            ULONG ulLinesScrolled;

            if (pCtlData->lTopItem >= ulVisibleLines)
               ulLinesScrolled = ulVisibleLines;
            else
               ulLinesScrolled = pCtlData->lTopItem;

            pCtlData->lTopItem -= ulLinesScrolled;
            WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight * ulLinesScrolled),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_PAGEDOWN:
         if (pCtlData->lTopItem < (pCtlData->lItemCount+1 - ulVisibleLines))
         {
            ULONG ulLinesScrolled;
            ULONG ulNewTop;

            ulLinesScrolled = pCtlData->lVisibleLines -1;
            if (pCtlData->lTopItem + ulLinesScrolled + pCtlData->lVisibleLines >
                pCtlData->lItemCount)
            {
               ulNewTop = pCtlData->lItemCount - pCtlData->lVisibleLines;
               ulLinesScrolled = ulNewTop - pCtlData->lTopItem;
            }

            pCtlData->lTopItem += ulLinesScrolled;

            WinScrollWindow(hwnd, 0, (pCtlData->lLineHeight * ulLinesScrolled),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_SLIDERTRACK:
         {
            LONG lLinesScrolled=(sSliderPos - pCtlData->lTopItem);

            pCtlData->lTopItem=(ULONG) sSliderPos;
            if (ABS(pCtlData->lLineHeight * lLinesScrolled) > 32000)
               WinInvalidateRect(hwnd, NULL, TRUE);
            else
               WinScrollWindow(hwnd, 0, (pCtlData->lLineHeight * lLinesScrolled),
                               &pCtlData->rclItems, &pCtlData->rclItems,
                               (HRGN) NULL, NULL,
                               SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      default:
         break;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryItem                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Daten eines Items ab                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: Item-Index                                              */
/*            pRecord: Zeiger auf Record-Buffer                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL QueryItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem,
                      PMLISTRECORD pRecord)
{
   if (lItem < 0 ||
       lItem >= pCtlData->lItemCount)
      return FALSE;

   if (!pRecord)
      return FALSE;

   /* Daten kopieren */
   memcpy(pRecord, &(pCtlData->pRecords[lItem]), sizeof(MLISTRECORD));

   /* Warnings vermeiden */
   hwnd = hwnd;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CalcItemRect                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet das Rectangle des angegebenen Items               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: Item-Index                                              */
/*            pDestRect: Ergebnis-RECTL                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CalcItemRect(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem,
                         PRECTL pDestRect)
{
   if (lItem < 0 ||
       lItem >= pCtlData->lItemCount)
      return FALSE;

   if (!pDestRect)
      return FALSE;

   pDestRect->yTop = pCtlData->rclItems.yTop -
                     ((lItem - pCtlData->lTopItem) * pCtlData->lLineHeight);
   pDestRect->yBottom = pDestRect->yTop - pCtlData->lLineHeight;
   pDestRect->xLeft   = pCtlData->rclItems.xLeft;
   pDestRect->xRight  = pCtlData->rclItems.xRight;

   /* Warnings vermeiden */
   hwnd = hwnd;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CharList                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wertet die Char-Meldungen aus                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            usFlags: Flags                                                 */
/*            usVK: Virtual Key Code                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Taste verarbeitet                                    */
/*                FALSE Taste nicht verarbeitet                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CharList(HWND hwnd, PLBCTLDATA pCtlData, USHORT usFlags,
                     USHORT usVK, USHORT usChar)
{
   RECTL rec;
   ULONG ulOldCrs;
   LONG  lVisibleLines = pCtlData->lVisibleLines;
   ULONG ulNewTop;

   if (usFlags & KC_KEYUP)
      return FALSE;

   if (usFlags & KC_VIRTUALKEY)
   {
      switch(usVK)
      {
         case VK_DOWN:
            if (pCtlData->lItemCount == 0 ||
                pCtlData->lCrsItem >= (pCtlData->lItemCount-1))
               return TRUE; /* Ende der Liste */

            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey = MKEY_DOWN;
               }
            }

            pCtlData->lCrsItem++;
            WinShowCursor(hwnd, FALSE);
            ShiftIntoView(hwnd, pCtlData, pCtlData->lCrsItem);
            if (usFlags & KC_SHIFT)
               ShiftSelect(hwnd, pCtlData, pCtlData->lCrsItem-1, MKEY_DOWN);
            else
               SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);

            return TRUE;

         case VK_UP:
            if (pCtlData->lCrsItem == 0)
               return TRUE; /* Anfang der Liste */

            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey = MKEY_UP;
               }
            }

            pCtlData->lCrsItem--;
            WinShowCursor(hwnd, FALSE);
            ShiftIntoView(hwnd, pCtlData, pCtlData->lCrsItem);
            if (usFlags & KC_SHIFT)
               ShiftSelect(hwnd, pCtlData, pCtlData->lCrsItem+1, MKEY_UP);
            else
               SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);
            return TRUE;

         case VK_HOME:
            if (pCtlData->lCrsItem == 0)
               return TRUE; /* Anfang der Liste */
            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;

               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = 0;
               pCtlData->pRecords[pCtlData->lCrsItem].flRecFlags |= LISTFLAG_SELECTED;
               ShiftIntoView(hwnd, pCtlData, 0);
               CalcScrollBar(hwnd, pCtlData);
               SetCursor(hwnd, pCtlData);
               CalcItemRect(hwnd, pCtlData, ulOldCrs, &rec);
               WinInvalidateRect(hwnd, &rec, TRUE);
               CalcItemRect(hwnd, pCtlData, pCtlData->lCrsItem, &rec);
               WinInvalidateRect(hwnd, &rec, TRUE);
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey= MKEY_UP;
               }

               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = 0;
               ShiftIntoView(hwnd, pCtlData, 0);
               CalcScrollBar(hwnd, pCtlData);
               ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_UP);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

         case VK_END:
            if (pCtlData->lItemCount == 0 ||
                pCtlData->lCrsItem >= (pCtlData->lItemCount-1))
               return TRUE; /* Anfang der Liste */
            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;

               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = pCtlData->lItemCount-1;
               ScrollToItem(hwnd, pCtlData, pCtlData->lItemCount-1);
               pCtlData->pRecords[pCtlData->lCrsItem].flRecFlags |= LISTFLAG_SELECTED;
               CalcScrollBar(hwnd, pCtlData);
               SetCursor(hwnd, pCtlData);
               CalcItemRect(hwnd, pCtlData, ulOldCrs, &rec);
               WinInvalidateRect(hwnd, &rec, TRUE);
               CalcItemRect(hwnd, pCtlData, pCtlData->lCrsItem, &rec);
               WinInvalidateRect(hwnd, &rec, TRUE);
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey= MKEY_DOWN;
               }

               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = pCtlData->lItemCount-1;
               ShiftIntoView(hwnd, pCtlData,pCtlData->lItemCount-1);
               CalcScrollBar(hwnd, pCtlData);
               ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_DOWN);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

         case VK_PAGEDOWN:
            if (pCtlData->lItemCount == 0 ||
                pCtlData->lCrsItem >= (pCtlData->lItemCount-1))
               return TRUE;
            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey= MKEY_DOWN;
               }
            }

            if (((LONG)pCtlData->lTopItem + lVisibleLines) >= pCtlData->lItemCount)
            {
               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = pCtlData->lItemCount-1;
               ScrollToItem(hwnd, pCtlData, pCtlData->lCrsItem);
               if (usFlags & KC_SHIFT)
                  ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_DOWN);
               else
                  SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
               SetCursor(hwnd, pCtlData);
            }
            else
            {
               ulNewTop = pCtlData->lTopItem + lVisibleLines;
               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = ulNewTop;
               ScrollToItem(hwnd, pCtlData, ulNewTop);
               if (usFlags & KC_SHIFT)
                  ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_DOWN);
               else
                  SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

         case VK_PAGEUP:
            if (pCtlData->lCrsItem == 0)
               return TRUE;
            if (!(usFlags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;
            }
            else
            {
               /* Anker setzen */
               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  pCtlData->lAnchorItem = pCtlData->lCrsItem;
                  pCtlData->ulPrevKey= MKEY_UP;
               }
            }

            if (((LONG)pCtlData->lTopItem - lVisibleLines) < 0)
            {
               ulNewTop = 0;
               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = ulNewTop;
               ScrollToItem(hwnd, pCtlData, ulNewTop);
               if (usFlags & KC_SHIFT)
                  ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_UP);
               else
                  SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
               SetCursor(hwnd, pCtlData);
            }
            else
            {
               ulNewTop = pCtlData->lTopItem - lVisibleLines;
               ulOldCrs = pCtlData->lCrsItem;
               pCtlData->lCrsItem = ulNewTop;
               pCtlData->lTopItem = ulNewTop;
               WinShowCursor(hwnd, FALSE);
               if (usFlags & KC_SHIFT)
                  ShiftSelect(hwnd, pCtlData, ulOldCrs, MKEY_UP);
               else
                  SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
               CalcScrollBar(hwnd, pCtlData);
               WinInvalidateRect(hwnd, NULL, FALSE);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

         case VK_NEWLINE:
         case VK_ENTER:
            /* Notification senden */
            if (pCtlData->lCrsItem < pCtlData->lItemCount)
               Notify(hwnd, MLIN_ENTER, MPFROMLONG(pCtlData->lCrsItem));
            return TRUE;

         case VK_TAB:
            WinSetFocus(HWND_DESKTOP,
                        WinEnumDlgItem(WinQueryWindow(hwnd, QW_PARENT),
                                       hwnd,
                                       EDI_NEXTTABITEM));
            return TRUE;

         case VK_BACKTAB:
            WinSetFocus(HWND_DESKTOP,
                        WinEnumDlgItem(WinQueryWindow(hwnd, QW_PARENT),
                                       hwnd,
                                       EDI_PREVTABITEM));
            return TRUE;

         default:
            break;
      }
   }

#if 0
   if (usFlags & KC_CHAR)
   {
#endif
      switch((unsigned char)usChar)
      {
         case '/':
            if (usFlags & KC_CTRL)
            {
               SelectAll(hwnd, pCtlData);
               WinInvalidateRect(hwnd, NULL, TRUE);
               return TRUE;
            }
            else
               return FALSE;

         case '\\':
            if (usFlags & KC_CTRL)
            {
               DeselectAll(hwnd, pCtlData);
               WinInvalidateRect(hwnd, NULL, TRUE);
               return TRUE;
            }
            else
               return FALSE;

         default:
            return FALSE;
      }
#if 0
   }
   else
      return FALSE;
#endif
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ShiftSelect                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt Range-Select durch mit Selektion und Deselektion     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lOldCrs:  vorherige Cursor-Position                            */
/*            ulAction: Bewegungstyp, MKEY_UP oder MKEY_DOWN                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ShiftSelect(HWND hwnd, PLBCTLDATA pCtlData, LONG lOldCrs, ULONG ulAction)
{
   LONG i, lStep;

   if (pCtlData->ulPrevKey != ulAction)
   {
      if (pCtlData->ulPrevKey == MKEY_UP)
      {
         lStep = (pCtlData->lAnchorItem <= pCtlData->lCrsItem)?pCtlData->lAnchorItem:pCtlData->lCrsItem;

         for (i=lOldCrs; i < lStep; i++)
            DeSelectItem(hwnd, pCtlData, i);

         for (i= pCtlData->lAnchorItem; i<=pCtlData->lCrsItem; i++)
            SelectItem(hwnd, pCtlData, i);

         if (pCtlData->lCrsItem > pCtlData->lAnchorItem)
            pCtlData->ulPrevKey = MKEY_DOWN;
      }
      else
         if (pCtlData->ulPrevKey == MKEY_DOWN)
         {
            lStep = (pCtlData->lAnchorItem >= pCtlData->lCrsItem)?pCtlData->lAnchorItem:pCtlData->lCrsItem;

            for (i=lOldCrs; i > lStep; i--)
               DeSelectItem(hwnd, pCtlData, i);

            for (i= pCtlData->lAnchorItem; i>=pCtlData->lCrsItem; i--)
               SelectItem(hwnd, pCtlData, i);

            if (pCtlData->lCrsItem < pCtlData->lAnchorItem)
               pCtlData->ulPrevKey = MKEY_UP;
         }
   }
   else
   {
      if (pCtlData->lCrsItem < lOldCrs)
      {
         for (i=lOldCrs; i >= pCtlData->lCrsItem; i--)
            SelectItem(hwnd, pCtlData, i);
      }
      else
         if (pCtlData->lCrsItem > lOldCrs)
         {
            for (i=lOldCrs; i<=pCtlData->lCrsItem; i++)
               SelectItem(hwnd, pCtlData, i);
         }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SelectItem                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Selektiert ein Item, Neuzeichnen falls notwendig            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: gewuenschtes Item                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem)
{
   if (!(pCtlData->pRecords[lItem].flRecFlags & LISTFLAG_SELECTED))
   {
      RECTL rec;

      pCtlData->pRecords[lItem].flRecFlags |= LISTFLAG_SELECTED;
      CalcItemRect(hwnd, pCtlData, lItem, &rec);
      WinInvalidateRect(hwnd, &rec, TRUE);
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeSelectItem                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: De-selektiert ein Item, Neuzeichnen falls notwendig         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: gewuenschtes Item                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DeSelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem)
{
   if (pCtlData->pRecords[lItem].flRecFlags & LISTFLAG_SELECTED)
   {
      RECTL rec;

      pCtlData->pRecords[lItem].flRecFlags &= ~LISTFLAG_SELECTED;
      CalcItemRect(hwnd, pCtlData, lItem, &rec);
      WinInvalidateRect(hwnd, &rec, TRUE);
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateCursor                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt den Cursor                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CreateCursor(HWND hwnd, PLBCTLDATA pCtlData)
{
   RECTL CrsRect;

   if (CalcItemRect(hwnd, pCtlData, pCtlData->lCrsItem, &CrsRect))
   {
      WinInflateRect(WinQueryAnchorBlock(hwnd), &CrsRect, -1, -1);

      WinCreateCursor(hwnd, CrsRect.xLeft, CrsRect.yBottom,
                      CrsRect.xRight - CrsRect.xLeft,
                      CrsRect.yTop   - CrsRect.yBottom,
                      CURSOR_HALFTONE | CURSOR_FRAME,
                      &pCtlData->rclItems);

      WinShowCursor(hwnd, TRUE);
   }
   else
      WinShowCursor(hwnd, FALSE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetCursor                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Cursorposition neu                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SetCursor(HWND hwnd, PLBCTLDATA pCtlData)
{
   RECTL CrsRect;

   if (CalcItemRect(hwnd, pCtlData, pCtlData->lCrsItem, &CrsRect))
   {
      WinInflateRect(WinQueryAnchorBlock(hwnd), &CrsRect, -1, -1);

      WinCreateCursor(hwnd, CrsRect.xLeft, CrsRect.yBottom,
                      CrsRect.xRight - CrsRect.xLeft,
                      CrsRect.yTop   - CrsRect.yBottom,
                      CURSOR_SETPOS,
                      &pCtlData->rclItems);

      WinShowCursor(hwnd, TRUE);
   }
   else
      WinShowCursor(hwnd, FALSE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ClickItem                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet einen Mausklick auf ein Item                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            x: X-Koordinate relativ zu List-Window                         */
/*            y: Y-Koordinate relativ zu List-Window                         */
/*            flags: Keyboard-Flags                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ClickItem(HWND hwnd, PLBCTLDATA pCtlData, LONG x, LONG y,
                      SHORT flags)
{
   POINTL pt;

   pt.x = x;
   pt.y = y;

   if (WinPtInRect(WinQueryAnchorBlock(hwnd), &pCtlData->rclItems, &pt))
   {
      LONG lItem;

      if ((lItem = ItemFromY(hwnd, pCtlData, pt.y /*- pCtlData->rclItems.yBottom*/))!=MLIT_NONE)
      {
         RECTL rec;
         LONG lOldCrs =pCtlData->lCrsItem;

         pCtlData->lCrsItem = lItem;

         if (!(flags & KC_CTRL))
         {
            if (!(flags & KC_SHIFT))
            {
               DeselectAll(hwnd, pCtlData);
               SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
               SetCursor(hwnd, pCtlData);
               /* Anker freigeben */
               pCtlData->lAnchorItem = MLIT_NONE;
               pCtlData->ulPrevKey = MKEY_NONE;
            }
            else
            {
               /* Range select vom Cursor-Item bis zum neuen Item */
               ULONG ulAction;

               if (lOldCrs > pCtlData->lCrsItem)
                  ulAction = MKEY_UP;
               else
                  ulAction = MKEY_DOWN;

               if (pCtlData->lAnchorItem == MLIT_NONE)
               {
                  /* Anker setzen */
                  pCtlData->lAnchorItem = lOldCrs;
                  pCtlData->ulPrevKey = ulAction;
               }

               ShiftSelect(hwnd, pCtlData, lOldCrs, ulAction);
               SetCursor(hwnd, pCtlData);
            }
         }
         else
         {
            pCtlData->pRecords[pCtlData->lCrsItem].flRecFlags ^= LISTFLAG_SELECTED;
            CalcItemRect(hwnd, pCtlData, lItem, &rec);
            WinInvalidateRect(hwnd, &rec, TRUE);
            SetCursor(hwnd, pCtlData);
            /* Anker freigeben */
            pCtlData->lAnchorItem = MLIT_NONE;
            pCtlData->ulPrevKey = MKEY_NONE;
         }
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ShiftIntoView                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scrollt die Liste so, dass das Item komplett sichtbar ist   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: Item-Index                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ShiftIntoView(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem)
{
   RECTL rec;
   LONG lVisibleLines = pCtlData->lVisibleLines;
   LONG lLinesToScroll;

   CalcItemRect(hwnd, pCtlData, lItem, &rec);

   if (rec.yBottom < pCtlData->rclItems.yBottom) /* nach unten */
   {
      lLinesToScroll = lItem - (pCtlData->lTopItem + lVisibleLines)+1;
      pCtlData->lTopItem += lLinesToScroll;
      if (ABS(pCtlData->lLineHeight * lLinesToScroll) > 32000)
         WinInvalidateRect(hwnd, NULL, TRUE);
      else
         WinScrollWindow(hwnd, 0, pCtlData->lLineHeight * lLinesToScroll,
                         &pCtlData->rclItems, &pCtlData->rclItems,
                         (HRGN) NULL, NULL,
                         SW_INVALIDATERGN);
      return;
   }
   else
      if (rec.yTop > pCtlData->rclItems.yTop)   /* nach oben */
      {
         lLinesToScroll = pCtlData->lTopItem - lItem;
         pCtlData->lTopItem -= lLinesToScroll;
         if (ABS(pCtlData->lLineHeight * lLinesToScroll) > 32000)
            WinInvalidateRect(hwnd, NULL, TRUE);
         else
            WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight * lLinesToScroll),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
         return;
      }
      else
         return;  /* Item war schon komplett sichtbar */
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeselectAll                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Hebt das Select-Flag fuer alle Items auf und zeichnet       */
/*               gff neu.                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DeselectAll(HWND hwnd, PLBCTLDATA pCtlData)
{
   int i;
   RECTL rec, invrec={0,0,0,0};
   BOOL bRedraw = FALSE;

   for (i = 0; i < pCtlData->lItemCount; i++)
      if (pCtlData->pRecords[i].flRecFlags & LISTFLAG_SELECTED)
      {
         pCtlData->pRecords[i].flRecFlags &= ~LISTFLAG_SELECTED;

         CalcItemRect(hwnd, pCtlData, i, &rec);
         WinUnionRect(WinQueryAnchorBlock(hwnd), &invrec, &rec, &invrec);

         bRedraw = TRUE;
      }

   if (bRedraw)
      WinInvalidateRect(hwnd, &invrec, TRUE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SelectAll                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt das Select-Flag fuer alle Items und zeichnet          */
/*               gff neu.                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SelectAll(HWND hwnd, PLBCTLDATA pCtlData)
{
   int i;
   RECTL rec, invrec={0,0,0,0};
   BOOL bRedraw = FALSE;

   for (i = 0; i < pCtlData->lItemCount; i++)
      if (!(pCtlData->pRecords[i].flRecFlags & LISTFLAG_SELECTED))
      {
         pCtlData->pRecords[i].flRecFlags |= LISTFLAG_SELECTED;

         CalcItemRect(hwnd, pCtlData, i, &rec);
         WinUnionRect(WinQueryAnchorBlock(hwnd), &invrec, &rec, &invrec);

         bRedraw = TRUE;
      }

   if (bRedraw)
      WinInvalidateRect(hwnd, &invrec, TRUE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MouseMove                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wertet die Mausbewegungen aus                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            y: Y-Koordinate                                                */
/*            flags: Keyboard-Flags                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  MouseMove(HWND hwnd, PLBCTLDATA pCtlData, SHORT y, SHORT flags)
{
   LONG lItem;

   flags = flags;

   lItem = ItemFromY(hwnd, pCtlData, y);

   if (lItem != MLIT_NONE)
   {
      /* Klick auf Item */

      if (lItem != pCtlData->lPrevItem)
      {
         /* Item hat gewechselt */
         RECTL rec;
         LONG lChaItem;

         pCtlData->lCrsItem = lItem;
         SetCursor(hwnd, pCtlData);
         /* vom letzten Record bis zum neuen Record demarkieren */
         if (lItem > pCtlData->lPrevItem)
         {
            /* nach unten */
            for (lChaItem = pCtlData->lPrevItem; lChaItem < lItem; lChaItem++)
            {
               if ((pCtlData->pRecords[lChaItem].flRecFlags & LISTFLAG_SELECTED))
               {
                  pCtlData->pRecords[lChaItem].flRecFlags &= ~LISTFLAG_SELECTED;
                  CalcItemRect(hwnd, pCtlData, lChaItem, &rec);
                  WinInvalidateRect(hwnd, &rec, TRUE);
               }
            }
         }
         else
            /* nach oben markieren */
            for (lChaItem = pCtlData->lPrevItem; lChaItem > lItem; lChaItem--)
            {
               if ((pCtlData->pRecords[lChaItem].flRecFlags & LISTFLAG_SELECTED))
               {
                  pCtlData->pRecords[lChaItem].flRecFlags &= ~LISTFLAG_SELECTED;
                  CalcItemRect(hwnd, pCtlData, lChaItem, &rec);
                  WinInvalidateRect(hwnd, &rec, TRUE);
               }
            }

         /* vom Anker bis zum neuen Record markieren */
         if (lItem > pCtlData->lSwipeAnchor)
         {
            /* nach unten markieren */
            for (lChaItem = pCtlData->lSwipeAnchor; lChaItem <= lItem; lChaItem++)
            {
               if (!(pCtlData->pRecords[lChaItem].flRecFlags & LISTFLAG_SELECTED))
               {
                  pCtlData->pRecords[lChaItem].flRecFlags |= LISTFLAG_SELECTED;
                  CalcItemRect(hwnd, pCtlData, lChaItem, &rec);
                  WinInvalidateRect(hwnd, &rec, TRUE);
               }
            }
         }
         else
            /* nach oben markieren */
            for (lChaItem = pCtlData->lSwipeAnchor; lChaItem >= lItem; lChaItem--)
            {
               if (!(pCtlData->pRecords[lChaItem].flRecFlags & LISTFLAG_SELECTED))
               {
                  pCtlData->pRecords[lChaItem].flRecFlags |= LISTFLAG_SELECTED;
                  CalcItemRect(hwnd, pCtlData, lChaItem, &rec);
                  WinInvalidateRect(hwnd, &rec, TRUE);
               }
            }

         pCtlData->lPrevItem = lItem;
      }
   }
   else
   {
      if (y < pCtlData->rclItems.yBottom)
      {
         /* Nach unten gezogen */
         if (!pCtlData->bTimer )
         {
            DragDown(hwnd, pCtlData);
            pCtlData->bTimer = TRUE;
            WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL,
                          WinQuerySysValue(HWND_DESKTOP, SV_SCROLLRATE));
         }
      }
      else
      {
         if (y > pCtlData->rclItems.yTop)
         {
            /* Nach oben gezogen */
            if (!pCtlData->bTimer )
            {
               DragUp(hwnd, pCtlData);
               pCtlData->bTimer = TRUE;
               WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL,
                             WinQuerySysValue(HWND_DESKTOP, SV_SCROLLRATE));
            }
         }
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryFirstSelected                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt den Item-Index des ersten ausgewaehlten Items          */
/*               zurueck                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Item-Index                                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG QueryFirstSelected(HWND hwnd, PLBCTLDATA pCtlData)
{
   int i;

   hwnd = hwnd;

   for (i=0; i < pCtlData->lItemCount; i++)
      if (pCtlData->pRecords[i].flRecFlags & LISTFLAG_SELECTED)
         return i;

   return MLIT_NONE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryNextSelected                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht den naechsten ausgewaehlten Eintrag                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lPrevItem: Item, ab dem gesucht werden soll                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Item-Index, oder MLIT_NONE                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG QueryNextSelected(HWND hwnd, PLBCTLDATA pCtlData, LONG lPrevItem)
{
   int i;

   hwnd = hwnd;

   if (lPrevItem < 0)
      return MLIT_NONE;

   for (i=lPrevItem+1; i < pCtlData->lItemCount; i++)
      if (pCtlData->pRecords[i].flRecFlags & LISTFLAG_SELECTED)
         return i;

   return MLIT_NONE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListTimer                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wertet die Timer-Message aus                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  ListTimer(HWND hwnd, PLBCTLDATA pCtlData)
{
   POINTL ptl;

   /* Pointer-Position im Window ermitteln */
   WinQueryPointerPos(HWND_DESKTOP, &ptl);
   WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, TRUE);

   if (WinPtInRect(WinQueryAnchorBlock(hwnd), &pCtlData->rclItems, &ptl) )
   {
      /* Pointer ist wieder drin, Ende der Show */
      pCtlData->bTimer = FALSE;
      WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL);
   }
   else
   {
     /* drueber ? */
     if (ptl.y > pCtlData->rclItems.yTop )
        DragUp(hwnd, pCtlData);
     else
       /* drunter ? */
       if (ptl.y < pCtlData->rclItems.yBottom )
          DragDown(hwnd, pCtlData);
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DragUp                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scrollt die Liste nach oben, wenn Swipe-Select ueber den    */
/*               oberen Rand                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DragUp(HWND hwnd, PLBCTLDATA pCtlData)
{
   while (pCtlData->lCrsItem >= pCtlData->lTopItem)
   {
      if (pCtlData->lCrsItem > pCtlData->lSwipeAnchor)
         DeSelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
      else
         SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
      pCtlData->lCrsItem--;
   }
   if (pCtlData->lCrsItem < 0)
      pCtlData->lCrsItem = 0;

   if (pCtlData->lTopItem > 0)
   {
      pCtlData->lTopItem--;
      pCtlData->lPrevItem = pCtlData->lCrsItem;

      WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight),
                      &pCtlData->rclItems, &pCtlData->rclItems,
                      (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
      SetCursor(hwnd, pCtlData);
      WinUpdateWindow(hwnd);
      CalcScrollBar(hwnd, pCtlData);
   }
   else
   {
      pCtlData->lPrevItem = pCtlData->lCrsItem;
      SetCursor(hwnd, pCtlData);
   }


   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DragDown                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scroll die Liste nach unten, wenn Swipe-Select unter den    */
/*               unteren Rand                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  DragDown(HWND hwnd, PLBCTLDATA pCtlData)
{
   LONG lVisibleLines = pCtlData->lVisibleLines;

   while (pCtlData->lCrsItem < pCtlData->lTopItem + lVisibleLines)
   {
      if (pCtlData->lCrsItem > pCtlData->lSwipeAnchor)
         SelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
      else
         DeSelectItem(hwnd, pCtlData, pCtlData->lCrsItem);
      pCtlData->lCrsItem++;
   }
   if (pCtlData->lItemCount > 0 &&
       pCtlData->lCrsItem >= pCtlData->lItemCount)
      pCtlData->lCrsItem = pCtlData->lItemCount-1;

   if (pCtlData->lTopItem + lVisibleLines < pCtlData->lItemCount)
   {
      pCtlData->lTopItem++;
      pCtlData->lPrevItem = pCtlData->lCrsItem;

      WinScrollWindow(hwnd, 0, pCtlData->lLineHeight,
                      &pCtlData->rclItems, &pCtlData->rclItems,
                      (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
      SetCursor(hwnd, pCtlData);
      WinUpdateWindow(hwnd);
      CalcScrollBar(hwnd, pCtlData);
   }
   else
   {
      pCtlData->lPrevItem = pCtlData->lCrsItem;
      SetCursor(hwnd, pCtlData);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OverSeparator                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Stellt fest, ob der Pointer ueber einer Trennlinie ist,     */
/*               setzt den Pointer ggf. neu                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            x: X-Koordinate des Pointers                                   */
/*            y: Y-Koordinate des Pointers                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  OverSeparator(HWND hwnd, PLBCTLDATA pCtlData, SHORT x, SHORT y)
{
   SHORT sSepPos;

   hwnd = hwnd;

   if (y < pCtlData->rclItems.yBottom ||
       y > pCtlData->rclItems.yTop    ||
       x < pCtlData->rclItems.xLeft   ||
       x > pCtlData->rclItems.xRight)
   {
      /* Ausserhalb */
      pCtlData->ulNrSeparator = SEPA_NONE;

      return;
   }

   /* erste Linie */
   sSepPos = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth;

   if ((sSepPos - 2 <= x) &&
       (sSepPos + 2 >= x))
   {
      pCtlData->ulNrSeparator = SEPA_NUMFROM;

      return;
   }

   /* zweite Linie */
   sSepPos = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth;

   if ((sSepPos - 2 <= x) &&
       (sSepPos + 2 >= x))
   {
      pCtlData->ulNrSeparator = SEPA_FROMTO;

      return;
   }

   /* dritte Linie */
   sSepPos = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth;

   if ((sSepPos - 2 <= x) &&
       (sSepPos + 2 >= x))
   {
      pCtlData->ulNrSeparator = SEPA_TOSUBJ;

      return;
   }

   /* vierte Linie */
   sSepPos = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth +
                                        pCtlData->ulSubjWidth;

   if ((sSepPos - 2 <= x) &&
       (sSepPos + 2 >= x))
   {
      pCtlData->ulNrSeparator = SEPA_SUBJWR;

      return;
   }

   /* f］fte Linie */
   sSepPos = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth +
                                        pCtlData->ulSubjWidth +
                                        pCtlData->ulStampWrittenWidth;

   if ((sSepPos - 2 <= x) &&
       (sSepPos + 2 >= x))
   {
      pCtlData->ulNrSeparator = SEPA_WRARR;

      return;
   }

   pCtlData->ulNrSeparator = SEPA_NONE;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TrackSeparator                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verschiebt den Separator                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  TrackSeparator(HWND hwnd, PLBCTLDATA pCtlData)
{
   TRACKINFO TrackInfo;
   LONG lOldX;

   /* Daten vorbelegen */
   TrackInfo.cxBorder = 1;
   TrackInfo.cyBorder = 1;
   TrackInfo.cxGrid = 1;
   TrackInfo.cyGrid = 1;
   TrackInfo.cxKeyboard = 1;
   TrackInfo.cyKeyboard = 1;

   WinCopyRect(WinQueryAnchorBlock(hwnd), &TrackInfo.rclBoundary, &pCtlData->rclItems);

   switch(pCtlData->ulNrSeparator)
   {
      case SEPA_NUMFROM:
         lOldX = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth;
         TrackInfo.rclBoundary.xLeft = pCtlData->rclItems.xLeft;
         TrackInfo.rclBoundary.xRight = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth;
         break;

      case SEPA_FROMTO:
         lOldX = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                            pCtlData->ulFromWidth;
         TrackInfo.rclBoundary.xLeft = pCtlData->rclItems.xLeft +
                                       pCtlData->ulNrWidth;
         TrackInfo.rclBoundary.xRight = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth;
         break;

      case SEPA_TOSUBJ:
         lOldX = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                            pCtlData->ulFromWidth +
                                            pCtlData->ulToWidth;
         TrackInfo.rclBoundary.xLeft = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth;
         TrackInfo.rclBoundary.xRight = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth +
                                        pCtlData->ulSubjWidth;
         break;

      case SEPA_SUBJWR:
         lOldX = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                            pCtlData->ulFromWidth +
                                            pCtlData->ulToWidth +
                                            pCtlData->ulSubjWidth;
         TrackInfo.rclBoundary.xLeft = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth;
         TrackInfo.rclBoundary.xRight = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth +
                                        pCtlData->ulSubjWidth +
                                        pCtlData->ulStampWrittenWidth;
         break;

      case SEPA_WRARR:
         lOldX = pCtlData->rclItems.xLeft + pCtlData->ulNrWidth +
                                            pCtlData->ulFromWidth +
                                            pCtlData->ulToWidth +
                                            pCtlData->ulSubjWidth +
                                            pCtlData->ulStampWrittenWidth;
         TrackInfo.rclBoundary.xLeft = pCtlData->rclItems.xLeft +
                                        pCtlData->ulNrWidth +
                                        pCtlData->ulFromWidth +
                                        pCtlData->ulToWidth +
                                        pCtlData->ulSubjWidth;
         TrackInfo.rclBoundary.xRight = pCtlData->rclItems.xRight;
         break;

      default:
         return;
   }

   TrackInfo.rclBoundary.yBottom = pCtlData->rclItems.yBottom;
   TrackInfo.rclBoundary.yTop = pCtlData->rclItems.yTop;

   WinInflateRect(WinQueryAnchorBlock(hwnd), &TrackInfo.rclBoundary, -5, 0);

   TrackInfo.rclTrack.xLeft = lOldX;
   TrackInfo.rclTrack.xRight = lOldX+1;
   TrackInfo.rclTrack.yBottom = pCtlData->rclItems.yBottom;
   TrackInfo.rclTrack.yTop = pCtlData->rclItems.yTop;

   TrackInfo.ptlMinTrackSize.x = 1;
   TrackInfo.ptlMinTrackSize.y = 1;
   TrackInfo.ptlMaxTrackSize.x = 1;
   TrackInfo.ptlMaxTrackSize.y = pCtlData->rclItems.yTop;

   TrackInfo.fs = TF_ALLINBOUNDARY | TF_MOVE;

   /* Track */
   if (WinTrackRect(hwnd, NULLHANDLE, &TrackInfo))
   {
      LONG lMoved;  /* Anzahl Pixel der Verschiebung */
                    /* < 0 : nach links  */
                    /* > 0 : nach rechts */

      /* Verschiebung berechnen */
      lMoved = TrackInfo.rclTrack.xLeft - lOldX;

      if (lMoved == 0)
         return;  /* keine Verschiebung */

      /* Linie verschieben */
      switch(pCtlData->ulNrSeparator)
      {
         case SEPA_NONE:  /* kann nicht vorkommen */
            return;

         case SEPA_NUMFROM:
            pCtlData->ulNrWidth   += lMoved;
            pCtlData->ulFromWidth -= lMoved;
            break;

         case SEPA_FROMTO:
            pCtlData->ulFromWidth += lMoved;
            pCtlData->ulToWidth   -= lMoved;
            break;

         case SEPA_TOSUBJ:
            pCtlData->ulToWidth   += lMoved;
            pCtlData->ulSubjWidth -= lMoved;
            break;

         case SEPA_SUBJWR:
            pCtlData->ulSubjWidth   += lMoved;
            pCtlData->ulStampWrittenWidth -= lMoved;
            break;

         case SEPA_WRARR:
            pCtlData->ulStampWrittenWidth   += lMoved;
            pCtlData->ulStampArrivedWidth -= lMoved;
            break;

         default:
            return;
      }

      /* Verteilung neu berechnen */
      RecalcColumns(hwnd, pCtlData);

      /* neu zeichnen */
      WinInvalidateRect(hwnd, NULL, TRUE);

      /* Notification */
      Notify(hwnd, MLIN_SEPACHANGED, NULL);
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RecalcColumns                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet aus den Spaltenbreiten die neue Spalten-          */
/*               verteilung                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void  RecalcColumns(HWND hwnd, PLBCTLDATA pCtlData)
{
   ULONG ulW = pCtlData->rclItems.xRight - pCtlData->rclItems.xLeft;

   hwnd = hwnd;

   pCtlData->ulNrPercent = pCtlData->ulNrWidth * 1000 / ulW;
   if (pCtlData->ulNrPercent < 1)
      pCtlData->ulNrPercent = 1;
   pCtlData->ulFromPercent = pCtlData->ulFromWidth * 1000 / ulW;
   if (pCtlData->ulFromPercent < 1)
      pCtlData->ulFromPercent = 1;
   pCtlData->ulToPercent = pCtlData->ulToWidth * 1000 / ulW;
   if (pCtlData->ulToPercent < 1)
      pCtlData->ulToPercent = 1;
   pCtlData->ulSubjPercent = pCtlData->ulSubjWidth * 1000 / ulW;
   if (pCtlData->ulSubjPercent < 1)
      pCtlData->ulSubjPercent = 1;
   pCtlData->ulStampWrittenPercent = pCtlData->ulStampWrittenWidth * 1000 / ulW;
   if (pCtlData->ulStampWrittenPercent < 1)
      pCtlData->ulStampWrittenPercent = 1;

   pCtlData->ulStampArrivedPercent = 1000 - pCtlData->ulNrPercent -
                                   pCtlData->ulFromPercent -
                                   pCtlData->ulToPercent - pCtlData->ulSubjPercent -
                                   pCtlData->ulStampWrittenPercent;
   if (pCtlData->ulStampArrivedPercent < 1)
      pCtlData->ulStampArrivedPercent = 1;
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: Emphasize                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt oder loescht Source-Emphasis                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*            lItem: Item-Index oder MLIT_NONE                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Ist lItem == MLIT_NONE, so wird die Source-Emphasis von allen  */
/*            Items weggenommen. Wenn lItem auch Selected ist, so wird       */
/*            die Source-Emphasis auch bei allen anderen selektierten Items  */
/*            eingeschaltet, sonst nur bei diesem Item.                      */
/*---------------------------------------------------------------------------*/

static BOOL  Emphasize(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem)
{
   if (lItem >= (LONG) pCtlData->lItemCount ||
       (lItem < 0 && lItem != MLIT_NONE))
      return FALSE;

   if (lItem == MLIT_NONE)
   {
      /* ueberall ausschalten */
      int i;
      RECTL rec;

      for (i= 0; i < pCtlData->lItemCount; i++)
         if (pCtlData->pRecords[i].flRecFlags & LISTFLAG_SOURCE)
         {
            pCtlData->pRecords[i].flRecFlags &= ~LISTFLAG_SOURCE;

            CalcItemRect(hwnd, pCtlData, i, &rec);
            WinInvalidateRect(hwnd, &rec, TRUE);
         }
   }
   else
   {
      if (pCtlData->pRecords[lItem].flRecFlags & LISTFLAG_SELECTED)
      {
         int i;
         RECTL rec;

         /* Item ist selected, bei allen selected Items einschalten */

         for (i= 0; i < pCtlData->lItemCount; i++)
            if (pCtlData->pRecords[i].flRecFlags & LISTFLAG_SELECTED)
            {
               if (!(pCtlData->pRecords[i].flRecFlags & LISTFLAG_SOURCE))
               {
                  pCtlData->pRecords[i].flRecFlags |= LISTFLAG_SOURCE;

                  CalcItemRect(hwnd, pCtlData, i, &rec);
                  WinInvalidateRect(hwnd, &rec, TRUE);
               }
            }
      }
      else
      {
         /* beim Item einschalten */
         if (!(pCtlData->pRecords[lItem].flRecFlags & LISTFLAG_SOURCE))
         {
            RECTL rec;

            pCtlData->pRecords[lItem].flRecFlags |= LISTFLAG_SOURCE;

            CalcItemRect(hwnd, pCtlData, lItem, &rec);
            WinInvalidateRect(hwnd, &rec, TRUE);
         }
      }
   }

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetPointer                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt den richtigen Maus-Pointer                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle                                           */
/*            pCtlData: Zeiger auf Instanzdaten                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Maus-Pointer gesetzt                                 */
/*                FALSE Default-Maus-Pointer                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*---------------------------------------------------------------------------*/

static BOOL SetPointer(HWND hwnd, PLBCTLDATA pCtlData)
{
   HPOINTER hptr;

   hwnd = hwnd;

   if (pCtlData->ulNrSeparator == SEPA_NONE)
      return FALSE;

   hptr = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE);
   WinSetPointer(HWND_DESKTOP, hptr);

   return TRUE;
}

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}

static LONG QueryVisibleLines(PLBCTLDATA pCtlData)
{
   return (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
          pCtlData->lLineHeight;
}
