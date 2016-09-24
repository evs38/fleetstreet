/* PRINTSETUP.H */

MRESULT EXPENTRY PrintSetupProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#pragma pack(4)

typedef struct
{
  USHORT cb;
  BOOL   bDirty;
  WINPOS DlgPos;
  ULONG  ulOutput;   /* OUTPUT_* */
  LONG   lLeft;
  LONG   lRight;
  LONG   lTop;
  LONG   lBottom;
  char   pchHeaderFont[FACESIZE+5];
  char   pchTextFont[FACESIZE+5];

  /* INPUT PARAMETERS: assign these before calling SetupPrinter() */
  LONG           lWorldCoordinates; /* PU_TWIPS or whatever */
  HAB            hab;               /* application's anchor block */

  /* PROFILED PARAMETERS: store these two items on app close.
     Retrieve them on app open.
     Note pDriverData->cb is length of DRIVDATA structure. */
  CHAR           szPreferredQueue[ 64 ];  /* name of user's preferred queue */
  PDRIVDATA      pDriverData;             /* driver's data */

  /* OUTPUT PARAMETERS: for use by SetupPrinter() and the application */
  HDC            hdcPrinterInfo;    /* printer info DC */
  HPS            hpsPrinterInfo;    /* printer info PS */
  LONG           lDCType;           /* DC type suitable for DevOpenDC() param 2*/
  PDEVOPENDATA   pDevOpenData;      /* suitable for DevOpenDC() parameter 5 */
  DEVOPENSTRUC   devopenstruc;      /* pdevopendata points to this.         */
  LONG           cQueues;           /* count of queues in PRQINFO3 array    */
  PPRQINFO3      pQueueInfo;        /* pointer to array of PRQINFO3         */
} PRINTSETUP, *PPRINTSETUP;

#pragma pack()

#define OUTPUT_AREA     0x01UL
#define OUTPUT_DATE     0x02UL
#define OUTPUT_ATTRIB   0x04UL
#define OUTPUT_PAGENUM  0x08UL

ULONG InitPrintSetup(PPRINTSETUP pPrintSetup, HAB hab);
ULONG TermPrintSetup(PPRINTSETUP pPrintSetup);

/* Ende PRINTSETUP.H */
