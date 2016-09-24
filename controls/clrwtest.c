
/* Program name:   Colours.C  Title: A Color-Full Example:              */
/*                                   Using Color In Control Design      */
/* OS/2 Developer Magazine, Issue:  Sept '93, page 46                   */
/* Author:  Chris Andrew WordPerfect Corp.                              */
/*          Mark Benge   IBM Corp.                                      */
/*          Matt Smith   Prominare Inc.                                 */
/* Description:  Colour Wheel sample control with threading.            */
/*                                                                      */
/* Program Requirements:  OS/2 2.x                                      */
/*                        IBM C Set/2                                   */
/*                        WATCOM C 386/9.0                              */
/*                        Borland C++ for OS/2                          */
/*                        Zortech C++ for OS/2                          */
/*                        OS/2 Toolkit                                  */

/************************************************************************/
/************************************************************************/
/*                     DISCLAIMER OF WARRANTIES.                        */
/************************************************************************/
/************************************************************************/
/*     The following [enclosed] code is library code created by the     */
/*     authors.  This source code is  provided to you solely            */
/*     for the purpose of assisting you in the development of your      */
/*     applications.  The code is provided "AS IS", without             */
/*     warranty of any kind.  The authors shall not be liable           */
/*     for any damages arising out of your use of the library code,     */
/*     even if they have been advised of the possibility of such        */
/*     damages.                                                         */
/************************************************************************/
/************************************************************************/

#define INCL_DOS                   /* Include OS/2 DOS Kernal           */
#define INCL_GPI                   /* Include OS/2 PM Windows Interface */
#define INCL_WIN                   /* Include OS/2 PM Windows Interface */

static char *PROGID = "@(#)colours.c:2.07";

#include <math.h>
#include <os2.h>
#include <string.h>
#include <stdio.h>

#include "clrwhl.h"

/* Equivalent command line invocation of each module using the          */
/* IBM C Set++ Compiler Version 2.0 is:                                 */
/*                                                                      */
/*     Icc -G3s- -O+ -C -W3 -Fofilename filename.C                      */

/* Filename:   Colours.C                                                */

/*  Version:   2.10                                                     */
/*  Created:   1990-12-29                                               */
/*  Revised:   1992-11-07                                               */


/* -------------------------------------------------------------------- */

/* --- Module Definitions --------------------------------------------- */

HPOINTER hptrWait;
HPOINTER hptrArrow;
HWND     hwndDriver;               /* Program Window Handle             */
HWND     hwndDriverFrame;          /* Program Frame Handle              */
HAB      hAB;                      /* Program Anchor Block Handle       */
HMQ      hmqDriver;                /* Program Message Queue Handle      */

/* --- Internal Structures -------------------------------------------- */

MRESULT EXPENTRY DriverWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#pragma subtitle("   Driver Client Window - Client Window Procedure")
#pragma page( )

/* --- DriverWndProc ------------------------------------------------ */
/*                                                                      */
/*     This function is used to process the messages for the main       */
/*     Client Window.                                                   */
/*                                                                      */
/*     Upon Entry:                                                      */
/*                                                                      */
/*     HWND   hWnd; = Window Handle                                     */
/*     ULONG  msg;  = PM Message                                        */
/*     MPARAM mp1;  = Message Parameter 1                               */
/*     MPARAM mp2;  = Message Parameter 2                               */
/*                                                                      */
/*     Upon Exit:                                                       */
/*                                                                      */
/*     EditWndProc = Message Handling Result                            */
/*                                                                      */
/* -------------------------------------------------------------------- */

MRESULT EXPENTRY DriverWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
CHAR        szStrBuf[64];          /* String Buffer                     */
RGB2        rgb2;                  /* RGB2 Structure                    */
HPS         hPS;                   /* Presentation Space Handle         */
RECTL       rcl;                   /* Client Window Rectangle           */
CLRWHLCDATA cwcd;                  /* Colour Wheel Data                 */

