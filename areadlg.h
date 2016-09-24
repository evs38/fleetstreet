/* AREADLG.H */

MRESULT EXPENTRY AreaListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
void MarkAllAreas(PAREALIST arealist, LONG lFolderID, ULONG ulFlags);

/* ulIncludeTypes */
#define INCLUDE_NET          1UL
#define INCLUDE_ECHO         2UL
#define INCLUDE_LOCAL        4UL
#define INCLUDE_ALL          (INCLUDE_NET|INCLUDE_ECHO|INCLUDE_LOCAL)


typedef struct _arealistpar {
          USHORT cb;
          ULONG  idTitle;         /* 0 = Default */
          PSZ    pchString;
          ULONG  ulIncludeTypes;
          BOOL   bExtendedSel;
          BOOL   bChange;
       } AREALISTPAR;             /* Fuer Stringuebergabe an Area-Dialog */

