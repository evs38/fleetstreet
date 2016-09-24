/* Message list private header */

/* List box control data */
typedef struct _LBCTLDATA {
             LONG  lItemCount;       /* Anzahl der Items   */
             LONG  lItemAlloc;       /* Speicher f. Items  */
             LONG  lCrsItem;         /* Item mit Cursor    */
             LONG  lTopItem;         /* Item am Anfang der Liste */
             HWND  hwndScroll;       /* Scrollbar-Handle   */
             LONG  lLineHeight;      /* Zeilenhoehe        */
             LONG  lUnreadClr;       /* Farbe f. Ungelesen */
             LONG  lFromClr;         /* Farbe f. FromMe    */
             LONG  lToClr;           /* Farbe f. ToMe      */
             RECTL rclWindow;        /* Window-Groesse     */
             RECTL rclItems;         /* Window-Groesse Item-Bereich */
             ULONG ulNrWidth;        /* Spaltenbreiten     */
             ULONG ulFromWidth;
             ULONG ulToWidth;
             ULONG ulSubjWidth;
             ULONG ulStampWrittenWidth;
             ULONG ulStampArrivedWidth;
             ULONG ulNrPercent;      /* Spaltenbreiten in % */
             ULONG ulFromPercent;
             ULONG ulToPercent;
             ULONG ulSubjPercent;
             ULONG ulStampWrittenPercent;
             ULONG ulStampArrivedPercent;
             LONG  cxScroll;         /* Breite des Scrollbars */
             PMLISTRECORD pRecords;  /* Array der Records  */
             LONG  lPrevItem;        /* vorheriges Item bei Swipe-Select */
             BOOL  bSwipe;           /* Swipe-Select ist im Gange ? */
             BOOL  bTimer;           /* Timer laeuft ? */
             LONG  lSwipeAnchor;     /* Anker-Item bei Swipe-Select */
             LONG  lAnchorItem;      /* Anker-Item bei Range-Select */
             ULONG ulPrevKey;        /* vorher betaetigte Taste */
             HPOINTER hptrNow;       /* aktueller Pointer */
             ULONG ulNrSeparator;    /* Pointer Åber Separator n (0=keiner) */
             LONG  lVisibleLines;
          } LBCTLDATA, *PLBCTLDATA;

#define MKEY_NONE      0
#define MKEY_UP        1
#define MKEY_DOWN      2

#define SEPA_NONE      0    /* keine Trennlinie               */
#define SEPA_NUMFROM   1    /* Trennlinie zw. Nummer und From */
#define SEPA_FROMTO    2    /* Trennlinie zw. From und To     */
#define SEPA_TOSUBJ    3    /* Trennlinie zw. To und Subj     */
#define SEPA_SUBJWR    4    /* Trennlinie zw. Subj und Written*/
#define SEPA_WRARR     5    /* Trennlinie zw. Written und Arrived*/