switch ( msg )
   {
                       /* Perform window initialization                 */
   case WM_CREATE :
       hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);
       WinSetPointer(HWND_DESKTOP, hptrWait = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));
       cwcd.cb = sizeof(CLRWHLCDATA);
       cwcd.lAngle         = 5L;
       cwcd.lSaturationInc = 5L;
       cwcd.lRadius        = 150L;
       WinCreateWindow(hWnd, WC_COLORWHEEL, "",
                       WS_VISIBLE | 
                       CWS_AUTOSIZE | CWS_BITMAP | CWS_THREADED | CWS_RGB,
                       50L, 50L, 400L, 400L, hWnd,
                       HWND_TOP, 10L, (PVOID)&cwcd, (PVOID)NULL);
       break;

   case WM_CONTROL :
       switch ( SHORT2FROMMP(mp1) )
           {
           case CWN_RGBCLRSELECTED :
               switch ( SHORT1FROMMP(mp1) )
                   {
                   case 10L :
                       memmove(&rgb2, &mp2, sizeof(RGB2));
                       sprintf(szStrBuf, "Red: %d  Green: %d  Blue: %d",
                               rgb2.bRed, rgb2.bGreen, rgb2.bBlue);
                       rcl.xLeft = rcl.yBottom = 10L;
                       rcl.yTop = 30L;
                       rcl.xRight = 300L;
                       WinDrawText(hPS = WinGetPS(hWnd), -1, szStrBuf,
                                   &rcl, CLR_BLACK, CLR_WHITE,
                                   DT_LEFT | DT_BOTTOM | DT_ERASERECT);
                       WinReleasePS(hPS);
                       break;
                   }
               break;
           }
       break;
                       /* Erase background                              */

   case WM_ERASEBACKGROUND :
       WinQueryWindowRect(hWnd, &rcl);
       WinFillRect((HPS)mp1, &rcl, SYSCLR_APPWORKSPACE);
       break;
                       /* Paint the main client window                  */
   case WM_PAINT :
       WinSetPointer(HWND_DESKTOP, hptrWait);

       WinFillRect(hPS = WinBeginPaint(hWnd, (HPS)NULL, &rcl), &rcl, SYSCLR_APPWORKSPACE);

       WinEndPaint(hPS);
       WinSetPointer(HWND_DESKTOP, hptrArrow);
       break;
                       /* Close Down                                    */
   case WM_CLOSE :

       WinPostMsg(hWnd, WM_QUIT, 0L, 0L);
       break;

   case WM_DESTROY :
       WinDestroyPointer(hptrArrow);
       WinDestroyPointer(hptrWait);
       break;
                       /* Default message processing                    */
   default:
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}
#pragma subtitle("   Program Controller")
#pragma page( )

/* --- Main Program Controller ---------------------------------------- */

INT main(INT argc, CHAR *argv[ ])

{
QMSG       qmsg;                   /* PM Message Queue Holder           */
ULONG      flCreateFlags;          /* Window Creation Flags             */

                       /* Initialize the program for PM and create the  */
                       /* message queue                                 */

WinSetCp(hmqDriver = WinCreateMsgQueue(hAB = WinInitialize(0), 0), 850);

                       /* Register the main program window class        */

if ( !WinRegisterClass(hAB, pszDriverClassName, (PFNWP)DriverWndProc,
                       CS_CLIPCHILDREN | CS_SYNCPAINT | CS_SIZEREDRAW, 0) )
   {
   return(1);
   }

if (!RegisterColorWheel(hAB))
   return 1;

                       /* Create the main program window but do not     */
                       /* show it yet                                   */

flCreateFlags = FCF_TITLEBAR | FCF_NOBYTEALIGN | FCF_SYSMENU | FCF_SIZEBORDER | FCF_SHELLPOSITION;
if ( !(hwndDriverFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
                                              &flCreateFlags,
                                              pszDriverClassName, NULL, 0L,
                                             (HMODULE)0L, 0L,
                                              &hwndDriver)) )
   {
   return(1);
   }
                       /* Retrieve and then dispatch messages           */

while ( WinGetMsg(hAB, &qmsg, (HWND)NULL, 0, 0) )
   WinDispatchMsg(hAB, &qmsg);

WinDestroyWindow(hwndDriverFrame);

WinDestroyMsgQueue(hmqDriver);

                       /* Notify PM that main program thread not needed */
                       /* any longer                                    */
WinTerminate(hAB);

return(0);
}
