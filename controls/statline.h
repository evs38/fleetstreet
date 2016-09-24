/* STATLINE.H */

#define WC_STATUSLINE         "FleetStatusLine"

/* Messages */
#define STLM_SETBORDERSIZE     (WM_USER+1 )
#define STLM_QUERYBORDERSIZE   (WM_USER+2 )
#define STLM_DRAWBORDERLINE    (WM_USER+3 )
#define STLM_ADDFIELD          (WM_USER+6 )
#define STLM_REMOVEFIELD       (WM_USER+7 )
#define STLM_QUERYFIELDCOUNT   (WM_USER+8 )
#define STLM_SETFIELDCOLOR     (WM_USER+9 )
#define STLM_QUERYFIELDCOLOR   (WM_USER+10)
#define STLM_SETFIELDFONT      (WM_USER+11)
#define STLM_QUERYFIELDFONT    (WM_USER+12)
#define STLM_SETFIELDTEXT      (WM_USER+13)
#define STLM_QUERYFIELDTEXT    (WM_USER+14)
#define STLM_SETFIELDWIDTH     (WM_USER+15)
#define STLM_SETTIMEOUT        (WM_USER+16)

/* Notification codes */
#define STLN_CONTEXTMENU        1     /* mp2: ULONG ulFieldID */
#define STLN_OPEN               2     /* mp2: ULONG ulFieldID */
#define STLN_CHORD              3
#define STLN_BEGINDRAG          4     /* mp2: ULONG ulFieldID */
#define STLN_ENDDRAG            5     /* mp2: ULONG ulFieldID */
#define STLN_DRAGOVER           6     /* mp2: PSTLDRAGNOTIFY pSTLDragNotify */
#define STLN_DRAGLEAVE          7     /* mp2: PSTLDRAGNOTIFY pSTLDragNotify */
#define STLN_DROP               8     /* mp2: PSTLDRAGNOTIFY pSTLDragNotify */
#define STLN_DROPHELP           9     /* mp2: PSTLDRAGNOTIFY pSTLDragNotify */
#define STLN_COLORCHANGED      10
#define STLN_FONTCHANGED       11
#define STLN_TEXTEDIT          12     /* mp2: ULONG ulFieldID */

typedef struct _STLDRAGNOTIFY {
             ULONG     ulFieldID;     /* Field-ID des Drag-Ereignisses */
             PDRAGINFO pDragInfo;     /* Pointer auf DRAGINFO-Struktur */
          } STLDRAGNOTIFY, PSTLDRAGNOTIFY;

/* Flags */
#define STLF_FIXED             0x0000  /* Groesse ist fest     */
#define STLF_VARIABLE          0x0001  /* Groesse ist variabel */
#define STLF_LEFT              0x0000  /* linksbuendig         */
#define STLF_RIGHT             0x0002  /* rechtsbuendig        */
#define STLF_CENTER            0x0004  /* zentriert            */
#define STLF_ALIGN_MASK        0x0006  /* Maske f. Alignment   */
#define STLF_FLAT              0x0000  /* Kein 3D-Effekt       */
#define STLF_3D                0x0008  /* 3D-Effekt            */
#define STLF_TEXT              0x0000  /* Text wird dargestellt */
#define STLF_CHECK             0x0010  /* Haekchen wird dargestellt */
#define STLF_PROGRESS          0x0020  /* Progress-Indicator wird dargestellt */

/* Field-IDs */
#define STLI_FIRST             0x0000
#define STLI_LAST              0x0001

/* Styles */
#define STLS_BORDER            0x0001  /* Rahmen um Statuszeile */

#define TID_MSGTIMEOUT         0x5000  /* Timeout f. Anzeige */

/* Typen */
typedef struct _slfieldinfo {
        ULONG ulFlags;
        LONG  lFieldSize;
        LONG  lColorForeground;
        PSZ   pszFontName;
        ULONG ulTimeout;     /* 0 = ohne Timeout */
     } STLFIELDINFO, *PSTLFIELDINFO;

/*--------------------------- Funktionsprototypen ---------------------------*/

BOOL EXPENTRY RegisterStatusLineClass(HAB hab);
