/* MSGLIST.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY MsgListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

typedef struct _MSGLISTPAR {
                 USHORT cb;
                 ULONG  msgnum;
                 ULONG  ulMsgID;
               } MSGLISTPAR;

typedef struct _MSGLISTOPTIONS {
                 LONG lBackClr;           /* Farben */
                 LONG lForeClr;
                 LONG lUnreadClr;
                 LONG lFromClr;
                 LONG lToClr;
                 ULONG ulNrPercent;       /* Spaltenbreiten */
                 ULONG ulFromPercent;
                 ULONG ulToPercent;
                 ULONG ulSubjPercent;
                 char  mlistfont[FACESIZE+5];      /* Font */
                 ULONG ulFlags;
                 WINPOS ListPos;
                 ULONG ulStampWrittenPercent;
                 ULONG ulStampArrivedPercent;
               } MSGLISTOPTIONS, *PMSGLISTOPTIONS;

#define MLISTFLAG_FOREGROUND  0x01UL

#define WORK_DELETE   0
#define WORK_EXPORT   1
#define WORK_PRINT    2
#define WORK_COPY     3
#define WORK_MOVE     4
#define WORK_MARK     5
#define WORK_MARKALL  6

typedef struct _WORKDATA {
             struct _WORKDATA *next;
             PULONG MsgIDArray;                /* Message-ID-Array */
             ULONG ulArraySize;                /* Anzahl der Elemente im Array > 0 */
             char pchSrcArea[LEN_AREATAG+1];   /* Quellarea bei copy und move */
             char pchDestArea[LEN_AREATAG+1];  /* Zielarea bei copy und move */
             char pchDestFile[LEN_PATHNAME+1]; /* Zielfile bei export */
             ULONG flWorkToDo;                 /* was der Thread machen soll */
             PPRINTDEST pPrintDest;            /* NULL: normal drucken */
             ULONG  ulExportOptions;           /* Optionen bei Export */
             ULONG  ulCopyMove;                /* Optionen f. Copy/Move */
          } WORKDATA, *PWORKDATA;

void _Optlink WorkerThread(PVOID pThreadData);
void MarkAllMessages(char *pchAreaTag);
