/* FINDDLG.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY FindProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY FindResultsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

typedef struct _resultrecord {
                  MINIRECORDCORE RecordCore;
                  ULONG ulMsgNum;
                  ULONG ulMsgID;
                  ULONG ulFlags;
                  ULONG ulHow;
                  ULONG ulWhere;
                  PSZ pchAreaTag;
                  PSZ pchFrom;
                  PSZ pchSubj;
                  PSZ pchText;
                } RESULTRECORD;

typedef struct _foundmsg {
                  struct _foundmsg *next;
                  ULONG ulMsgNum;
                  ULONG ulMsgID;
                  MSGHEADER Header;
                  PCHAR pchFindText;
                  ULONG ulHow;
                  ULONG ulWhere;
                } FOUNDMSG, *PFOUNDMSG;

typedef struct _findresultlist {
                  char  pchAreaTag[LEN_AREATAG+1];
                  ULONG ulFindType;
                  ULONG ulFoundMsgs;
               } FINDRESULTLIST, *PFINDRESULTLIST;

typedef struct {
                  USHORT cb;
                  BOOL   bAllMsgs;
                  BOOL   bReserved;    /* unbenutzt */
                  BOOL   bAllNames;
               } PERSMAILOPT, *PPERSMAILOPT;

#define NUM_BACKTEXTS    10

typedef struct
{
   char pchWhat[200];
   char pchBackTexts[NUM_BACKTEXTS][200];
   ULONG ulWhere;
   ULONG ulHow;
   ULONG ulWAreas;
   ULONG ulFuzzyLevel;
   PCHAR pchAreas;
   PERSMAILOPT PersMailOpt;
   PVOID pOptData;
} FINDJOB, *PFINDJOB;

#define FINDWHERE_FROM     0x01
#define FINDWHERE_TO       0x02
#define FINDWHERE_SUBJ     0x04
#define FINDWHERE_TEXT     0x08

#define FINDHOW_SENS          0x01
#define FINDHOW_CASE          0x02
#define FINDHOW_FUZZY         0x03
#define FINDHOW_REGEX         0x04
#define FINDHOW_PERSMAIL      0x05
#define FINDHOW_UNSENT        0x06
#define FINDHOW_METHOD_MASK   0xff  /* Maske f. obiges */

#define FINDHOW_UNREADONLY    0x100

#define FINDAREAS_CURRENT  0x00       /* 00000000 */
#define FINDAREAS_ALL      0x01       /* 00000001 */
#define FINDAREAS_CUSTOMN  0x02       /* 00000010 */
#define FINDAREAS_TYPE     0x03       /* 00000011 */

#define FINDAREAS_NM       0x10       /* 00010000 */
#define FINDAREAS_ECHO     0x20       /* 00100000 */
#define FINDAREAS_LOCAL    0x40       /* 01000000 */
#define FINDAREAS_PRIV     0x80       /* 10000000 */

typedef struct {
                 PCHAR pchText;
                 ULONG ulHow;
                 ULONG ulWhere;
               } JUMPINFO, *PJUMPINFO;

MRESULT EXPENTRY SearchProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

typedef struct
{
   USHORT  cb;
   char    pchSearchText[LEN_SEARCHTEXT+1];
   ULONG   ulSearchFlags;
   WINPOS  DlgPos;
} SEARCHPAR, *PSEARCHPAR;

#define SEARCHFLAG_CASESENSITIVE   0x0001UL

