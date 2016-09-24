
/* Program name: ClrWheel.C   Title: A Color-Full Example:            */
/*                             Using Color In Control Design      */
/* OS/2      Developer Magazine, Issue:  Sept '93, page 46                   */
/* Author:  Chris Andrew WordPerfect Corp.                        */
/*          Mark Benge       IBM Corp.                              */
/*          Matt Smith       Prominare Inc.                              */
/* Description:       Colour      Wheel sample control with threading.            */
/*                                                      */
/* Program Requirements:  OS/2 2.x                              */
/*                    IBM C      Set/2                              */
/*                    WATCOM C 386/9.0                        */
/*                    Borland C++ for OS/2                        */
/*                    Zortech C++ for OS/2                        */
/*                    OS/2 Toolkit                              */

/************************************************************************/
/************************************************************************/
/*                   DISCLAIMER OF WARRANTIES.                  */
/************************************************************************/
/************************************************************************/
/*     The following [enclosed]      code is      library      code created by      the      */
/*     authors.       This source code is  provided to you solely            */
/*     for the purpose of assisting you      in the development of your      */
/*     applications.  The code is provided "AS IS", without            */
/*     warranty      of any kind.  The authors shall      not be liable            */
/*     for any damages arising out of your use of the library code,      */
/*     even if they have been advised of the possibility of such      */
/*     damages.                                                */
/************************************************************************/
/************************************************************************/
      
#define      INCL_DOS               /* Include OS/2 DOS Kernal            */
#define      INCL_GPI               /* Include OS/2 PM GPI Interface      */
#define      INCL_WIN               /* Include OS/2 PM Windows Interface      */

#include <memory.h>
#include <math.h>
#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include "colorwheel.h"
#include "clrwhl.h"

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))

/* Equivalent command line invocation (C Set++):                  */
/*                                                      */
/*     Icc -C -G3e -O+ -W3 ClrWheel.C                              */

/* Filename:   ClrWheel.C                                    */

/*  Version:   2.10                                          */
/*  Created:   1992-11-28                                    */
/*  Revised:   1993-03-02                                    */

/* Routines:   LONG lHSBtoRGB(LONG lHue, LONG lSaturation);            */
/*             HBITMAP hbmCreateBitmap(PCLRWHEEL pcw);                  */
/*             VOID DrawClrWheel(HPS hPS, PCLRWHEEL pcw);            */
/*             VOID _System BitmapConstructThread(PCLRWHEEL pcw);      */
/*             MRESULT EXPENTRY      ClrWheelWndProc(HWND hWnd, ULONG msg,      */
/*                                    MPARAM mp1, MPARAM mp2);*/


/************************************************************************/
/************************************************************************/
/*                                                      */
/* Styles supported:                                          */
/*                                                      */
/*     CWS_SOLIDCLR    : Uses solid colours when rendering            */
/*     CWS_HSB             : Colour      Notification in      HSB                  */
/*                   CWN_HSBCLRSELECTED                        */
/*     CWS_RGB             : Colour      Notification in      RGB                  */
/*                   CWN_RGBCLRSELECTED                        */
/*     CWS_BITMAP      : Use bitmap for      render image of      wheel            */
/*     CWS_AUTOSIZE    : Radius      determined by control                  */
/*     CWS_THREADED    : Use threading when creating bitmap            */
/*                                                      */
/* Notification      messages:                                    */
/*                                                      */
/*     CWN_RGBCLRSELECTED : mp2      : RGB2 Value                        */
/*         RGB colour selected (based on style CWS_RGB)                  */
/*     CWN_HSBCLRSELECTED : mp2      : Low Short = Hue       (Angle)      */
/*                          Hi  Short = Saturation (Radius)      */
/*         HSB colour selected (based on style CWS_HSB)                  */
/*     CWN_XHAIRSELECTED  : mp2      : 0L                              */
/*         Cross hair selected (button 1 down event)                  */
/*     CWN_XHAIRRELEASED  : mp2      : 0L                              */
/*         Cross hair released (button 1 up event)                  */
/*                                                      */
/* Control messages:                                          */
/*                                                      */
/*     CWM_QUERYXHAIR         : Query cross hair location                  */
/*         Input:                                          */
/*               mp1 = 0L;                                    */
/*               mp2 = 0L;                                    */
/*         Ouptut:                                          */
/*               x = SHORT1FROMMR(mr);                        */
/*               y = SHORT2FROMMR(mr);                        */
/*     CWM_SETXHAIR         : Set cross hair location                  */
/*         Input:                                          */
/*               mp1 = x;                                    */
/*               mp2 = y;                                    */
/*         Ouptut:                                          */
/*               mr =      TRUE  :      position accepted and cross hair      */
/*                        updated                              */
/*               mr =      FALSE :      invalid      position specified, cross hair      */
/*                        not updated                        */
/*     CWM_QUERYRGBCLR         : Query current RGB colour under cross hair      */
/*         Input:                                          */
/*               mp1 = 0L;                                    */
/*               mp2 = 0L;                                    */
/*         Ouptut:                                          */
/*               mr =      RGB2 rgb2  : RGB Colour                        */
/*     CWM_SETRGBCLR         : Set cross hair location using RGB colour      */
/*         Input:                                          */
/*               mp1 = RGB2 rgb2;                              */
/*               mp2 = 0L;                                    */
/*         Ouptut:                                          */
/*               x = SHORT1FROMMR(mr);                        */
/*               y = SHORT2FROMMR(mr);                        */
/*     CWM_QUERYHSBCLR         : Query current RGB colour under cross hair      */
/*         Input:                                          */
/*               mp1 = 0L;                                    */
/*               mp2 = 0L;                                    */
/*         Ouptut:                                          */
/*               sHue             = SHORT1FROMMR(mr);                  */
/*               sSaturation = SHORT2FROMMR(mr);                  */
/*     CWM_SETHSBCLR         : Set cross hair location using HSB colour      */
/*         Input:                                          */
/*               mp1 = lHue;                                    */
/*               mp2 = lSaturation;                              */
/*         Ouptut:                                          */
/*               x = SHORT1FROMMR(mr);                        */
/*               y = SHORT2FROMMR(mr);                        */
/*                                                      */
/************************************************************************/
/************************************************************************/

