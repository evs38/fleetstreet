/* List box private header */

typedef struct
{
   PCHAR pchLine;
   ULONG ulFlags;
   PVOID pItemHandle;
} MLISTRECORD, *PMLISTRECORD;

/* Record flags */
#define LISTFLAG_SELECTED   0x00000001          /* selektiert         */

/* List box control data */
typedef struct _LBCTLDATA {
             LONG  lItemCount;       /* Anzahl der Items   */
             LONG  lItemAlloc;       /* Speicher f. Items  */
             PMLISTRECORD pRecords;  /* Array der Records  */

             LONG  lCrsItem;         /* Item mit Cursor    */
             LONG  lTopItem;         /* Item am Anfang der Liste */
             LONG  lPrevItem;        /* vorheriges Item bei Swipe-Select */

             BOOL  bSelect;          /* TRUE=Select, FALSE=Deselect by Swipe */

             HWND  hwndScroll;       /* Scrollbar-Handle   */
             LONG  cxScroll;         /* Breite des Scrollbars */
             LONG  lLineHeight;      /* Zeilenhoehe        */
             RECTL rclWindow;        /* Window-Groesse     */
             RECTL rclItems;         /* Window-Groesse Item-Bereich */

             BOOL  bSwipe;           /* Swipe-Select ist im Gange ? */
             BOOL  bTimer;           /* Timer laeuft ? */
             ULONG ulPrevKey;        /* vorher betaetigte Taste */
             SHORT sScrollFactor;
          } LBCTLDATA, *PLBCTLDATA;

#define MKEY_NONE      0
#define MKEY_UP        1
#define MKEY_DOWN      2

