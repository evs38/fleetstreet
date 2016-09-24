/*---------------------------------------------------------------------------+
 | Titel: COLORSELECT.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 19.05.94                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Fensterklasse f. Farbauswahl                                            |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_PM
#include <os2.h>
#include "clrwhl.h"
#include "clrsel.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef MIN
#define MIN(x,y) ((x)>(y)?(y):(x))
#endif

#ifndef MAX
#define MAX(x,y) ((x)<(y)?(y):(x))
#endif

/*---------------------------------- Typen ----------------------------------*/

typedef struct {
          HWND hwndWheel;
          HWND hwndSlider;
          BOOL bNotify;
          LONG lSliderMax;
        } CLRSELDATA, *PCLRSELDATA;

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/
static MRESULT EXPENTRY ClrSelWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname: RegisterColorSelect                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Registriert die Fensterklasse "ColorSelect"                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anchor Blcok der registrierenden Anwendung                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE: Erfolg                                               */
/*                FALSE: Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Registriert auch vorher "ColorWheel"                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL EXPENTRY RegisterColorSelect(HAB hab)
{
   if (!RegisterColorWheel(hab))
      return FALSE;

   if (!WinRegisterClass(hab, WC_COLORSELECT, ClrSelWndProc,
                         CS_CLIPCHILDREN | CS_SYNCPAINT | CS_SIZEREDRAW,
                         sizeof(PVOID)))
      return FALSE;
   else
      return TRUE;

}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ClrSelWndProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure der Klasse                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (WndProc)                                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY ClrSelWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PCLRSELDATA pClrSelData = (PCLRSELDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_CREATE:
         {
            PCREATESTRUCT pCreate = (PCREATESTRUCT) mp2;
            CLRWHLCDATA CWheel;
            SLDCDATA SLData;
            LONG lcxWheel;

            /* Speicher fuer Window-Daten anfordern */
            if (DosAllocMem((PPVOID) &pClrSelData, sizeof(CLRSELDATA),
                            PAG_COMMIT | PAG_READ | PAG_WRITE))
               return (MRESULT) TRUE;

            WinSetWindowULong(hwnd, QWL_USER, (ULONG) pClrSelData);
            pClrSelData->bNotify = TRUE;

            lcxWheel = MIN(pCreate->cx, pCreate->cy);

            /* Color-Wheel erzeugen */
            CWheel.cb = sizeof(CWheel);
            CWheel.lAngle         = 10L;
            CWheel.lSaturationInc = 5L;
            CWheel.lRadius        = lcxWheel/2;

            pClrSelData->hwndWheel = WinCreateWindow(hwnd, WC_COLORWHEEL, "",
                            WS_VISIBLE |
                            CWS_AUTOSIZE | CWS_BITMAP | CWS_THREADED | CWS_RGB,
                            0, 0,
                            lcxWheel,
                            lcxWheel,
                            hwnd,
                            HWND_TOP, CSEL_ID_WHEEL, &CWheel, NULL);

            if (!pClrSelData->hwndWheel)  /* schiefgelaufen */
            {
               DosFreeMem(pClrSelData);
               return (MRESULT) TRUE;
            }

            /* Slider erzeugen */
            SLData.cbSize             = sizeof(SLData);
            SLData.usScale1Increments = pClrSelData->lSliderMax = pCreate->cy - 20;
            SLData.usScale1Spacing    = 0;

            pClrSelData->hwndSlider = WinCreateWindow(hwnd, WC_SLIDER, "",
                             WS_VISIBLE | SLS_VERTICAL | SLS_HOMEBOTTOM |
                             SLS_RIBBONSTRIP,
                             lcxWheel, 0,
                             pCreate->cx - lcxWheel,
                             pCreate->cy,
                             hwnd, HWND_TOP, CSEL_ID_SLID, &SLData, NULL);

            if (!pClrSelData->hwndSlider)  /* schiefgelaufen */
            {
               WinDestroyWindow(pClrSelData->hwndWheel);
               DosFreeMem(pClrSelData);
               return (MRESULT) TRUE;
            }
         }
         break;

      case WM_DESTROY:
         WinDestroyWindow(pClrSelData->hwndWheel);
         WinDestroyWindow(pClrSelData->hwndSlider);
         DosFreeMem(pClrSelData);
         break;

      case WM_CONTROL:
         if (!pClrSelData->bNotify)
            break;
         switch(SHORT1FROMMP(mp1))
         {
            case CSEL_ID_WHEEL:
               if (SHORT2FROMMP(mp1) == CWN_RGBCLRSELECTED)
               {
                  RGB rgb;
                  HPS hps;
                  ULONG ulColor;
                  ULONG ulSlider;

                  /* Farbe in Color-Wheel ausgewaehlt */
                  /* mp2 ist RGB vom Color-Wheel */

                  /* Slider abfragen */
                  ulSlider= (ULONG) WinSendMsg(pClrSelData->hwndSlider, SLM_QUERYSLIDERINFO,
                             MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                             NULL);

                  ulSlider = (ulSlider * 100) / pClrSelData->lSliderMax;

                  if (ulSlider > 100)
                     ulSlider = 100;

                  /* RGB berechnen */
                  rgb.bRed   = (((((ULONG) mp2) >> 16) & 0xff) * ulSlider)/100+1;
                  rgb.bGreen = (((((ULONG) mp2) >> 8) & 0xff) * ulSlider)/100+1;
                  rgb.bBlue  = ((((ULONG) mp2) & 0xff) * ulSlider)/100+1;

                  hps = WinGetScreenPS(HWND_DESKTOP);
                  ulColor= GpiQueryNearestColor(hps, 0, ulColor);
                  WinReleasePS(hps);

                  /* Notification an Owner */
                  ulColor= (rgb.bRed << 16) | (rgb.bGreen << 8) | rgb.bBlue;
                  WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                             MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), CLSN_RGB),
                             MPFROMLONG(ulColor));
               }
               break;

            case CSEL_ID_SLID:
               if (SHORT2FROMMP(mp1) == SLN_CHANGE ||
                   SHORT2FROMMP(mp1) == SLN_SLIDERTRACK)
               {
                  RGB rgb;
                  HPS hps;
                  ULONG ulColor;
                  ULONG ulSlider = (ULONG) mp2;

                  ulSlider = (ulSlider * 100) / pClrSelData->lSliderMax;

                  /* Slider bewegt sich */
                  /* mp2 ist Slider-Position */

                  /* Color-Wheel abfragen */
                  ulColor = (ULONG) WinSendMsg(pClrSelData->hwndWheel, CWM_QUERYRGBCLR,
                                       NULL, NULL);

                  /* RGB berechnen */
                  rgb.bRed   = (((ulColor >> 16) & 0xff) * ulSlider)/100+1;
                  rgb.bGreen = (((ulColor >> 8) & 0xff) * ulSlider)/100+1;
                  rgb.bBlue  = ((ulColor & 0xff) * ulSlider)/100+1;

                  /* Notification an Owner */
                  ulColor= (rgb.bRed << 16) | (rgb.bGreen << 8) | rgb.bBlue;

                  hps = WinGetScreenPS(HWND_DESKTOP);
                  ulColor= GpiQueryNearestColor(hps, 0, ulColor);
                  WinReleasePS(hps);

                  WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), WM_CONTROL,
                             MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID),
                                          CLSN_RGB),
                             MPFROMLONG(ulColor));
               }
               break;

            default:
               break;
         }
         break;

      case CLSM_QUERYRGB:  /* RGB-Farbe abfragen, mp1 = PRGB */
         break;

      case CLSM_SETRGB:    /* RGB-Farbe setzen, mp1 = PRGB */
         {
            ULONG ulSlider, ulSlPos;
            RGB* prgb = (RGB*) mp1;
            RGB2 rgb = {0, 0, 0, 0};
            ULONG ulR, ulG, ulB;

            ulSlider = (MAX(prgb->bRed, MAX(prgb->bGreen, prgb->bBlue)) * 100) / 255;

            if (ulSlider > 100)
               ulSlider = 100;

            if (ulSlider)
            {
               ulR = (prgb->bRed   * 100) / ulSlider;
               ulG = (prgb->bGreen * 100) / ulSlider;
               ulB = (prgb->bBlue  * 100) / ulSlider;

               if (ulR > 255)
                  ulR = 255;
               if (ulG > 255)
                  ulG = 255;
               if (ulB > 255)
                  ulB = 255;

               rgb.bRed   = ulR;
               rgb.bGreen = ulG;
               rgb.bBlue  = ulB;
            }
            else
            {
               rgb.bRed   = 255;
               rgb.bGreen = 255;
               rgb.bBlue  = 255;
            }

            pClrSelData->bNotify = FALSE;
            ulSlPos=(ulSlider * pClrSelData->lSliderMax) / 100;
            if (ulSlPos >= pClrSelData->lSliderMax)
               ulSlPos = pClrSelData->lSliderMax-1;
            WinSendMsg(pClrSelData->hwndSlider, SLM_SETSLIDERINFO,
                       MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                       MPFROMSHORT(ulSlPos));

            WinSendMsg(pClrSelData->hwndWheel, CWM_SETRGBCLR,
                       &rgb, NULL);
            pClrSelData->bNotify = TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/

