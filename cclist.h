/* CCLIST.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY CCListSelectProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CCFolderProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CCListContentsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
void AddToCCList(HWND hwndOwner, MSGHEADER *pHeader);

typedef struct {
           USHORT cb;
           ULONG ulSelectID;
           BOOL  bEmptyLists;
         } CCSELECTPAR;

typedef struct _cclistpar {
            USHORT    cb;
            PCCLIST   pList;
            PCCANCHOR pCCAnchor;
          } CCLISTPAR, *PCCLISTPAR;

