/* TOOLBAR.H */
/* Definitionen der FleetStreet-Toolbar */

/* Fensterklassen-Name */
#define WC_TOOLBAR   "FleetStreet Toolbar"

/* Styles */
#define TBS_HORIZONTAL    0UL  /* Horizontale Toolbar */
#define TBS_VERTICAL      1UL  /* vertikale Toolbar */
#define TBS_SCROLLABLE    0UL  /* Scrollbar, wenn Inhalt kleiner als Fenster */
#define TBS_WRAPPING      2UL  /* Umbruch, wenn Inhalt kleiner als Fenster */
#define TBS_BORDER        4UL  /* Rahmen um das Gesamtfenster */

/* Messages */
#define TBM_ADDITEM         (WM_USER+1)
#define TBM_DELETEITEM      (WM_USER+2)
#define TBM_QUERYITEMDATA   (WM_USER+3)
#define TBM_SETITEMDATA     (WM_USER+4)
#define TBM_ENABLECMD       (WM_USER+5)
#define TBM_DISABLECMD      (WM_USER+6)
#define TBM_QUERYITEMPARAMS (WM_USER+7)
#define TBM_SETCHANGED      (WM_USER+8)
#define TBM_QUERYCHANGED    (WM_USER+9)
#define TBM_QUERYITEMCOUNT  (WM_USER+10)
#define TBM_QUERYREQHEIGHT  (WM_USER+11)
#define TBM_QUERYFIRSTITEM  (WM_USER+12)
#define TBM_QUERYNEXTITEM   (WM_USER+13)
#define TBM_QUERYITEMBITMAP (WM_USER+14)
#define TBM_ISCMDENABLED    (WM_USER+15)
#define TBM_DELETEALLITEMS  (WM_USER+16)

/* Notification-Codes */
#define TBN_SCROLLED       1
#define TBN_CONTEXTMENU    2
#define TBN_ITEMDELETED    3    /* nur bei Drag */
#define TBN_ITEMADDED      4    /* nur bei Drag */
#define TBN_ITEMMOVED      5    /* nur bei Drag */

/* Item-Daten */
#pragma pack(4)

typedef struct
{
   ULONG   ulCommandID;
   ULONG   ulBitmapID;
   ULONG   ulFlags;
   ULONG   ulParamSize;
   PVOID   pItemParams;
} TOOLBARITEM, *PTOOLBARITEM;

typedef struct
{
   USHORT cb;
   LONG   lBorderX;
   LONG   lBorderY;
   LONG   lButtonSpacing;
   LONG   lExtraSpacing;
} TBCTLDATA, *PTBCTLDATA;

#pragma pack()

/* Defaults f. Control-Daten */
#define BORDERSIZE_X   5
#define BORDERSIZE_Y   5

#define BUTTONSPACING   0
#define EXTRASPACING    5


/* Item-Flags */
#define TBITEM_SPACER       1UL
#define TBITEM_DISABLED     2UL
#define TBITEM_DRAGABLE     4UL
#define TBITEM_DELETEABLE   8UL
#define TBITEM_LINEBREAK   16UL

/* Hinzufuegen */
#define ADDITEM_LAST      0L
#define ADDITEM_FIRST     1L
/* sonst: hinter dieser ID */

/* Abfragen */
#define TBQUERY_OK        1L
#define TBQUERY_ERROR     0L
#define TBQUERY_NOMORE    2L

/* Drag-Drop-Definitionen */

#define TBDRAGTYPE    "FleetStreet Toolbar Button"
#define TBDRAGMETHOD  "DRM_FLEET"
#define TBDRAGFORMAT  "DRF_TBBUTTON"
#define TBDRAGRMF     "<" TBDRAGMETHOD "," TBDRAGFORMAT ">"
#define TBDRAGRMFDEL  TBDRAGRMF ",<DRM_DISCARD,DRF_UNKNOWN>"

/*--------------------------- Funktionsprototypen ---------------------------*/

BOOL EXPENTRY RegisterToolbar(HAB hab);
