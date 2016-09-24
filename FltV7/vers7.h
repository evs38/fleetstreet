/* VERS7.H */

/* This file is (C) 1995 by Michael Hohner */
/* See VERS7DLL.INF for detailed information */

/* Length definitions */

#define LEN_USERNAME       35  /* not including \0 byte */
#define LEN_SYSTEMNAME    100
#define LEN_PHONENR        50
#define LEN_LOCATION       50
#define LEN_PASSWORD        8

#ifdef __IBMCPP__
   #pragma pack(1)
#endif

/* Fido address type */

typedef struct
{
   USHORT usZone;
   USHORT usNet;
   USHORT usNode;
   USHORT usPoint;
} FTNADDRESS, *PFTNADDRESS;

#ifdef __IBMCPP__
   #pragma pack()
#endif

/* Node data structure */

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
} NODEDATA, *PNODEDATA;

/* Lookup handle */
typedef PVOID HV7LOOKUP;

#ifdef __cplusplus
extern "C" {
#endif

ULONG APIENTRY FLTV7OpenSearch(HV7LOOKUP *pHandle, char *pchSearchName,
                               char *pchIndexFile, char *pchDataFile,
                               ULONG ulSearchType);
ULONG APIENTRY FLTV7SearchNext(HV7LOOKUP hLookup, PNODEDATA pNodeData);
ULONG APIENTRY FLTV7CloseSearch(HV7LOOKUP hLookup);

#ifdef __cplusplus
}
#endif

/* Error codes */

#define V7ERR_OK             0
#define V7ERR_IDXOPENERR     1
#define V7ERR_DATOPENERR     2
#define V7ERR_IDXREADERR     3
#define V7ERR_DATREADERR     4
#define V7ERR_NOENTRY        5
#define V7ERR_NOMEM          6
#define V7ERR_INVHANDLE      7
#define V7ERR_INVSEARCHTYPE  8

/* Search types */

#define V7SEARCHTYPE_NAME             0
#define V7SEARCHTYPE_ADDRESS          1


/* End of VERS7.H */