static LONG      lHSBtoRGB(LONG lHue, LONG lSaturation);
int _CRT_init(void);
void _CRT_term(void);
ULONG _System _DLL_InitTerm(HMODULE ulModHandle, ULONG ulFlag);

static HMODULE hmodThisModule;


#pragma      subtitle("   Colour Wheel - HSB to RGB Conversion Function")
#pragma      page( )

/* --- lHSBtoRGB --------------------------------------      [ Private ] ---      */
/*                                                      */
/*     This function is      used to      determine the RGB value      for a given      */
/*     hue and saturation.  The      function is based upon the algorithm      */
/*     presented in the      Foly and van Dam book "Computer Graphics:       */
/*     Principles and Practice" Second Addition, page 593 Figure        */
/*     13.34.  The routine has been adapted to ignore the brightness      */
/*     since it      will be      constant within      this implementation.            */
/*     The Hue value corresponds to the      angle and the Saturation      */
/*     corresponds to the radius of the      circle.       The Hue value is      */
/*     from 0 to 360ø and the Saturation value is from 0 to 100.  The      */
/*     RGB values are defined by the system to be from 0 to 255.      */
/*                                                      */
/*     Upon Entry:                                          */
/*                                                      */
/*     LONG lHue;       = Hue Value                              */
/*     LONG lSaturation; = Saturation Value                        */
/*                                                      */
/*     Upon Exit:                                          */
/*                                                      */
/*     lHSBtoRGB = Resultant RGB Colour                              */
/*                                                      */
/* --------------------------------------------------------------------      */

static LONG lHSBtoRGB(LONG lHue, LONG lSaturation)

{
RGB2   rgb2;                     /* RGB Colour Holder                  */
PLONG  plClr = (PLONG)&rgb2;         /* Long Pointer to RGB Colour Holder      */
            
                   /* Initialize the options component of the RGB      */
                   /* holder since it is a reserved      value            */
rgb2.fcOptions = 0;
                   /* Check      to see if the saturation level is 0      */
                   /* in which case      the colour should be white      */
if ( lSaturation == 0 )
   {
   rgb2.bRed   =
   rgb2.bBlue  =
   rgb2.bGreen = 255;
   }
else
   {
                   /* Check      to see if the hue is at      its maximum      */
                   /* value      in which case the hue should revert to      */
                   /* its lower limit                        */
   if (      lHue ==      360L )
       lHue = 0L;
                   /* Break      the hue      into 6 wedges which corresponds      */
                   /* to red, yellow, green, cyan, blue and      magenta      */
   switch ( lHue / 60L )
       {
                   /* Red wedge                              */
       case 0 :
         rgb2.bRed   = (BYTE)255;
         rgb2.bGreen = (BYTE)((((100 - ((lSaturation * (100 -      ((lHue % 60L) *      100L) /      60)) / 100L))) * 255L) / 100L);
         rgb2.bBlue  = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         break;
                   /* Yellow wedge                              */
       case 1 :
         rgb2.bRed   = (BYTE)((((100 - ((lSaturation * ((lHue      % 60L) * 100L) / 60) / 100L))) * 255L) / 100L);
         rgb2.bGreen = (BYTE)255;
         rgb2.bBlue  = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         break;
                   /* Green      wedge                              */
       case 2 :
         rgb2.bRed   = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         rgb2.bGreen = (BYTE)255;
         rgb2.bBlue  = (BYTE)((((100 - ((lSaturation * (100 -      ((lHue % 60L) *      100L) /      60)) / 100L))) * 255L) / 100L);
         break;
                   /* Cyan wedge                              */
       case 3 :
         rgb2.bRed   = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         rgb2.bGreen = (BYTE)((((100 - ((lSaturation * ((lHue      % 60L) * 100L) / 60) / 100L))) * 255L) / 100L);
         rgb2.bBlue  = (BYTE)255;
         break;
                   /* Blue wedge                              */
       case 4 :
         rgb2.bRed   = (BYTE)((((100 - ((lSaturation * (100 -      ((lHue % 60L) *      100L) /      60)) / 100L))) * 255L) / 100L);
         rgb2.bGreen = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         rgb2.bBlue  = (BYTE)255;
         break;
                   /* Magenta wedge                              */
       case 5 :
         rgb2.bRed   = (BYTE)255;
         rgb2.bGreen = (BYTE)(((100 -      lSaturation) * 255L) / 100L);
         rgb2.bBlue  = (BYTE)((((100 - ((lSaturation * ((lHue      % 60L) * 100L) / 60) / 100L))) * 255L) / 100L);
         break;
       }
   }
return(*plClr);
}
#pragma      subtitle("   Colour Wheel - Control Window Procedure")
#pragma      page( )

/* --- ClrWheelWndProc ------------------------------------------------      */
/*                                                      */
/*     This function is      used to      process      the messages for the Colour      */
/*     Wheel control window.  It should      be noted that all angles      */
/*     when used with sin and cosine functions are in radians.            */
/*                                                      */
/*     Upon Entry:                                          */
/*                                                      */
/*     HWND   hWnd; = Window Handle                              */
/*     ULONG  msg;  = PM Message                              */
/*     MPARAM mp1;  = Message Parameter      1                        */
/*     MPARAM mp2;  = Message Parameter      2                        */
/*                                                      */
/*     Upon Exit:                                          */
/*                                                      */
/*     ClrWheelWndProc = Message Handling Result                  */
/*                                                      */
/* --------------------------------------------------------------------      */

MRESULT      EXPENTRY ClrWheelWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
BOOL            fReturn;               /* Message Return Flag            */
HPS            hPS;               /* Presentation Space Handle            */
LONG            lMax;               /* Maximum Value                  */
LONG            lMin;               /* Minimum Value                  */
LONG            lAngle;               /* Angle Holder                  */
LONG            lRadius;               /* Radius Holder                  */
PCLRWHEEL     pcw;               /* Colour Wheel Internal Data Pointer*/
PCLRWHLCDATA  pcwd;               /* Colour Wheel Data      Pointer            */
PCREATESTRUCT pcrst;               /* Create Structure Pointer            */
POINTL            ptl;               /* Display Point                  */
PRGB2            prgb2;               /* RGB2 Structure Pointer            */
double            rdAngle;               /* Angle                        */
register INT x,      y;               /* Loop Counter                  */

