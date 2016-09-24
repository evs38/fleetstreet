/* V7BROWSE.H, Nodeliste-Browsing */

/* Strukturen f. Nodenummern-Browsing */

#pragma pack(2)
typedef struct {
          FTNADDRESS NodeAddr;       /* Addresse z:n/n.p */
          LONG lDataOffs;         /* Offset im Daten-File */
          ULONG usNetComp;
       } NODEINDEX, *PNODEINDEX;

typedef struct netindex {
          struct netindex *next;
          USHORT usNet;           /* Net-Nummer */
          PNODEINDEX pStart;      /* Erstes Vorkommen im Node-Index */
       } NETINDEX, *PNETINDEX;

typedef struct zoneindex {
          struct zoneindex *next;
          USHORT usZone;          /* Zonen-Nummer */
          PNODEINDEX pStart;      /* erstes Vorkommen im Node-Index */
          PNETINDEX pNets;        /* Netze dieser Zone */
       } ZONEINDEX, *PZONEINDEX;

typedef struct {
          ULONG ulNumNodes;       /* Anzahl der Nodes (Array-Groesse) */
          PNODEINDEX pNodeIndex;  /* Array der Nodes */
          PZONEINDEX pZoneIndex;  /* Zonen-Index */
          FILE  *pfDataFile;      /* File-Pointer f. Daten-File */
          FILE  *pfDataDTP;       /* File-Pointer f. Daten-File (V7+) */
          V7P_HEADER V7PHeader;
          int        BusyFile;
       } NODEBROWSE, *PNODEBROWSE;

/* Strukturen f. Namen-Browsing */

typedef struct {
          char pchSysopName[LEN_USERNAME+1]; /* Sysop-Name  */
          LONG lDataOffs;                    /* Offset im Datenfile */
       } NAMEINDEX, *PNAMEINDEX;

typedef struct
{
   ULONG ulNumNames;                  /* Anzahl der Sysop-Namen */
   PNAMEINDEX pNameIndex;             /* Array der Namen */
   PNAMEINDEX Alpha[28];              /* Index der Namen (A-Z, Num) */
   FILE  *pfDataFile;                 /* File-Pointer f. Daten-File */
   FILE  *pfDataDTP;                  /* File-Pointer f. Daten-File (V7+) */
   V7P_HEADER V7PHeader;
   int        BusyFile;
} NAMEBROWSE, *PNAMEBROWSE;

#pragma pack()

/* Prototypen */

int FLTV7OpenNodeBrowse(char *pchIndexFile, char *pchDataFile, PNODEBROWSE pNodeBrowse);
int FLTV7CloseNodeBrowse(PNODEBROWSE pNodeBrowse);
int FLTV7ReadNodeData(PNODEBROWSE pNodeBrowse, PNODEINDEX pNodeIndex, PNODEDATA pNodeData);

int FLTV7OpenNameBrowse(char *pchIndexFile, char *pchDataFile, PNAMEBROWSE pNameBrowse);
int FLTV7CloseNameBrowse(PNAMEBROWSE pNameBrowse);
int FLTV7ReadNameData(PNAMEBROWSE pNameBrowse, PNAMEINDEX pNameIndex, PNODEDATA pNodeData);

