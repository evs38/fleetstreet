/*---------------------------------------------------------------------------+
 | Titel: FONTDISP.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 23.09.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x, 3.x PM                                                  |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Font Display Control                                                  |
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

#include <string.h>

#include "fontdisp.h"

/*--------------------------------- Defines ---------------------------------*/

#define FLAG_NOTIFYFONT      1UL
#define FLAG_NO_NOTIFYFONT   0UL

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY FontDisplayProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void PaintFontDisplay(HWND hwnd);
static LONG QueryColor(HWND hwnd, ULONG idAttr1, ULONG idAttr2, LONG lSysColor);
static MRESULT Notify(HWND hwnd, USHORT usNotifyCode, MPARAM mp2);
static ULONG QueryFont(HWND hwnd, char *pchBuffer, ULONG ulBufferSize);

/*-----------------------------------------------------------------------------
 | Funktionsname: RegisterFontDisplay
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Registriert die Fensterklasse
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hab: Anchor-Block
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: TRUE:  Erfolg
 |                FALSE: Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

BOOL RegisterFontDisplay(HAB hab)
{
   return WinRegisterClass(hab, WC_FONTDISPLAY,
                           FontDisplayProc, CS_SIZEREDRAW, sizeof(ULONG));
}

/*-----------------------------------------------------------------------------
 | Funktionsname: FontDisplayProc
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

static MRESULT EXPENTRY FontDisplayProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      /* System-Messages */
      case WM_CREATE:
         WinSetWindowULong(hwnd, QWL_USER, FLAG_NOTIFYFONT);
         return (MRESULT) FALSE;

      case WM_DESTROY:
         break;

      case WM_PRESPARAMCHANGED:
         WinInvalidateRect(hwnd, NULL, TRUE); /* neu zeichnen */
         if (WinQueryWindowULong(hwnd, QWL_USER) == FLAG_NOTIFYFONT &&
             (ULONG) mp1 == PP_FONTNAMESIZE)
            Notify(hwnd, FDN_FONTCHANGED, (MPARAM) hwnd);
         break;

      case WM_PAINT:
         PaintFontDisplay(hwnd);
         return (MRESULT) 0;

      case WM_OPEN:
         Notify(hwnd, FDN_OPEN, (MPARAM) hwnd);
         break;

      case WM_CONTEXTMENU:
         Notify(hwnd, FDN_CONTEXTMENU, mp1);
         break;

      case WM_SETWINDOWPARAMS:
         break;

      case WM_QUERYWINDOWPARAMS:
         break;

      case WM_QUERYDLGCODE:
         return (MRESULT) DLGC_STATIC;

      case WM_HELP:
         break;

      /* User-Messages */
      /* Font setzen, mp1: PCHAR pchFontNameSize */
      case FDM_SETFONT:
         WinSetWindowULong(hwnd, QWL_USER, FLAG_NO_NOTIFYFONT);
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, strlen((PCHAR) mp1)+1, mp1);
         WinSetWindowULong(hwnd, QWL_USER, FLAG_NOTIFYFONT);
         return (MRESULT) TRUE;

      /* Font abfragen, mp1: PCHAR pchFontNameSize; mp2: ULONG ulBufferSize */
      case FDM_QUERYFONT:
         return (MRESULT) QueryFont(hwnd, (PCHAR) mp1, (ULONG) mp2);

      default:
         break;
   }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*-----------------------------------------------------------------------------
 | Funktionsname: PaintFontDisplay
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Zeichnet das Control neu
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwnd: Window-Handle des Controls
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void PaintFontDisplay(HWND hwnd)
{
   ULONG ulStyle;
   HPS hps;
   RECTL WinRect, PaintRect;
   LONG lBackColor, lForeColor, lBorderColor;
   POINTL pointl;
   char pchText[FACESIZE+5]="";

   hps = WinBeginPaint(hwnd, NULLHANDLE, &PaintRect);

   WinQueryWindowRect(hwnd, &WinRect);

   /* Farben vorbereiten */
   lBackColor = QueryColor(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, SYSCLR_WINDOW);
   lForeColor = QueryColor(hwnd, PP_FOREGROUNDCOLOR, PP_FOREGROUNDCOLORINDEX, SYSCLR_WINDOWTEXT);
   lBorderColor = QueryColor(hwnd, PP_BORDERCOLOR, PP_BORDERCOLORINDEX, SYSCLR_WINDOWFRAME);

   /* Auf RGB-Farben umschalten */
   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   /* Hintergrund */
   WinFillRect(hps, &PaintRect, lBackColor);

   ulStyle = WinQueryWindowULong(hwnd, QWL_STYLE);

   /* Rahmen */
   if (!(ulStyle & FDS_NO_BORDER))
   {
      GpiSetColor(hps, lBorderColor);

      pointl.x = 0;
      pointl.y = 0;
      GpiSetCurrentPosition(hps, &pointl);

      pointl.x = WinRect.xRight-1;
      pointl.y = WinRect.yTop-1;
      GpiBox(hps, DRO_OUTLINE, &pointl, 0, 0);

      WinInflateRect(NULLHANDLE, &WinRect, -2, -2);
   }

   /* Text */
   if (!QueryFont(hwnd, pchText, sizeof(pchText)))
      WinDrawText(hps, -1, "<Default>", &WinRect, lForeColor, lBackColor, DT_VCENTER);
   else
      WinDrawText(hps, -1, pchText, &WinRect, lForeColor, lBackColor, DT_VCENTER);

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

static MRESULT Notify(HWND hwnd, USHORT usNotifyCode, MPARAM mp2)
{
   return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), usNotifyCode),
                     mp2);
}

static ULONG QueryFont(HWND hwnd, char *pchBuffer, ULONG ulBufferSize)
{
   return WinQueryPresParam(hwnd, PP_FONTNAMESIZE, 0, NULL, ulBufferSize, pchBuffer, QPF_NOINHERIT);
}
/*-------------------------------- Modulende --------------------------------*/