switch ( msg )
   {

/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      1: Control creation coding                              */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control creation                                          */
   /*********************************************************************/

   case      WM_CREATE :
                   /* Get the address of the CTLDATA structure that      */
                   /* may contain the bitmap information that the      */
                   /* control can use during its creation instead      */
                   /* of using messages to set the button images      */

       if ( pcwd = (PCLRWHLCDATA)PVOIDFROMMP(mp1) )

                   /* Check      to see that the      structure passed is      */
                   /* what is expected and if not, return            */
                   /* indicating that the control window should      */
                   /* not be further created                  */

                   /*************************************************/
                   /*  NOTE:   OS/2      2.0 requires the first element      */
                   /*         of the CTLDATA structure to be the      */
                   /*         size      of the structure and this      */
                   /*         value must be less than 64 KB      */
                   /*************************************************/

         if (      (pcwd->cb != sizeof(CLRWHLCDATA)) )
             return(MRFROMLONG(TRUE));

                   /* Allocate memory for internal control data      */

       DosAllocMem((PPVOID)&pcw, sizeof(CLRWHEEL),
               PAG_READ | PAG_WRITE      | PAG_COMMIT);

                   /* Save the address of the internal control data      */
                   /* in the control's reserved memory to allow it  */
                   /* to be      referenced as required by the control      */

       WinSetWindowPtr(hWnd, QUCWP_WNDP, (PVOID)pcw);

                   /* Get the control's creation structure address  */
                   /* to copy the relevant information such      as the      */
                   /* size,      position and text of the control into      */
                   /* the internal control data                  */

       pcrst = (PCREATESTRUCT)PVOIDFROMMP(mp2);

                   /* Save the owner and parent of the control so      */
                   /* notification messages      can be sent back to      */
                   /* the proper locations within the owning      */
                   /* application                              */

       pcw->hWnd       = hWnd;
       pcw->hwndOwner  = pcrst->hwndOwner;
       pcw->hwndParent = pcrst->hwndParent;

                   /* Save the ID of the control, style and      save      */
                   /* the default style along with the normal      */
                   /* arrow      pointer      handle which will be used when      */
                   /* the pointer passes over the control            */

       pcw->id             = pcrst->id;
       pcw->flStyle    = pcrst->flStyle;
       pcw->hptrArrow  = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW,
                                  FALSE);

                   /* Check      to see if the style for      the control      */
                   /* indicates that pure colours (solid) should      */
                   /* be used                              */

       if ( pcrst->flStyle & CWS_SOLIDCLR )
         pcw->ulOptions = LCOL_PURECOLOR;

                   /* Save the initial size      of the control            */

       pcw->rcl.xRight      = pcrst->cx;
       pcw->rcl.yTop      = pcrst->cy;
       pcw->cx            = pcrst->cx;
       pcw->cy            = pcrst->cy;

                   /* Save the co-ordinates      of the rectangle for      */
                   /* the bitmap display if      it is required            */

       pcw->aptl[1].x =      pcw->aptl[3].x = pcrst->cx - 1L;
       pcw->aptl[1].y =      pcw->aptl[3].y = pcrst->cy - 1L;

                   /* Calculate the      origin point for the circle      */

       pcw->ptlOrigin.x      = (pcrst->cx - 2L) / 2L;
       pcw->ptlOrigin.y      = (pcrst->cy - 2L) / 2L;

       pcw->aptlXHair[0].x = 0L;
       pcw->aptlXHair[0].y = pcw->ptlOrigin.y;
       pcw->aptlXHair[1].x = pcrst->cx - 1L;
       pcw->aptlXHair[1].y = pcw->ptlOrigin.y;
       pcw->aptlXHair[2].x = pcw->ptlOrigin.x;
       pcw->aptlXHair[2].y = 0L;
       pcw->aptlXHair[3].x = pcw->ptlOrigin.x;
       pcw->aptlXHair[3].y = pcw->cy - 1L;

                   /* Determine if CTLDATA was specified for the      */
                   /* control and if the case, get the data      and      */
                   /* set up the control internal values            */
       if ( pcwd )
         {
                   /* Determine if the angle is within the defined      */
                   /* range      for the      colour wheel                  */

         if (      (pcwd->lAngle <      1L) || (pcwd->lAngle > 45L)  )

                   /* Specified angle for the wedges is outside      */
                   /* the defined limits for the colour wheel, set      */
                   /* the angle to the default value of 10ø            */

             pcw->lAngle = 10L;
         else
                   /* Valid      value specified, save the angle            */
                   /* within the internal data structure            */

             pcw->lAngle = pcwd->lAngle;

                   /* Determine if the saturation increment      outside      */
                   /* the defined limits for the colour wheel      */

         if (      (pcwd->lSaturationInc <      1L) || (pcwd->lSaturationInc > 25L) )

                   /* Specified saturation increment outside the      */
                   /* defined limits for the colour      wheel, set the      */
                   /* saturation increment to 10                  */

             pcw->lSaturationInc = 10L;
         else
                   /* Valid      value specified, save the saturation      */
                   /* within the internal data structure            */

             pcw->lSaturationInc = pcwd->lSaturationInc;

                   /* Determine if the radius specified is within      */
                   /* the limits of      the control size and is      not      */
                   /* negative                              */

         if (      ((pcwd->lRadius      < 1L) || (pcwd->lRadius      > pcrst->cx) ||
            (pcwd->lRadius > pcrst->cy)) ||      (pcw->flStyle &      CWS_AUTOSIZE) )

                   /* Invalid size given or      autosizing requested,      */
                   /* form the radius for the wheel                  */

             if ( pcrst->cx <      pcrst->cy )
               pcw->lRadius      = (pcrst->cx - 2L) / 2L;
             else
               pcw->lRadius      = (pcrst->cy - 2L) / 2L;
         else
                   /* Valid      radius given, save the value internally      */

             pcw->lRadius = pcwd->lRadius;
         }
       else
         {
                   /* No CTLDATA specified,      use default angle and      */
                   /* saturation.  For the radius, use the best      */
                   /* fit for the width or height                  */

         pcw->lAngle             = 10L;
         pcw->lSaturationInc = 10L;
         if (      pcrst->cx < pcrst->cy )
             pcw->lRadius = pcw->ptlOrigin.x;
         else
             pcw->lRadius = pcw->ptlOrigin.y;
         }
       /* Bitmap laden */
       hPS = WinGetPS(hWnd);
       pcw->hbm      = GpiLoadBitmap(hPS, hmodThisModule, 1, 0, 0);
       WinReleasePS(hPS);
       break;

