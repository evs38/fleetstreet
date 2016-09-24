/* FLTV7.H */

typedef struct
{
   FTNADDRESS Address;
   char SysopName[LEN_USERNAME+1];
   char SystemName[LEN_SYSTEMNAME+1];
   char PhoneNr[LEN_PHONENR+1];
   char Location[LEN_LOCATION+1];
   char Password[LEN_PASSWORD+1];
   ULONG ModemType;
   ULONG BaudRate;
   ULONG UserCost;
   ULONG CallCost;
   BOOL isZC;
   BOOL isRC;
   BOOL isHost;
   BOOL isHub;
   BOOL isCM;
   BOOL isMO;
} NODEDATA, *PNODEDATA;


typedef PVOID HV7LOOKUP;

ULONG FLTV7OpenSearch(HV7LOOKUP *pHandle, char *pchSearchName,
                      char *pchIndexFile, char *pchDataFile,
                      ULONG ulSearchType);
ULONG FLTV7SearchNext(HV7LOOKUP hLookup, PNODEDATA pNodeData);
ULONG FLTV7CloseSearch(HV7LOOKUP hLookup);

/* Fehlercodes */

#define V7ERR_OK             0
#define V7ERR_IDXOPENERR     1
#define V7ERR_DATOPENERR     2
#define V7ERR_IDXREADERR     3
#define V7ERR_DATREADERR     4
#define V7ERR_NOENTRY        5
#define V7ERR_NOMEM          6
#define V7ERR_INVHANDLE      7
#define V7ERR_INVSEARCHTYPE  8

/* Suchtypen */

#define V7SEARCHTYPE_NAME             0
#define V7SEARCHTYPE_ADDRESS          1
#define V7SEARCHTYPE_ADDRESS_JOKERS   2  /* nicht impl. */
#define V7SEARCHTYPE_NAME_FUZZY       3  /* nicht impl. */


/* Ende FLTV7.H */
