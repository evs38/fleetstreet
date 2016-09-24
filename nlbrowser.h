/* NLBROWSER.H */

/* Defines */

#define BROWSEMODE_NODE    0
#define BROWSEMODE_NAME    1

/* Typen */

typedef _Packed struct {
      WINPOS    BrowserPos;
      ULONG     ulLastMode;
      char      pchLastDomain[LEN_DOMAIN+1];
      LONG      lSplitbar;
      BOOL      bIcons;
      BOOL      bNoPoints;
   } BROWSEROPTIONS, *PBROWSEROPTIONS;

/* Funktionen */

MRESULT EXPENTRY NLBrowserProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

