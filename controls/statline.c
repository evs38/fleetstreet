/*---------------------------------------------------------------------------+
 | Titel: STATLINE.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 23.01.94                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Statusline-Control, s. STATLINE.TXT                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_DOSMISC
#define INCL_PM
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\dump\pmassert.h"
#include "statline.h"

/*--------------------------------- Defines ---------------------------------*/

/* #define FULLFUNC */

/*---------------------------------- Typen ----------------------------------*/

typedef struct _STLFIELDINFO2
{
   struct _STLFIELDINFO2 * next;
   struct _STLFIELDINFO2 * prev;
   STLFIELDINFO SLFieldInfo;  /* Field-Infos aus der Message */
   PCHAR pchText;             /* Feld-Text                   */
   ULONG ulBufferSize;        /* Alloziierter Puffer f. Text */
   LONG  lCurrentSize;        /* Aktuelle Feldbreite         */
   LONG  lCurrentPos;         /* Aktuelle Feldposition       */
   ULONG ulFieldID;           /* Feld-ID                     */
   LONG  lProgress;
} STLFIELDINFO2, *PSTLFIELDINFO2;

typedef struct _STLCTRLDATA
{
   ULONG ulFieldCount;        /* Anzahl der Felder           */
   PSTLFIELDINFO2 pSLFields;  /* Kette der Felder            */
   PSTLFIELDINFO2 pSLFieldsLast;  /* Ende der Kette der Felder */
   LONG    lBorderWidth;         /* Rahmenbreite                */
   ULONG   ulHighID;            /* Hoechste verwendete Feld-ID */
   HBITMAP hbmCheck;          /* Bitmap-Handle f. Check      */
   LONG    lProgressColor;
} STLCTRLDATA, *PSTLCTRLDATA;

/*---------------------------- Globale Variablen ----------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY StatusLineProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

static void PaintStatusline(HWND hwnd);
static void CreateStatusline(HWND hwnd, PSTLCTRLDATA pSLCtrlData);
static void RecalcFields(PSTLCTRLDATA pSLCtrlData, LONG cx);
static ULONG AddField(PSTLCTRLDATA pSLCtrlData, PSTLFIELDINFO pSLFieldInfo, ULONG ulAfter);
static BOOL RemoveField(PSTLCTRLDATA pSLCtrlData, ULONG ulField);
static PSTLFIELDINFO2 FindField(PSTLFIELDINFO2 pFields, ULONG ulFieldID);
static BOOL SetText(HWND hwnd, PSTLCTRLDATA pCtlData, PSTLFIELDINFO2 pField, PCHAR pchNewText);
static BOOL SetWindowParams(HWND hwnd, PWNDPARAMS pWndParams, PSTLCTRLDATA pSLCtrlData);
static ULONG FindFieldFromX(PSTLCTRLDATA pSLCtrlData, long x);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);
static BOOL SetFieldWidth(PSTLCTRLDATA pSLCtrlData, ULONG ulField, LONG lWidth);

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterStatusLineClass                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Registriert die Statusline-Klasse                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anchor-Block der registrierenden Applikation              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: FALSE Fehler                                               */
/*                TRUE  Erfolg                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL EXPENTRY RegisterStatusLineClass(HAB hab)
{
   if (!WinRegisterClass(hab,
                         WC_STATUSLINE,
                         StatusLineProc,
                         CS_SYNCPAINT,
                         sizeof(PVOID)))  /* 1 Pointer */
      return FALSE;
   else
      return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: StatusLineProc                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure des Statusline-Controls                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (Window-Procedure)                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: siehe STATLINE.TXT                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY StatusLineProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PSTLCTRLDATA pSLCtrlData=(PSTLCTRLDATA) WinQueryWindowULong(hwnd, QWL_USER);
   PSTLFIELDINFO2 pFields=NULL;
   MRESULT mr;
   SWP swp;

   switch (message)
   {
      case WM_CREATE:
         /* Control-Daten */
         pSLCtrlData = calloc(1, sizeof(STLCTRLDATA));
         PMASSERT(pSLCtrlData != NULL, "Out of memory");
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pSLCtrlData);

         /* restliche Initialisierung */
         CreateStatusline(hwnd, pSLCtrlData);
         break;

      case WM_DESTROY:
         for (pFields=pSLCtrlData->pSLFields; pFields; pFields=pFields->next)
            if (pFields->pchText)
               free(pFields->pchText);
         if (pSLCtrlData->hbmCheck)
            GpiDeleteBitmap(pSLCtrlData->hbmCheck);
         free(pSLCtrlData);
         break;

      case WM_SIZE:
         RecalcFields(pSLCtrlData, SHORT1FROMMP(mp2));
         break;

      case WM_PAINT:
         PaintStatusline(hwnd);
         break;

      case WM_SETWINDOWPARAMS:
         mr=(MRESULT)SetWindowParams(hwnd, (PWNDPARAMS) mp1, pSLCtrlData);
         return mr;

      case WM_QUERYWINDOWPARAMS:
         break;

      case WM_PRESPARAMCHANGED:
         WinInvalidateRect(hwnd, NULL, TRUE);
         break;

      case WM_OPEN:
         if (!mp2)   /* nur v. Pointer-Event */
         {
            Notify(hwnd, STLN_OPEN, MPFROMLONG(FindFieldFromX(pSLCtrlData, SHORT1FROMMP(mp1))));
            return (MRESULT) TRUE;
         }
         break;

