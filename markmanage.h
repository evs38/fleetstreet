/* MARKMANAGE.H */

/* Datentypen */

#define SIZE_BUCKET   32    /* muss gerade sein! */

typedef struct {
         ULONG    ulMsgID;
         ULONG    ulMsgNr;
         char     pchFrom[LEN_USERNAME+1];
         char     pchSubj[LEN_SUBJECT+1];
         char     pchFindText[LEN_FINDTEXT+1];
         ULONG    ulFlags;
         ULONG    ulWhere;
         ULONG    ulHow;
      } MARKERITEM, *PMARKERITEM;

#define MARKFLAG_FIND      0x01UL
#define MARKFLAG_PERSMAIL  0x02UL
#define MARKFLAG_MANUAL    0x04UL
#define MARKFLAG_UNSENT    0x08UL
#define MARKFLAG_ALL       0xFFFFFFFFUL

typedef struct _MARKERBUCKET {
         struct _MARKERBUCKET *next;
         struct _MARKERBUCKET *prev;
         ULONG                ulCountItems;
         MARKERITEM           aItems[SIZE_BUCKET];
      } MARKERBUCKET, *PMARKERBUCKET;

typedef struct _MARKERAREA {
         struct _MARKERAREA *next;
         struct _MARKERAREA *prev;
         char               pchAreaTag[LEN_AREATAG+1];
         PMARKERBUCKET      pBuckets;
         BOOL               bDirty;
      } MARKERAREA, *PMARKERAREA;

typedef struct {
         PMARKERAREA pAreas;
         BOOL        bDirty;
         HMTX        hmtxAccess;
      } MARKERLIST, *PMARKERLIST;


/*--------------------------- Funktionsprototypen ---------------------------*/

int MarkMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulMsgNr,
                MSGHEADER *pHeader, PCHAR pchFindText, ULONG ulFlags, ULONG ulHow, ULONG ulWhere);
int UnmarkMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulFlags);
BOOL IsMessageMarked(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, ULONG ulFlags);
int OpenMarkerList(PMARKERLIST pList);
int CloseMarkerList(PMARKERLIST pList);
int ChangeMarkedMessage(PMARKERLIST pList, PCHAR pchAreaTag, ULONG ulMsgID, MSGHEADER *pHeader);

int ReadIniMarkers(HINI inifile, PMARKERLIST pList);
int SaveIniMarkers(HINI inifile, PMARKERLIST pList);
int CheckMarkerAreas(PMARKERLIST pList);
