/* colorwheel.h */

#define USER_RESERVED    16    /* Control Reserved Memory Size */

#define QUCWP_DATA (QWL_USER +  4) /* Pointer to Styles Data Pointer */
#define QUCWP_HEAP (QWL_USER +  8) /* Pointer to Heap Handle  */
#define QUCWP_WNDP (QWL_USER + 12) /* Pointer to Heap Pointer  */

/* --- Colour Wheel Window Information Structures ---------------------	*/

typedef struct _CLRWHEEL
   {
   HWND      hWnd;              /* Control Window Handle  */
   HPS       hpsBitmap;         /* Bitmap Presentation Space Handle */
   ULONG     id;                /* ID Value    */
   ULONG     flStyle;           /* Style    */
   BOOL      fCapture;          /* Capture Flag   */
   ARCPARAMS ap;                /* Arc Parameters   */
   RECTL     rcl;               /* Frame Rectangle   */
   POINTL    aptlXHair[4];      /* Cross Hair Point Array  */
   POINTL    aptl[4];           /* Bitmap Point Array  */
   LONG      lRadius;           /* Radius    */
   LONG      lAngle;            /* Angle    */
   LONG      lSaturationInc;    /* Saturation Increment  */
   LONG      cx;                /* Control Width   */
   LONG      cy;                /* Control Height   */
   POINTL    ptlOrigin;         /* Wheel Origin   */
   HWND      hwndOwner;         /* Owner Window Handle  */
   HWND      hwndParent;        /* Parent Window Handle  */
   HPOINTER  hptrArrow;         /* Arrow Pointer   */
   HBITMAP   hbm;               /* Bitmap Handle   */
   ULONG     ulOptions;         /* Colour Table Options  */
   ULONG     cPlanes;           /* Bitmap Planes Count  */
   ULONG     cBitCount;         /* Bitmap Bits per Plane Count */
   } CLRWHEEL ;

typedef CLRWHEEL *PCLRWHEEL;