/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      2: Control sizing                                    */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control changing      size                                    */
   /*********************************************************************/

   case      WM_SIZE      :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Save the new sizes within the      control      info      */

       pcw->rcl.xRight      = SHORT1FROMMP(mp2);
       pcw->rcl.yTop      = SHORT2FROMMP(mp2);
       pcw->cx            = SHORT1FROMMP(mp2);
       pcw->cy            = SHORT2FROMMP(mp2);
       pcw->ptlOrigin.x      = SHORT1FROMMP(mp2) / 2L;
       pcw->ptlOrigin.y      = SHORT2FROMMP(mp2) / 2L;

       pcw->aptl[1].x =      pcw->aptl[3].x = SHORT1FROMMP(mp2) - 1L;
       pcw->aptl[1].y =      pcw->aptl[3].y = SHORT2FROMMP(mp2) - 1L;
       if ( pcw->flStyle & CWS_AUTOSIZE      )
         if (      pcw->cx      < pcw->cy )
             pcw->lRadius = pcw->ptlOrigin.x;
         else
             pcw->lRadius = pcw->ptlOrigin.y;
       break;

/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      3: Mouse input interface                              */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Mouse Button 1 selection                                    */
   /*********************************************************************/

   case      WM_BUTTON1DOWN :
                   /* Get the current mouse      pointer      position and      */
                   /* place      within the test      point structure            */

       ptl.x = SHORT1FROMMP(mp1);
       ptl.y = SHORT2FROMMP(mp1);

                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Set up the arc parameters                  */

       pcw->ap.lP = pcw->ap.lQ = pcw->lRadius;
       GpiSetArcParams(hPS = WinGetPS(hWnd), &pcw->ap);

                   /* Create the clipping path to encompass      just      */
                   /* the colour wheel itself.  To do this,      open a      */
                   /* normal path and draw a circle      into the path      */
                   /* that has the same radius as the colour wheel.      */

       GpiBeginPath(hPS, 1L);
       GpiMove(hPS, &pcw->ptlOrigin);
       GpiFullArc(hPS, DRO_OUTLINE, MAKEFIXED(1, 0));
       GpiEndPath(hPS);
                   /* Circle complete, convert the normal path to a      */
                   /* clip path to allow the determination of the      */
                   /* mouse      button click within the      colour wheel or      */
                   /* outside the edge of the wheel      but still      */
                   /* within the limits of the control itself      */

       GpiSetClipPath(hPS, 1L, SCP_ALTERNATE | SCP_AND);

                   /* Check      to see if the mouse pointer button      */
                   /* click      within the confines of the colour      */
                   /* wheel                                    */

       if ( GpiPtVisible(hPS, &ptl) == PVIS_VISIBLE )
         {
                   /* Mouse      click within the colour      wheel, capture      */
                   /* the mouse for      this control only until      the      */
                   /* button 1 of the mouse      is released            */

         WinSetCapture(HWND_DESKTOP, hWnd);
         pcw->fCapture = TRUE;

                   /* Send notification message to control owner      */
                   /* that cross hair has been selected            */
                   /*                                    */
                   /* Notification:      CWN_XHAIRSELECTED            */
                   /* mp2:            N/A                        */

         WinSendMsg(pcw->hwndOwner, WM_CONTROL,
                  MPFROM2SHORT(pcw->id, CWN_XHAIRSELECTED),      0L);

                   /* Set the mix mode for line invert            */

         GpiSetMix(hPS, FM_INVERT);

                   /* Erase      the current cross hair by drawing it      */
                   /* again      which will cause the previous colours      */
                   /* to be      displayed since      it is being drawn in      */
                   /* invert mix mode                        */

         GpiMove(hPS,      pcw->aptlXHair);
         GpiLine(hPS,      &pcw->aptlXHair[1]);

         GpiMove(hPS,      &pcw->aptlXHair[2]);
         GpiLine(hPS,      &pcw->aptlXHair[3]);

                   /* Save the new position      of the mouse pointer      */
                   /* within the cross hair      point array            */

         pcw->aptlXHair[0].y =
         pcw->aptlXHair[1].y = SHORT2FROMMP(mp1);
         pcw->aptlXHair[2].x =
         pcw->aptlXHair[3].x = SHORT1FROMMP(mp1);

                   /* Draw the new cross hair within the invert      */
                   /* mix mode                              */

         GpiMove(hPS,      pcw->aptlXHair);
         GpiLine(hPS,      &pcw->aptlXHair[1]);

         GpiMove(hPS,      &pcw->aptlXHair[2]);
         GpiLine(hPS,      &pcw->aptlXHair[3]);
         }
                   /* Release the controls presentation space      */
       WinReleasePS(hPS);
       break;

   /*********************************************************************/
   /*  Mouse Button 1 release                                    */
   /*********************************************************************/

   case      WM_BUTTON1UP :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Determine if the control in capture mode in      */
                   /* case,      the current position of      the mouse      */
                   /* pointer needs      to be checked and the colour      */
                   /* determined                              */

       if ( pcw->fCapture )
         {
                   /* Release the mouse capture to allow other      */
                   /* windows the ability of receiving mouse input      */

         WinSetCapture(HWND_DESKTOP, (HWND)NULL);
         pcw->fCapture = FALSE;

                   /* Send notification message to control owner      */
                   /* that cross hair has been released            */
                   /*                                    */
                   /* Notification:      CWN_XHAIRRELEASED            */
                   /* mp2:            N/A                        */

         WinSendMsg(pcw->hwndOwner, WM_CONTROL,
                  MPFROM2SHORT(pcw->id, CWN_XHAIRRELEASED),      0L);

                   /* Check      to see if notification of RGB or HSB      */
                   /* colour selected required                  */

         if (      pcw->flStyle & (CWS_RGB      | CWS_HSB) )
             {
                   /* Check      to see if the mouse pointer at the      */
                   /* origin of the      colour wheel in      the horizontal      */
                   /* direction                              */

             if ( pcw->aptlXHair[2].x      == pcw->ptlOrigin.x )

                   /* Mouse      pointer      aligning horizontally at zero      */
                   /* in the colour      wheel.      Need to      specify      in      */
                   /* radians the angle in which the cross is      */
                   /* resting.  Check to see if the      y position      */
                   /* positive which indicates the angle is      90ø      */
                   /* otherwise it is 270ø.                        */

               if (      (lRadius = pcw->aptlXHair[0].y - pcw->ptlOrigin.y) < 0 )
                   rdAngle = 4.712388980;
               else
                   rdAngle = 1.570796327;
             else
                   /* Check      to see if the mouse pointer at the      */
                   /* origin of the      colour wheel in      the vertical      */
                   /* direction                              */

               if (      pcw->aptlXHair[0].y == pcw->ptlOrigin.y      )

                   /* Mouse      pointer      aligning horizontally at zero      */
                   /* in the colour      wheel.      Need to      specify      in      */
                   /* radians the angle in which the cross is      */
                   /* resting.  Check to see if the      y position      */
                   /* positive which indicates the angle is      0ø      */
                   /* otherwise it is 180ø.                        */

                   if ( (lRadius = pcw->aptlXHair[2].x - pcw->ptlOrigin.x) < 0 )
                     rdAngle = 3.141592654;
                   else
                     rdAngle = 0.0;
               else
                   {
                   /* Determine the      absolute x and y co-ordinates      */
                   /* based      upon cartesian system.      All trig      */
                   /* functions are      within the 0 to      90ø range.      */

                   if ( pcw->aptlXHair[0].y      < pcw->ptlOrigin.y )
                     y = pcw->ptlOrigin.y      - pcw->aptlXHair[0].y;
                   else
                     y = pcw->aptlXHair[0].y - pcw->ptlOrigin.y;

                   if ( pcw->aptlXHair[2].x      < pcw->ptlOrigin.x )
                     x = pcw->ptlOrigin.x      - pcw->aptlXHair[2].x;
                   else
                     x = pcw->aptlXHair[2].x - pcw->ptlOrigin.x;

                   /* Calculate the      radius from the      x and y            */
                   /* position of the mouse      using trig and right      */
                   /* angle      geometry                        */

                   lRadius = (LONG)((double)y / sin(rdAngle      = atan2((double)y, (double)x)));

                   /* Determine the      quadrant that the mouse      pointer      */
                   /* was in to be able to form correct angle      */

                   if ( pcw->aptlXHair[0].y      < pcw->ptlOrigin.y )
                     if (      pcw->aptlXHair[2].x > pcw->ptlOrigin.x )
                         rdAngle = 6.283185307 - rdAngle;
                     else
                         rdAngle += 3.141592654;
                   else
                     if (      pcw->aptlXHair[2].x < pcw->ptlOrigin.x )
                         rdAngle = 3.141592654 - rdAngle;
                   }
                   /* When the style of the      control      requests RGB      */
                   /* notification,      convert      the hue      and saturation      */
                   /* (angle and radius) to      RGB and      create a      */
                   /* notification message package that is sent      */
                   /* back to the owner.                        */
                   /*                                    */
                   /* Notification:      CWN_RGBCLRSELECTED            */
                   /* mp2:            RGB2 value                  */

             if ( pcw->flStyle & CWS_RGB )
               WinSendMsg(pcw->hwndOwner, WM_CONTROL,
                        MPFROM2SHORT(pcw->id, CWN_RGBCLRSELECTED),
                        MPFROMLONG(lHSBtoRGB((LONG)((rdAngle * 360.0) / 6.283185307),
                               lRadius * 100 / pcw->lRadius)));

                   /* When the style of the      control      requests HSB      */
                   /* notification,      create a notification message      */
                   /* package that is sent back to the owner.      */
                   /*                                    */
                   /* Notification:      CWN_HSBCLRSELECTED            */
                   /* mp2:            Low Short = Hue      (Angle)            */
                   /*            Hi  Short = Saturation (Radius)      */

             if ( pcw->flStyle & CWS_HSB )
               WinSendMsg(pcw->hwndOwner, WM_CONTROL,
                        MPFROM2SHORT(pcw->id, CWN_HSBCLRSELECTED),
                        MPFROMLONG(MAKELONG((LONG)rdAngle, lRadius * 100 / pcw->lRadius)));
             }
         }
       break;

   /*********************************************************************/
   /*  Mouse moving                                          */
   /*********************************************************************/

   case      WM_MOUSEMOVE :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Check      to see if in capture mode and if the      */
                   /* case,      need to      move the cross hair that is      */
                   /* used to denote the current colour selected      */

       if ( pcw->fCapture )
         {
                   /* Get the current mouse      pointer      position and      */
                   /* place      within the test      point structure            */

         ptl.x = SHORT1FROMMP(mp1);
         ptl.y = SHORT2FROMMP(mp1);

                   /* Set up the arc parameters                  */

         pcw->ap.lP =      pcw->ap.lQ = pcw->lRadius;
         GpiSetArcParams(hPS = WinGetPS(hWnd), &pcw->ap);

                   /* Create the clipping path to encompass      just      */
                   /* the colour wheel itself.  To do this,      open a      */
                   /* normal path and draw a circle      into the path      */
                   /* that has the same radius as the colour wheel.      */

         GpiBeginPath(hPS, 1L);
         GpiMove(hPS,      &pcw->ptlOrigin);
         GpiFullArc(hPS, DRO_OUTLINE,      MAKEFIXED(1, 0));
         GpiEndPath(hPS);

                   /* Circle complete, convert the normal path to a      */
                   /* clip path to allow the determination of the      */
                   /* mouse      button click within the      colour wheel or      */
                   /* outside the edge of the wheel      but still      */
                   /* within the limits of the control itself      */

         GpiSetClipPath(hPS, 1L, SCP_ALTERNATE | SCP_AND);

                   /* Check      to see if the mouse pointer button      */
                   /* click      within the confines of the colour      */
                   /* wheel                                    */

         if (      GpiPtVisible(hPS, &ptl)      == PVIS_VISIBLE      )
             {
                   /* Set the mix mode for line invert            */

             GpiSetMix(hPS, FM_INVERT);

                   /* Erase      the current cross hair by drawing it      */
                   /* again      which will cause the previous colours      */
                   /* to be      displayed since      it is being drawn in      */
                   /* invert mix mode                        */

             GpiMove(hPS, pcw->aptlXHair);
             GpiLine(hPS, &pcw->aptlXHair[1]);

             GpiMove(hPS, &pcw->aptlXHair[2]);
             GpiLine(hPS, &pcw->aptlXHair[3]);

                   /* Save the new position      of the mouse pointer      */
                   /* within the cross hair      point array            */

             pcw->aptlXHair[0].y =
             pcw->aptlXHair[1].y = SHORT2FROMMP(mp1);
             pcw->aptlXHair[2].x =
             pcw->aptlXHair[3].x = SHORT1FROMMP(mp1);

                   /* Draw the new cross hair within the invert      */
                   /* mix mode                              */

             GpiMove(hPS, pcw->aptlXHair);
             GpiLine(hPS, &pcw->aptlXHair[1]);

             GpiMove(hPS, &pcw->aptlXHair[2]);
             GpiLine(hPS, &pcw->aptlXHair[3]);
             }
                   /* Release the controls presentation space      */

         WinReleasePS(hPS);
         }
                   /* Set the mouse      pointer      to the arrow shape      */
                   /* while      the mouse within the control            */

       WinSetPointer(HWND_DESKTOP, pcw->hptrArrow);
       break;

