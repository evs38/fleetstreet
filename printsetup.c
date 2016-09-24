/*---------------------------------------------------------------------------+
 | Titel: PRINTSETUP.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 23.09.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x, 3.x PM                                                  |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Printer-Setup von FleetStreet                                           |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_SPLDOSPRINT
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "resids.h"
#include "utility.h"
#include "controls\fontdisp.h"
#include "printsetup.h"
#include "printmsg\setup.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HWND hwndhelp;

static PCHAR BorderValues[]=
{
   "0,0", "0,1", "0,2", "0,3", "0,4", "0,5", "0,6", "0,7", "0,8", "0,9",
   "1,0", "1,1", "1,2", "1,3", "1,4", "1,5", "1,6", "1,7", "1,8", "1,9",
   "2,0", "2,1", "2,2", "2,3", "2,4", "2,5", "2,6", "2,7", "2,8", "2,9",
   "3,0", "3,1", "3,2", "3,3", "3,4", "3,5", "3,6", "3,7", "3,8", "3,9",
   "4,0", "4,1", "4,2", "4,3", "4,4", "4,5", "4,6", "4,7", "4,8", "4,9",
   "5,0", "5,1", "5,2", "5,3", "5,4", "5,5", "5,6", "5,7", "5,8", "5,9",
   "6,0", "6,1", "6,2", "6,3", "6,4", "6,5", "6,6", "6,7", "6,8", "6,9",
   "7,0", "7,1", "7,2", "7,3", "7,4", "7,5", "7,6", "7,7", "7,8", "7,9",
   "8,0", "8,1", "8,2", "8,3", "8,4", "8,5", "8,6", "8,7", "8,8", "8,9",
   "9,0", "9,1", "9,2", "9,3", "9,4", "9,5", "9,6", "9,7", "9,8", "9,9",
   "10,0"
};

#define NumBorderValues (sizeof(BorderValues)/sizeof(BorderValues[0]))

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static void InsertPrintSetupPages(HWND notebook, PPRINTSETUP pData);
static MRESULT EXPENTRY PSListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY PSBordersProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY PSFontsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY PSOutputProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void ConfigurePrinter(HWND hwndListbox, PPRINTSETUP pPrintSetup);
static LONG QuerySpinValue(HWND hwnd);
static LONG GetFontByDlg(HWND hwndOwner, PPRINTSETUP pPrintSetup, char *pchFont, ULONG idFontDisp);


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

MRESULT EXPENTRY PrintSetupProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRINTSETUP pPrintSetup = WinQueryWindowPtr(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         pPrintSetup = (PPRINTSETUP) mp2;
         InitPrintSetup(pPrintSetup, anchor);
         InsertPrintSetupPages(WinWindowFromID(hwnd, IDD_PRINTSETUP+1), pPrintSetup);

         RestoreWinPos(hwnd, &pPrintSetup->DlgPos, TRUE, TRUE);
         WinSetWindowPtr(hwnd, QWL_USER, pPrintSetup);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_PRINTSETUP+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (WinQueryWindowPtr(hwnd, QWL_USER))
            SaveWinPos(hwnd, (PSWP) mp1, &pPrintSetup->DlgPos, &pPrintSetup->bDirty);
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
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertPrintSetupPages                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt die Settings-Pages in das Notebook fuer den           */
/*               Printer-Setup ein.                                          */
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

