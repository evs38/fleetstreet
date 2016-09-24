/*---------------------------------------------------------------------------+
 | Titel: LISTBOX.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 20.10.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Schnellere Listbox f. FleetStreet                                       |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen: Dieses Control bildet eine normale Listbox nach.             |
 |              Abweichungen: Alle Item-Handles sind LONGs, d.h. es sind     |
 |                            mehr als 32768 Items m波lich.                  |
 |                            Nur eingeschr�nkte Funktionalitд.             |
 |                                                                           |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "..\main.h"       /* Stringlaengendefinitionen   */
#include "..\util\fltutil.h"
#include "..\dump\pmassert.h"
#include "listboxprv.h"
#include "listbox.h"       /* externes Interface          */

/*--------------------------------- Defines ---------------------------------*/

#define ALLOC_BLOCKSIZE     1000   /* Anzahl der Items pro Allokationsblock */
#define EXTRA_BYTES            4   /* Anzahl der Bytes in den Window-Words  */

#ifndef ABS
#define ABS(x) (((x)>=0)?(x):-(x))
#endif

/*---------------------------------- Typen ----------------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY ListBoxProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
static void  DrawList(HWND hwnd, PLBCTLDATA pCtlData);
static BOOL  CreateList(HWND hwnd, PCREATESTRUCT pCreate);
static BOOL  DestroyList(HWND hwnd, PLBCTLDATA pCtlData);
static LONG  AddItem(HWND hwnd, PLBCTLDATA pCtlData, PCHAR pchText);
static LONG  AddArray(HWND hwnd, PLBCTLDATA pCtlData, PLBOXINFO pLBoxInfo, PCHAR *ppLines);
static BOOL  ClearList(HWND hwnd, PLBCTLDATA pCtlData);
static BOOL  QueryItemText(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem, PCHAR pchDest);

static BOOL  ScrollToItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void  CalcScrollBar(HWND hwnd, PLBCTLDATA pCtlData);
static void  SizeList(HWND hwnd, SHORT x, SHORT y, PLBCTLDATA pCtlData);
static void  ScrollList(HWND hwnd, PLBCTLDATA pCtlData, SHORT sSliderPos,
                        USHORT usCommand);
static LONG  GetFontHeight(HWND hwnd);
static LONG  ItemFromY(HWND hwnd, PLBCTLDATA pCtlData, LONG y);
static BOOL  CharList(HWND hwnd, PLBCTLDATA pCtlData, USHORT usFlags,
                      USHORT usVK, USHORT usChar);
static BOOL  CalcItemRect(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem,
                          PRECTL pDestRect);
static void  CreateCursor(HWND hwnd, PLBCTLDATA pCtlData);
static void  SetCursor(HWND hwnd, PLBCTLDATA pCtlData);
static void  ClickItem(HWND hwnd, PLBCTLDATA pCtlData, LONG x, LONG y);
static void  ShiftIntoView(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void  DeselectAll(HWND hwnd, PLBCTLDATA pCtlData);
static void  SelectAll(HWND hwnd, PLBCTLDATA pCtlData);
static void  MouseMove(HWND hwnd, PLBCTLDATA pCtlData, SHORT y, SHORT flags);
static LONG  QuerySelection(HWND hwnd, PLBCTLDATA pCtlData, LONG lStart);
static void  ListTimer(HWND hwnd, PLBCTLDATA pCtlData);
static void  DragUp(HWND hwnd, PLBCTLDATA pCtlData);
static void  DragDown(HWND hwnd, PLBCTLDATA pCtlData);
static void SelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static void DeSelectItem(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);
static LONG QueryColor(HWND hwnd, ULONG idAttr1, ULONG idAttr2, LONG lSysColor);
static LONG SearchString(PLBCTLDATA pCtlData, LONG lStart, USHORT usOptions, PCHAR pchText);
static void CalcScrollFactor(PLBCTLDATA pCtlData);

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterListBox                                            */
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