/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      4: Painting and      display                                    */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Paint control                                          */
   /*********************************************************************/

   case      WM_PAINT :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

       hPS = WinBeginPaint(hWnd, (HPS)NULL, (PRECTL)NULL);

                   /* Check      to see if the control is threaded and      */
                   /* has a      bitmap display                        */

       if ( (pcw->flStyle & (CWS_BITMAP      | CWS_THREADED)) == (CWS_BITMAP      | CWS_THREADED)      )

                   /* Threaded version of the control being      used,      */
                   /* check      to make      sure that the thread is      active      */
                   /* which      indicates that the bitmap is still      */
                   /* being      constructed and      that the outline of the      */
                   /* should only be shown.       This acts like      a      */
                   /* prevent semaphore except that      no blocking on      */
                   /* the semaphore      occurs as this would block the      */
                   /* rest of the users PM interface.            */

       if ( pcw->hbm )
       {
         RECTL rclDest;

         WinQueryWindowRect(hWnd, &rclDest);
         WinDrawBitmap(hPS, pcw->hbm, NULL, (PPOINTL)&rclDest, 0, 0, DBM_NORMAL | DBM_STRETCH);
       }

                   /* Set up the arc parameters for      the clipping      */
                   /* path                                    */

       pcw->ap.lP = pcw->ap.lQ = pcw->lRadius;
       GpiSetArcParams(hPS, &pcw->ap);

                   /* Create the clipping path to encompass      just      */
                   /* the colour wheel itself.  To do this,      open a      */
                   /* normal path and draw a circle      into the path      */
                   /* that has the same radius as the colour wheel.      */

       GpiBeginPath(hPS, 1L);
       GpiMove(hPS, &pcw->ptlOrigin);
       GpiFullArc(hPS, DRO_OUTLINE, MAKEFIXED(1, 0));
       GpiEndPath(hPS);
                   /* Circle complete, convert the normal path to a      */
                   /* clip path to allow the determination of the      */
                   /* mouse      button click within the      colour wheel or      */
                   /* outside the edge of the wheel      but still      */
                   /* within the limits of the control itself      */

       GpiSetClipPath(hPS, 1L, SCP_ALTERNATE | SCP_AND);

                   /* Set the mix mode for line invert            */

       GpiSetMix(hPS, FM_INVERT);

                   /* Draw the new cross hair within the invert      */
                   /* mix mode                              */

       GpiMove(hPS, pcw->aptlXHair);
       GpiLine(hPS, &pcw->aptlXHair[1]);

       GpiMove(hPS, &pcw->aptlXHair[2]);
       GpiLine(hPS, &pcw->aptlXHair[3]);
       WinEndPaint(hPS);
       break;

