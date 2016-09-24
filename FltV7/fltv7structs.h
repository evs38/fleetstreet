/* FLTV7STRUCTS.H, interne Strukturen v. FLTV7.C */

#pragma pack(1)

/* Control-Block */
typedef struct
{
   USHORT    CtlBlkSize; /* Blocksize of Index Blocks   */
   LONG      CtlRoot;    /* Block number of Root        */
   LONG      CtlHiBlk;   /* Block number of last block  */
   LONG      CtlLoLeaf;  /* Block number of first leaf  */
   LONG      CtlHiLeaf;  /* Block number of last leaf   */
   LONG      CtlFree;    /* Head of freelist            */
   USHORT    CtlLvls;    /* Number of index levels      */
   USHORT    CtlParity;  /* XOR of above fields         */
} CTLBLOCK, *PCTLBLOCK;

/* Baum-Knoten */
typedef struct
{
   LONG      IndxFirst;  /* Pointer to next lower level */
   LONG      IndxBLink;  /* Pointer to previous link    */
   LONG      IndxFLink;  /* Pointer to next link        */
   SHORT     IndxCnt;    /* Count of Items in block     */
   USHORT    IndxStr;    /* Offset in block of 1st str  */
   /* If IndxFirst is NOT -1, this is INode:          */
   struct _IndxRef
   {
      USHORT   IndxOfs; /* Offset of string into block */
      USHORT   IndxLen; /* Length of string            */
      LONG     IndxData;/* Record number of string     */
      LONG     IndxPtr; /* Block number of lower index */
   } IndxRef[20];
} INODEBLOCK, *PINODEBLOCK;

/* Blatt-Knoten */
typedef struct
{
                       /* IndxFirst is -1 in LNodes   */
   LONG      IndxFirst;  /* Pointer to next lower level */
   LONG      IndxBLink;  /* Pointer to previous link    */
   LONG      IndxFLink;  /* Pointer to next link        */
   SHORT     IndxCnt;    /* Count of Items in block     */
   USHORT    IndxStr;    /* Offset in block of 1st str  */
   struct _LeafRef
   {
      USHORT   KeyOfs;  /* Offset of string into block */
      USHORT   KeyLen;  /* Length of string            */
      LONG     KeyVal;  /* Pointer to data block       */
   } LeafRef[30];
} LNODEBLOCK, *PLNODEBLOCK;

typedef union
{
   CTLBLOCK   CtlBlock;
   INODEBLOCK INodeBlock;
   LNODEBLOCK LNodeBlock;
   char       achRawNdx[512];
   USHORT     ausRawNdx[256];
} NDX, *PNDX;

typedef struct
{
   SHORT    Zone;
   SHORT    Net;
   SHORT    Node;
   SHORT    HubNode;        /* If node is a point, this is point number. */
   USHORT   CallCost;       /* phone company's charge */
   USHORT   MsgFee;         /* Amount charged to user for a message */
   USHORT   NodeFlags;      /* set of flags (see below) */
   BYTE     ModemType;      /* RESERVED for modem type */
   BYTE     Phone_len;
   BYTE     Password_len;
   BYTE     Bname_len;
   BYTE     Sname_len;
   BYTE     Cname_len;
   BYTE     pack_len;
   BYTE     BaudRate;       /* baud rate divided by 300 */
} VERS7, *PVERS7;

/* Node-Flags */

#define B_hub    0x0001  /* node is a net hub     0000000000000001 */
#define B_host   0x0002  /* node is a net host    0000000000000010 */
#define B_region 0x0004  /* node is region coord  0000000000000100 */
#define B_zone   0x0008  /* is a zone gateway     0000000000001000 */
#define B_CM     0x0010  /* runs continuous mail  0000000000010000 */
#define B_point  0x1000  /* node is a point       0001000000000000 */
#define B_MO     0x2000  /* node is mail only     0010000000000000 */


#ifndef LEN_USERNAME
  #define LEN_USERNAME 35
#endif
#ifndef LEN_5DADDRESS
  #define LEN_5DADDRESS 40
#endif

/* Strukturen Version 7 Plus */

typedef struct
{
   USHORT Size;
   char   Version;
   char   AllFixSize;
   char   AddFixSize;
} V7P_HEADER;

/* Datenstruktur f. Handle */

typedef struct _INDEXNODE
{
   struct _INDEXNODE *next;    /* Verzeigerung */
   int   iPos;                 /* Position im Node */
   SHORT count;                /* Anzahl der Saetze im Node */
   NDX   node;                 /* Der Node halt */
} INDEXNODE, *PINDEXNODE;

typedef struct
{
   char       pchName[LEN_USERNAME+1];     /* zu suchender Name    */
   FTNADDRESS Addr;                        /* zu suchende Adresse  */
   int        (*CompareFunc)(void*, void*, unsigned int); /* Vergleichsfunktion */
   void       *pCompElem;          /* zu vergleichendes Element */
   FILE       *streamIndex;        /* File-Handle der Index-Datei  */
   FILE       *streamData;         /* File-Handle der Daten-Datei  */
   FILE       *streamDTP;          /* File-Handle der Daten-Datei  (V7+) */
   V7P_HEADER V7PHeader;
   int        BusyFile;
   USHORT     CtlBlkSize;          /* Blockgroesse der Index-Datei */
   BOOL       bStarted;            /* Suche hat begonnen           */
   BOOL       bAtEnd;              /* Suche ist zu Ende            */
   LONG       lRecNum;             /* Interner Merker f. Record    */
   LONG       lFoundRec;           /* Letzter gefundener Offset (f. Dupe-Check) */
   PINDEXNODE pNodeStack;          /* Anfang des Node-Stacks       */
   ULONG      stacksize;           /* Anzahl der gestackten Nodes  */
   ULONG      ulSearchType;
} V7LOOKUP, *PV7LOOKUP;

#pragma pack()
