/* VIEWER_INT.H */

typedef struct viewerline
{
   PCHAR pchLine;
   ULONG ulLineLen;
   ULONG ulFlags;    /* Zeilentyp etc. */
   struct viewerline *nextseg;
   struct viewerline *prevseg;
} VIEWERLINE, *PVIEWERLINE;

/* ermittelte Zeilen-Typen */
#define LINE_TEXT    1UL
#define LINE_QUOTE   2UL
#define LINE_TEAR    3UL
#define LINE_ORIGIN  4UL
#define LINE_QUOTE2  5UL

#define LINE_TYPE_MASK 0xfUL

#define LINE_HIGHLIGHT  0x80000000UL
#define LINESEG_ITALIC  0x08000000UL
#define LINESEG_UNDER   0x40000000UL
#define LINESEG_BOLD    0x20000000UL
#define LINESEG_NEWLINE 0x10000000UL

/* Instanzdaten v. Messageviewer */
typedef struct
{
   PCHAR       pchMessageText;   /* Messagetext, 0-terminiert             */
   ULONG       ulMessageBufLen;  /* Puffergroesse                         */
   PVIEWERLINE *pLines;          /* Formatierte Zeilen                    */

   LONG        lColorQuote;      /* Farbe f. Quote                        */
   LONG        lColorQuote2;     /* Farbe f. Quote 2                      */
   LONG        lColorTearline;   /* Farbe f. Tearline                     */
   LONG        lColorOrigin;     /* Farbe f. Origin                       */

   ULONG       ulFirstLine;      /* Nummer der ersten sichtbaren Zeile    */
   ULONG       ulCountLines;     /* Anzahl der Zeilen                     */
   ULONG       ulVisibleLines;   /* Anzahl voll sichtbarer Zeilen         */
   LONG        lAnchorLine;      /* Anker bei Select (-1 == nichts)       */
   LONG        lCrsLine;         /* Cursor bei Select                     */
   LONG        lPrevLine;        /* vorher aktive Zeile bei Select        */
   BOOL        bCapture;         /* Capture aktiv                         */
   POINTL      ptlStartSelect;   /* Pointer-Position beim Start des Select*/

   LONG        lFontHeight;      /* Font-Hoehe                            */
   LONG        lWidths[256];     /* Fontweitentabelle                     */
   LONG        lWidthsBold[256]; /* Fontweitentabelle f. Bold             */
   LONG        lWidthsItalic[256]; /* Fontweitentabelle f. Italic           */
   LONG        lMaxAscender;
   FATTRS      FontAttrs;        /* f. Fett   */
   FATTRS      FontAttrsIta;     /* f. Italic */
   BOOL        bHighlight;
   PLONG       pIncrements;      /* f. GpiCharStringPos */

   LONG        lBorder;          /* Randabstand links und rechts          */
   UCHAR       uchQuoteChar;     /* Zeichen f. Quote                      */
   RECTL       recWindow;        /* Fensterregion ohne Scrollbar          */
   ULONG       ulStyle;          /* Style                                 */
   BOOL        bDragEnabled;     /* Drag-Drop-Features enabled            */
   HPOINTER    hptrMessage;      /* Icon f. Drag                          */
   BOOL        bTimer;           /* Timer laeuft ?                        */
} VIEWERPARAMS, *PVIEWERPARAMS;

/* Ende VIEWER_INT.H */
