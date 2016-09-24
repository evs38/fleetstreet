/* ECHOMANAGER.H */

/*---------------------------------- Typen ----------------------------------*/
#pragma pack(4)

typedef struct uplink
{
   struct uplink *next;
   struct uplink *prev;

   PCHAR pchUplinkAreas;
   char pchEchoMgrName[LEN_USERNAME+1];
   char pchEchoMgrAddress[LEN_5DADDRESS+1];
   char pchMyAddress[LEN_5DADDRESS+1];
   char pchPassword[20];
} UPLINK, *PUPLINK;

typedef struct
{
   char pchDllName[LEN_PATHNAME+1];
   PVOID pDllParams;
   ULONG ulParamLen;
   WINPOS FolderPos;
   LONG   lFolderFore;
   LONG   lFolderBack;
   char   pchFolderFont[FACESIZE+5];
   WINPOS SettingsPos;
   PUPLINK pUplinks;
   PUPLINK pUplinksLast;
   BOOL bDirty;
} ECHOMGROPT, *PECHOMGROPT;

#pragma pack()

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY EchoMgrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
int ExtractUplinkFromMessage(HWND hwndClient, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PECHOMGROPT pEchoMgrOpt);


/* Ende von ECHOMANAGER.H */