#ifdef FULLFUNC
      case WM_CONTEXTMENU:
         if (!mp2)   /* nur v. Pointer-Event */
         {
            Notify(hwnd, STLN_CONTEXTMENU, MPFROMLONG(FindFieldFromX(pSLCtrlData, SHORT1FROMMP(mp1))));
         }
         break;

      case WM_BEGINDRAG:
         if (!mp2)   /* nur v. Pointer-Event */
         {
            Notify(hwnd, STLN_BEGINDRAG, MPFROMLONG(FindFieldFromX(pSLCtrlData, SHORT1FROMMP(mp1))));
         }
         break;

      case WM_ENDDRAG:
         if (!mp2)   /* nur v. Pointer-Event */
         {
            Notify(hwnd, STLN_ENDDRAG, MPFROMLONG(FindFieldFromX(pSLCtrlData, SHORT1FROMMP(mp1))));
         }
         break;

      case WM_TEXTEDIT:
         if (!mp2)   /* nur v. Pointer-Event */
         {
            Notify(hwnd, STLN_TEXTEDIT, MPFROMLONG(FindFieldFromX(pSLCtrlData, SHORT1FROMMP(mp1))));
         }
         break;
#endif

      case WM_TIMER:
         if (SHORT1FROMMP(mp1) == TID_MSGTIMEOUT)
         {
            /* Timer stoppen */
            WinStopTimer(WinQueryAnchorBlock(hwnd) , hwnd, TID_MSGTIMEOUT);

            /* Alle Felder mit Timeout loeschen */
            for (pFields=pSLCtrlData->pSLFields; pFields; pFields=pFields->next)
               if (pFields->SLFieldInfo.ulTimeout)
                  WinSendMsg(hwnd, STLM_SETFIELDTEXT, MPFROMLONG(pFields->ulFieldID), "");
         }
         break;

