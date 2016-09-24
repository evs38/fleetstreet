/* REQUEST.H */

MRESULT EXPENTRY RequestProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void FreeRequestList(PREQUESTLIST pList);

typedef struct requestpar
{
   USHORT      cb;
   PSZ         pchFiles;
   char        pchReqName[LEN_USERNAME+1];
   char        pchReqAddr[LEN_5DADDRESS+1];
   char        pchDestArea[LEN_AREATAG+1];
   PAREALIST   arealist;
   BOOL        bDirect;
   ULONG       ulAttrib;
   PREQUESTLIST pReqList;
} REQUESTPAR, *PREQUESTPAR;


/* Ende REQUEST.H */
