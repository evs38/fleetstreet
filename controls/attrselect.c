/*---------------------------------------------------------------------------+
 | Titel: ATTRSELECT.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 06.02.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Attribut-Selektion                                                     |
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

#include "attrselect.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

typedef struct
{
   HWND hwndValueSet;
   ULONG ulAttrib;
   ULONG ulAttribMask;
} ATTRIBSELECTDATA, *PATTRIBSELECTDATA;

/*---------------------------- Globale Variablen ----------------------------*/

static char *AttribText[32]=
{
   "prv", "cra", "rcvd", "sent", "f/a", "tran", "orph", "k/s",
   "loc", "hold", "read", "freq", "rrq", "rcpt", "aud", "upd",
   "scn", "arc/s", "dir", "trf", "kif", "imm", "gat", "fpu",
   "hub", "keep", "npd", "del", "", "", "", ""
};

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY AttribSelectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static BOOL CreateWindow(HWND hwnd, PCREATESTRUCT pCreate);
static void DestroyWindow(PATTRIBSELECTDATA pWinData);
static BOOL SetAttrib(HWND hwnd, PATTRIBSELECTDATA pWinData, ULONG ulAttrib, ULONG ulAttribMask);
static ULONG QueryAttrib(PATTRIBSELECTDATA pWinData);
static void AttribClicked(HWND hwnd, PATTRIBSELECTDATA pWinData, SHORT Row, SHORT Col);
static BOOL DrawItem(PATTRIBSELECTDATA pWinData, POWNERITEM pOwnerItem);

