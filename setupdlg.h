/* SETUPDLG.H */

typedef struct _nbpage {
          ULONG PageID;
          HWND  hwndPage;
          ULONG resID;
          PFNWP DlgProc;
        } NBPAGE;

/*--------------------------- Funktionsprototypen ---------------------------*/

MRESULT EXPENTRY OptionsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
LONG GetPathname(HWND hwndOwner, char *pchPathname);
void InsertEmptyPage(HWND notebook, ULONG stringID, NBPAGE *Page);
void LoadPage(HWND notebook, NBPAGE *Page, PVOID pCreateParam);

/* Dialogstrings */
#define IDST_TAB_USER               400
#define IDST_TAB_ADDRESSES          401
#define IDST_TAB_DEFORIGIN          402
#define IDST_TAB_NODELIST           404
#define IDST_TAB_DOMAINS            405
#define IDST_TAB_MODEMTYPES         406
#define IDST_TAB_MACROS             407
#define IDST_TAB_MACROS2            408
#define IDST_TAB_MACROS3            409
#define IDST_TAB_MSGOPT             410
#define IDST_TAB_REGISTER           412
#define IDST_TAB_SQUISHOPT          413
#define IDST_TAB_PRINTER            414
#define IDST_TAB_STARTUP            417
#define IDST_TAB_EDITOROPT          418
#define IDST_TAB_DRIVEREMAP         419
#define IDST_TAB_SAFETY             420
#define IDST_TAB_TOSSERPATHS        421

