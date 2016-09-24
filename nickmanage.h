/* NICKMANAGE.H */

/*---------------------------------- Typen ----------------------------------*/

/* Ein Nickname */

#define NICKFLAG_NOTEMPLATE      0x01UL

typedef _Packed struct nickname
{
   _Packed struct nickname *next;
   _Packed struct nickname *prev;
   BOOL   bDirty;

   char   usertag[LEN_USERNAME+1];
   char   username[LEN_USERNAME+1];
   char   address[LEN_5DADDRESS+1];
   char   subjectline[LEN_SUBJECT+1];
   char   firstline[LEN_FIRSTLINE+1];
   char   *pchComment;
   ULONG  ulAttrib;    /* ATTRIB_* */
   ULONG  ulFlags;     /* s.o. */
} NICKNAME, *PNICKNAME;

/* Listen-Kopf */

typedef _Packed struct
{
   PNICKNAME pFirstEntry;
   ULONG     ulNumEntries;
   BOOL      bDirty;
   WINPOS    FolderPos;
} NICKNAMELIST, *PNICKNAMELIST;


/*--------------------------- Funktionsprototypen ---------------------------*/

PNICKNAME AddNickname(PNICKNAMELIST pList, PNICKNAME pNickname, BOOL bMarkDirty);
PNICKNAME FindNickname(PNICKNAMELIST pList, PCHAR pchNickname, PNICKNAME pSearchAfter);
PNICKNAME FindNicknameSens(PNICKNAMELIST pList, PCHAR pchNickname);
int DeleteNickname(PNICKNAMELIST pList, PCHAR pchNickname);
int ChangeNickname(PNICKNAMELIST pList, PNICKNAME pNickToChange, PNICKNAME pNewNickname);

/* Ende NICKMANAGE.H */