/*-----------------------------------------------------------------------------
 | Funktionsname: RegisterAttribSelect
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Registriert die Fensterklasse
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hab: Anchor-Block
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: TRUE:  Erfolg
 |                FALSE: Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

BOOL RegisterAttribSelect(HAB hab)
{
   return WinRegisterClass(hab, WC_ATTRIBSELECT,
                           AttribSelectProc, CS_SIZEREDRAW, sizeof(PVOID));
}

/*-----------------------------------------------------------------------------
 | Funktionsname: AttribSelectProc
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fensterprozedur des Controls
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: WINPROC
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: MRESULT
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AttribSelectProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PATTRIBSELECTDATA pWinData = WinQueryWindowPtr(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_CREATE:
         return (MRESULT) CreateWindow(hwnd, (PCREATESTRUCT) mp2);

      case WM_DESTROY:
         DestroyWindow(pWinData);
         break;

      case WM_HELP:
         /* Weiterleiten mit eigener ID */
         mp1 = MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID));
         break;

      case ATTSM_SETATTRIB:
         return (MRESULT) SetAttrib(hwnd, pWinData, (ULONG) mp1, (ULONG) mp2);

      case ATTSM_QUERYATTRIB:
         return (MRESULT) QueryAttrib(pWinData);

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == ATTSID_VALUE)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case VN_ENTER:
                  AttribClicked(hwnd, pWinData, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
                  break;

               case VN_HELP:
#if 0
                  WinPostMsg(WinQueryWindow(hwnd, QW_OWNER), WM_HELP,
                             MPFROMSHORT(WinQueryWindowUShort(hwnd, QWS_ID)),
                             mp2);
#endif
                  break;
            }
         }
         break;

      case WM_DRAWITEM:
         if (SHORT1FROMMP(mp1) == ATTSID_VALUE)
            return (MRESULT) DrawItem(pWinData, (POWNERITEM) mp2);
         else
            break;

      case WM_QUERYDLGCODE:
         return WinSendMsg(pWinData->hwndValueSet, msg, mp1, mp2);

      default:
         break;
   }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CreateWindow
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Initialisierung des Fensters
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pCreate: Erzeugungs-Parameter
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: FALSE: Weiter
 |                TRUE: Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static BOOL CreateWindow(HWND hwnd, PCREATESTRUCT pCreate)
{
   PATTRIBSELECTDATA pWinData;
   VSCDATA vscdata = {sizeof(vscdata), 4, 8};

   pWinData = calloc(1, sizeof(ATTRIBSELECTDATA));

   if (pWinData)
   {
      WinSetWindowPtr(hwnd, QWL_USER, pWinData);

      pWinData->hwndValueSet = WinCreateWindow(hwnd, WC_VALUESET, NULL,
                                 WS_VISIBLE |
                                 VS_TEXT | VS_BORDER | VS_ITEMBORDER,
                                 0, 0,
                                 pCreate->cx, pCreate->cy,
                                 hwnd, HWND_TOP,
                                 ATTSID_VALUE,
                                 &vscdata,
                                 NULL);

      if (pWinData->hwndValueSet)
      {
         int i;

         for (i=0; i<32; i++)
         {
            WinSendMsg(pWinData->hwndValueSet, VM_SETITEM,
                       MPFROM2SHORT(i/8+1, i%8+1),
                       AttribText[i]);
            WinSendMsg(pWinData->hwndValueSet, VM_SETITEMATTR,
                       MPFROM2SHORT(i/8+1, i%8+1),
                       MPFROM2SHORT(VIA_OWNERDRAW, TRUE));
         }
         return FALSE;
      }
      else
      {
         free(pWinData);
         return TRUE;
      }
   }
   else
      return TRUE;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: DestroyWindow
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: RÑumt das Fenster auf
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pWinData: Instanzdaten
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static void DestroyWindow(PATTRIBSELECTDATA pWinData)
{
   free(pWinData);
   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: SetAttrib
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Setzt Attribute und Maske
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pWinData: Instanzdaten
 |            ulAttrib: Neue Attribute
 |            ulAttribMask: Neue Maske
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: TRUE: Erfolg
 |                FALSE: Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static BOOL SetAttrib(HWND hwnd, PATTRIBSELECTDATA pWinData, ULONG ulAttrib, ULONG ulAttribMask)
{
   int i=0;
   ULONG ulMask = 1UL;

   pWinData->ulAttrib = ulAttrib;
   pWinData->ulAttribMask = ulAttribMask;

   for (i=0; i<32; i++, ulMask <<= 1)
   {
      WinSendMsg(pWinData->hwndValueSet, VM_SETITEMATTR,
                 MPFROM2SHORT(i/8+1, i%8+1),
                 MPFROM2SHORT(VIA_DISABLED, !(pWinData->ulAttribMask & ulMask)));
   }

   WinInvalidateRect(hwnd, NULL, TRUE);

   return TRUE;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: QueryAttrib
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fragt die momentanen Attribute ab
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pWinData: Instanzdaten
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: Attribute
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static ULONG QueryAttrib(PATTRIBSELECTDATA pWinData)
{
   return pWinData->ulAttrib;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: AttribClicked
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Schaltet ein Attribut um, nachdem der User daruf geklickt hat
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pWinData: Instanzdaten
 |            Row: Zeile
 |            Col: Spalte
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static void AttribClicked(HWND hwnd, PATTRIBSELECTDATA pWinData, SHORT Row, SHORT Col)
{
   ULONG ulMask;

   ulMask = (Row-1)*8 + (Col-1);
   ulMask = 1UL << ulMask;

   if (pWinData->ulAttribMask & ulMask)
   {
      pWinData->ulAttrib ^= ulMask;

      WinInvalidateRect(hwnd, NULL, TRUE);
   #if 0
      WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                 MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID),
                              ATTSN_ATTRIBCHANGED),
                 MPFROMLONG(pWinData->ulAttrib));
   #endif
   }
   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: DrawItem
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Zeichnet ein Attribut-Feld
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle
 |            pWinData: Instanzdaten
 |            pOwnerItem: Draw-Anweisungen
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: TRUE: Feld gezeichnet
 |                FALSE: Feld nicht gezeichnet, VS mu· selbst zeichnen
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static BOOL DrawItem(PATTRIBSELECTDATA pWinData, POWNERITEM pOwnerItem)
{
   if (pOwnerItem->idItem == VDA_ITEM)
   {
      int iItem;
      ULONG ulMask;

      iItem = (SHORT1FROMMP(pOwnerItem->hItem)-1) * 8+
              (SHORT2FROMMP(pOwnerItem->hItem)-1);
      ulMask = 1UL << iItem;

      if ((pWinData->ulAttrib & ulMask) &&
          (pWinData->ulAttribMask & ulMask))
      {
         /* gesetzt */
         WinDrawText(pOwnerItem->hps, -1, AttribText[iItem],
                     &pOwnerItem->rclItem, CLR_WHITE, CLR_RED,
                     DT_CENTER | DT_VCENTER | DT_ERASERECT);

         return TRUE;
      }
      else
         return FALSE;
   }
   else
      return FALSE;
}
/*-------------------------------- Modulende --------------------------------*/


