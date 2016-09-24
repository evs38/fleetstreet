/* Interne Toolbar-Definitionen */

#pragma pack(4)

typedef struct tbitemlist
{
   struct tbitemlist *next;
   struct tbitemlist *prev;

   TOOLBARITEM TBItem;
   HWND        hwndButton;      /* Button-Window */
   ULONG       ulInternalID;    /* Interne Button-ID */
   LONG        lCreateCX;       /* x-Groesse beim Erzeugen */
   LONG        lCreateCY;       /* y-Groesse beim Erzeugen */
} TBITEMLIST, *PTBITEMLIST;


typedef struct
{
   PTBITEMLIST pItemList;        /* Item-Liste */
   PTBITEMLIST pItemListLast;    /* Ende der Item-Liste */
   ULONG       ulItemCount;      /* Anzahl der Items */
   BOOL        bDirty;           /* TRUE: Aenderung in den Items */
   ULONG       ulButtonID;       /* Speicher f. interne Button-IDs */
   LONG        lMaxHeight;       /* max. Hoehe der Buttons */
   PTBITEMLIST pCurrentQuery;    /* f. Iteration */
   LONG        lButtonSpacing;   /* normaler Abstand zwischen zwei Buttons */
   LONG        lExtraSpacing;    /* zusaetzlicher Abstand bei Spacer-Attribut */
   LONG        lBorderX;         /* Rand in X-Richtung */
   LONG        lBorderY;         /* Rand in Y-Richtung */
   PDRAGINFO   pDraginfo;
   HWND        hwndScroll;
   LONG        lScrollOffset;
   LONG        lScrollHeight;
   LONG        lTotlWidth;
   LONG        lWinWidth;
   LONG        lAvgWidth;
} TOOLBARDATA, *PTOOLBARDATA;

#pragma pack()
