/*---------------------------------------------------------------------------+
 | Titel: MSGVIEWER.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 21.02.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Message-Viewer-Control f. FleetStreet                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "..\main.h"
#include "..\resids.h"
#include "editwin.h"
#include "..\util\fltutil.h"
#include "..\dump\pmassert.h"
#include "viewer_int.h"
#include "util.h"
#include "reflow.h"
#include "msgviewer.h"

/*--------------------------------- Defines ---------------------------------*/

/* Drag-Definitionen, irgendwann vereinheitlichen und ins Owner-Window
   verlegen */

#define DRAGTYPE "FleetStreet Message," DRT_TEXT
#define DRAGRMF  "<DRM_FLEET,DRF_FLEETMSG>,<DRM_OS2FILE,DRF_OEMTEXT>,<DRM_PRINT,DRF_UNKNOWN>,<DRM_DISCARD,DRF_OEMTEXT>"

#define DRAGTYPE2 "FleetStreet Message"
#define DRAGRMF2  "<DRM_FLEET,DRF_FLEETMSG>"

/* Absolutwert */
#define ABS(x) (((x)<0)?-(x):(x))

#define TID_AUTOSCROLL  99
#define FONTID_BOLD     1L
#define FONTID_ITALIC   2L
#define DEFAULT_QUOTE_CHAR  '>'


/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static RENDERPAR RenderPar;

