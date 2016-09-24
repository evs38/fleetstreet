/* MAINWINDOW.H */

/* Instanzdaten f. Mainwindow */

typedef struct {
         ULONG  idMessageField;   /* Field-IDs v. Statuszeile */
         ULONG  idCursorField;
         ULONG  idKeybField;
         ULONG  idAddressField;
         ULONG  idNumberField;
         ULONG  idCheckField;
         ULONG  idProgressField;
         LONG   lLastProgress;
         BOOL   bInsert;          /* Insert-Mode */
         ULONG  ulCurrentID;      /* Message-ID der aktuellen Message */
         HWND   hwndPopup;        /* Popup-Menue */
         USHORT usPopupControl;   /* Control-ID f. Popup-Menue */
         HWND   hwndRexxMenu;     /* Window-Handle des Rexx-Menues */
         ULONG  ulCurrentHook;    /* momentan bearbeiteter Hook */
         AREADEFLIST *pArea;
         HWND   hwndToolbarPopup;
         ULONG  ulSourceWindow;
      } WINDOWDATA, *PWINDOWDATA;

#define CURRENTHOOK_NONE     0
#define CURRENTHOOK_EXIT     1
#define CURRENTHOOK_PRESAVE  2

#define SOURCEWIN_NONE       0
#define SOURCEWIN_AREA       1
#define SOURCEWIN_MSGLIST    2
#define SOURCEWIN_THREAD     3
#define SOURCEWIN_BOOKMARKS  4

/* f. Frame-Subclass */
#define FID_TOOLBAR                0x8020
#define FID_STATUSLINE             0x8021

/*--------------------------- Funktionsprototypen ---------------------------*/
void UpdateDisplay(BOOL isNewArea, BOOL withText);
void DisplayAttrib(ULONG attr);
void QueryLayout(HWND fenster);
void InitMenus(USHORT usMenuID, HWND hwndMenuWnd);
void StatusChanged(HWND hwndClient, int newstatus);
void InitMainWindow(HWND fenster);
void DisplayStatusText(SHORT ctrlID);
void DisplayMessage(BOOL withText);
void InsertMacro(HWND hwndEdit, USHORT MacroID);
void ShowFleetWindow(USHORT usMenuID);
void DisplayMenuHelp(HWND hwndClient, SHORT ctrlID);
void DisplayMsgText(HWND hwndClient, FTNMESSAGE *msginfo);
void SwitchEditor(HWND hwndClient, char *pchDestArea, BOOL bEdit);
void UpdateMsgNum(HWND hwndClient, PWINDOWDATA pWindowData);
void UpdateButtons(AREADEFLIST *zeiger);
void UpdateCheckField(AREADEFLIST *zeiger, PWINDOWDATA pWindowData);
void OpenEditPopup(HWND hwndClient, PWINDOWDATA pWindowData, BOOL bKeyBoard);
void QueryOpenWindows(POPENWIN pOpenWindows);
void RestoreOpenWindows(POPENWIN pOpenWindows);
void FLTLAY_ResizeMainWindow(HWND hwndClient, SHORT cx, SHORT cy);
USHORT FLTLAY_QueryNextFocus(USHORT usCurrent, BOOL bSkipAddresses);
USHORT FLTLAY_QueryPrevFocus(USHORT usCurrent, BOOL bSkipAddresses);
MRESULT EXPENTRY ToolbarFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
int CreateStatusLine(HWND hwndFrame);
int CreateToolbar(HWND hwndFrame);
int OpenToolbarContext(HWND hwndClient, PWINDOWDATA pWindowData, ULONG ulButtonID);
int SwitchToolbarPos(HWND hwndClient, ULONG ulCommand);
int SwitchToolbarSize(HWND hwndClient);
void SetTranslateMode(BOOL bTranslate);
void BackToWindow(PWINDOWDATA pWindowData);

int OpenProgressBar(PWINDOWDATA pWindowData, PCHAR pchAreaTag);
int CloseProgressBar(PWINDOWDATA pWindowData);
int ProgressBarProgress(PWINDOWDATA pWindowData, LONG lPercent);
