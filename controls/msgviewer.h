/* MSGVIEWER.H */

#define WC_MSGVIEWER      "MessageViewer"

/* Notification Codes */
/* Message auf Shredder gedroppt */
#define MSGVN_DISCARDCURRENT  0x0001

/* Message auf Drucker gedroppt, mp2: PPRINTDEST */
#define MSGVN_PRINTCURRENT    0x0002

/* Export aus Message-Viewer an Datei, mp2: PRENDERPAR */
#define MSGVN_EXPORTCURRENT   0x0003

#define MSGVN_PRESPARAMCHANGED 0x0004

/* Messages */
/* Message exportiert, mp1: PDRAGTRANSFER */
#define MSGVM_EXPORTED       (WM_USER+1)

/* Farbe setzen, mp1: Color-ID, mp2: LONG Color-Index */
#define MSGVM_SETCOLOR       (WM_USER+2)

/* Farbe abfragen, mp1: Color-ID, mp2: PLONG Color-Index */
#define MSGVM_QUERYCOLOR     (WM_USER+3)

/* An den Anfang des Textes scrollen */
#define MSGVM_SCROLLTOTOP    (WM_USER+4)

/* Markierung bzw. gesamten Text ins Clipboard kopieren */
#define MSGVM_COPY           (WM_USER+5)

/* Drag abschalten (z.B. in Areas ohne Message) */
#define MSGVM_DISABLEDRAG    (WM_USER+6)

/* Drag erm”glichen */
#define MSGVM_ENABLEDRAG     (WM_USER+7)

/* Text suchen, mp1: pchText, mp2: ulOptions */
#define MSGVM_FINDTEXT       (WM_USER+8)

/* Highlighting ein/aus */
#define MSGVM_ENABLEHIGHLIGHT (WM_USER+9)
#define MSGVM_QUERYHIGHLIGHT  (WM_USER+10)

/* Quote-Char einstellen */
#define MSGVM_SETQUOTECHAR    (WM_USER+13)

/* Color-IDs */
#define MSGVCLR_BACKGROUND    1
#define MSGVCLR_TEXT          2
#define MSGVCLR_QUOTE         3
#define MSGVCLR_TEARLINE      4
#define MSGVCLR_ORIGIN        5
#define MSGVCLR_QUOTE2        6

/* Child-Controls */
#define MSGVID_VSCROLL     0x0001

/* Styles */
#define MSGVS_VSCROLL      0x00000001L
#define MSGVS_BORDER       0x00000002L

/*--------------------------- Funktionsprototypen ---------------------------*/

BOOL EXPENTRY RegisterMsgViewer(HAB hab);

/* Alte Struktur, Drag-Drop sollte irgendwann ins Owner-Window
   verlagert werden */

#ifndef RENDERPAR_DEFINED
typedef struct {
                 PDRAGTRANSFER pDragTransfer;
                 PCHAR pchFileName;
              } RENDERPAR, *PRENDERPAR;
#define RENDERPAR_DEFINED
#endif

/* Ende MSGVIEWER.H */