/*----------------------- interne Funktionsprototypen -----------------------*/
static MRESULT EXPENTRY MsgViewerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void PaintMsgViewer(HWND hwnd);
static BOOL CreateMsgViewer(HWND hwnd, PVOID pCtrlData, PCREATESTRUCT pCreateStruct);
static void CleanupMsgViewer(HWND hwnd);
static void SizeMsgViewer(HWND hwnd, SHORT cx, SHORT cy, PVIEWERPARAMS pViewerParams);
static BOOL SetWindowParams(HWND hwnd, PWNDPARAMS pWndParams);
static BOOL QueryWindowParams(HWND hwnd, PWNDPARAMS pWndParams);
static void SetScrollParms(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void ScrollMsgViewer(HWND hwnd, SHORT sSliderPos, USHORT usCommand);
static BOOL CharMsgViewer(HWND hwnd, USHORT usFlags, USHORT usVK);
static BOOL CopyToClip(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void SetColors(HWND hwnd, LONG lColorID, LONG lRGBColor);
static void QueryColors(HWND hwnd, LONG lColorID, PLONG plRGBColor);

static void CreateCursor(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void DestroyCursor(HWND hwnd);
static BOOL BeginSelect(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void MouseMove(HWND hwnd, PVIEWERPARAMS pViewerParams, SHORT y);

static void ViewerTimer(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void DragUp(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void DragDown(HWND hwnd, PVIEWERPARAMS pViewerParams);
static void QueryFontMetrics(HWND hwnd, PVIEWERPARAMS pViewerParams);
static LONG CreateBoldFont(HPS hps, PVIEWERPARAMS pViewerParams, PFONTMETRICS pFontMetrics);
static LONG CreateItalicFont(HPS hps, PVIEWERPARAMS pViewerParams, PFONTMETRICS pFontMetrics);
static LONG CreateLogFont(HPS hps, PVIEWERPARAMS pViewerParams);

static LONG FindText(PVIEWERPARAMS pViewerParams, PCHAR pchText, BOOL bCaseSensitive);
static BOOL ScrollLineIntoView(HWND hwnd, PVIEWERPARAMS pViewerParams, ULONG ulLine);
static PCHAR QueryFirstLinePtr(PVIEWERPARAMS pViewerParams);
static void SetFirstLinePtr(PVIEWERPARAMS pViewerParams, PCHAR pchStart);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);
static ULONG CalcVisibleLines(PVIEWERPARAMS pViewerParams);

static LONG CalcIncrements(PVIEWERPARAMS pViewerParams, PVIEWERLINE pLine);

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterMsgViewer                                          */
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

BOOL EXPENTRY RegisterMsgViewer(HAB hab)
{
   /* Neue Fensterklasse registrieren */
   if (!WinRegisterClass(hab,
                         WC_MSGVIEWER,
                         MsgViewerProc,
                         CS_SYNCPAINT,
                         sizeof(PVOID) * 2))  /* 2 Pointer */
      return FALSE;
   else
      return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MsgViewerProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fenster-Prozedur des Message-Viewers                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Window-Proc                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MsgViewerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage;
   PDRAGTRANSFER pDragTransfer;
   USHORT usReply;
   char *szFullSourceName;
   ULONG ulStrLen;
   LONG lFoundLine;
   PCHAR pchSave;
   PVIEWERPARAMS pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(parent, 1);

   switch (message)
   {
      case WM_BEGINDRAG:
         if (!pViewerParams->bDragEnabled)
            break;
         pDraginfo=DrgAllocDraginfo(1);
         pDraginfo->usOperation=DO_COPY;
         pDraginfo->hwndSource=parent;

         /* Drag-Item vorbereiten*/
         dItem.hwndItem=parent;
         dItem.ulItemID=0;
         dItem.hstrType=DrgAddStrHandle(DRAGTYPE);
         dItem.hstrRMF=DrgAddStrHandle(DRAGRMF);
         dItem.hstrContainerName=DrgAddStrHandle("c:\\");
         dItem.hstrSourceName=NULLHANDLE;
         dItem.hstrTargetName=DrgAddStrHandle("FleetMsg.Txt");
         dItem.fsControl=0;
         dItem.fsSupportedOps=DO_COPYABLE;
         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

         /* Drag-Image vorbereiten */
         dImage.cb=sizeof(DRAGIMAGE);
         dImage.hImage=pViewerParams->hptrMessage;
         dImage.fl=DRG_ICON;
         dImage.cxOffset=0;
         dImage.cyOffset=0;

         /* Und los gehts */
         DrgDrag(parent, pDraginfo, &dImage, 1, VK_ENDDRAG, NULL);
         DrgFreeDraginfo(pDraginfo);
         break;

      case DM_DISCARDOBJECT:
         WinPostMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROL,
                    MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID), MSGVN_DISCARDCURRENT),
                    NULL);
         return (MRESULT) DRR_SOURCE;

      case DM_PRINTOBJECT:
         Notify(parent, MSGVN_PRINTCURRENT, mp2);
         return (MRESULT) DRR_SOURCE;

      case DM_RENDER:
         pDragTransfer=(PDRAGTRANSFER) mp1;
         usReply=TRUE;

         ulStrLen=DrgQueryStrNameLen(pDragTransfer->hstrSelectedRMF);
         szFullSourceName=malloc(ulStrLen+1);
         DrgQueryStrName(pDragTransfer->hstrSelectedRMF, ulStrLen+1, szFullSourceName);

         if (strcmp(szFullSourceName, "<DRM_OS2FILE,DRF_TEXT>"))
            usReply=FALSE;
         free(szFullSourceName);
         if (pDragTransfer->usOperation != DO_COPY)
            usReply=FALSE;
         pDragTransfer->fsReply=0;
         pDragTransfer->pditem->hstrSourceName=DrgAddStrHandle("FleetMsg.Txt");

         ulStrLen=DrgQueryStrNameLen(pDragTransfer->hstrRenderToName);
         szFullSourceName=malloc(ulStrLen+1);
         DrgQueryStrName(pDragTransfer->hstrRenderToName, ulStrLen+1, szFullSourceName);

         RenderPar.pDragTransfer=pDragTransfer;
         RenderPar.pchFileName=szFullSourceName;
         WinPostMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROL,
                    MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID), MSGVN_EXPORTCURRENT),
                    &RenderPar);
         return (MRESULT) usReply;

      case WM_CONTEXTMENU:
         /* Resource-ID des Menues abfragen */
         Notify(parent, MLN_CONTEXTMENU, mp2);
         break;

      case MSGVM_EXPORTED:
         pDragTransfer=(PDRAGTRANSFER) mp1;
         DrgPostTransferMsg(pDragTransfer->hwndClient, DM_RENDERCOMPLETE,
                            pDragTransfer, DMFL_RENDEROK, 0, TRUE);
         DrgFreeDragtransfer(pDragTransfer);
         return (MRESULT) FALSE;

      case MSGVM_SCROLLTOTOP:
         /* Auf erste Zeile setzen */
         pViewerParams->ulFirstLine=0;
         /* neu zeichnen */
         WinInvalidateRect(parent, &pViewerParams->recWindow, FALSE);
         /* Scrollbar anpassen */
         SetScrollParms(parent, pViewerParams);
         return (MRESULT) TRUE;

      case WM_SETFOCUS:
         if (mp1)
         {
            /* bekomme Fokus */
            CreateCursor(parent, pViewerParams);
         }
         else
         {
            /* verliere Fokus */
            DestroyCursor(parent);
         }
         break;

      case WM_PAINT:
         PaintMsgViewer(parent);
         return (MRESULT) FALSE;

      case WM_CREATE:
         return (MRESULT) CreateMsgViewer(parent, (PVOID) mp1, (PCREATESTRUCT) mp2);

      case WM_DESTROY:
         CleanupMsgViewer(parent);
         break;

      case WM_PRESPARAMCHANGED:
         if (LONGFROMMP(mp1) == PP_FONTNAMESIZE)
         {
            /* Neu umbrechen (Font k馬nte sich ge�ndert haben) */
            QueryFontMetrics(parent, pViewerParams);
            pViewerParams->ulVisibleLines = CalcVisibleLines(pViewerParams);
            pchSave = QueryFirstLinePtr(pViewerParams);
            ReflowText(parent, pViewerParams);

            pViewerParams->lAnchorLine = -1;
            DestroyCursor(parent);

            SetFirstLinePtr(pViewerParams, pchSave);
            SetScrollParms(parent, pViewerParams);
         }

         /* neu zeichnen */
         WinInvalidateRect(parent, NULL, TRUE);

         /* Notification */
         Notify(parent, MSGVN_PRESPARAMCHANGED, &RenderPar);
         break;

      case WM_SETWINDOWPARAMS:
         return (MPARAM) SetWindowParams(parent, (PWNDPARAMS) mp1);

      case WM_QUERYWINDOWPARAMS:
         return (MPARAM) QueryWindowParams(parent, (PWNDPARAMS) mp1);

      case WM_SIZE:
         pViewerParams->lAnchorLine = -1;
         DestroyCursor(parent);

         /* Groesse anpassen */
         pchSave = QueryFirstLinePtr(pViewerParams);
         SizeMsgViewer(parent, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2), pViewerParams);
         pViewerParams->ulVisibleLines = CalcVisibleLines(pViewerParams);
         /* und neu umbrechen */
         ReflowText(parent, pViewerParams);
         SetFirstLinePtr(pViewerParams, pchSave);
         /* Scrollbar nach der Zeilenzahl neu einstellen */
         SetScrollParms(parent, pViewerParams);
         break;

      case WM_TIMER:
         if (SHORT1FROMMP(mp1) == TID_AUTOSCROLL)
            ViewerTimer(parent, pViewerParams);
         break;

      case WM_VSCROLL:
         ScrollMsgViewer(parent, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
         break;

      case WM_CHAR:
         return (MPARAM) CharMsgViewer(parent, SHORT1FROMMP(mp1), SHORT2FROMMP(mp2));

      case WM_BUTTON1DOWN:
         /* Klick auf den Viewer, Focus holen, damit Tastendruecke ankommen */
         WinSetFocus(HWND_DESKTOP, parent);

         /* Position merken */
         pViewerParams->ptlStartSelect.x = SHORT1FROMMP(mp1);
         pViewerParams->ptlStartSelect.y = SHORT2FROMMP(mp1);
         break;

      case WM_BUTTON2DOWN:
      case WM_BUTTON3DOWN:
         /* Klick auf den Viewer, Focus holen, damit Tastendruecke ankommen */
         WinSetFocus(HWND_DESKTOP, parent);
         break;

      case WM_BEGINSELECT:
         /* Capture einschalten */
         if (BeginSelect(parent, pViewerParams))
            if (!pViewerParams->bCapture)
            {
               WinSetCapture(HWND_DESKTOP, parent);
               pViewerParams->bCapture = TRUE;
            }
         break;

      case WM_ENDSELECT:
         /* Capture ausschalten */
         if (pViewerParams->bCapture)
         {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pViewerParams->bCapture = FALSE;

            /* Selektion neu berechnen */
            if (pViewerParams->lAnchorLine > pViewerParams->lCrsLine)
            {
               /* umdrehen */
               LONG lTemp;

               lTemp = pViewerParams->lAnchorLine;
               pViewerParams->lAnchorLine = pViewerParams->lCrsLine;
               pViewerParams->lCrsLine = lTemp;
            }

            /* Timer evtl. stoppen */
            if (pViewerParams->bTimer)
            {
               pViewerParams->bTimer = FALSE;
               WinStopTimer(WinQueryAnchorBlock(parent), parent, TID_AUTOSCROLL);
            }
         }
         break;

      case WM_SINGLESELECT:
         /* nur geklickt, Selektion loeschen */
         pViewerParams->lAnchorLine = -1;
         DestroyCursor(parent);
         break;

      case WM_MOUSEMOVE:
         WinSendMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROLPOINTER,
                    MPFROMSHORT(WinQueryWindowUShort(parent, QWS_ID)),
                    NULL);
         if (pViewerParams->bCapture)
            MouseMove(parent, pViewerParams, SHORT2FROMMP(mp1));
         break;

      case MSGVM_COPY:
      case MLM_COPY:
         return (MRESULT) CopyToClip(parent, pViewerParams);

      case MSGVM_SETCOLOR:
         SetColors(parent, LONGFROMMP(mp1), LONGFROMMP(mp2));
         break;

      case MSGVM_QUERYCOLOR:
         QueryColors(parent, LONGFROMMP(mp1), (PLONG) mp2);
         break;

      case MSGVM_DISABLEDRAG:
         pViewerParams->bDragEnabled=FALSE;
         break;

      case MSGVM_ENABLEDRAG:
         pViewerParams->bDragEnabled=TRUE;
         break;

      case MSGVM_FINDTEXT:
         lFoundLine = FindText(pViewerParams, (PCHAR) mp1, (BOOL) mp2);
         if (lFoundLine >= 0)
         {
            ScrollLineIntoView(parent, pViewerParams, lFoundLine);
            return (MRESULT) TRUE;
         }
         else
            return (MRESULT) FALSE;

      case MSGVM_ENABLEHIGHLIGHT:
         pViewerParams->bHighlight = (BOOL) mp1;

         pViewerParams->lAnchorLine = -1;
         DestroyCursor(parent);

         /* und neu umbrechen */
         pchSave = QueryFirstLinePtr(pViewerParams);
         ReflowText(parent, pViewerParams);
         SetFirstLinePtr(pViewerParams, pchSave);
         /* Scrollbar neu einstellen */
         SetScrollParms(parent, pViewerParams);

         /* neu zeichnen */
         WinInvalidateRect(parent, NULL, TRUE);

         return (MRESULT) TRUE;

      case MSGVM_QUERYHIGHLIGHT:
         return (MRESULT) pViewerParams->bHighlight;

      case MSGVM_SETQUOTECHAR:
         pViewerParams->uchQuoteChar = (UCHAR) mp1;
         ReflowText(parent, pViewerParams);
         return (MRESULT) TRUE;

      default:
         break;
   }
   return WinDefWindowProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PaintMsgViewer                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_PAINT des Viewers                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void PaintMsgViewer(HWND hwnd)
{
   RECTL rectl, rectl3;
   HPS hps;
   LONG lBackColor=RGB_WHITE;
   LONG lForeColor=RGB_BLACK;
   LONG lBorderColor=RGB_BLACK;
   LONG lShadowColor=RGB_BLACK;
   LONG lHBackColor=RGB_BLACK;
   LONG lHForeColor=RGB_BLACK;
   PVIEWERPARAMS pViewerParams;
   LONG lFontHeight=0;
   ULONG ulLine=0;
   POINTL pointl;
   LONG lOldFore=-1;
   PVIEWERLINE pCurrentLine;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   hps=WinBeginPaint(hwnd, NULLHANDLE, &rectl);

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

   /* Rahmen-Farben */
   lBorderColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWFRAME, 0);
   lShadowColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);
   lHBackColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_HILITEBACKGROUND, 0);
   lHForeColor=WinQuerySysColor(HWND_DESKTOP, SYSCLR_HILITEFOREGROUND, 0);

   /* Auf RGB schalten */
   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

   CreateLogFont(hps, pViewerParams);

   /* Hintergrund loeschen */
   WinFillRect(hps, &pViewerParams->recWindow, lBackColor);

   /* Rahmen */
   if (pViewerParams->ulStyle & MSGVS_BORDER)
   {
      GpiSetColor(hps, lBorderColor);

      pointl.x=pViewerParams->recWindow.xRight;
      pointl.y=1;
      GpiMove(hps, &pointl);

      pointl.x=0;
      pointl.y=1;
      GpiLine(hps, &pointl);

      pointl.x=0;
      pointl.y=pViewerParams->recWindow.yTop+1;
      GpiLine(hps, &pointl);

      pointl.x=pViewerParams->recWindow.xRight;
      pointl.y=pViewerParams->recWindow.yTop+1;
      GpiLine(hps, &pointl);

      GpiSetColor(hps, lShadowColor);

      pointl.x=pViewerParams->recWindow.xRight;
      pointl.y=0;
      GpiMove(hps, &pointl);

      pointl.x=0;
      pointl.y=0;
      GpiLine(hps, &pointl);

      pointl.x=1;
      pointl.y=2;
      GpiMove(hps, &pointl);

      pointl.x=1;
      pointl.y=pViewerParams->recWindow.yTop;
      GpiLine(hps, &pointl);

      pointl.x=pViewerParams->recWindow.xRight;
      pointl.y=pViewerParams->recWindow.yTop;
      GpiLine(hps, &pointl);
   }

   /* Message-Text */
   if (pViewerParams->pchMessageText)
   {
      lFontHeight=pViewerParams->lFontHeight;

      ulLine = pViewerParams->ulFirstLine;

      /* restliche Zeilen zeichnen, falls in Update-Region */
      while(ulLine < pViewerParams->ulCountLines)
      {
         rectl3.xLeft=pViewerParams->recWindow.xLeft + pViewerParams->lBorder;
         rectl3.xRight=pViewerParams->recWindow.xRight - pViewerParams->lBorder;
         rectl3.yTop=pViewerParams->recWindow.yTop-(ulLine - pViewerParams->ulFirstLine)*lFontHeight;
         rectl3.yBottom=rectl3.yTop-lFontHeight;

         if (rectl3.yBottom < pViewerParams->recWindow.yBottom)
            rectl3.yBottom = pViewerParams->recWindow.yBottom;

         if (!(rectl3.yTop < rectl.yBottom ||
             rectl3.yBottom > rectl.yTop))
         {
            LONG lDrawFore=0, lDrawBack=0;
            ULONG ulDrawFlags = CHS_VECTOR;
            POINTL StartPoint;

            pCurrentLine = pViewerParams->pLines[ulLine];

            /* in der untersten Zeile ggf. Clipping einschalten */
            if ((rectl3.yTop - rectl3.yBottom) < lFontHeight)
               ulDrawFlags |= CHS_CLIP;

            StartPoint.x = rectl3.xLeft;
            StartPoint.y = rectl3.yTop - pViewerParams->lMaxAscender;

            GpiSetCurrentPosition(hps, &StartPoint);

            while (pCurrentLine)
            {
               if (pCurrentLine->ulFlags & LINESEG_UNDER)
                  ulDrawFlags |= CHS_UNDERSCORE;
               else
                  ulDrawFlags &= ~CHS_UNDERSCORE;

               if (pCurrentLine->ulFlags & LINESEG_BOLD)
                  GpiSetCharSet(hps, FONTID_BOLD);
               else
                  if (pCurrentLine->ulFlags & LINESEG_ITALIC)
                     GpiSetCharSet(hps, FONTID_ITALIC);
                  else
                     GpiSetCharSet(hps, LCID_DEFAULT);

               if (pCurrentLine->ulFlags & LINE_HIGHLIGHT)
               {
                  lDrawFore = lHForeColor;
                  lDrawBack = lHBackColor;
                  ulDrawFlags |= CHS_OPAQUE; /* Hintergrund einfaerben */
               }
               else
               {
                  switch(pCurrentLine->ulFlags & LINE_TYPE_MASK)
                  {
                     case LINE_TEXT:
                        lDrawFore = lForeColor;
                        break;

                     case LINE_QUOTE:
                        lDrawFore = pViewerParams->lColorQuote;
                        break;

                     case LINE_QUOTE2:
                        lDrawFore = pViewerParams->lColorQuote2;
                        break;

                     case LINE_TEAR:
                        lDrawFore = pViewerParams->lColorTearline;
                        break;

                     case LINE_ORIGIN:
                        lDrawFore = pViewerParams->lColorOrigin;
                        break;

                     default:
                        break;
                  }
               }

               if (lOldFore != lDrawFore)
               {
                  /* ge�nderte Farbe umschalten */
                  GpiSetColor(hps, lDrawFore);
                  lOldFore = lDrawFore;
               }
               if (ulDrawFlags & CHS_OPAQUE)
                  GpiSetBackColor(hps, lDrawBack);

               if (pCurrentLine->ulLineLen)
               {
                  LONG lLen;

                  lLen = CalcIncrements(pViewerParams, pCurrentLine);

                  GpiCharStringPos(hps, &rectl3, ulDrawFlags,
                                  pCurrentLine->ulLineLen,
                                  pCurrentLine->pchLine,
                                  pViewerParams->pIncrements);

#if 0
                  GpiQueryCurrentPosition(hps, &StartPoint);
#else
                  StartPoint.x += lLen;
                  rectl3.xLeft += lLen;
                  GpiSetCurrentPosition(hps, &StartPoint);
#endif
               }
#if 0
               rectl3.xLeft = StartPoint.x;
#endif

               if (pCurrentLine->ulFlags & LINESEG_NEWLINE)
                  pCurrentLine = NULL;
               else
                  pCurrentLine = pCurrentLine->nextseg;
            }
         }
         ulLine++;
      }
   }

   GpiSetCharSet(hps, LCID_DEFAULT);
