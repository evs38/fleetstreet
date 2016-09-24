/* UTILITY.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
int QueryCurrent(PAREALIST AreaList, PCHAR tag);
int WriteEchotoss(PAREALIST AreaList, PCHAR pchEchoToss);
BOOL AlreadyRunning(void);

BOOL QueryWinPos(HWND hwnd, WINPOS *pWinPos);
BOOL RestoreWinPos(HWND hwnd, WINPOS *pWinPos, BOOL SizeIt, BOOL bShow);

void QueryForeground(HWND hwnd, LONG *lColor);
void QueryControlForeground(HWND hwndParent, ULONG id, LONG *lColor);
void SetForeground(HWND hwnd, LONG *lColor);
void QueryBackground(HWND hwnd, LONG *lColor);
void QueryControlBackground(HWND hwndParent, ULONG id, LONG *lColor);
void SetBackground(HWND hwnd, LONG *lColor);
void QueryFont(HWND hwnd, char *pchFacename);
void QueryControlFont(HWND hwndParent, ULONG id, char *pchFacename);
void SetFont(HWND hwnd, char *pchFacename);
int ImportFile(HWND hwndClient, PCHAR pchLastFileName, BOOL bConvert, BOOL bAsk);
int ExportFile(HWND hwndOwner, PCHAR pchLastExport, BOOL bAsk, PULONG pulOptions);
BOOL GetExportName(HWND hwndOwner, PCHAR pchFileName, PULONG pulExportOptions);
void Notify(HWND hwndOwner, ULONG idString);
int WriteMessage(PCHAR pchFileName, FTNMESSAGE *Message, MSGHEADER *Header, PCHAR tag,
                 ULONG ulOptions);

#define EXPORT_WITHHEADER    0x01UL
#define EXPORT_APPEND        0x02UL
#define EXPORT_SEPARATOR     0x04UL

void CleanupDomains(PDOMAINS *ppDomains);
PDOMAINS QueryDomain(PDOMAINS domains, char *pchDomainName);
int StartShell(void);
USHORT MessageBox(HWND hwndOwner, ULONG ulIDMessage, ULONG ulIDTitle,
                  USHORT usWinID, ULONG flStyle);
char *QueryNextArea(PAREALIST arealist, char *pchCurrent);
HWND ReplaceSysMenu(HWND hwndDlg, HWND hwndPopupMenu, USHORT usSubID);
void ResetMenuStyle(HWND hwndPopup, HWND hwndDialog);
BOOL CalcClientRect(HAB hab, HWND hwndFrame, PRECTL prclResult);
BOOL SizeToClient(HAB hab, PSWP pSwp, HWND hwndDialog, ULONG ulControlID);
BOOL SaveWinPos(HWND hwnd, PSWP pSwp, PWINPOS pWinPos, PBOOL pbDirty);

char *CreateUniqueName(ULONG ulStringID, PVOID pData,
                       int (*CompareFunc)(PVOID, char*),
                       ULONG ulBufferLen, char *pchBuffer);

void SetNotebookParams(HWND hwndNotebook, USHORT usTabX);
ULONG InsertOnePage(HWND notebook, ULONG resourceID, ULONG stringID, PFNWP dlgproc, PVOID dlgpar);
BOOL SetFocus(HWND hwnd);
BOOL SetFocusControl(HWND hwndParent, ULONG ulID);
MRESULT SendMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
LONG LoadString(ULONG idString, LONG lBufferMax, PSZ pszBuffer);
HSWITCH AddToWindowList(HWND hwndDlg);
ULONG RemoveFromWindowList(HSWITCH hSwitch);
HPOINTER LoadIcon(ULONG ulIconID);
MRESULT RedirectCommand(MPARAM mp1, MPARAM mp2);
void SwitchAccels(HWND hwndFrame, ULONG ulAccelNum);
void SwitchAccelsMain(HWND hwndFrame, ULONG ulAccelNum);
void SetInitialAccel(HWND hwndFrame);