#ifdef FULLFUNC
      case DM_DRAGOVER:
         {
            STLDRAGNOTIFY STLDragNotify;
            POINTL pointl;

            pointl.x= SHORT1FROMMP(mp2);
            pointl.y= SHORT2FROMMP(mp2);

            WinMapWindowPoints(HWND_DESKTOP, hwnd, &pointl, 1);

            STLDragNotify.pDragInfo= (PDRAGINFO) mp1;
            STLDragNotify.ulFieldID= FindFieldFromX(pSLCtrlData, pointl.x);

            Notify(hwnd, STLN_DRAGOVER, &STLDragNotify);
         }
         break;

      case DM_DRAGLEAVE:
         {
            STLDRAGNOTIFY STLDragNotify;

            STLDragNotify.pDragInfo= (PDRAGINFO) mp1;
            STLDragNotify.ulFieldID= 0;

            Notify(hwnd, STLN_DRAGLEAVE, &STLDragNotify);
         }
         break;

      case DM_DROP:
         {
            STLDRAGNOTIFY STLDragNotify;

            STLDragNotify.pDragInfo= (PDRAGINFO) mp1;
            STLDragNotify.ulFieldID= 0;

            Notify(hwnd, STLN_DROP, &STLDragNotify);
         }
         break;

      case DM_DROPHELP:
         {
            STLDRAGNOTIFY STLDragNotify;

            STLDragNotify.pDragInfo= (PDRAGINFO) mp1;
            STLDragNotify.ulFieldID= 0;

            Notify(hwnd, STLN_DROPHELP, &STLDragNotify);
         }
         break;

      case STLM_SETBORDERSIZE:
         pSLCtrlData->lBorderWidth=(LONG) mp1;
         WinInvalidateRect(hwnd, NULL, TRUE);
         return (MRESULT) TRUE;

      case STLM_QUERYBORDERSIZE:
         return (MRESULT) pSLCtrlData->lBorderWidth;

      case STLM_DRAWBORDERLINE:
         break;
#endif

      case STLM_ADDFIELD:  /* mp1: PSLFIELDINFO, mp2 ULONG */
         mr=(MRESULT) AddField(pSLCtrlData, (PSTLFIELDINFO) mp1, (ULONG) mp2);
         WinQueryWindowPos(hwnd, &swp);
         RecalcFields(pSLCtrlData, swp.cx);
         WinInvalidateRect(hwnd, NULL, TRUE);
         return mr;

      case STLM_REMOVEFIELD:
         mr=(MRESULT) RemoveField(pSLCtrlData, (ULONG) mp1);
         WinQueryWindowPos(hwnd, &swp);
         RecalcFields(pSLCtrlData, swp.cx);
         WinInvalidateRect(hwnd, NULL, TRUE);
         break;

#ifdef FULLFUNC
      case STLM_QUERYFIELDCOUNT:
         return (MPARAM) pSLCtrlData->ulFieldCount;

      case STLM_SETFIELDCOLOR:
         pFields=FindField(pSLCtrlData->pSLFields, (ULONG) mp1);
         if (pFields)
         {
            pFields->SLFieldInfo.lColorForeground=(LONG) mp2;
            WinInvalidateRect(hwnd, NULL, TRUE);
            return (MRESULT) TRUE;
         }
         else
            return (MRESULT) FALSE;

      case STLM_QUERYFIELDCOLOR:
         pFields=FindField(pSLCtrlData->pSLFields, (ULONG) mp1);
         if (pFields)
            mr=(MRESULT)pFields->SLFieldInfo.lColorForeground;
         else
            mr=(MRESULT)-1;
         return mr;

      case STLM_SETFIELDFONT:
         WinInvalidateRect(hwnd, NULL, TRUE);
         break;

      case STLM_QUERYFIELDFONT:
         break;
#endif

      /* Feld-Text neu setzen, mp1: FieldID, mp2: Field-Text */
      case STLM_SETFIELDTEXT:
         pFields=FindField(pSLCtrlData->pSLFields, (ULONG) mp1);
         if (pFields)
         {
            if (SetText(hwnd, pSLCtrlData, pFields, (PCHAR) mp2))
               return (MRESULT) TRUE;
            else
               return (MRESULT) FALSE;
         }
         else
            return (MRESULT) FALSE;

#ifdef FULLFUNC
      /* Feld-Text abfragen, mp1: Field-ID, mp2: Field-Text */
      case STLM_QUERYFIELDTEXT:
         pFields=FindField(pSLCtrlData->pSLFields, (ULONG) mp1);
         if (pFields)
         {
            if (pFields->pchText)
               strcpy((PCHAR) mp2, pFields->pchText);
            else
               *((PCHAR)mp2)='\0';
            return (MRESULT) mp2;
         }
         else
            return (MRESULT) FALSE;

      case STLM_QUERYFIELDDATA:
         break;