static void InsertPrintSetupPages(HWND notebook, PPRINTSETUP pData)
{
   SetNotebookParams(notebook, 80);

   InsertOnePage(notebook, IDD_PS_LIST,    IDST_TAB_PS_LIST,    PSListProc,    pData);
   InsertOnePage(notebook, IDD_PS_BORDERS, IDST_TAB_PS_BORDERS, PSBordersProc, pData);
   InsertOnePage(notebook, IDD_PS_FONTS,   IDST_TAB_PS_FONTS,   PSFontsProc,   pData);
   InsertOnePage(notebook, IDD_PS_OUTPUT,  IDST_TAB_PS_OUTPUT,  PSOutputProc,  pData);

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

static MRESULT EXPENTRY PSListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRINTSETUP pPrintSetup = WinQueryWindowPtr(hwnd, QWL_USER);
   HWND hwndListbox;
   int i, index;
   char *psz;
   PPRQINFO3 pqi;

   switch(msg)
   {
      case WM_INITDLG:
         pPrintSetup = (PPRINTSETUP) mp2;
         WinSetWindowPtr(hwnd, QWL_USER, pPrintSetup);

         /* fill listbox with print queues */
         hwndListbox = WinWindowFromID( hwnd, IDD_PS_LIST+2 );

         for( i = 0; i < pPrintSetup->cQueues; i++ )
         {
           /* Display printer comment if possible, else use queue name for display. */
           psz = *pPrintSetup->pQueueInfo[i].pszComment ? pPrintSetup->pQueueInfo[i].pszComment : pPrintSetup->pQueueInfo[i].pszName;

           index = (LONG) WinSendMsg( hwndListbox, LM_INSERTITEM, (MPARAM)LIT_END, (MPARAM)psz );

           if( 0 == strcmp( pPrintSetup->pQueueInfo[i].pszName, pPrintSetup->szPreferredQueue ))
           {
             /* Hilight preferred queue. */
             WinSendMsg( hwndListbox, LM_SELECTITEM, (MPARAM)index, (MPARAM)TRUE );
           }
         }
         if (pPrintSetup->cQueues)
            WinEnableControl(hwnd, IDD_PS_LIST+3, TRUE);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_PS_LIST+3:
               hwndListbox = WinWindowFromID( hwnd, IDD_PS_LIST+2 );
               ConfigurePrinter(hwndListbox, pPrintSetup);
               return (MRESULT) FALSE;

            default:
               return (MRESULT) FALSE;
         }

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_PS_LIST+2 &&
             SHORT2FROMMP(mp1) == LN_ENTER)
            ConfigurePrinter((HWND) mp2, pPrintSetup);
         break;

      case WM_DESTROY:
         hwndListbox = WinWindowFromID( hwnd, IDD_PS_LIST+2 );

         index = (LONG) WinSendMsg( hwndListbox, LM_QUERYSELECTION, (MPARAM)LIT_FIRST, 0 );
         if (index != LIT_NONE)
         {
            pqi = &pPrintSetup->pQueueInfo[ index ];

            if (pPrintSetup->pDriverData)
               free(pPrintSetup->pDriverData);
            pPrintSetup->pDriverData = malloc(pqi->pDriverData->cb);
            memcpy(pPrintSetup->pDriverData, pqi->pDriverData, pqi->pDriverData->cb);

            /* New preferred queue. Modify the one in the PRINTERSETUP structure. */
            strcpy( pPrintSetup->szPreferredQueue, pqi->pszName );
            pPrintSetup->bDirty = TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static void ConfigurePrinter(HWND hwndListbox, PPRINTSETUP pPrintSetup)
{
   int index;

   index = (LONG) WinSendMsg( hwndListbox, LM_QUERYSELECTION, (MPARAM)LIT_FIRST, 0 );
   if (index != LIT_NONE)
   {
      CHAR szDriverName[ 64 ];
      CHAR szDeviceName[ 48 ];
      char *pch;
      PPRQINFO3 pqi;

      pqi = &pPrintSetup->pQueueInfo[ index ];

      /* Call DevPostDeviceModes() to present the job setup dialog of the printer driver.
         pqi->pszDriverName is DRIVER.Device format. Separate them. */

      strcpy( szDriverName, pqi->pszDriverName );

      pch = strchr( szDriverName, '.' );
      if( pch )
      {
        strcpy( szDeviceName, pch+1 );
        *pch = 0;
      }
      else
        *szDeviceName = 0;

      /* There may be more than one printer on this print queue */
      pch = strchr( pqi->pszPrinters, ',' );
      if( pch )
        *pch = 0;

      /* Present the job properties dialog to the user. */
      DevPostDeviceModes( pPrintSetup->hab,
                          pqi->pDriverData,
                          szDriverName, szDeviceName,
                          pqi->pszPrinters,
                          DPDM_POSTJOBPROP );
      pPrintSetup->bDirty = TRUE;
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

static MRESULT EXPENTRY PSBordersProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRINTSETUP pPrintSetup = WinQueryWindowPtr(hwnd, QWL_USER);
   LONG lTemp;

   switch(msg)
   {
      case WM_INITDLG:
         pPrintSetup = (PPRINTSETUP) mp2;
         WinSetWindowPtr(hwnd, QWL_USER, pPrintSetup);

         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+5, SPBM_SETARRAY,
                           BorderValues, MPFROMSHORT(NumBorderValues));
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+5, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pPrintSetup->lLeft), NULL);
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+6, SPBM_SETARRAY,
                           BorderValues, MPFROMSHORT(NumBorderValues));
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+6, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pPrintSetup->lRight), NULL);
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+7, SPBM_SETARRAY,
                           BorderValues, MPFROMSHORT(NumBorderValues));
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+7, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pPrintSetup->lTop), NULL);
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+8, SPBM_SETARRAY,
                           BorderValues, MPFROMSHORT(NumBorderValues));
         WinSendDlgItemMsg(hwnd, IDD_PS_BORDERS+8, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pPrintSetup->lBottom), NULL);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         lTemp = QuerySpinValue(WinWindowFromID(hwnd, IDD_PS_BORDERS+5));
         if (pPrintSetup->lLeft != lTemp)
         {
            pPrintSetup->lLeft = lTemp;
            pPrintSetup->bDirty = TRUE;
         }
         lTemp = QuerySpinValue(WinWindowFromID(hwnd, IDD_PS_BORDERS+6));
         if (pPrintSetup->lRight != lTemp)
         {
            pPrintSetup->lRight = lTemp;
            pPrintSetup->bDirty = TRUE;
         }
         lTemp = QuerySpinValue(WinWindowFromID(hwnd, IDD_PS_BORDERS+7));
         if (pPrintSetup->lTop != lTemp)
         {
            pPrintSetup->lTop = lTemp;
            pPrintSetup->bDirty = TRUE;
         }
         lTemp = QuerySpinValue(WinWindowFromID(hwnd, IDD_PS_BORDERS+8));
         if (pPrintSetup->lBottom != lTemp)
         {
            pPrintSetup->lBottom = lTemp;
            pPrintSetup->bDirty = TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static LONG QuerySpinValue(HWND hwnd)
{
   LONG lTemp=0;
   char pchTemp[10]={0};

   if (WinSendMsg(hwnd, SPBM_QUERYVALUE, pchTemp,
                  MPFROM2SHORT(sizeof(pchTemp), SPBQ_DONOTUPDATE)))
   {
      /* Wert im Array suchen */
      while (lTemp < NumBorderValues && strcmp(pchTemp, BorderValues[lTemp]))
         lTemp++;

      if (lTemp >= NumBorderValues)
         lTemp = 0;
   }
   else
      lTemp = 10;  /* 1 cm */

   return lTemp;
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

static MRESULT EXPENTRY PSFontsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRINTSETUP pPrintSetup = WinQueryWindowPtr(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         pPrintSetup = (PPRINTSETUP) mp2;
         WinSetWindowPtr(hwnd, QWL_USER, pPrintSetup);

         if (pPrintSetup->pchHeaderFont[0])
            WinSendDlgItemMsg(hwnd, IDD_PS_FONTS+3, FDM_SETFONT, pPrintSetup->pchHeaderFont, NULL);
         if (pPrintSetup->pchTextFont[0])
            WinSendDlgItemMsg(hwnd, IDD_PS_FONTS+4, FDM_SETFONT, pPrintSetup->pchTextFont, NULL);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_PS_FONTS+3: /* Header-Font */
               if (SHORT2FROMMP(mp1) == FDN_FONTCHANGED)
               {
                  WinSendMsg((HWND)mp2, FDM_QUERYFONT, pPrintSetup->pchHeaderFont,
                             MPFROMLONG(sizeof(pPrintSetup->pchHeaderFont)));
                  pPrintSetup->bDirty = TRUE;
               }
               if (SHORT2FROMMP(mp1) == FDN_OPEN)
               {
                  if (GetFontByDlg(hwnd, pPrintSetup, pPrintSetup->pchHeaderFont, IDD_PS_FONTS+3) == DID_OK)
                  {
                     WinSendMsg((HWND)mp2, FDM_SETFONT, pPrintSetup->pchHeaderFont, NULL);
                     pPrintSetup->bDirty = TRUE;
                  }
               }
               break;

            case IDD_PS_FONTS+4: /* Text-Font */
               if (SHORT2FROMMP(mp1) == FDN_FONTCHANGED)
               {
                  WinSendMsg((HWND)mp2, FDM_QUERYFONT, pPrintSetup->pchTextFont,
                             MPFROMLONG(sizeof(pPrintSetup->pchTextFont)));
                  pPrintSetup->bDirty = TRUE;
               }
               if (SHORT2FROMMP(mp1) == FDN_OPEN)
               {
                  if (GetFontByDlg(hwnd, pPrintSetup, pPrintSetup->pchTextFont, IDD_PS_FONTS+4) == DID_OK)
                  {
                     WinSendMsg((HWND)mp2, FDM_SETFONT, pPrintSetup->pchTextFont, NULL);
                     pPrintSetup->bDirty = TRUE;
                  }
               }
               break;
         }
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

static MRESULT EXPENTRY PSOutputProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPRINTSETUP pPrintSetup = WinQueryWindowPtr(hwnd, QWL_USER);
   ULONG ulTemp;

   switch(msg)
   {
      case WM_INITDLG:
         pPrintSetup = (PPRINTSETUP) mp2;
         WinSetWindowPtr(hwnd, QWL_USER, pPrintSetup);

         if (pPrintSetup->ulOutput & OUTPUT_AREA)
            WinCheckButton(hwnd, IDD_PS_OUTPUT+2, TRUE);
         if (pPrintSetup->ulOutput & OUTPUT_DATE)
            WinCheckButton(hwnd, IDD_PS_OUTPUT+3, TRUE);
         if (pPrintSetup->ulOutput & OUTPUT_ATTRIB)
            WinCheckButton(hwnd, IDD_PS_OUTPUT+4, TRUE);
         if (pPrintSetup->ulOutput & OUTPUT_PAGENUM)
            WinCheckButton(hwnd, IDD_PS_OUTPUT+5, TRUE);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         ulTemp = 0;
         if (WinQueryButtonCheckstate(hwnd, IDD_PS_OUTPUT+2))
            ulTemp |= OUTPUT_AREA;
         if (WinQueryButtonCheckstate(hwnd, IDD_PS_OUTPUT+3))
            ulTemp |= OUTPUT_DATE;
         if (WinQueryButtonCheckstate(hwnd, IDD_PS_OUTPUT+4))
            ulTemp |= OUTPUT_ATTRIB;
         if (WinQueryButtonCheckstate(hwnd, IDD_PS_OUTPUT+5))
            ulTemp |= OUTPUT_PAGENUM;
         if (pPrintSetup->ulOutput != ulTemp)
         {
            pPrintSetup->ulOutput = ulTemp;
            pPrintSetup->bDirty = TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

ULONG InitPrintSetup(PPRINTSETUP pPrintSetup, HAB hab)
{
   pPrintSetup->hab = hab;
   pPrintSetup->lWorldCoordinates = PU_LOMETRIC;

   return SetupPrinter(pPrintSetup);
}

ULONG TermPrintSetup(PPRINTSETUP pPrintSetup)
{
   return CleanupPrinter(pPrintSetup);
}

static LONG GetFontByDlg(HWND hwndOwner, PPRINTSETUP pPrintSetup, char *pchFont, ULONG idFontDisp)
{
   FONTDLG FontDlg;
   char pchFamily[FACESIZE+5]="";
   long lTemp;
   FONTMETRICS FontMetrics;
   HPS hps;

   hps = WinGetPS(WinWindowFromID(hwndOwner, idFontDisp));

   GpiQueryFontMetrics(hps, sizeof(FontMetrics), &FontMetrics);

   strcpy(pchFamily, FontMetrics.szFamilyname);

   WinReleasePS(hps);

   lTemp = strtol(pchFont, NULL, 10);

   memset(&FontDlg, 0, sizeof(FontDlg));

   FontDlg.cbSize = sizeof(FontDlg);
   FontDlg.hpsPrinter = pPrintSetup->hpsPrinterInfo;
   FontDlg.pszFamilyname = pchFamily;
   FontDlg.usFamilyBufLen = sizeof(pchFamily);
   FontDlg.fxPointSize = MAKEFIXED(lTemp, 0);
   FontDlg.fl = FNTS_CENTER | FNTS_VECTORONLY;
   FontDlg.flFlags = FNTF_NOVIEWSCREENFONTS;
   FontDlg.clrFore = CLR_BLACK;
   FontDlg.clrBack = CLR_WHITE;

   WinFontDlg(HWND_DESKTOP, hwndOwner, &FontDlg);

   if (FontDlg.lReturn == DID_OK)
   {
      sprintf(pchFont, "%d.%s", (int) FIXEDINT(FontDlg.fxPointSize), FontDlg.fAttrs.szFacename);
   }

   return FontDlg.lReturn;
}
/*-------------------------------- Modulende --------------------------------*/

