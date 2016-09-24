/* REQUEST_MANAGE.H */

/* File-Listen */

#define LEN_LISTDESC 100

typedef struct filelist
{
   struct filelist *next;
   struct filelist *prev;
   BOOL bDirty;
   char pchFileName[LEN_PATHNAME+1];
   char pchAddress[LEN_5DADDRESS+1];
   char pchDesc[LEN_LISTDESC+1];
} FILELIST, *PFILELIST;

/* allgemeine Daten */

typedef struct
{
   char   pchDestArea[LEN_AREATAG+1];
   BOOL   bDirectReq;
   ULONG  ulAttrib;
   WINPOS ReqPos;
   WINPOS ListAddPos;
   WINPOS FileAddPos;
   WINPOS PasswdPos;
   WINPOS SearchPos;
   LONG   lListFore;
   LONG   lListBack;
   char   pchListFont[FACESIZE+5];
   char   pchLastSearch[LEN_SEARCHTEXT+1];
   ULONG  ulSearchFlags;
   PFILELIST pFirstList;
   BOOL   bListDirty;
   BOOL   bDirty;
} REQUESTOPT, *PREQUESTOPT;

/* gelesene File-Liste */

typedef struct ramlist
{
   struct ramlist *next;
   char *pchLine;
} RAMLIST, *PRAMLIST;


/* Parameter f. Liste Lesen */

typedef struct
{
   PFILELIST pList;
   HWND      hwndNotify;
   ULONG     ulRetCode;
   PRAMLIST  pReadList;
   BOOL      bStop;
} FILELISTREAD, *PFILELISTREAD;

#define FILELIST_OK            0
#define FILELIST_NOTF          1
#define FILELIST_READERR       2
#define FILELIST_STOPPED       3


/* Notify-Message */

#define REQM_LISTREAD          (WM_USER+1)


/* Prototypen */

PFILELIST AddNewFileList(PFILELIST *ppList, PFILELIST pNewList, PBOOL pbDirty);
BOOL DeleteFileList(PFILELIST *ppList, PFILELIST pDelList, PBOOL pbDirty);
void _Optlink ListReadThread(PVOID pData);
PRAMLIST MessageToFileList(char *pchMessageText);
void FreeFileList(PRAMLIST pList);
BOOL IsFileName(char *pchName, BOOL bDotRequired);


/* Ende REQUEST_MANAGE.H */