#endif

      case STLM_SETFIELDWIDTH:
         mr = (MRESULT) SetFieldWidth(pSLCtrlData, (ULONG) mp1, (LONG) mp2);
         WinQueryWindowPos(hwnd, &swp);
         RecalcFields(pSLCtrlData, swp.cx);
         WinInvalidateRect(hwnd, NULL, TRUE);
         return mr;

      case STLM_SETTIMEOUT:
         pFields=FindField(pSLCtrlData->pSLFields, (ULONG) mp1);
         if (pFields)
         {
            pFields->SLFieldInfo.ulTimeout = (ULONG) mp2;
            return (MRESULT) TRUE;
         }
         else
            return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefWindowProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PaintStatusline                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Paint-Funktion f〉 das Statusline-Control                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle des Controls                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void PaintStatusline(HWND hwnd)
{
   RECTL rectl, rectl2;
   POINTL pointl, pointl2;
   HPS hps;
   LONG lx=0;
   LONG lBackColor, lForeColor;
   LONG lBorderColor;
   LONG lNWColor;
   LONG lSEColor;
   PSTLCTRLDATA pSLCtrlData=(PSTLCTRLDATA) WinQueryWindowULong(hwnd, QWL_USER);
   PSTLFIELDINFO2 pFields=NULL;

   hps=WinBeginPaint(hwnd, NULLHANDLE, &rectl);

   WinQueryWindowRect(hwnd, &rectl);

   /* Farben vorbereiten */
   WinQueryPresParam(hwnd,
                     PP_BACKGROUNDCOLOR,
                     PP_BACKGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBackColor,
                     QPF_ID2COLORINDEX);

   WinQueryPresParam(hwnd,
                     PP_FOREGROUNDCOLOR,
                     PP_FOREGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lForeColor,
                     QPF_ID2COLORINDEX);

   WinQueryPresParam(hwnd,
                     PP_BORDERCOLOR,
                     PP_BORDERCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lBorderColor,
                     QPF_ID2COLORINDEX);

   WinQueryPresParam(hwnd,
                     PP_USER,
                     0,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lNWColor,
                     0);

   WinQueryPresParam(hwnd,
                     PP_USER+1,
                     0,
                     NULL,
                     sizeof(LONG),
                     (PVOID)&lSEColor,
                     0);

   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   /* Hintergrund loeschen */
   WinFillRect(hps, &rectl, lBackColor);

   if (WinQueryWindowULong(hwnd, QWL_STYLE) & STLS_BORDER)
   {
      /* Rahmen zeichnen */
      GpiSetColor(hps, lBorderColor);
      pointl.x=rectl.xLeft;
      pointl.y=rectl.yBottom;
      GpiMove(hps, &pointl);

      pointl.x=rectl.xRight-1;
      pointl.y=rectl.yTop-1;
      GpiBox(hps, DRO_OUTLINE, &pointl, 0, 0);
   }

   /* Alle Felder zeichnen */
   for (lx=0, pFields=pSLCtrlData->pSLFields; pFields; pFields=pFields->next)
   {
      if (pFields->SLFieldInfo.ulFlags & STLF_3D)
      {
         /* Rahmen um Feld zeichnen */
         GpiSetColor(hps, lNWColor);

         pointl.x=rectl.xLeft+pSLCtrlData->lBorderWidth+lx;
         pointl.y=rectl.yBottom+pSLCtrlData->lBorderWidth;
         GpiMove(hps, &pointl);
         pointl2.x=rectl.xLeft+pSLCtrlData->lBorderWidth+lx;
         pointl2.y=rectl.yTop-1-pSLCtrlData->lBorderWidth;
         if (pointl2.y < pointl.y)
            pointl2.y=pointl.y;
         GpiLine(hps, &pointl2);
         pointl2.x=rectl.xLeft+lx+pFields->lCurrentSize-pSLCtrlData->lBorderWidth;
         if (pointl2.x < pointl.x)
            pointl2.x=pointl.x;
         if (pointl2.x > rectl.xRight-pSLCtrlData->lBorderWidth)
            pointl2.x = rectl.xRight-pSLCtrlData->lBorderWidth;
         GpiLine(hps, &pointl2);

         GpiSetColor(hps, lSEColor);

         pointl.x=rectl.xLeft+pSLCtrlData->lBorderWidth+lx;
         pointl.y=rectl.yBottom+pSLCtrlData->lBorderWidth;
         GpiMove(hps, &pointl);
         pointl2.x=rectl.xLeft+lx+pFields->lCurrentSize-pSLCtrlData->lBorderWidth;
         pointl2.y=rectl.yBottom+pSLCtrlData->lBorderWidth;
         if (pointl2.x < pointl.x)
            pointl2.x=pointl.x;
         if (pointl2.x > rectl.xRight-pSLCtrlData->lBorderWidth)
            pointl2.x = rectl.xRight-pSLCtrlData->lBorderWidth;
         GpiLine(hps, &pointl2);
         pointl.y=rectl.yTop-1-pSLCtrlData->lBorderWidth;
         if (pointl2.y < pointl.y)
            pointl2.y=pointl.y;
         GpiLine(hps, &pointl2);
      }

      if (pFields->SLFieldInfo.ulFlags & STLF_CHECK)
      {
         /* Checkmark anzeigen */
         if (pFields->lProgress)
         {
            HBITMAP hbm=pSLCtrlData->hbmCheck;
            POINTL ptl={0,0};
            BITMAPINFOHEADER BmpHeader={sizeof(BITMAPINFOHEADER)};

            if (hbm)
            {
               if (GpiQueryBitmapParameters(hbm, &BmpHeader))
               {
                  ptl.x = rectl.xLeft+pSLCtrlData->lBorderWidth+lx+2+
                          (pFields->lCurrentSize-pSLCtrlData->lBorderWidth-6)/2-
                          BmpHeader.cx/2;
                  ptl.y = rectl.yBottom+pSLCtrlData->lBorderWidth+2+
                          (rectl.yTop-1-pSLCtrlData->lBorderWidth-6)/2-
                          BmpHeader.cx/2;

                  WinDrawBitmap(hps, hbm, NULL, &ptl,
                                pFields->SLFieldInfo.lColorForeground,
                                lBackColor,
                                DBM_NORMAL);
               }
            }
         }
      }
      else
         if (pFields->SLFieldInfo.ulFlags & STLF_PROGRESS)
         {
            LONG lPixels;

            rectl2.xLeft=rectl.xLeft+pSLCtrlData->lBorderWidth+lx+3;
            rectl2.yBottom=rectl.yBottom+pSLCtrlData->lBorderWidth+3;
            rectl2.xRight=rectl.xLeft+lx+pFields->lCurrentSize-pSLCtrlData->lBorderWidth-3;
            rectl2.yTop=rectl.yTop-1-pSLCtrlData->lBorderWidth-3;

            lPixels = ((rectl2.xRight - rectl2.xLeft) * pFields->lProgress) / 100;
            if (lPixels)
            {
               char pchTemp[50];
               LONG lRight;

               lRight = rectl2.xRight;
               rectl2.xRight = rectl2.xLeft + lPixels;
               WinFillRect(hps, &rectl2, pSLCtrlData->lProgressColor);

               rectl2.xRight = lRight;
               sprintf(pchTemp, "%d%%", pFields->lProgress);
               WinDrawText(hps, -1, pchTemp, &rectl2, pFields->SLFieldInfo.lColorForeground, 0,
                           DT_CENTER | DT_VCENTER);
            }
         }
         else
            /* Text zeichnen */
            if (pFields->pchText)
            {
               GpiSetColor(hps, pFields->SLFieldInfo.lColorForeground);

               rectl2.xLeft=rectl.xLeft+pSLCtrlData->lBorderWidth+lx+3;
               rectl2.yBottom=rectl.yBottom+pSLCtrlData->lBorderWidth+3;
               rectl2.xRight=rectl.xLeft+lx+pFields->lCurrentSize-pSLCtrlData->lBorderWidth-3;
               rectl2.yTop=rectl.yTop-1-pSLCtrlData->lBorderWidth-3;

               if (rectl2.xRight < rectl2.xLeft)
                  rectl2.xRight = rectl2.xLeft;
               if (rectl2.yTop < rectl2.yBottom)
                  rectl2.yTop = rectl2.yBottom;
               if (rectl2.xRight > rectl.xRight-pSLCtrlData->lBorderWidth)
                  rectl2.xRight = rectl.xRight-pSLCtrlData->lBorderWidth;

               switch(pFields->SLFieldInfo.ulFlags & STLF_ALIGN_MASK)
               {
                  case STLF_CENTER:
                     WinDrawText(hps, -1, pFields->pchText, &rectl2, 0, 0,
                                 DT_CENTER | DT_VCENTER | DT_TEXTATTRS);
                     break;

                  case STLF_RIGHT:
                     WinDrawText(hps, -1, pFields->pchText, &rectl2, 0, 0,
                                 DT_RIGHT | DT_VCENTER | DT_TEXTATTRS);
                     break;

                  case STLF_LEFT:
                  default:
                     WinDrawText(hps, -1, pFields->pchText, &rectl2, 0, 0,
                                 DT_LEFT | DT_VCENTER | DT_TEXTATTRS);
                     break;
               }
            }

      lx+=pFields->lCurrentSize;
   }

   WinEndPaint(hps);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateStatusline                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Initialisierungsfunktion f. Statusline-Control              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle des Controls                               */
/*            pSLCtrlData: Zeiger auf Instanzdaten                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CreateStatusline(HWND hwnd, PSTLCTRLDATA pSLCtrlData)
{
   LONG lColor;
   HPS hps;

   /* Presentation-Parameter vorbereiten */

   /* Hintergrund grau */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
   WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &lColor);

   /* Schrift schwarz */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0);
   WinSetPresParam(hwnd, PP_FOREGROUNDCOLOR, sizeof(LONG), &lColor);

   /* Rahmen */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWFRAME, 0);
   WinSetPresParam(hwnd, PP_BORDERCOLOR, sizeof(LONG), &lColor);

   /* Schatten links oben */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);
   WinSetPresParam(hwnd, PP_USER, sizeof(LONG), &lColor);

   /* Schatten rechts unten */
   lColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);
   WinSetPresParam(hwnd, PP_USER+1, sizeof(LONG), &lColor);

   /* Instanzdaten */
   pSLCtrlData->lBorderWidth=4;
   pSLCtrlData->ulHighID=2;
   pSLCtrlData->lProgressColor=RGB_GREEN;


   hps=WinGetPS(hwnd);
   pSLCtrlData->hbmCheck=GpiLoadBitmap(hps, NULLHANDLE, 12, 0, 0);
   WinReleasePS(hps);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RecalcFields                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Berechnet die Feldbreiten neu                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle des Controls                               */