/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      5: Control defined message handling                        */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Cross-hair position query                              */
   /*********************************************************************/
   /*********************************************************************/
   /*                                                      */
   /*  Message:                                                */
   /*             CWM_QUERYXHAIR                                    */
   /*                                                      */
   /*  Input:                                                */
   /*             mp1 = 0L;                                    */
   /*             mp2 = 0L;                                    */
   /*                                                      */
   /*  Return:                                                */
   /*             x = SHORT1FROMMR(mr);                              */
   /*             y = SHORT2FROMMR(mr);                              */
   /*                                                      */
   /*********************************************************************/

   case      CWM_QUERYXHAIR :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Return the cross position with the x in the      */
                   /* lower      portion      and the      y in the upper portion      */
                   /* of the return      value                        */

       return(MRFROMLONG(MAKELONG((USHORT)pcw->aptlXHair[2].x, (USHORT)pcw->aptlXHair[0].y)));

   /*********************************************************************/
   /*  Cross-hair position set                                    */
   /*********************************************************************/
   /*********************************************************************/
   /*                                                      */
   /*  Message:                                                */
   /*             CWM_SETXHAIR                                    */
   /*                                                      */
   /*  Input:                                                */
   /*             mp1 = x;                                          */
   /*             mp2 = y;                                          */
   /*                                                      */
   /*  Return:                                                */
   /*             TRUE  : position      accepted and cross hair      updated            */
   /*             FALSE : invalid position      specified, cross hair not      */
   /*                   updated                                    */
   /*                                                      */
   /*********************************************************************/

   case      CWM_SETXHAIR :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Get the position and place within the      test      */
                   /* point      structure                        */

       ptl.x = LONGFROMMP(mp1);
       ptl.y = LONGFROMMP(mp2);

                   /* Set up the arc parameters                  */

       pcw->ap.lP = pcw->ap.lQ = pcw->lRadius;
       GpiSetArcParams(hPS = WinGetPS(hWnd), &pcw->ap);

                   /* Create the clipping path to encompass      just      */
                   /* the colour wheel itself.  To do this,      open a      */
                   /* normal path and draw a circle      into the path      */
                   /* that has the same radius as the colour wheel.      */

       GpiBeginPath(hPS, 1L);
       GpiMove(hPS, &pcw->ptlOrigin);
       GpiFullArc(hPS, DRO_OUTLINE, MAKEFIXED(1, 0));
       GpiEndPath(hPS);
                   /* Circle complete, convert the normal path to a      */
                   /* clip path to allow the determination of the      */
                   /* mouse      button click within the      colour wheel or      */
                   /* outside the edge of the wheel      but still      */
                   /* within the limits of the control itself      */

       GpiSetClipPath(hPS, 1L, SCP_ALTERNATE | SCP_AND);

                   /* Check      to see if the mouse pointer button      */
                   /* click      within the confines of the colour      */
                   /* wheel                                    */

       if ( GpiPtVisible(hPS, &ptl) == PVIS_VISIBLE )
         {
                   /* Set the mix mode for line invert            */

         GpiSetMix(hPS, FM_INVERT);

                   /* Erase      the current cross hair by drawing it      */
                   /* again      which will cause the previous colours      */
                   /* to be      displayed since      it is being drawn in      */
                   /* invert mix mode                        */

         GpiMove(hPS,      pcw->aptlXHair);
         GpiLine(hPS,      &pcw->aptlXHair[1]);

         GpiMove(hPS,      &pcw->aptlXHair[2]);
         GpiLine(hPS,      &pcw->aptlXHair[3]);

                   /* Save the new position      of the mouse pointer      */
                   /* within the cross hair      point array            */

         pcw->aptlXHair[0].y =
         pcw->aptlXHair[1].y = LONGFROMMP(mp2);
         pcw->aptlXHair[2].x =
         pcw->aptlXHair[3].x = LONGFROMMP(mp1);

                   /* Draw the new cross hair within the invert      */
                   /* mix mode                              */

         GpiMove(hPS,      pcw->aptlXHair);
         GpiLine(hPS,      &pcw->aptlXHair[1]);

         GpiMove(hPS,      &pcw->aptlXHair[2]);
         GpiLine(hPS,      &pcw->aptlXHair[3]);
         fReturn = TRUE;
         }
       else
         fReturn = FALSE;

                   /* Release the controls presentation space      */
       WinReleasePS(hPS);
       return(MRFROMLONG(fReturn));

   /*********************************************************************/
   /*  RGB colour query                                          */
   /*********************************************************************/
   /*********************************************************************/
   /*                                                      */
   /*  Message:                                                */
   /*             CWM_QUERYRGBCLR                                    */
   /*                                                      */
   /*  Input:                                                */
   /*             mp1 = 0L;                                    */
   /*             mp2 = 0L;                                    */
   /*                                                      */
   /*  Return:                                                */
   /*             RGB2 rgb2 = LONGFROMMR(mr);                        */
   /*                                                      */
   /*********************************************************************/

   case      CWM_QUERYRGBCLR      :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Check      to see if the mouse pointer at the      */
                   /* origin of the      colour wheel in      the horizontal      */
                   /* direction                              */

       if ( pcw->aptlXHair[2].x      == pcw->ptlOrigin.x )

                   /* Mouse      pointer      aligning horizontally at zero      */
                   /* in the colour      wheel.      Need to      specify      in      */
                   /* radians the angle in which the cross is      */
                   /* resting.  Check to see if the      y position      */
                   /* positive which indicates the angle is      90ø      */
                   /* otherwise it is 270ø.                        */

             if ( (lRadius = pcw->aptlXHair[0].y - pcw->ptlOrigin.y) < 0 )
               rdAngle = 4.712388980;
             else
               rdAngle = 1.570796327;
         else
                   /* Check      to see if the mouse pointer at the      */
                   /* origin of the      colour wheel in      the vertical      */
                   /* direction                              */

             if ( pcw->aptlXHair[0].y      == pcw->ptlOrigin.y )

                   /* Mouse      pointer      aligning horizontally at zero      */
                   /* in the colour      wheel.      Need to      specify      in      */
                   /* radians the angle in which the cross is      */
                   /* resting.  Check to see if the      y position      */
                   /* positive which indicates the angle is      0ø      */
                   /* otherwise it is 180ø.                        */

               if (      (lRadius = pcw->aptlXHair[2].x - pcw->ptlOrigin.x) < 0 )
                   rdAngle = 3.141592654;
               else
                   rdAngle = 0.0;
             else
               {
                   /* Determine the      absolute x and y co-ordinates      */
                   /* based      upon cartesian system.      All trig      */
                   /* functions are      within the 0 to      90ø range.      */

               if (      pcw->aptlXHair[0].y < pcw->ptlOrigin.y )
                   y = pcw->ptlOrigin.y - pcw->aptlXHair[0].y;
               else
                   y = pcw->aptlXHair[0].y - pcw->ptlOrigin.y;

               if (      pcw->aptlXHair[2].x < pcw->ptlOrigin.x )
                   x = pcw->ptlOrigin.x - pcw->aptlXHair[2].x;
               else
                   x = pcw->aptlXHair[2].x - pcw->ptlOrigin.x;

                   /* Calculate the      radius from the      x and y            */
                   /* position of the mouse      using trig and right      */
                   /* angle      geometry                        */

               lRadius = (LONG)((double)y /      sin(rdAngle = atan2((double)y, (double)x)));

                   /* Determine the      quadrant that the mouse      pointer      */
                   /* was in to be able to form correct angle      */

               if (      pcw->aptlXHair[0].y < pcw->ptlOrigin.y )
                   if ( pcw->aptlXHair[2].x      > pcw->ptlOrigin.x )
                     rdAngle = 6.283185307 - rdAngle;
                   else
                     rdAngle += 3.141592654;
               else
                   if ( pcw->aptlXHair[2].x      < pcw->ptlOrigin.x )
                     rdAngle = 3.141592654 - rdAngle;
               }
       return(MRFROMLONG(lHSBtoRGB((LONG)((rdAngle * 360.0) / 6.283185307),
            lRadius *      100 / pcw->lRadius)));

   /*********************************************************************/
   /*  RGB colour set                                          */
   /*********************************************************************/
   /*********************************************************************/
   /*                                                      */
   /*  Message:                                                */
   /*             CWM_SETRGBCLR                                    */
   /*                                                      */
   /*  Input:                                                */
   /*             mp1 = RGB2 rgb2;                                    */
   /*             mp2 = 0L;                                    */
   /*                                                      */
   /*  Return:                                                */
   /*             x = SHORT1FROMMR(mr);                              */
   /*             y = SHORT2FROMMR(mr);                              */
   /*                                                      */
   /*********************************************************************/

   case      CWM_SETRGBCLR :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);

                   /* Get the RGB from the message parameter      */

       prgb2 = (PRGB2)mp1;

       lMin = min(prgb2->bRed, min(prgb2->bGreen, prgb2->bBlue));

       if ( lMax = max(prgb2->bRed, max(prgb2->bGreen, prgb2->bBlue)) )
         {
         lRadius = ((lMax - lMin) * pcw->lRadius) / lMax;

         if (      lMax ==      lMin )
             lAngle =      0L;
         else
             if ( prgb2->bRed      == lMax      )
               lAngle = ((prgb2->bGreen - prgb2->bBlue) * 60L) / (lMax - lMin);
             else
               if (      prgb2->bGreen == lMax )
                   lAngle =      120 + ((prgb2->bBlue - prgb2->bRed) * 60L) / (lMax - lMin);
               else
                   if ( prgb2->bBlue == lMax )
                     lAngle = 240      + ((prgb2->bRed      - prgb2->bGreen) * 60L)      / (lMax      - lMin);

         if (      lAngle < 0L )
             lAngle += 360;
         }
       else
         lRadius = lAngle = 0L;

                   /* Set up the arc parameters                  */

       pcw->ap.lP = pcw->ap.lQ = pcw->lRadius;
       GpiSetArcParams(hPS = WinGetPS(hWnd), &pcw->ap);

                   /* Create the clipping path to encompass      just      */
                   /* the colour wheel itself.  To do this,      open a      */
                   /* normal path and draw a circle      into the path      */
                   /* that has the same radius as the colour wheel.      */

       GpiBeginPath(hPS, 1L);
       GpiMove(hPS, &pcw->ptlOrigin);
       GpiFullArc(hPS, DRO_OUTLINE, MAKEFIXED(1, 0));
       GpiEndPath(hPS);
                   /* Circle complete, convert the normal path to a      */
                   /* clip path to allow the determination of the      */
                   /* mouse      button click within the      colour wheel or      */
                   /* outside the edge of the wheel      but still      */
                   /* within the limits of the control itself      */

       GpiSetClipPath(hPS, 1L, SCP_ALTERNATE | SCP_AND);

                   /* Set the mix mode for line invert            */

       GpiSetMix(hPS, FM_INVERT);

                   /* Erase      the current cross hair by drawing it      */
                   /* again      which will cause the previous colours      */
                   /* to be      displayed since      it is being drawn in      */
                   /* invert mix mode                        */

       GpiMove(hPS, pcw->aptlXHair);
       GpiLine(hPS, &pcw->aptlXHair[1]);

       GpiMove(hPS, &pcw->aptlXHair[2]);
       GpiLine(hPS, &pcw->aptlXHair[3]);

                   /* Save the new position      of the mouse pointer      */
                   /* within the cross hair      point array            */

       pcw->aptlXHair[2].x =
       pcw->aptlXHair[3].x = (LONG)((double)lRadius * cos(rdAngle = (double)lAngle / 57.29577951)) +
                       pcw->ptlOrigin.x;
       pcw->aptlXHair[0].y =
       pcw->aptlXHair[1].y = (LONG)((double)lRadius * sin(rdAngle)) + pcw->ptlOrigin.y;

                   /* Draw the new cross hair within the invert      */
                   /* mix mode                              */

       GpiMove(hPS, pcw->aptlXHair);
       GpiLine(hPS, &pcw->aptlXHair[1]);

       GpiMove(hPS, &pcw->aptlXHair[2]);
       GpiLine(hPS, &pcw->aptlXHair[3]);

                   /* Release the controls presentation space      */

       WinReleasePS(hPS);
       return(MRFROMLONG(MAKELONG((USHORT)pcw->aptlXHair[2].x, (USHORT)pcw->aptlXHair[0].y)));


