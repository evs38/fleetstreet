/* CLRWHL.H */

/* --- Colour Wheel Support -------------------------------------------	*/

#define WC_COLORWHEEL   "ColorWheel"

typedef struct _CLRWHLCDATA
   {
   ULONG    cb;             /* Structure Size   */
   LONG     lAngle;         /* Angle    */
   LONG     lSaturationInc; /* Saturation Increment  */
   LONG     lRadius;        /* Radius    */
   } CLRWHLCDATA ;

typedef CLRWHLCDATA *PCLRWHLCDATA;

#define CWN_RGBCLRSELECTED  750L
#define CWN_HSBCLRSELECTED  751L
#define CWN_XHAIRSELECTED   752L
#define CWN_XHAIRRELEASED   753L

#define CWS_SOLIDCLR        0x0000001L
#define CWS_HSB             0x0000002L
#define CWS_RGB             0x0000004L
#define CWS_BITMAP          0x0000008L
#define CWS_AUTOSIZE        0x0000010L
#define CWS_THREADED        0x0000020L

#define CWM_QUERYXHAIR        (WM_USER+ 32)
#define CWM_SETXHAIR          (WM_USER+ 33)
#define CWM_QUERYRGBCLR       (WM_USER+ 34)
#define CWM_SETRGBCLR         (WM_USER+ 35)
#define CWM_QUERYHSBCLR       (WM_USER+ 36)
#define CWM_SETHSBCLR         (WM_USER+ 37)

BOOL EXPENTRY RegisterColorWheel(HAB hab);
MRESULT EXPENTRY ClrWheelWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);
