/* MLIST.H */
/* Message list control */

/* Window Class */
#define WC_MSGLISTBOX  "MsgListBox"

/* Message list record */
typedef struct
{
   ULONG     ulMsgID;
   char      pchFrom[LEN_USERNAME+1];
   char      pchTo[LEN_USERNAME+1];
   char      pchSubject[LEN_SUBJECT+1];
   ULONG     flRecFlags;
   TIMESTAMP StampWritten;
   TIMESTAMP StampArrived;
} MLISTRECORD, *PMLISTRECORD;

/* Record flags */
#define LISTFLAG_READ       0x00000001          /* gelesen            */
#define LISTFLAG_FROMME     0x00000002          /* selbst geschrieben */
#define LISTFLAG_TOME       0x00000004          /* an mich            */
#define LISTFLAG_LOADED     0x00000008          /* Inhalt geladen     */
#define LISTFLAG_ERROR      0x00000010          /* Fehler beim Lesen  */
#define LISTFLAG_SELECTED   0x00000020          /* selektiert         */
#define LISTFLAG_SOURCE     0x00000040          /* Source-Emphasis    */

/* Messages */
#define MLIM_CLEARLIST      (WM_USER+1)         /* Liste loeschen       */
#define MLIM_ADDITEM        (WM_USER+2)         /* ein Item hinzufuegen */
#define MLIM_DELITEM        (WM_USER+3)         /* ein Item loeschen    */
#define MLIM_SETCOLORS      (WM_USER+4)         /* Farben einstellen    */
#define MLIM_QUERYCOLORS    (WM_USER+5)
#define MLIM_SETCOLUMNS     (WM_USER+6)
#define MLIM_SCROLLTO       (WM_USER+7)
#define MLIM_QUERYITEMCOUNT (WM_USER+8)
#define MLIM_FINDUMSGID     (WM_USER+9)
#define MLIM_UPDATEITEM     (WM_USER+10)
#define MLIM_QUERYITEM      (WM_USER+11)
#define MLIM_ADDITEMARRAY   (WM_USER+12)
#define MLIM_QUERYFSELECT   (WM_USER+13)
#define MLIM_QUERYNSELECT   (WM_USER+14)
#define MLIM_QUERYCRSITEM   (WM_USER+15)
#define MLIM_EMPHASIZEITEM  (WM_USER+16)
#define MLIM_QUERYCOLUMNS   (WM_USER+17)
#define MLIM_SELECTALL      (WM_USER+18)
#define MLIM_SELECTNONE     (WM_USER+19)
#define MLIM_SHIFTINTOVIEW  (WM_USER+20)

typedef struct
{
   LONG lUnreadClr;
   LONG lFromClr;
   LONG lToClr;
} MLISTCOLORS, *PMLISTCOLORS;

typedef struct
{
   ULONG ulNrPercent;
   ULONG ulFromPercent;
   ULONG ulToPercent;
   ULONG ulSubjPercent;
   ULONG ulStampWrittenPercent;
   ULONG ulStampArrivedPercent;
} MLISTCOLUMNS, *PMLISTCOLUMNS;

typedef struct
{
   LONG lItem;     /* Item-Index */
   LONG xContext;  /* x fuer Context-Menue */
   LONG yContext;  /* y fuer Context-Menue */
} MLCONTEXT, *PMLCONTEXT;

/* Items */
#define MLIT_FIRST           0
#define MLIT_LAST            -1
#define MLIT_NONE            -2

/* Control IDs */
#define MLID_VSCROLL          200

/* Timer ID */
#define TID_AUTOSCROLL        100

/* Notification codes */
#define MLIN_ENTER            1  /* mp2 = lItem      */
#define MLIN_CONTEXTMENU      2  /* mp2 = pMLContext */
#define MLIN_LOADITEM         3  /* mp2 = pItem      */
#define MLIN_BEGINDRAG        4  /* mp2 = lItem      */
#define MLIN_ENDDRAG          5
#define MLIN_SEPACHANGED      6
#define MLIN_PPARAMCHANGED    7

/* functions */
BOOL EXPENTRY RegisterMessageList(HAB hab);

