/* LOOKUPS.H */

typedef struct _lookuppar {
          USHORT cb;
          char pchName[LEN_USERNAME+1];
          PNODEDATA pNodes;
          int iCountNodes;
          ULONG ulSelected;
       } LOOKUPPAR;

MRESULT EXPENTRY LookupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
int LookupNodelists(char *pchName, PDOMAINS pDomain, PNODEDATA *ppResults, char *errdomain);
int LookupAddress(char *pchAddress, PDOMAINS pDomain, PNODEDATA *ppResults, char *errdomain);
char *NLFlagsToString(PNODEDATA pNodeData, PCHAR pchFlags);
char *NLModemToString(ULONG ulModemType, PCHAR pchModem);
BOOL PerformNameLookup(char *pchSearchName, HWND hwndDlg, ULONG ulFlags, char *pchFoundName, char *pchFoundAddress);
BOOL PerformNodeLookup(char *pchSearchAddress, HWND hwndDlg, char *pchFoundName);

#define LOOKUP_NORMAL       0UL
#define LOOKUP_FORCESELECT  1UL    /* Auswahldialog auch bei nur 1 gefundenem Dialog */