#if 0
   GpiDeleteSetId(hps, FONTID_BOLD);
   GpiDeleteSetId(hps, FONTID_ITALIC);
#endif
   WinEndPaint(hps);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CalcIncrements
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fuellt ein Array mit den Increments eines Strings
 |               (wird f. GpiCharStringAtPos benoetigt)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pViewerParams: Instanzdaten
 |            pLine: Zeiger auf Zeilensegment
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: Zeiger auf das gefuellte Array
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static LONG CalcIncrements(PVIEWERPARAMS pViewerParams, PVIEWERLINE pLine)
{
   PLONG pTable;
   int i;
   LONG lSum=0;

   if (pLine->ulFlags & LINESEG_BOLD)
      pTable = pViewerParams->lWidthsBold;
   else
      if (pLine->ulFlags & LINESEG_ITALIC)
         pTable = pViewerParams->lWidthsItalic;
      else
         pTable = pViewerParams->lWidths;

   for (i=0; i< pLine->ulLineLen; i++)
      lSum += pViewerParams->pIncrements[i] = pTable[pLine->pchLine[i]];

   return lSum;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateMsgViewer                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt die Child-Windows des Viewers, bereitet die         */
/*               Control-Daten vor                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pCtrlData: Zeiger auf Control-Daten                            */
/*            pCreateStruct: Zeiger auf CREATESTRUCT                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE   Fehler                                              */
/*                FALSE  OK                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CreateMsgViewer(HWND hwnd, PVOID pCtrlData, PCREATESTRUCT pCreateStruct)
{
   HWND hwndScroll;
   LONG cx;
   ULONG ulTextLen;
   PVIEWERPARAMS pViewerParams=NULL;
#if 0
   HDC hdc;
#endif

   /* Compiler beruhigen */
   pCtrlData=pCtrlData;

   /* Control-Daten erzeugen und initialisieren */
   pViewerParams = calloc(1, sizeof(VIEWERPARAMS));
   PMASSERT(pViewerParams != NULL, "Out of memory");
   WinSetWindowPtr(hwnd, 1, pViewerParams);

#if 0
   /* Presentation Space */
   hdc = WinOpenWindowDC(hwnd);
   if (hdc)
   {
      HAB hab = WinQueryAnchorBlock(hwnd);
      SIZEL size;

      size.cx = WinQuerySysValue( HWND_DESKTOP, SV_CXFULLSCREEN);
      size.cy = WinQuerySysValue( HWND_DESKTOP, SV_CYFULLSCREEN);

      pViewerParams->hps = GpiCreatePS(hab, hdc, &size,
                                       PU_PELS | GPIF_LONG | /*GPIT_MICRO |*/ GPIA_ASSOC);
      if (!pViewerParams->hps)
      {
         ERRORID err = WinGetLastError(hab);
         free(pViewerParams);
         return TRUE;
      }
   }
   else
   {
      free(pViewerParams);
      return TRUE;
   }
#endif

   if (pCreateStruct->pszText)
   {
      /* Message-Text angegeben, kopieren */
      ulTextLen=strlen(pCreateStruct->pszText)+1;
      pViewerParams->pchMessageText = malloc(ulTextLen);
      strcpy(pViewerParams->pchMessageText, pCreateStruct->pszText);
      pViewerParams->ulMessageBufLen=ulTextLen;
   }


   /* System-Breite des Scrollbars abfragen */
   cx=WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);

   /* Vertikale Scrollbar erzeugen */
   hwndScroll=WinCreateWindow(hwnd,
                              WC_SCROLLBAR,
                              NULL,
                              WS_VISIBLE |
                              WS_SYNCPAINT |
                              SBS_VERT,
                              pCreateStruct->cx-cx,
                              0,
                              cx,
                              pCreateStruct->cy,
                              hwnd,
                              HWND_TOP,
                              MSGVID_VSCROLL,
                              NULL,
                              NULL);

   /* Instanzdaten vorbelegen */
   pViewerParams->ulStyle= pCreateStruct->flStyle;
   pViewerParams->lColorQuote=RGB_BLUE;
   pViewerParams->lColorQuote2=0x008F00FFL;
   pViewerParams->lColorTearline=RGB_RED;
   pViewerParams->lColorOrigin=RGB_GREEN;
   pViewerParams->bDragEnabled=FALSE;
   pViewerParams->lAnchorLine = -1;
   pViewerParams->uchQuoteChar = DEFAULT_QUOTE_CHAR;

   /* Gr批e anpassen */
   SizeMsgViewer(hwnd, pCreateStruct->cx, pCreateStruct->cy, pViewerParams);

   QueryFontMetrics(hwnd, pViewerParams);
   pViewerParams->ulVisibleLines = CalcVisibleLines(pViewerParams);

   /* Text neu umbrechen, falls vorhanden */
   ReflowText(hwnd, pViewerParams);
   SetScrollParms(hwnd, pViewerParams);

   /* Icon f. Drag laden */
   pViewerParams->hptrMessage=WinLoadPointer(HWND_DESKTOP, NULLHANDLE, IDIC_MESSAGE);

   if (hwndScroll)
      return FALSE;
   else
      return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupMsgViewer                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Raeumt die Window-Daten auf bei WM_DESTROY                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CleanupMsgViewer(HWND hwnd)
{
   PVIEWERPARAMS pViewerParams;

   /* Scrollbar loeschen */
   WinDestroyWindow(WinWindowFromID(hwnd, MSGVID_VSCROLL));

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   /* Speicher f. Messagetext und Control-Data freigeben */
   if (pViewerParams->pchMessageText)
      free(pViewerParams->pchMessageText);
   FreeLines(pViewerParams);

   /* Icon freigeben */
   WinDestroyPointer(pViewerParams->hptrMessage);

#if 0
   GpiDestroyPS(pViewerParams->hps);
#endif

   /* Instanzdaten freigeben */
   free(pViewerParams);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SizeMsgViewer                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Passt die Groesse des Message-Viewers an                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            cx: neue x-Groesse                                             */
/*            cy: neue y-Groesse                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SizeMsgViewer(HWND hwnd, SHORT cx, SHORT cy, PVIEWERPARAMS pViewerParams)
{
   LONG cxScroll;

   cxScroll=WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);

   /* Scrollbar an Fenster anpassen */
   WinSetWindowPos(WinWindowFromID(hwnd, MSGVID_VSCROLL),
                   NULLHANDLE,
                   cx-cxScroll,
                   0,
                   cxScroll,
                   cy,
                   SWP_SIZE | SWP_MOVE);

   /* Fl�che f. Text merken */
   pViewerParams->recWindow.xLeft=2;
   pViewerParams->recWindow.xRight=cx-cxScroll;
   pViewerParams->recWindow.yTop=cy-2;
   pViewerParams->recWindow.yBottom=2;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetWindowParams                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_SETWINDOWPARAMS                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pWndParams: Zeiger auf WNDPARAMS                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Erfolg                                                */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL SetWindowParams(HWND hwnd, PWNDPARAMS pWndParams)
{
   PVIEWERPARAMS pViewerParams;
   ULONG ulTextLen;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   switch (pWndParams->fsStatus)
   {
      /* Wird implizit auch von WinSetWindowText aufgerufen */
      case WPM_TEXT:
         if (pWndParams->pszText)
         {
            ulTextLen=strlen(pWndParams->pszText)+1;
            if (!pViewerParams->pchMessageText)
            {
               /* neu allokieren */
               pViewerParams->pchMessageText = malloc(ulTextLen);
               pViewerParams->ulMessageBufLen=ulTextLen;
            }
            else
               if (ulTextLen > pViewerParams->ulMessageBufLen)
               {
                  /* alter Platz zu klein */
                  free(pViewerParams->pchMessageText);
                  pViewerParams->pchMessageText = malloc(ulTextLen);
                  pViewerParams->ulMessageBufLen=ulTextLen;
               }

            memcpy(pViewerParams->pchMessageText, pWndParams->pszText, ulTextLen);
         }
         else
         {
            /* Text loeschen */
            if (pViewerParams->pchMessageText)
               free(pViewerParams->pchMessageText);
            pViewerParams->pchMessageText = NULL;
            pViewerParams->ulMessageBufLen=0;
         }
         _heapmin();

         pViewerParams->lAnchorLine = -1;
         DestroyCursor(hwnd);
         break;

      case WPM_PRESPARAMS:
         break;

      default:
         break;
   }

   /* Text neu umbrechen */
   pViewerParams->ulFirstLine=0;
   ReflowText(hwnd, pViewerParams);
   SetScrollParms(hwnd, pViewerParams);


   /* Neuzeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryWindowParams                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_QUERYWINDOWPARAMS                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pWndParams: Zeiger auf WNDPARAMS                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Erfolg                                                */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL QueryWindowParams(HWND hwnd, PWNDPARAMS pWndParams)
{
   PVIEWERPARAMS pViewerParams;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   switch (pWndParams->fsStatus)
   {
      /* Wird implizit durch WinQueryWindowText aufgerufen */
      case WPM_TEXT:
         if (pWndParams->pszText)
         {
            if (pViewerParams->pchMessageText)
               strncpy(pWndParams->pszText, pViewerParams->pchMessageText,
                       pWndParams->cchText);
            else
               pWndParams->pszText[0]='\0';
         }
         break;

      case WPM_PRESPARAMS:
         break;

      default:
         break;
   }
   return TRUE;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: SetScrollParms                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Scrollbar-Werte nach dem dargestellten            */
/*               Text neu                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SetScrollParms(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   SHORT sVisibleLines, sScrollerBottom;

   hwnd = hwnd;

   sVisibleLines=pViewerParams->ulVisibleLines;
   sScrollerBottom=pViewerParams->ulCountLines - sVisibleLines;

   WinSendDlgItemMsg(hwnd, MSGVID_VSCROLL, SBM_SETSCROLLBAR,
                     MPFROMSHORT(pViewerParams->ulFirstLine),
                     MPFROM2SHORT(0, sScrollerBottom));

   WinSendDlgItemMsg(hwnd, MSGVID_VSCROLL, SBM_SETTHUMBSIZE,
                     MPFROM2SHORT(sVisibleLines, pViewerParams->ulCountLines),
                     NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScrollMsgViewer                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_VSCROLL fuer den Viewer                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            sSliderPos: Slider-Position in der Message                     */
/*            usCommand: Kommando-Code                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ScrollMsgViewer(HWND hwnd, SHORT sSliderPos, USHORT usCommand)
{
   PVIEWERPARAMS pViewerParams;
   ULONG ulVisibleLines;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   switch(usCommand)
   {
      case SB_LINEUP:
         if (pViewerParams->ulFirstLine > 0)
         {
            pViewerParams->ulFirstLine--;
            WinScrollWindow(hwnd, 0, -(pViewerParams->lFontHeight),
                            &pViewerParams->recWindow, &pViewerParams->recWindow, (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            SetScrollParms(hwnd, pViewerParams);
         }
         break;

      case SB_LINEDOWN:
         ulVisibleLines=pViewerParams->ulVisibleLines;
         if (pViewerParams->ulFirstLine < (pViewerParams->ulCountLines - ulVisibleLines))
         {
            pViewerParams->ulFirstLine++;
            WinScrollWindow(hwnd, 0, pViewerParams->lFontHeight,
                            &pViewerParams->recWindow, &pViewerParams->recWindow, (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            SetScrollParms(hwnd, pViewerParams);
         }
         break;

      case SB_PAGEUP:
         ulVisibleLines=pViewerParams->ulVisibleLines;
         if (pViewerParams->ulFirstLine > 0)
         {
            ULONG ulLinesScrolled;

            if (pViewerParams->ulFirstLine >= ulVisibleLines)
               ulLinesScrolled = ulVisibleLines;
            else
               ulLinesScrolled = pViewerParams->ulFirstLine; /* - 0 */

            pViewerParams->ulFirstLine -= ulLinesScrolled;
            WinScrollWindow(hwnd, 0, -(pViewerParams->lFontHeight * ulLinesScrolled),
                            &pViewerParams->recWindow, &pViewerParams->recWindow, (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            SetScrollParms(hwnd, pViewerParams);
         }
         break;

      case SB_PAGEDOWN:
         ulVisibleLines=pViewerParams->ulVisibleLines;
         if (pViewerParams->ulFirstLine < (pViewerParams->ulCountLines+1 - ulVisibleLines))
         {
            ULONG ulLinesScrolled;
            ULONG ulNewTop;

            ulLinesScrolled = pViewerParams->ulVisibleLines -1;

            if (pViewerParams->ulFirstLine + ulLinesScrolled + pViewerParams->ulVisibleLines >
                pViewerParams->ulCountLines)
            {
               ulNewTop = pViewerParams->ulCountLines - pViewerParams->ulVisibleLines;
               ulLinesScrolled = ulNewTop - pViewerParams->ulFirstLine;
            }

            pViewerParams->ulFirstLine+=ulLinesScrolled;

            WinScrollWindow(hwnd, 0, (pViewerParams->lFontHeight * ulLinesScrolled),
                            &pViewerParams->recWindow, &pViewerParams->recWindow, (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            SetScrollParms(hwnd, pViewerParams);
         }
         break;

      case SB_SLIDERTRACK:
         {
            LONG lLinesScrolled=(sSliderPos - pViewerParams->ulFirstLine);

            pViewerParams->ulFirstLine=(ULONG) sSliderPos;
            WinScrollWindow(hwnd, 0, (pViewerParams->lFontHeight * lLinesScrolled),
                            &pViewerParams->recWindow, &pViewerParams->recWindow, (HRGN) NULL, NULL,
                            SW_INVALIDATERGN);
            SetScrollParms(hwnd, pViewerParams);
         }
         break;

      default:
         break;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CharMsgViewer                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_CHAR    fuer den Viewer                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            usFlags: Key-Flags                                             */
/*            usVK: Virtual-Key-Code                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Taste verarbeitet                                    */
/*                FALSE Taste nicht verarbeitet                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CharMsgViewer(HWND hwnd, USHORT usFlags, USHORT usVK)
{
   PVIEWERPARAMS pViewerParams;
   ULONG ulVisibleLines;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   if ((usFlags & KC_VIRTUALKEY) && !(usFlags & KC_KEYUP))
   {
      if (!(usFlags & KC_SHIFT) &&
          !(usFlags & KC_CTRL)  &&
          !(usFlags & KC_ALT))
      {
         if (!WinIsWindowEnabled(WinWindowFromID(hwnd, MSGVID_VSCROLL)))
         {
            return FALSE;
         }

         switch(usVK)
         {
            case VK_UP:
               if (pViewerParams->ulFirstLine)
                  ScrollMsgViewer(hwnd, 0, SB_LINEUP);
               return TRUE;

            case VK_DOWN:
               ulVisibleLines=pViewerParams->ulVisibleLines;
               if (pViewerParams->ulFirstLine != (pViewerParams->ulCountLines+1 - ulVisibleLines))
                  ScrollMsgViewer(hwnd, 0, SB_LINEDOWN);
               return TRUE;

            case VK_PAGEUP:
               if (pViewerParams->ulFirstLine)
                  ScrollMsgViewer(hwnd, 0, SB_PAGEUP);
               return TRUE;

            case VK_PAGEDOWN:
               ulVisibleLines=pViewerParams->ulVisibleLines;
               if (pViewerParams->ulFirstLine != (pViewerParams->ulCountLines+1 - ulVisibleLines))
                  ScrollMsgViewer(hwnd, 0, SB_PAGEDOWN);
               return TRUE;

            case VK_END:
               ulVisibleLines=pViewerParams->ulVisibleLines;
               if (pViewerParams->ulFirstLine != (pViewerParams->ulCountLines - ulVisibleLines))
               {
                  pViewerParams->ulFirstLine=pViewerParams->ulCountLines - ulVisibleLines;
                  WinInvalidateRect(hwnd, &pViewerParams->recWindow, FALSE);
                  SetScrollParms(hwnd, pViewerParams);
               }
               return TRUE;

            case VK_HOME:
               if (pViewerParams->ulFirstLine)
               {
                  pViewerParams->ulFirstLine=0;
                  WinInvalidateRect(hwnd, &pViewerParams->recWindow, FALSE);
                  SetScrollParms(hwnd, pViewerParams);
               }
               return TRUE;

            default:
               break;
         }
      }
      else
         if ((usFlags & KC_CTRL) &&
             usVK == VK_INSERT)
         {
            WinSendMsg(hwnd, MSGVM_COPY, NULL, NULL);
            return TRUE;
         }
   }
   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CopyToClip                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Kopiert den Messagetext ins Clipboard                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL CopyToClip(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   HAB hab;
   PVOID pMem=NULL;

   if (hab=WinQueryAnchorBlock(hwnd))
   {
      /* Clipboard oeffnen */
      if (WinOpenClipbrd(hab))
      {
         /* Speicherbedarf ermitteln */
         ULONG ulTextLen=0;
         PCHAR pchTemp=pViewerParams->pchMessageText;
         PCHAR pchStart, pchEnd;

         if (pViewerParams->lAnchorLine == -1)
         {
            /* gesamten Text */
            while (*pchTemp)
            {
               if (*pchTemp=='\n')
                  ulTextLen+=2;  /* Aus \n mach \r\n */
               else
                  ulTextLen++;
               pchTemp++;
            }
         }
         else
         {
            /* ausgewaehlten Text */
            ULONG ulLine;
            PVIEWERLINE pLine;

            /* erste zu kopierende Zeile suchen */
            ulLine = pViewerParams->lAnchorLine;
            pchStart = pViewerParams->pLines[ulLine]->pchLine;

            /* Segmente der letzten Zeile durchgehen */
            pLine = pViewerParams->pLines[pViewerParams->lCrsLine];
            pchEnd = pLine->pchLine + pLine->ulLineLen;
            while (!(pLine->ulFlags & LINESEG_NEWLINE))
            {
               pLine = pLine->nextseg;
               pchEnd = pLine->pchLine + pLine->ulLineLen;
            }

            for ( pchTemp = pchStart; pchTemp <= pchEnd; pchTemp++)
            {
               while (*pchTemp)
               {
                  if (*pchTemp=='\n')
                     ulTextLen+=2;  /* Aus \n mach \r\n */
                  else
                     ulTextLen++;
                  pchTemp++;
               }
               pchTemp++;
            }
         }
         /* plus Nullbyte */
         ulTextLen++;

         if (!DosAllocSharedMem(&pMem, NULL, ulTextLen,
                                PAG_COMMIT | PAG_WRITE | OBJ_GIVEABLE))
         {
            PCHAR pchDest=(PCHAR) pMem;

            if (pViewerParams->lAnchorLine == -1)
            {
               /* gesamten Text in Puffer kopieren */
               pchTemp=pViewerParams->pchMessageText;

               while (*pchTemp)
               {
                  if (*pchTemp == '\n')
                  {
                     *pchDest++='\r';
                     *pchDest++='\n';
                  }
                  else
                     *pchDest++=*pchTemp;
                  pchTemp++;
               }
            }
            else
            {
               /* ausgewaehlten Text */

               for ( pchTemp =pchStart; pchTemp <= pchEnd; pchTemp++)
               {
                  if (*pchTemp=='\n')
                  {
                     *pchDest++='\r';
                     *pchDest++='\n';
                  }
                  else
                     *pchDest++ = *pchTemp;
               }
               /* *pchDest++='\r';
               *pchDest++='\n';*/
            }
            *pchDest='\0';

            /* Clipboard loeschen */
            WinEmptyClipbrd(hab);

            /* Daten ins Clipboard */
            if (WinSetClipbrdData(hab, (ULONG) pMem, CF_TEXT, CFI_POINTER))
               WinCloseClipbrd(hab);
            else
            {
               WinCloseClipbrd(hab);
               return FALSE;
            }
         }
         else
         {
            WinCloseClipbrd(hab);
            return FALSE;
         }
      }
      else
         return FALSE;
   }
   else
      return FALSE;

   return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetColor                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt eine Farbe                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            lColorID: Color-ID                                             */
/*            lRGBColor: RGB-Farbe                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SetColors(HWND hwnd, LONG lColorID, LONG lRGBColor)
{
   PVIEWERPARAMS pViewerParams;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   switch(lColorID)
   {
      case MSGVCLR_BACKGROUND:
         WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, sizeof(LONG), &lRGBColor);
         break;

      case MSGVCLR_TEXT:
         WinSetPresParam(hwnd, PP_FOREGROUNDCOLOR, sizeof(LONG), &lRGBColor);
         break;

      case MSGVCLR_QUOTE:
         pViewerParams->lColorQuote=lRGBColor;
         break;

      case MSGVCLR_QUOTE2:
         pViewerParams->lColorQuote2=lRGBColor;
         break;

      case MSGVCLR_TEARLINE:
         pViewerParams->lColorTearline=lRGBColor;
         break;

      case MSGVCLR_ORIGIN:
         pViewerParams->lColorOrigin=lRGBColor;
         break;

      default:
         break;

   }
   /* Neu zeichnen */
   WinInvalidateRect(hwnd, NULL, TRUE);
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryColors                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fragt eine Farbe ab                                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            lColorID: Color-ID                                             */
/*            plRGBColor: Zeiger auf Farbpuffer                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void QueryColors(HWND hwnd, LONG lColorID, PLONG plRGBColor)
{
   PVIEWERPARAMS pViewerParams;

   pViewerParams=(PVIEWERPARAMS)WinQueryWindowPtr(hwnd, 1);

   switch(lColorID)
   {
      case MSGVCLR_BACKGROUND:
         WinQueryPresParam(hwnd,
                           PP_BACKGROUNDCOLOR,
                           PP_BACKGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)plRGBColor,
                           QPF_ID2COLORINDEX);
         break;

      case MSGVCLR_TEXT:
         WinQueryPresParam(hwnd,
                           PP_FOREGROUNDCOLOR,
                           PP_FOREGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)plRGBColor,
                           QPF_ID2COLORINDEX);
         break;

      case MSGVCLR_QUOTE:
         *plRGBColor=pViewerParams->lColorQuote;
         break;

      case MSGVCLR_QUOTE2:
         *plRGBColor=pViewerParams->lColorQuote2;
         break;

      case MSGVCLR_TEARLINE:
         *plRGBColor=pViewerParams->lColorTearline;
         break;

      case MSGVCLR_ORIGIN:
         *plRGBColor=pViewerParams->lColorOrigin;
         break;

      default:
         break;

   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: BeginSelect                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beginnt die Selektion per Maus                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Selektion beginnen                                   */
/*                FALSE Selektion nicht beginnen                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL BeginSelect(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   LONG lClickLine=0;

   if (!pViewerParams->ulCountLines)
      return FALSE;  /* kein Text */

   lClickLine = pViewerParams->ulFirstLine +
                (pViewerParams->recWindow.yTop - pViewerParams->ptlStartSelect.y)/
                pViewerParams->lFontHeight;

   if (lClickLine >= pViewerParams->ulCountLines)
   {
      pViewerParams->lAnchorLine = -1;
      return FALSE;
   }
   else
   {
      pViewerParams->lAnchorLine = lClickLine;
      pViewerParams->lCrsLine    = lClickLine;
      pViewerParams->lPrevLine   = lClickLine;

      CreateCursor(hwnd, pViewerParams);
      return TRUE;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MouseMove                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet WM_MOUSEMOVE bei der Selektion                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*            x: x-Koordinate                                                */
/*            y: y-Koordinate                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void MouseMove(HWND hwnd, PVIEWERPARAMS pViewerParams, SHORT y)
{
   LONG lMoveLine=0;

   if (y < 0)
   {
      /* Nach unten gezogen */
      if (!pViewerParams->bTimer )
      {
         DragDown(hwnd, pViewerParams);
         pViewerParams->bTimer = TRUE;
         WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL,
                       WinQuerySysValue(HWND_DESKTOP, SV_SCROLLRATE));
      }
   }
   else
      if (y > pViewerParams->recWindow.yTop)
      {
         /* Nach oben gezogen */
         if (!pViewerParams->bTimer )
         {
            DragUp(hwnd, pViewerParams);
            pViewerParams->bTimer = TRUE;
            WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL,
                          WinQuerySysValue(HWND_DESKTOP, SV_SCROLLRATE));
         }
      }
      else
      {
         lMoveLine = pViewerParams->ulFirstLine +
                      (pViewerParams->recWindow.yTop - y)/
                      pViewerParams->lFontHeight;

         if (lMoveLine >= pViewerParams->ulCountLines)
            lMoveLine = pViewerParams->ulCountLines-1;
         if (lMoveLine < 0)
            lMoveLine = 0;

         if (lMoveLine != pViewerParams->lPrevLine)
         {
            pViewerParams->lPrevLine = lMoveLine;
            pViewerParams->lCrsLine  = lMoveLine;
            CreateCursor(hwnd, pViewerParams);
         }
      }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateCursor                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt und positioniert den Cursor, der die Selektion      */
/*               umrahmt                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CreateCursor(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   RECTL rclCrs;
   LONG lTopLine;
   LONG lBotLine;

   if (pViewerParams->lAnchorLine == -1)
     return;   /* keine Auswahl */

   if (pViewerParams->lAnchorLine < pViewerParams->lCrsLine)
   {
      lTopLine = pViewerParams->lAnchorLine;
      lBotLine = pViewerParams->lCrsLine;
   }
   else
   {
      lBotLine = pViewerParams->lAnchorLine;
      lTopLine = pViewerParams->lCrsLine;
   }

   rclCrs.xLeft  = pViewerParams->recWindow.xLeft;
   rclCrs.xRight = pViewerParams->recWindow.xRight;
   rclCrs.yTop   = pViewerParams->recWindow.yTop -
                   (lTopLine - pViewerParams->ulFirstLine) *
                   pViewerParams->lFontHeight;
   rclCrs.yBottom= pViewerParams->recWindow.yTop -
                   (lBotLine - pViewerParams->ulFirstLine + 1) *
                   pViewerParams->lFontHeight;

   WinInflateRect(WinQueryAnchorBlock(hwnd), &rclCrs, -2, 0);

   /* Cursor erzeugen */
   WinCreateCursor(hwnd, rclCrs.xLeft, rclCrs.yBottom,
                   rclCrs.xRight - rclCrs.xLeft,
                   rclCrs.yTop - rclCrs.yBottom,
                   CURSOR_FRAME /*| CURSOR_HALFTONE*/,
                   NULL);
   WinShowCursor(hwnd, TRUE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DestroyCursor                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht den Cursor                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DestroyCursor(HWND hwnd)
{
   /* Cursor loeschen */
   WinDestroyCursor(hwnd);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ViewerTimer                                                */
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

static void  ViewerTimer(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   POINTL ptl;

   /* Pointer-Position im Window ermitteln */
   WinQueryPointerPos(HWND_DESKTOP, &ptl);
   WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, TRUE);

   if (WinPtInRect(WinQueryAnchorBlock(hwnd), &pViewerParams->recWindow, &ptl) )
   {
      /* Pointer ist wieder drin, Ende der Show */
      pViewerParams->bTimer = FALSE;
      WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_AUTOSCROLL);
   }
   else
   {
     /* drueber ? */
     if (ptl.y > pViewerParams->recWindow.yTop )
        DragUp(hwnd, pViewerParams);
     else
       /* drunter ? */
       if (ptl.y < pViewerParams->recWindow.yBottom )
          DragDown(hwnd, pViewerParams);
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

static void DragUp(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   if (pViewerParams->ulFirstLine > 0)
   {
      pViewerParams->ulFirstLine--;
      pViewerParams->lCrsLine = pViewerParams->ulFirstLine;

      WinScrollWindow(hwnd, 0, -(pViewerParams->lFontHeight),
                      &pViewerParams->recWindow, &pViewerParams->recWindow,
                      (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
      CreateCursor(hwnd, pViewerParams);
      WinUpdateWindow(hwnd);
      SetScrollParms(hwnd, pViewerParams);
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

static void  DragDown(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   LONG lVisibleLines;

   lVisibleLines = pViewerParams->ulVisibleLines;

   if (pViewerParams->ulFirstLine + lVisibleLines < pViewerParams->ulCountLines)
   {
      pViewerParams->ulFirstLine++;
      if (pViewerParams->lCrsLine < pViewerParams->ulCountLines-1)
         pViewerParams->lCrsLine++;

      WinScrollWindow(hwnd, 0, pViewerParams->lFontHeight,
                      &pViewerParams->recWindow, &pViewerParams->recWindow,
                      (HRGN) NULL, NULL,
                      SW_INVALIDATERGN);
      CreateCursor(hwnd, pViewerParams);
      WinUpdateWindow(hwnd);
      SetScrollParms(hwnd, pViewerParams);
   }
   else
   {
      if (pViewerParams->lCrsLine < pViewerParams->ulCountLines-1)
         pViewerParams->lCrsLine++;
      CreateCursor(hwnd, pViewerParams);
   }
   return;
}

static LONG FindText(PVIEWERPARAMS pViewerParams, PCHAR pchText, BOOL bCaseSensitive)
{
   PCHAR pchFound= (PCHAR)1;
   PCHAR pchStep = pViewerParams->pchMessageText;
   int i=0;
   LONG lFound=-1;
   ULONG ulLen;
   PVIEWERLINE pLine;

   if (!pchText)
      return -1;

   ulLen = strlen(pchText);

   while (pchFound)
   {
      /* Text suchen */
      if (!bCaseSensitive)
         pchFound=stristr(pchStep, pchText);
      else
         pchFound=strstr(pchStep, pchText);

      if (pchFound)
      {
         while (i < pViewerParams->ulCountLines && pViewerParams->pLines[i]->pchLine <= pchFound)
            i++;

         if (lFound < 0)
            lFound = i-1;

         pLine = pViewerParams->pLines[i-1];

         while (pLine)
         {
            pLine->ulFlags |= LINE_HIGHLIGHT;
            if (pLine->ulFlags & LINESEG_NEWLINE)
               pLine=NULL;
            else
               pLine = pLine->nextseg;
         }
      }

      pchStep = pchFound+ulLen;
   }

   return lFound;
}

static BOOL ScrollLineIntoView(HWND hwnd, PVIEWERPARAMS pViewerParams, ULONG ulLine)
{
   ULONG ulVisibleLines = pViewerParams->ulVisibleLines;
   LONG lNewFirst;

   /* ins Sichtbare scrollen */
   lNewFirst = (LONG) ulLine - (LONG) (ulVisibleLines/2);
   if (lNewFirst < 0)
      lNewFirst = 0;

   pViewerParams->ulFirstLine = lNewFirst;

   WinInvalidateRect(hwnd, NULL, TRUE);
   SetScrollParms(hwnd, pViewerParams);

   return TRUE;
}

static void QueryFontMetrics(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   FONTMETRICS FontMetrics;
   HPS hps;

   hps = WinGetPS(hwnd);

   /* Font-Hoehe bestimmen */
   GpiQueryFontMetrics(hps, sizeof(FontMetrics), &FontMetrics);
   pViewerParams->lFontHeight=FontMetrics.lMaxAscender + FontMetrics.lMaxDescender;
   pViewerParams->lBorder=FontMetrics.lAveCharWidth/3;
   pViewerParams->lMaxAscender = FontMetrics.lMaxAscender;

   GpiQueryWidthTable(hps, 0, 256, pViewerParams->lWidths);

   CreateBoldFont(hps, pViewerParams, &FontMetrics);
   GpiSetCharSet(hps, FONTID_BOLD);
   GpiQueryWidthTable(hps, 0, 256, pViewerParams->lWidthsBold);

   CreateItalicFont(hps, pViewerParams, &FontMetrics);
   GpiSetCharSet(hps, FONTID_ITALIC);
   GpiQueryWidthTable(hps, 0, 256, pViewerParams->lWidthsItalic);

   WinReleasePS(hps);

   return;
}

static PCHAR QueryFirstLinePtr(PVIEWERPARAMS pViewerParams)
{
   if (pViewerParams->ulCountLines &&
       pViewerParams->pLines)
   {
      return pViewerParams->pLines[pViewerParams->ulFirstLine]->pchLine;
   }
   else
      return NULL;
}

static void SetFirstLinePtr(PVIEWERPARAMS pViewerParams, PCHAR pchStart)
{
   int i;

   for (i=0; i < pViewerParams->ulCountLines; i++)
   {
      if (pViewerParams->pLines[i]->pchLine > pchStart)
      {
         pViewerParams->ulFirstLine = i?(i-1):0;
         break;
      }
   }
   return;
}

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd,QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}

static LONG CreateBoldFont(HPS hps, PVIEWERPARAMS pViewerParams, PFONTMETRICS pFontMetrics)
{
   LONG rc=0;

   /* alten Font loeschen */
   GpiSetCharSet(hps, 0L);
   GpiDeleteSetId(hps, FONTID_BOLD);

   /* Create-Struktur aufbauen */
   memset(&pViewerParams->FontAttrs, 0, sizeof(pViewerParams->FontAttrs));
   pViewerParams->FontAttrs.usRecordLength = sizeof(pViewerParams->FontAttrs);
   if (!(pFontMetrics->fsDefn & FM_DEFN_OUTLINE))
   {
      pViewerParams->FontAttrs.lMaxBaselineExt = pFontMetrics->lMaxBaselineExt;
      pViewerParams->FontAttrs.lAveCharWidth = pFontMetrics->lAveCharWidth;
   }
   pViewerParams->FontAttrs.usCodePage = pFontMetrics->usCodePage;
   pViewerParams->FontAttrs.fsFontUse = FATTR_FONTUSE_NOMIX;
   if (pFontMetrics->fsDefn & FM_DEFN_OUTLINE)
      pViewerParams->FontAttrs.fsFontUse |= FATTR_FONTUSE_OUTLINE;
   strcpy(pViewerParams->FontAttrs.szFacename, pFontMetrics->szFamilyname);
   strcat(pViewerParams->FontAttrs.szFacename, " Bold");

   if (GpiCreateLogFont(hps, NULL, FONTID_BOLD, &pViewerParams->FontAttrs) != FONT_MATCH)
   {
      /* mit Simulation versuchen */
      pViewerParams->FontAttrs.fsSelection = FATTR_SEL_BOLD;
      strcpy(pViewerParams->FontAttrs.szFacename, pFontMetrics->szFamilyname);
      GpiCreateLogFont(hps, NULL, FONTID_BOLD, &pViewerParams->FontAttrs);
   }

   return rc;
}

static LONG CreateItalicFont(HPS hps, PVIEWERPARAMS pViewerParams, PFONTMETRICS pFontMetrics)
{
   LONG rc=0;

   /* alten Font loeschen */
   GpiSetCharSet(hps, 0L);
   GpiDeleteSetId(hps, FONTID_ITALIC);

   /* Create-Struktur aufbauen */
   memset(&pViewerParams->FontAttrsIta, 0, sizeof(pViewerParams->FontAttrsIta));
   pViewerParams->FontAttrsIta.usRecordLength = sizeof(pViewerParams->FontAttrsIta);
   if (!(pFontMetrics->fsDefn & FM_DEFN_OUTLINE))
   {
      pViewerParams->FontAttrsIta.lMaxBaselineExt = pFontMetrics->lMaxBaselineExt;
      pViewerParams->FontAttrsIta.lAveCharWidth = pFontMetrics->lAveCharWidth;
   }
   pViewerParams->FontAttrsIta.usCodePage = pFontMetrics->usCodePage;
   pViewerParams->FontAttrsIta.fsFontUse = FATTR_FONTUSE_NOMIX;
   if (pFontMetrics->fsDefn & FM_DEFN_OUTLINE)
      pViewerParams->FontAttrsIta.fsFontUse |= FATTR_FONTUSE_OUTLINE;
   strcpy(pViewerParams->FontAttrsIta.szFacename, pFontMetrics->szFamilyname);
   strcat(pViewerParams->FontAttrsIta.szFacename, " Italic");

   if (GpiCreateLogFont(hps, NULL, FONTID_ITALIC, &pViewerParams->FontAttrsIta) != FONT_MATCH)
   {
      /* mit Simulation versuchen */
      pViewerParams->FontAttrsIta.fsSelection = FATTR_SEL_ITALIC;
      strcpy(pViewerParams->FontAttrsIta.szFacename, pFontMetrics->szFamilyname);
      GpiCreateLogFont(hps, NULL, FONTID_ITALIC, &pViewerParams->FontAttrsIta);
   }

   return rc;
}

static LONG CreateLogFont(HPS hps, PVIEWERPARAMS pViewerParams)
{
   GpiCreateLogFont(hps, NULL, FONTID_ITALIC, &pViewerParams->FontAttrsIta);
   return GpiCreateLogFont(hps, NULL, FONTID_BOLD, &pViewerParams->FontAttrs);
}

static ULONG CalcVisibleLines(PVIEWERPARAMS pViewerParams)
{
   return (pViewerParams->recWindow.yTop - pViewerParams->recWindow.yBottom)/
          pViewerParams->lFontHeight;
}
/*-------------------------------- Modulende --------------------------------*/