/*            pSLCtrlData: Zeiger auf Instanzdaten                           */
/*            cx: neue Fensterbreite                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void RecalcFields(PSTLCTRLDATA pSLCtrlData, LONG cx)
{
   PSTLFIELDINFO2 pFields;
   ULONG ulFixedSum=0;
   LONG lRest=0;
   LONG lPos=0;

   /* Gesamtlaenge der fixen Felder berechnen */
   for (pFields=pSLCtrlData->pSLFields; pFields; pFields=pFields->next)
      if (!(pFields->SLFieldInfo.ulFlags & STLF_VARIABLE))
         ulFixedSum+=pFields->SLFieldInfo.lFieldSize;

   lRest=cx-ulFixedSum;

   if (lRest < 0)
      lRest=0;

   /* Laenge der variablen Felder berechnen */
   for (pFields=pSLCtrlData->pSLFields; pFields; pFields=pFields->next)
   {
      if (!(pFields->SLFieldInfo.ulFlags & STLF_VARIABLE))
         pFields->lCurrentSize=pFields->SLFieldInfo.lFieldSize;
      else
         pFields->lCurrentSize=lRest*pFields->SLFieldInfo.lFieldSize/100;
      pFields->lCurrentPos=lPos;
      lPos+=pFields->lCurrentSize;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddField                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: F“t ein neues Feld hinzu                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle des Controls                               */
/*            pSLCtrlData: Zeiger auf Instanzdaten                           */
/*            pSLFieldInfo: Feld-Info                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Erfolg                                                */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG AddField(PSTLCTRLDATA pSLCtrlData, PSTLFIELDINFO pSLFieldInfo, ULONG ulAfter)
{
   PSTLFIELDINFO2 pField;
   ULONG ulFieldID;
   PSTLFIELDINFO2 pNewField;

   switch (ulAfter)
   {
      case STLI_FIRST:
         /* Am Anfang einfuegen */
         if (pSLCtrlData->pSLFields)
         {
            pNewField = calloc(1, sizeof(STLFIELDINFO2));
            PMASSERT(pNewField != NULL, "Out of memory");
            pNewField->next=pSLCtrlData->pSLFields;
            pSLCtrlData->pSLFields->prev=pNewField;
            pSLCtrlData->pSLFields=pNewField;
         }
         else
            pNewField = pSLCtrlData->pSLFields = pSLCtrlData->pSLFieldsLast = calloc(1, sizeof(STLFIELDINFO2));
         break;

      case STLI_LAST:
         /* Am Ende einfuegen */
         if (pSLCtrlData->pSLFields)
         {
            pNewField = calloc(1, sizeof(STLFIELDINFO2));
            PMASSERT(pNewField != NULL, "Out of memory");
            pNewField->prev=pSLCtrlData->pSLFieldsLast;
            pSLCtrlData->pSLFieldsLast->next=pNewField;
            pSLCtrlData->pSLFieldsLast=pNewField;
         }
         else
            pNewField = pSLCtrlData->pSLFields = pSLCtrlData->pSLFieldsLast = calloc(1, sizeof(STLFIELDINFO2));
         break;

      default:
         pField = FindField(pSLCtrlData->pSLFields, ulAfter);
         if (pField)
         {
            pNewField = calloc(1, sizeof(STLFIELDINFO2));
            PMASSERT(pNewField != NULL, "Out of memory");
            pNewField->next = pField->next;
            if (pField->next)
               pField->next->prev = pNewField;
            pNewField->prev = pField;
            pField->next = pNewField;

            if (pSLCtrlData->pSLFieldsLast == pField)
               pSLCtrlData->pSLFieldsLast = pNewField;
         }
         else
            return FALSE;
         break;
   }

   memcpy(&pNewField->SLFieldInfo, pSLFieldInfo, sizeof(STLFIELDINFO));

   /* Field-ID erzeugen */
   DosQuerySysInfo(QSV_TIME_LOW, QSV_TIME_LOW, &ulFieldID, sizeof(ULONG));

   while (ulFieldID <= pSLCtrlData->ulHighID)
      ulFieldID++;

   pNewField->ulFieldID=ulFieldID;

   pSLCtrlData->ulFieldCount++;
   pSLCtrlData->ulHighID=ulFieldID;

   return ulFieldID;
}

static BOOL RemoveField(PSTLCTRLDATA pSLCtrlData, ULONG ulField)
{
   PSTLFIELDINFO2 pDelField = FindField(pSLCtrlData->pSLFields, ulField);

   if (pDelField)
   {
      if (pDelField->pchText)
         free(pDelField->pchText);
      if (pDelField->next)
         pDelField->next->prev = pDelField->prev;
      if (pDelField->prev)
         pDelField->prev->next = pDelField->next;
      if (pSLCtrlData->pSLFields == pDelField)
         pSLCtrlData->pSLFields = pDelField->next;
      if (pSLCtrlData->pSLFieldsLast == pDelField)
         pSLCtrlData->pSLFieldsLast = pDelField->prev;

      return TRUE;
   }
   else
      return FALSE;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: FindField                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Hilfsfunktion, sucht ein Feld nach der Feld-ID              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pFields: Kette der Felder                                      */
/*            ulFieldID: Gesuchte Feld-ID                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf Feld-Definition oder NULL bei Fehler            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PSTLFIELDINFO2 FindField(PSTLFIELDINFO2 pFields, ULONG ulFieldID)
{
   while(pFields && pFields->ulFieldID != ulFieldID)
      pFields=pFields->next;

   return pFields;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetText                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Feld-Text neu setzen                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pField: Zeiger auf Felddefinition                              */
/*            pchNewText: Neuer Feld-Text                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Erfolg                                                */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetText(HWND hwnd, PSTLCTRLDATA pCtlData, PSTLFIELDINFO2 pField, PCHAR pchNewText)
{
   RECTL rectl;

   if (pField->SLFieldInfo.ulFlags & (STLF_CHECK | STLF_PROGRESS))
   {
      pField->lProgress = (LONG) pchNewText;
   }
   else
      if (pchNewText)
      {
         /* Neuen Text setzen */
         ULONG ulLen=strlen(pchNewText)+1;

         if (pField->pchText)
         {
            if (pField->ulBufferSize >= ulLen)
               memcpy(pField->pchText, pchNewText, ulLen);
            else
            {
               /* Puffer zu klein */
               free(pField->pchText);
               pField->pchText = malloc(ulLen);
               PMASSERT(pField->pchText != NULL, "Out of memory");
               memcpy(pField->pchText, pchNewText, ulLen);
               pField->ulBufferSize=ulLen;
            }
         }
         else
         {
            /* vorher kein Text */
            pField->pchText = malloc(ulLen);
            PMASSERT(pField->pchText != NULL, "Out of memory");
            memcpy(pField->pchText, pchNewText, ulLen);
            pField->ulBufferSize=ulLen;
         }
      }
      else
      {
         /* Text loeschen */
         if (pField->pchText)
            free(pField->pchText);
         pField->ulBufferSize=0;
         pField->pchText=NULL;
      }

   WinQueryWindowRect(hwnd, &rectl);
   rectl.xLeft=pField->lCurrentPos;
   rectl.xRight=pField->lCurrentPos+pField->lCurrentSize;

   WinInflateRect(WinQueryAnchorBlock(hwnd), &rectl,
                  -(pCtlData->lBorderWidth+1),
                  -(pCtlData->lBorderWidth+1));

   /* Bei neuem Text evtl. Timer starten oder zur…ksetzen */
   if (pField->pchText && *(pField->pchText) && pField->SLFieldInfo.ulTimeout)
      WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_MSGTIMEOUT,
                    pField->SLFieldInfo.ulTimeout);

   WinInvalidateRect(hwnd, &rectl, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetWindowParams                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_SETWINDOWPARAMS                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Statusline                             */
/*            pWndParams:  Window-Params-Struktur                            */
/*            pSLCtrlData: Instanzdaten                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE   Erfolg                                              */
/*                FALSE  Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Es wird nur das Setzen des Parameters "Text" implementiert.    */
/*            Der Window-Text wird als Text des ersten Feldes angesehen.     */
/*---------------------------------------------------------------------------*/

static BOOL SetWindowParams(HWND hwnd, PWNDPARAMS pWndParams, PSTLCTRLDATA pSLCtrlData)
{
   switch(pWndParams->fsStatus)
   {
      case WPM_TEXT:
         if (pSLCtrlData->pSLFields)
            return SetText(hwnd, pSLCtrlData, pSLCtrlData->pSLFields, pWndParams->pszText);
         else
         return FALSE;

      default:
         return FALSE;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindFieldFromX                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht die passende Field-ID zu einer gegebenen X-           */
/*               Koordinate. Wird fuer Notification-Messages benoetigt.      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Statusline                             */
/*            pSLCtrlData: Instanzdaten                                      */
/*            x:           X-Koordindate relativ zu hwnd                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0      Feld nicht gefunden                                 */
/*                sonst  Field-ID                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static ULONG FindFieldFromX(PSTLCTRLDATA pSLCtrlData, long x)
{
   PSTLFIELDINFO2 pField;

   pField= pSLCtrlData->pSLFields;

   while(pField)
   {
      if (pField->lCurrentPos <= x &&
          (pField->lCurrentPos + pField->lCurrentSize) > x)
         break;

      pField= pField->next;
   }

   if (pField)
      return pField->ulFieldID;
   else
      return 0;
}

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd,QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}

static BOOL SetFieldWidth(PSTLCTRLDATA pSLCtrlData, ULONG ulField, LONG lWidth)
{
   PSTLFIELDINFO2 pField;

   pField = FindField(pSLCtrlData->pSLFields, ulField);

   if (pField)
   {
      pField->SLFieldInfo.lFieldSize = lWidth;
      return TRUE;
   }
   else
      return FALSE;
}

/*-------------------------------- Modulende --------------------------------*/