BOOL EXPENTRY RegisterListBox(HAB hab)
{
   if (WinRegisterClass(hab,
                        WC_EXTLISTBOX,
                        ListBoxProc,
                        CS_CLIPCHILDREN | CS_SYNCPAINT,
                        EXTRA_BYTES))
      return TRUE;
   else
      return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListBoxProc                                                */
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

static MRESULT EXPENTRY ListBoxProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
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

      case WM_QUERYDLGCODE:
         return (MRESULT) DLGC_MLE;

      case WM_BUTTON1DOWN:
         {
            ClickItem(hwnd, pCtlData, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
            WinSetFocus(HWND_DESKTOP, hwnd);
            if (ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1)) != LIT_NONE)
            {
               WinSetCapture(HWND_DESKTOP, hwnd);
               pCtlData->bSwipe = TRUE;
               pCtlData->lPrevItem = ItemFromY(hwnd, pCtlData, SHORT2FROMMP(mp1));
               if (pCtlData->pRecords[pCtlData->lPrevItem].ulFlags & LISTFLAG_SELECTED)
                  pCtlData->bSelect=TRUE;
               else
                  pCtlData->bSelect=FALSE;
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
         break;

      case WM_BUTTON1UP:
         /* Capture ausschalten */
         if (WinQueryCapture(HWND_DESKTOP) == hwnd)
         {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pCtlData->bSwipe = FALSE;
            pCtlData->lPrevItem = LIT_NONE;
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
            if (lItem != LIT_NONE)
               Notify(hwnd, LN_ENTER, MPFROMLONG(lItem));
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
         CalcScrollBar(hwnd, pCtlData);
         WinInvalidateRect(hwnd, NULL, TRUE);
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

      case LM_DELETEALL:
         return (MRESULT) ClearList(hwnd, pCtlData);

      case LM_INSERTITEM:
         return (MRESULT) AddItem(hwnd, pCtlData, (PCHAR) mp2);

      case LM_INSERTMULTITEMS:
         return (MRESULT) AddArray(hwnd, pCtlData, (PLBOXINFO) mp1, (PCHAR*) mp2);

      case LM_QUERYITEMCOUNT:
         return (MRESULT) pCtlData->lItemCount;

      case LM_QUERYTOPINDEX:
         if (pCtlData->lItemCount)
            return (MRESULT) pCtlData->lTopItem;
         else
            return (MRESULT) LIT_NONE;

      case LM_SETTOPINDEX:
         return (MRESULT) ScrollToItem(hwnd, pCtlData, (LONG) mp1);

      case LM_QUERYITEMHANDLE:
         if (LONGFROMMP(mp1) < pCtlData->lItemCount)
            return pCtlData->pRecords[LONGFROMMP(mp1)].pItemHandle;
         else
            return NULL;

      case LM_SETITEMHANDLE:
         if (LONGFROMMP(mp1) < pCtlData->lItemCount)
         {
            pCtlData->pRecords[LONGFROMMP(mp1)].pItemHandle = (PVOID) mp2;

            return (MRESULT) TRUE;
         }
         else
            return (MRESULT) FALSE;

      case LM_QUERYSELECTION:
         return (MRESULT) QuerySelection(hwnd, pCtlData, (LONG) mp1);

      case LM_SELECTITEM:
         if (LONGFROMMP(mp1) == LIT_NONE)
         {
            DeselectAll(hwnd, pCtlData);
            return (MRESULT) TRUE;
         }
         else
            if (LONGFROMMP(mp1) == LIT_ALL)
            {
               SelectAll(hwnd, pCtlData);
               return (MRESULT) TRUE;
            }
            else
            {
               if (SHORT1FROMMP(mp2))
                  SelectItem(hwnd, pCtlData, (LONG) mp1);
               else
                  DeSelectItem(hwnd, pCtlData, (LONG) mp1);

               return (MRESULT) TRUE;
            }

      case LM_QUERYITEMTEXT:
         return (MRESULT) QueryItemText(hwnd, pCtlData, (LONG) mp1, (PCHAR) mp2);

      case LM_SEARCHSTRING:
         return (MRESULT) SearchString(pCtlData, ((PLBSEARCH)mp1)->lItemStart, ((PLBSEARCH)mp1)->usCmd, (PCHAR) mp2);

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

   /* Speicher f. Control-Data belegen */

   pCtlData = calloc(1, sizeof(LBCTLDATA));
   PMASSERT(pCtlData != NULL, "Out of memory");
   WinSetWindowULong(hwnd, QWL_USER, (ULONG) pCtlData);

   /* Daten initialisieren */
   pCtlData->lLineHeight = GetFontHeight(hwnd);

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

   /* Berechnungen */
   CalcScrollFactor(pCtlData);
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
   if (pCtlData->lItemCount)
   {
      long i=0;

      for ( ; i < pCtlData->lItemCount; i++)
         free(pCtlData->pRecords[i].pchLine);
   }

   if (pCtlData->pRecords)
      free(pCtlData->pRecords);

   /* Scrollbar */
   if (pCtlData->hwndScroll)
      WinDestroyWindow(pCtlData->hwndScroll);

   /* Control-Daten */
   free (pCtlData);
#if 0
   _heapmin();
#endif

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
      int i;

      for (i=0; i<pCtlData->lItemCount; i++)
         if (pCtlData->pRecords[i].pchLine)
            free(pCtlData->pRecords[i].pchLine);

      free(pCtlData->pRecords);
      pCtlData->pRecords = NULL;
      pCtlData->lItemCount = 0;
      pCtlData->lItemAlloc = 0;
      pCtlData->lCrsItem = 0;
      pCtlData->lTopItem = 0;
   }

   /* Scrollbar neu berechnen */
   CalcScrollFactor(pCtlData);
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

static LONG AddItem(HWND hwnd, PLBCTLDATA pCtlData, PCHAR pchText)
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
   pCtlData->pRecords[pCtlData->lItemCount].pchLine = strdup(pchText);
   pCtlData->pRecords[pCtlData->lItemCount].ulFlags = 0;
   pCtlData->pRecords[pCtlData->lItemCount].pItemHandle = NULL;

   /* Zaehler hoch */
   pCtlData->lItemCount++;

   if (pCtlData->lItemCount == 1)
   {
      /* erstes Element eingefuegt */
      pCtlData->lCrsItem = 0;
   }

   /* Scrollbar anpassen */
   CalcScrollFactor(pCtlData);
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

static LONG AddArray(HWND hwnd, PLBCTLDATA pCtlData, PLBOXINFO pLBoxInfo, PCHAR *ppchLines)
{
   int i;

   if (!pLBoxInfo->ulItemCount ||
       ppchLines == NULL)
      return 0;  /* nix zu tun */

   if (pCtlData->lItemCount + pLBoxInfo->ulItemCount > pCtlData->lItemAlloc)
   {
      /* Zu wenig Speicher, neu anfordern */
      PMLISTRECORD pNewArray;
      ULONG ulNewSize;

      ulNewSize = (pCtlData->lItemAlloc + pLBoxInfo->ulItemCount);

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
   for (i=0; i < pLBoxInfo->ulItemCount; i++)
   {
#if 0
      pCtlData->pRecords[pCtlData->lItemCount+i].pchLine = strdup(ppchLines[i]);
#else
      pCtlData->pRecords[pCtlData->lItemCount+i].pchLine = ppchLines[i];
#endif
      pCtlData->pRecords[pCtlData->lItemCount+i].ulFlags = 0;
      pCtlData->pRecords[pCtlData->lItemCount+i].pItemHandle = NULL;
   }

   if (pCtlData->lItemCount == 0)
   {
      /* erstes Element eingefuegt */
      pCtlData->lCrsItem = 0;
   }

   /* Zaehler hoch */
   pCtlData->lItemCount += pLBoxInfo->ulItemCount;

   /* Scrollbar anpassen */
   CalcScrollFactor(pCtlData);
   CalcScrollBar(hwnd, pCtlData);

   /* neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return pLBoxInfo->ulItemCount;
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
   LONG sVisibleLines, sScrollerBottom, sTop, sTotal;

   sVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
                   pCtlData->lLineHeight;

   sScrollerBottom= pCtlData->lItemCount - sVisibleLines;
   sTop = pCtlData->lTopItem;
   sTotal = pCtlData->lItemCount;

   sScrollerBottom /= pCtlData->sScrollFactor;
   sTop /= pCtlData->sScrollFactor;
   sVisibleLines /= pCtlData->sScrollFactor;
   sTotal /= pCtlData->sScrollFactor;


   WinSendMsg(pCtlData->hwndScroll, SBM_SETSCROLLBAR,
                     MPFROMSHORT(sTop),
                     MPFROM2SHORT(0, sScrollerBottom));

   WinSendMsg(pCtlData->hwndScroll, SBM_SETTHUMBSIZE,
                     MPFROM2SHORT(sVisibleLines, sTotal),
                     NULL);

   /* Warnings vermeiden */
   hwnd = hwnd;

   return;
}

static void CalcScrollFactor(PLBCTLDATA pCtlData)
{
   LONG lTest;

   lTest = pCtlData->lItemCount;
   pCtlData->sScrollFactor = 1;

   while (lTest > SHRT_MAX)
   {
      pCtlData->sScrollFactor *= 2;
      lTest = pCtlData->lItemCount / pCtlData->sScrollFactor;
   }

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

   return fm.lMaxBaselineExt;
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
   LONG lBorderColor, lShadowColor, lBackColor,
        lForeColor, lHBackColor, lHForeColor;

   hps = WinBeginPaint(hwnd, NULLHANDLE, &updrectl);

   /* Farben */
   lBackColor = QueryColor(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, SYSCLR_WINDOW);
   lForeColor = QueryColor(hwnd, PP_FOREGROUNDCOLOR, PP_FOREGROUNDCOLORINDEX, SYSCLR_WINDOWTEXT);
   lHBackColor = QueryColor(hwnd, PP_HILITEBACKGROUNDCOLOR, PP_HILITEBACKGROUNDCOLORINDEX, SYSCLR_HILITEBACKGROUND);
   lHForeColor = QueryColor(hwnd, PP_HILITEFOREGROUNDCOLOR, PP_HILITEFOREGROUNDCOLORINDEX, SYSCLR_HILITEFOREGROUND);
   lBorderColor = QueryColor(hwnd, PP_BORDERCOLOR, PP_BORDERCOLORINDEX, SYSCLR_WINDOWFRAME);

   /* Rahmen-Farben */
   lShadowColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);

   /* Auf RGB schalten */
   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   WinFillRect(hps, &updrectl, lBackColor);

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

   GpiSetColor(hps, lForeColor);

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
      {
         if (pCtlData->pRecords[ulLine].ulFlags & LISTFLAG_SELECTED)
            WinDrawText(hps, -1, pCtlData->pRecords[ulLine].pchLine, &drawrcl,
                        lHForeColor, lHBackColor, DT_LEFT | DT_TOP | DT_ERASERECT);
         else
            WinDrawText(hps, -1, pCtlData->pRecords[ulLine].pchLine, &drawrcl,
                        lForeColor, lBackColor, DT_LEFT | DT_TOP);
      }
      /* n�chstes Item */
      ulLine++;
   }

   /* Ende der Vorstellung */
   WinEndPaint(hps);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: QueryColor
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fragt eine Systemfarbe ab
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle des Controls
 |            idAttr1, idAttr2: Primвer und sekundвer Presentation-Parameter
 |            lSysColor: System-Farbe
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: Ermittelte Farbe (RGB)
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Wenn die PP_* nicht vorhanden sind, wird die System-Farbe
 |            zur…kgegeben
 +---------------------------------------------------------------------------*/

static LONG QueryColor(HWND hwnd, ULONG idAttr1, ULONG idAttr2, LONG lSysColor)
{
   LONG lTemp=0;

   if (!WinQueryPresParam(hwnd,
                          idAttr1, idAttr2,
                          NULL,
                          sizeof(LONG),
                          (PVOID)&lTemp,
                          QPF_ID2COLORINDEX))
      lTemp = WinQuerySysColor(HWND_DESKTOP, lSysColor, 0);

   return lTemp;
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
/* R…kgabewerte: LIT_NONE: kein Item getroffen                              */
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
     return LIT_NONE;

   lItem = ((pCtlData->rclItems.yTop - y)/
            pCtlData->lLineHeight) + (LONG)pCtlData->lTopItem;

   if (lItem >= pCtlData->lItemCount || lItem < 0)
      return LIT_NONE;
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
   ULONG ulVisibleLines;

   if (lItem < 0 ||
       lItem >= pCtlData->lItemCount ||
       pCtlData->lItemCount == 0)
      return FALSE;

   ulVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom) /
                    pCtlData->lLineHeight;

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

   CalcScrollBar(hwnd, pCtlData);
   SetCursor(hwnd, pCtlData);

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

   /* eventuell hochscrollen */
   ulVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom) /
                    pCtlData->lLineHeight;
   if (pCtlData->lItemCount > ulVisibleLines)
      if (pCtlData->lTopItem + ulVisibleLines >= pCtlData->lItemCount)
         pCtlData->lTopItem -= pCtlData->lTopItem + ulVisibleLines -
                                pCtlData->lItemCount;

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
   ULONG ulVisibleLines;

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
         ulVisibleLines=(pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
                        pCtlData->lLineHeight+1;
         if (pCtlData->lTopItem < (pCtlData->lItemCount+1 - ulVisibleLines))
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
         ulVisibleLines=(pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
                        pCtlData->lLineHeight+1;
         if (pCtlData->lTopItem > 0)
         {
            if (pCtlData->lTopItem >= ulVisibleLines)
               pCtlData->lTopItem-=ulVisibleLines;
            else
               pCtlData->lTopItem = 0;
            WinScrollWindow(hwnd, 0, -(pCtlData->lLineHeight * ulVisibleLines),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_PAGEDOWN:
         ulVisibleLines=(pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
                        pCtlData->lLineHeight+1;
         if (pCtlData->lTopItem < (pCtlData->lItemCount+1 - ulVisibleLines))
         {
            ULONG ulLinesScrolled;

            ulLinesScrolled=(pCtlData->lItemCount+1 - ulVisibleLines )-
                            pCtlData->lTopItem;
            if (ulLinesScrolled > ulVisibleLines)
               ulLinesScrolled = ulVisibleLines;
            pCtlData->lTopItem+=ulLinesScrolled;

            WinScrollWindow(hwnd, 0, (pCtlData->lLineHeight * ulLinesScrolled),
                            &pCtlData->rclItems, &pCtlData->rclItems,
                            (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            CalcScrollBar(hwnd, pCtlData);
         }
         break;

      case SB_SLIDERTRACK:
         {
            LONG lSliderPos = sSliderPos * pCtlData->sScrollFactor;
            LONG lLinesScrolled=(lSliderPos - pCtlData->lTopItem);

            pCtlData->lTopItem=lSliderPos;
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
/* Funktionsname: QueryItemText                                              */
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

static BOOL QueryItemText(HWND hwnd, PLBCTLDATA pCtlData, LONG lItem, PCHAR pchDest)
{
   if (lItem < 0 ||
       lItem >= pCtlData->lItemCount)
      return FALSE;

   if (!pchDest)
      return FALSE;

   /* Daten kopieren */
   strncpy(pchDest, pCtlData->pRecords[lItem].pchLine, 50);

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
   LONG  lVisibleLines;
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

            pCtlData->lCrsItem++;
            WinShowCursor(hwnd, FALSE);
            ShiftIntoView(hwnd, pCtlData, pCtlData->lCrsItem);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);

            return TRUE;

         case VK_UP:
            if (pCtlData->lCrsItem == 0)
               return TRUE; /* Anfang der Liste */

            pCtlData->lCrsItem--;
            WinShowCursor(hwnd, FALSE);
            ShiftIntoView(hwnd, pCtlData, pCtlData->lCrsItem);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);
            return TRUE;

         case VK_HOME:
            if (pCtlData->lCrsItem == 0)
               return TRUE; /* Anfang der Liste */

            pCtlData->lCrsItem = 0;
            ShiftIntoView(hwnd, pCtlData, 0);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);
            return TRUE;

         case VK_END:
            if (pCtlData->lItemCount == 0 ||
                pCtlData->lCrsItem >= (pCtlData->lItemCount-1))
               return TRUE; /* Ende der Liste */

            pCtlData->lCrsItem = pCtlData->lItemCount-1;
            ScrollToItem(hwnd, pCtlData, pCtlData->lItemCount-1);
            CalcScrollBar(hwnd, pCtlData);
            SetCursor(hwnd, pCtlData);
            return TRUE;

         case VK_PAGEDOWN:
            if (pCtlData->lItemCount == 0 ||
                pCtlData->lCrsItem >= (pCtlData->lItemCount-1))
               return TRUE;

            lVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom)/
                             pCtlData->lLineHeight;

            if (((LONG)pCtlData->lTopItem + lVisibleLines) >= pCtlData->lItemCount)
            {
               pCtlData->lCrsItem = pCtlData->lItemCount-1;
               ScrollToItem(hwnd, pCtlData, pCtlData->lCrsItem);
               SetCursor(hwnd, pCtlData);
            }
            else
            {
               ulNewTop = pCtlData->lTopItem + lVisibleLines;
               pCtlData->lCrsItem = ulNewTop;
               ScrollToItem(hwnd, pCtlData, ulNewTop);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

         case VK_PAGEUP:
            if (pCtlData->lCrsItem == 0)
               return TRUE;

            lVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom) /
                             pCtlData->lLineHeight;

            if (((LONG)pCtlData->lTopItem - lVisibleLines) < 0)
            {
               ulNewTop = 0;
               pCtlData->lCrsItem = ulNewTop;
               ScrollToItem(hwnd, pCtlData, ulNewTop);
               SetCursor(hwnd, pCtlData);
            }
            else
            {
               ulNewTop = pCtlData->lTopItem - lVisibleLines;
               pCtlData->lCrsItem = ulNewTop;
               pCtlData->lTopItem = ulNewTop;
               WinShowCursor(hwnd, FALSE);
               CalcScrollBar(hwnd, pCtlData);
               WinInvalidateRect(hwnd, NULL, FALSE);
               SetCursor(hwnd, pCtlData);
            }
            return TRUE;

#if 0
         case VK_NEWLINE:
         case VK_ENTER:
            /* Notification senden */
            if (pCtlData->lCrsItem < pCtlData->lItemCount)
               Notify(hwnd, LN_ENTER, (MPARAM) hwnd);
            return TRUE;
#endif

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

         case VK_SPACE:
            if (pCtlData->lItemCount)
            {
               RECTL rec;

               /* Select toggeln */
               pCtlData->pRecords[pCtlData->lCrsItem].ulFlags ^= LISTFLAG_SELECTED;
               CalcItemRect(hwnd, pCtlData, pCtlData->lCrsItem, &rec);
               WinInvalidateRect(hwnd, &rec, TRUE);

               /* Cursor eine Zeile nach unten setzen */
               if (pCtlData->lCrsItem < (pCtlData->lItemCount-1))
               {
                  pCtlData->lCrsItem++;
                  WinShowCursor(hwnd, FALSE);
                  ShiftIntoView(hwnd, pCtlData, pCtlData->lCrsItem);
                  CalcScrollBar(hwnd, pCtlData);
                  SetCursor(hwnd, pCtlData);
               }
            }
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
               return TRUE;
            }
            else
               return FALSE;

         case '\\':
            if (usFlags & KC_CTRL)
            {
               DeselectAll(hwnd, pCtlData);
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
   if (!(pCtlData->pRecords[lItem].ulFlags & LISTFLAG_SELECTED))
   {
      RECTL rec;

      pCtlData->pRecords[lItem].ulFlags |= LISTFLAG_SELECTED;
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
   if (pCtlData->pRecords[lItem].ulFlags & LISTFLAG_SELECTED)
   {
      RECTL rec;

      pCtlData->pRecords[lItem].ulFlags &= ~LISTFLAG_SELECTED;
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

static void ClickItem(HWND hwnd, PLBCTLDATA pCtlData, LONG x, LONG y)
{
   POINTL pt;

   pt.x = x;
   pt.y = y;

   if (WinPtInRect(WinQueryAnchorBlock(hwnd), &pCtlData->rclItems, &pt))
   {
      LONG lItem;

      if ((lItem = ItemFromY(hwnd, pCtlData, pt.y - pCtlData->rclItems.yBottom))!=LIT_NONE)
      {
         RECTL rec;

         pCtlData->lCrsItem = lItem;

         pCtlData->pRecords[pCtlData->lCrsItem].ulFlags ^= LISTFLAG_SELECTED;
         CalcItemRect(hwnd, pCtlData, lItem, &rec);
         WinInvalidateRect(hwnd, &rec, TRUE);
         SetCursor(hwnd, pCtlData);
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
   LONG lVisibleLines;
   LONG lLinesToScroll;

   CalcItemRect(hwnd, pCtlData, lItem, &rec);

   if (rec.yBottom < pCtlData->rclItems.yBottom) /* nach unten */
   {
      lVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom) /
                       pCtlData->lLineHeight;
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
   BOOL bRedraw = FALSE;

   for (i = 0; i < pCtlData->lItemCount; i++)
      if (pCtlData->pRecords[i].ulFlags & LISTFLAG_SELECTED)
      {
         pCtlData->pRecords[i].ulFlags &= ~LISTFLAG_SELECTED;

         bRedraw = TRUE;
      }

   if (bRedraw)
      WinInvalidateRect(hwnd, NULL, TRUE);

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
   BOOL bRedraw = FALSE;

   for (i = 0; i < pCtlData->lItemCount; i++)
      if (!(pCtlData->pRecords[i].ulFlags & LISTFLAG_SELECTED))
      {
         pCtlData->pRecords[i].ulFlags |= LISTFLAG_SELECTED;

         bRedraw = TRUE;
      }

   if (bRedraw)
      WinInvalidateRect(hwnd, NULL, TRUE);

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

   if (lItem != LIT_NONE)
   {
      /* Klick auf Item */

      if (lItem != pCtlData->lPrevItem)
      {
         /* Item hat gewechselt */
         LONG lChaItem = pCtlData->lPrevItem;

         while (lChaItem != lItem)
         {
            if (lChaItem > lItem)
               lChaItem--;
            else
               if (lChaItem < lItem)
                  lChaItem++;

            if (pCtlData->bSelect)
               SelectItem(hwnd, pCtlData, lChaItem);
            else
               DeSelectItem(hwnd, pCtlData, lChaItem);
         }

         pCtlData->lCrsItem = lItem;
         SetCursor(hwnd, pCtlData);

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

static LONG QuerySelection(HWND hwnd, PLBCTLDATA pCtlData, LONG lStart)
{
   int i;

   hwnd = hwnd;

   if (!pCtlData->lItemCount)
      return LIT_NONE;

   if (lStart == LIT_CURSOR)
      return pCtlData->lCrsItem;


   if (lStart == LIT_FIRST)
      lStart = -1;

   for (i=lStart+1; i < pCtlData->lItemCount; i++)
      if (pCtlData->pRecords[i].ulFlags & LISTFLAG_SELECTED)
         return i;

   return LIT_NONE;
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
      if (!pCtlData->bSelect)
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
   LONG lVisibleLines;
   lVisibleLines = (pCtlData->rclItems.yTop - pCtlData->rclItems.yBottom) /
                   pCtlData->lLineHeight;

   while (pCtlData->lCrsItem < pCtlData->lTopItem + lVisibleLines)
   {
      if (pCtlData->bSelect)
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

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}

static LONG SearchString(PLBCTLDATA pCtlData, LONG lStart, USHORT usOptions, PCHAR pchText)
{
   LONG i;
   char *pchFound;

   if (lStart == LIT_FIRST)
      lStart = -1;

   if (lStart >= pCtlData->lItemCount ||
       !pchText ||
       !pchText[0])
      return LIT_ERROR;

   for (i=lStart+1; i < pCtlData->lItemCount; i++) /* Zeilen durchsuchen */
   {
      if (usOptions & LSS_CASESENSITIVE)
         pchFound = strstr(pCtlData->pRecords[i].pchLine, pchText);
      else
         pchFound = stristr(pCtlData->pRecords[i].pchLine, pchText);

      if (pchFound)
         if (usOptions & LSS_PREFIX)
         {
            if (pchFound == pCtlData->pRecords[i].pchLine)
               return i;
         }
         else
            return i;
   }

   return LIT_NONE;
}


