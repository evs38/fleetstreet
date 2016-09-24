/* TOOLBARCONFIG.H */


/* Konfigurierte Toolbar */

typedef struct
{
   ULONG ulButtonID;
   ULONG ulFlags;
} BUTTONCONFIG, *PBUTTONCONFIG;

typedef struct
{
   USHORT        cb;
   BOOL          bDirty;
   ULONG         ulNumButtons;
   PBUTTONCONFIG pButtons;
   WINPOS        SelectPos;
   LONG          lLeftFore;
   LONG          lRightFore;
   LONG          lLeftBack;
   LONG          lRightBack;
   char          pchLeftFont[FACESIZE+5];
   char          pchRightFont[FACESIZE+5];
} TOOLBARCONFIG, *PTOOLBARCONFIG;


/* Funktionen */

MRESULT EXPENTRY ToolbarConfigProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void LoadDefaultToolbar(PTOOLBARCONFIG pConfig);
void RefreshToolbar(HWND hwndToolbar, PTOOLBARCONFIG pConfig, BOOL bSmallIcons);
ULONG QueryBitmap(ULONG ulButtonID, BOOL bSmall);


/* End of TOOLBARCONFIG.H */
