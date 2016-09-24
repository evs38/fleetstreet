/* DIALOGS.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY AttribProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ReplyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ReplyListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY KludgesProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CurrNameProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ExportProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MarkMsgsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ForwardProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY LookupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY RenumberProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

typedef struct stringpar
{
   USHORT cb;
   PSZ    pchString;
} STRINGPAR;             /* Fuer Stringuebergabe an Dialog */

typedef struct boolpar
{
   USHORT cb;
   BOOL   bBool;
} BOOLPAR;               /* Fuer Bool-Uebergabe an Dialog */

typedef struct ulpar
{
   USHORT cb;
   ULONG  ulValue;
} ULPAR;                 /* Fuer ULONG-Uebergabe an Dialog */

typedef struct replylistpar
{
   USHORT    cb;
   MSGHEADER *pHeader;
   ULONG     Selection;
} REPLYLISTPAR;

typedef struct replypar
{
   USHORT cb;
   BOOL   diffarea;
   ULONG  replydest;       /* REPLYDEST_* */
   ULONG  quoteoptions;    /* QUOTE_* */
   BOOL   isfwd;           /* nur Eingabe */
} REPLYPAR;

#define REPLYDEST_FROM    0
#define REPLYDEST_TO      1
#define REPLYDEST_ORIG    2

typedef struct currnamepar
{
   USHORT cb;
   char CurrName[LEN_USERNAME+1];
   char CurrAddr[LEN_5DADDRESS+1];
} CURRNAMEPAR;

typedef struct attribpar
{
   USHORT cb;
   ULONG  ulAttrib;      /* gesetzte Attribute */
   ULONG  ulAttribMask;  /* unterstuetzte Attribute */
   BOOL   bKeepRead;
} ATTRIBPAR;

typedef struct aboutpar
{
   USHORT cb;
   BOOL   bIntroDlg;
} ABOUTPAR;

typedef struct renumberpar
{
   USHORT      cb;
   char        pchArea[LEN_AREATAG+1];
   PAREALIST   arealist;
   HWND        hwndProgress;
} RENUMBERPAR;

