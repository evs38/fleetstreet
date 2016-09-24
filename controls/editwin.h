/* EDITWIN.H */

/* Messages fuer das neue MLE */
#define MLM_DELETELINE        (WM_USER+1)
#define MLM_SPLITQUOTED       (WM_USER+2)
#define MLM_CLEARALL          (WM_USER+3)

/* Tabulatorgroesse setzen, mp1: ULONG tabsize */
#define MLM_SETTABSIZE        (WM_USER+4)

/* Tabulatorgroesse setzen, MRESULT: ULONG tabsize */
#define MLM_QUERYTABSIZE      (WM_USER+5)

/* Translate-Modus einschalten, mp1: BOOL bTranslate */
#define MLM_SETTRANSLATE      (WM_USER+6)

/* Translate-Modus abfragen,  MRESULT: BOOL bTranslate */
#define MLM_QUERYTRANSLATE    (WM_USER+7)

/* Message exportiert, mp1: PDRAGTRANSFER */
#define MLM_EXPORTED          (WM_USER+8)

/* Notification, Abfrage des Kontextmenues */
#define MLN_CONTEXTMENU        0x0040

/* Notification, File auf MLE gedroppt, mp2: PSZ filename */
#define MLN_FILEDROPPED        0x0041

/* Notification, MLE auf Shredder gedroppt */
#define MLN_DISCARDCURRENT     0x0042

/* Notification, MLE auf Drucker gedroppt, mp2: PPRINTDEST */
#define MLN_PRINTCURRENT       0x0043

/* Notification, Export aus MLE an Datei, mp2: PSZ Filename */
#define MLN_EXPORTCURRENT      0x0044

/* Notification, Akt. Cursorposition, mp2: SHORT line, SHORT col */
#define MLN_CURSORPOS          0x0045

/* Notification, auf File-Entryfield wurde ein File gedroppt */
#define EN_FILEDROPPED  0x0100
#define EN_CONTEXTMENU  0x0101
#define EN_FILEATTACH   0x0102
#define EN_NODEDROPPED  0x0103  /* mp2: pchNodeAndName */

/*--------------------------- Funktionsprototypen ---------------------------*/
BOOL RegisterEditWin(HAB anchor);
MRESULT EXPENTRY FileEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY FidoEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

#ifndef RENDERPAR_DEFINED
typedef struct {
                 PDRAGTRANSFER pDragTransfer;
                 PCHAR pchFileName;
              } RENDERPAR, *PRENDERPAR;
#define RENDERPAR_DEFINED
#endif