/************************************************************************/
/************************************************************************/
/*                                                      */
/* Part      6: Control destruction coding                              */
/*                                                      */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control being destroyed,      perform      clean-up                  */
   /*********************************************************************/

   case      WM_DESTROY :
                   /* Get the address of the control info from the      */
                   /* control's reserved memory                     */

       pcw = (PCLRWHEEL)WinQueryWindowPtr(hWnd,      QUCWP_WNDP);
       WinDestroyPointer(pcw->hptrArrow);
       if ( pcw->hbm )
         GpiDeleteBitmap(pcw->hbm);
       DosFreeMem((PVOID)pcw);
       break;
                   /* Default message processing                  */
   default:
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}

BOOL EXPENTRY RegisterColorWheel(HAB hab)
{
   /* Register the main program window class        */

   if (!WinRegisterClass(hab, WC_COLORWHEEL, ClrWheelWndProc,
                         CS_CLIPCHILDREN | CS_SYNCPAINT | CS_SIZEREDRAW,
                         USER_RESERVED))
      return FALSE;
   else
      return TRUE;
}

ULONG _System _DLL_InitTerm(HMODULE ulModHandle, ULONG ulFlag)
{
   if (!ulFlag)
   {
      /* Init */
      hmodThisModule = ulModHandle;

      if (_CRT_init())
         return 0;
   }
#if 0
   else
     _CRT_term();
#endif

   return 1;
}

